declare i32 @putchar(i32)

define i32 @add(i32 %a, i32 %b) {
entry:
  %a1 = alloca i32
  %b2 = alloca i32
  %a3 = load i32, i32* %a1
  %b4 = load i32, i32* %b2
  %__addtmp = add i32 %a3, %b4
  ret i32 %__addtmp
}

define i32 @main() {
entry:
  %a = alloca i32
  store i32 5, i32* %a
  %b = alloca i32
  %a1 = load i32, i32* %a
  %__multmp = mul i32 %a1, 2
  store i32 %__multmp, i32* %b
  %a2 = load i32, i32* %a
  %__cmptemp = icmp sgt i32 %a2, 5
  br i1 %__cmptemp, label %then, label %else

then:                                             ; preds = %entry
  %a3 = load i32, i32* %a
  %b4 = load i32, i32* %b
  %__calltmp = call i32 @add(i32 %a3, i32 %b4)
  ret i32 %__calltmp
  br label %merge

else:                                             ; preds = %entry
  %b5 = load i32, i32* %b
  ret i32 %b5
  br label %merge

merge:                                            ; preds = %else, %then
  %a6 = load i32, i32* %a
  ret i32 %a6
}