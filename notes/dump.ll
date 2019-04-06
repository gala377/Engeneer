define i32 @put(i32 %v) {
entry:
  %v1 = alloca i32
  store i32 %v, i32* %v1
  %v2 = load i32, i32* %v1
  %__addtmp = add i32 %v2, 65
  %__calltmp = call i32 @putchar(i32 %__addtmp)
  ret i32 %__calltmp
}

declare i32 @putchar(i32)

define i32 @main() {
entry:
  %func = alloca i32 (i32)*
  store i32 (i32)* @put, i32 (i32)** %func
  %func1 = load i32 (i32)*, i32 (i32)** %func
  %__calltmp = call i32 %func1(i32 0)
  ret i32 0
}

