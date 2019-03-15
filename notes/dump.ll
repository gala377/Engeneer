declare i32 @putchar(i32)

define i32 @put(i32 %v) {
entry:
  %v1 = alloca i32
  store i32 %v, i32* %v1
  %v2 = load i32, i32* %v1
  %__addtmp = add i32 65, %v2
  %__calltmp = call i32 @putchar(i32 %__addtmp)
  ret i32 %__calltmp
}

define i32 @add(i32 %a, i32 %b) {
entry:
  %a1 = alloca i32
  store i32 %a, i32* %a1
  %b2 = alloca i32
  store i32 %b, i32* %b2
  %a3 = load i32, i32* %a1
  %b4 = load i32, i32* %b2
  %__addtmp = add i32 %a3, %b4
  ret i32 %__addtmp
}

define i32 @main() {
entry:
  %a = alloca i32
  store i32 0, i32* %a
  %b = alloca i32
  %a1 = load i32, i32* %a
  store i32 %a1, i32* %b
  %a2 = load i32, i32* %a
  %__calltmp = call i32 @put(i32 %a2)
  %b3 = load i32, i32* %b
  %__calltmp4 = call i32 @put(i32 %b3)
  %a5 = load i32, i32* %a
  %__cmptemp = icmp sgt i32 %a5, 5
  br i1 %__cmptemp, label %then, label %else

then:                                             ; preds = %entry
  store i32 1, i32* %a
  br label %merge15

else:                                             ; preds = %entry
  %a6 = load i32, i32* %a
  %__cmptemp7 = icmp sge i32 %a6, 5
  br i1 %__cmptemp7, label %then8, label %else9

then8:                                            ; preds = %else
  store i32 2, i32* %a
  br label %merge14

else9:                                            ; preds = %else
  %a10 = load i32, i32* %a
  %__cmptemp11 = icmp slt i32 %a10, 5
  br i1 %__cmptemp11, label %then12, label %else13

then12:                                           ; preds = %else9
  store i32 3, i32* %a
  br label %merge

else13:                                           ; preds = %else9
  store i32 4, i32* %a
  br label %merge

merge:                                            ; preds = %else13, %then12
  br label %merge14

merge14:                                          ; preds = %merge, %then8
  br label %merge15

merge15:                                          ; preds = %merge14, %then
  %a16 = load i32, i32* %a
  %__calltmp17 = call i32 @put(i32 %a16)
  %b18 = load i32, i32* %b
  %__calltmp19 = call i32 @add(i32 11, i32 %b18)
  store i32 %__calltmp19, i32* %b
  %b20 = load i32, i32* %b
  %__cmptemp21 = icmp sgt i32 %b20, 10
  br i1 %__cmptemp21, label %then22, label %merge23

then22:                                           ; preds = %merge15
  store i32 5, i32* %a
  br label %merge23

merge23:                                          ; preds = %then22, %merge15
  %a24 = load i32, i32* %a
  %__calltmp25 = call i32 @put(i32 %a24)
  %a26 = load i32, i32* %a
  ret i32 %a26
}