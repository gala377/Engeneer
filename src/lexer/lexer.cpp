//
// Created by igor on 15.12.18.
//

#include <lexer/lexer.h>
#include <lexer/helpers.h>


// todo max lenght for the indentifiers
// todo  error reporting instead of throw?
// todo  but should me abort? What kind of errors should they be?
// todo  like Syntax or what?

const Lexer::Lexer::SymbolMap Lexer::Lexer::_KEYWORDS = {
        {"if", Token::Id::If},
        {"else", Token::Id::Else},
        {"for", Token::Id::For},
        {"struct", Token::Id::Struct},
        {"wraps", Token::Id::Wraps},
        {"let", Token::Id::Let},
        {"return", Token::Id::Return},
        {"while", Token::Id::While},
        {"throw", Token::Id::Throw},
        {"break", Token::Id::Break},
        {"continue", Token::Id::Continue},
        {"const", Token::Id::Const},
        {"val", Token::Id::Val},
        {"as", Token::Id::As},
        {"fn", Token::Id::Fn},
        {"memory", Token::Id::Memory},
};

const Lexer::Lexer::SymbolMap Lexer::Lexer::_OPERATORS = {
        {"==", Token::Id::Equality},
        {"!=", Token::Id::Inequality},
        {"+", Token::Id::Plus},
        {"*", Token::Id::Multiplication},
        {"/", Token::Id::Division},
        {"=", Token::Id::Assignment},
        {">", Token::Id::GreaterThan},
        {"<", Token::Id::LessThan},
        {"%", Token::Id::Modulo},
        {";", Token::Id::Semicolon},
        {":", Token::Id::Colon},
        {"++", Token::Id::DoublePlus},
        {"--", Token::Id::DoubleMinus},
        {">=", Token::Id::GreaterEq},
        {"<=", Token::Id::LessEq},
        {"||", Token::Id::LogicalOr},
        {"&&", Token::Id::LogicalAnd},
        {"|", Token::Id::InclusiveOr},
        {"^", Token::Id::ExclusiveOr},
        {"&", Token::Id::And},
        {"<<", Token::Id::LeftShift},
        {">>", Token::Id::RightShift},


        {"(", Token::Id::LeftParenthesis},
        {")", Token::Id::RightParenthesis},
        {"{", Token::Id::LeftBrace},
        {"}", Token::Id::RightBrace},
        {"[", Token::Id::LeftSquareBracket},
        {"]", Token::Id::RightSquareBracket},

        {",", Token::Id::Comma},
        {"-", Token::Id::Minus},
        {"!", Token::Id::Negation},
        {".", Token::Id::Dot},
        {"->", Token::Id::Arrow},
};

Lexer::Lexer::Lexer(Source::Base &source):
    HandlingMixin(),
    _source(source),
    _curr_token(Token{Token::Id::End, "", _source}),
    _next_token(_curr_token) {
    _init_token_assemblers();
    _init_curr_token();
}

// todo how to?
Lexer::Lexer::Lexer(Source::Base &source, Exception::Handler &h):
    HandlingMixin(h),
    _source(source),
    _curr_token(Token{Token::Id::End, "", _source}),
    _next_token(_curr_token) {
    _init_token_assemblers();
    _init_curr_token();
}

void Lexer::Lexer::_init_token_assemblers() {
    _token_assemblers[TokenAssemblerId::Identifier] = std::bind(
        &Lexer::_process_identifier, this, std::placeholders::_1);
    _token_assemblers[TokenAssemblerId::Number] = std::bind(
        &Lexer::_process_numeric, this, std::placeholders::_1);
    _token_assemblers[TokenAssemblerId::Operator] = std::bind(
        &Lexer::_process_operator, this, std::placeholders::_1);
    _token_assemblers[TokenAssemblerId::String] = std::bind(
        &Lexer::_process_string, this, std::placeholders::_1);
    _token_assemblers[TokenAssemblerId::Blank] = std::bind(
        &Lexer::_process_blank_char, this, std::placeholders::_1);
    _token_assemblers[TokenAssemblerId::Eof] = std::bind(
        &Lexer::_process_eof, this, std::placeholders::_1);
    _token_assemblers[TokenAssemblerId::Comment] = std::bind(
         &Lexer::_process_comment, this, std::placeholders::_1);
}

void Lexer::Lexer::_init_curr_token() {
    next_token();
    next_token();
}

const Lexer::Token Lexer::Lexer::curr_token() const {
    return _curr_token;
}

const Lexer::Token Lexer::Lexer::peek() const {
    return _next_token;
}

// todo
// for now skips new lines and blank characters
// change in later
// maybe context again?
const Lexer::Token Lexer::Lexer::next_token() {
    auto ch = _source.curr_char();
    auto asm_id = char_to_assembler_id(ch);

    while(asm_id == TokenAssemblerId::Blank || asm_id == TokenAssemblerId::Comment) {
        if(asm_id == TokenAssemblerId::Comment) {
            _assemble_comment(ch);
        }
        ch = _source.next_char();
        asm_id = char_to_assembler_id(ch);
    }

    auto saved_pos = _source.curr_source_position();
    _curr_token = std::exchange(_next_token, _token_assemblers[asm_id](ch));
    auto curr_pos = _source.curr_source_position();

    _next_token.span = Token::Span {
        _source.name(),
        std::get<0>(saved_pos),
        std::get<1>(saved_pos),
        std::get<1>(curr_pos),
        _source.current_pointer(),
     };
    return _curr_token;
}

