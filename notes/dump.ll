declare i32 @putchar(i32)

define i32 @print_hello() {
entry:
  %__calltmp = call i32 @putchar(i32 72)
  %__calltmp1 = call i32 @putchar(i32 101)
  %__calltmp2 = call i32 @putchar(i32 108)
  %__calltmp3 = call i32 @putchar(i32 108)
  %__calltmp4 = call i32 @putchar(i32 111)
  ret i32 0
}

define i32 @print_space() {
entry:
  %__calltmp = call i32 @putchar(i32 32)
  ret i32 0
}

define i32 @print_world() {
entry:
  %__calltmp = call i32 @putchar(i32 87)
  %__calltmp1 = call i32 @putchar(i32 111)
  %__calltmp2 = call i32 @putchar(i32 114)
  %__calltmp3 = call i32 @putchar(i32 108)
  %__calltmp4 = call i32 @putchar(i32 100)
  ret i32 0
}

define i32 @main() {
entry:
  %__calltmp = call i32 @print_hello()
  %__calltmp1 = call i32 @print_space()
  %__calltmp2 = call i32 @print_world()
  ret i32 0
}