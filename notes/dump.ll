define i32 @main() {
entry:
  %b = alloca i32*
  %i = alloca i32
  %a = alloca [10 x i32]
  store i32 0, i32* %i
  br label %__whilecond

__whilecond:                                      ; preds = %__while, %entry
  %i1 = load i32, i32* %i
  %__cmptemp = icmp slt i32 %i1, 10
  br i1 %__cmptemp, label %__while, label %__whilecontr

__while:                                          ; preds = %__whilecond
  %i2 = load i32, i32* %i
  %__gep_adr = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 %i2
  %i3 = load i32, i32* %i
  store i32 %i3, i32* %__gep_adr
  %i4 = load i32, i32* %i
  %__addtmp = add i32 %i4, 1
  store i32 %__addtmp, i32* %i
  br label %__whilecond

__whilecontr:                                     ; preds = %__whilecond
  %__gep_adr5 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 0
  store i32* %__gep_adr5, i32** %b
  %b6 = load i32*, i32** %b
  %__deref_val = load i32, i32* %b6
  call void @test(i32 %__deref_val)
  ret i32 0
}

define void @put(i32 %v) {
entry:
  %v1 = alloca i32
  store i32 %v, i32* %v1
  %v2 = load i32, i32* %v1
  %__addtmp = add i32 65, %v2
  %__calltmp = call i32 @putchar(i32 %__addtmp)
  ret void
}

define void @put_ptr(i32* %v) {
entry:
  %v1 = alloca i32*
  store i32* %v, i32** %v1
  %v2 = load i32*, i32** %v1
  %__deref_val = load i32, i32* %v2
  call void @put(i32 %__deref_val)
  ret void
}

declare i32 @putchar(i32)

define void @test([0 x i32] %v) {
entry:
  %v1 = alloca [0 x i32]
  store [0 x i32] %v, [0 x i32]* %v1
  %__gep_adr = getelementptr [0 x i32], [0 x i32]* %v1, i32 0, i32 2
  %__gep_val = load i32, i32* %__gep_adr
  call void @put(i32 %__gep_val)
  ret void
}