const Lexer::Token Lexer::Lexer::make_token(Token::Id id, const std::string &symbol) {
    return Token{id, symbol, _source};
}

std::tuple<std::uint32_t, std::uint32_t> Lexer::Lexer::in_source_pos() const {
    return _source.curr_source_position();
}

Lexer::Lexer::TokenAssemblerId Lexer::Lexer::char_to_assembler_id(const char &ch) const {
    if(is_beginning_of_the_identifier(ch)) {
        return TokenAssemblerId::Identifier;
    } else if(is_digit(ch)) {
        return TokenAssemblerId::Number;
    } else if(is_part_of_operator(ch)) {
        return TokenAssemblerId::Operator;
    } else if(is_blank(ch)) {
        return TokenAssemblerId::Blank;
    } else if(is_string(ch)) {
        return TokenAssemblerId::String;
    } else if(is_eof(ch)) {
        return TokenAssemblerId::Eof;
    } else if(is_comment(ch)) {
        return TokenAssemblerId::Comment;
    }
    throw std::runtime_error("Unknown character!");
}


Lexer::Token Lexer::Lexer::_process_blank_char(char ch) {
    if(is_new_line(ch)) {
        return Token{Token::Id::NewLine, {ch}, _source};
    }
    return Token{Token::Id::Space, {ch}, _source};
}

// todo integer and float distinction later maybe double and so on
// because now it returns just integers
Lexer::Token Lexer::Lexer::_process_numeric(char ch) {
    auto [is_float, symbol] = _assemble_numeric(ch);
    return Token {
        is_float ? Token::Id::Float : Token::Id::Integer,
        symbol,
        _source
    };
}

Lexer::Token Lexer::Lexer::_process_identifier(char ch) {
    auto symbol = _assemble_identifier(ch);
    if(_KEYWORDS.find(symbol) != _KEYWORDS.end()) {
        return Token{(*_KEYWORDS.find(symbol)).second, symbol, _source};
    }
    return Token{Token::Id::Identifier, symbol, _source};
}


Lexer::Token Lexer::Lexer::_process_operator(char ch) {
    auto symbol = _assemble_operator(ch);
    return Token{(*_OPERATORS.find(symbol)).second, symbol, _source};
}

Lexer::Token Lexer::Lexer::_process_string(char ch) {
    auto symbol = _assemble_string(ch);
    return Token{Token::Id::String, symbol, _source};
}

Lexer::Token Lexer::Lexer::_process_comment(char ch) {
    auto symbol = _assemble_comment(ch);
    return Token{Token::Id::Comment, symbol, _source};
}

std::tuple<bool, std::string> Lexer::Lexer::_assemble_numeric(char current) {
    std::string symbol;
    bool is_float = false;

    if(current != '0') {
        while (is_digit(current)) {
            symbol += std::exchange(current, _source.next_char());
            if (is_alpha(current)) {
                throw std::runtime_error("Expected numeric value got " + std::string{current});
            }
        }
    } else {
        symbol += std::exchange(current, _source.next_char());
        if(is_digit(current)) {
            throw std::runtime_error("Integers cannot begin with 0");
        }
    }
    if(current == '.') {
        is_float = true;
        symbol += std::exchange(current, _source.next_char());
        if(!is_digit(current)) {
            throw std::runtime_error("Expected at least one digit after '.'");
        }
        while (is_digit(current)) {
            symbol += std::exchange(current, _source.next_char());
            if (is_alpha(current)) {
                throw std::runtime_error("Expected numeric value got " + std::string{current});
            }
        }
    }
    if(symbol.empty()) {
        throw std::runtime_error("Could not assemble const expr!");
    }

    return std::make_tuple(is_float, symbol);
}

std::string Lexer::Lexer::_assemble_identifier(char current) {
    std::string symbol;
    while(is_part_of_identifier(current)) {
        symbol += std::exchange(current, _source.next_char());
    }
    if(symbol.empty()) {
        throw std::runtime_error("Could not assemble identifier!");
    }
    return symbol;
}


std::string Lexer::Lexer::_assemble_operator(char current) {
    std::string symbol;

    while(is_part_of_operator(current)) {
        auto guess = symbol + current;
        if(_OPERATORS.find(guess) == _OPERATORS.end()) {
            return symbol;
        }
        symbol = guess;
        current = _source.next_char();
    }
    if (symbol.empty()) {
        throw std::runtime_error("Could not assemble operator");
    }
    return symbol;
}

std::string Lexer::Lexer::_assemble_string(char current) {
    std::string symbol;
    current = _source.next_char();
    while(!is_string(current)) {
        if(is_eof(current)) {
            throw std::runtime_error("Expected end of string got eof");
        }
        symbol += current;
        current = _source.next_char();
    }
    // eating ' " ' character
    _source.next_char();
    return symbol;
}

Lexer::Token Lexer::Lexer::_process_eof(char current) {
    return Token{
        Token::Id::End,
        {current},
        _source,
    };
}

std::string Lexer::Lexer::_assemble_comment(char ch) {
    std::string symbol;
    ch = _source.next_char();
    while(!(is_new_line(ch) || is_eof(ch))) {
        symbol += ch;
        ch =  _source.next_char();
    }
    return symbol;
}
