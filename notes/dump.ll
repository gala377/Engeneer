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

define i32 @main() {
entry:
  %j = alloca i32
  %i = alloca i32
  store i32 0, i32* %i
  br label %__whilecond

__whilecond:                                      ; preds = %__ifcontr17, %entry
  %i1 = load i32, i32* %i
  %__cmptemp = icmp slt i32 %i1, 10
  br i1 %__cmptemp, label %__while, label %__whilecontr18

__while:                                          ; preds = %__whilecond
  store i32 0, i32* %j
  br label %__whilecond2

__whilecond2:                                     ; preds = %__ifcontr, %__iftrue, %__while
  %j3 = load i32, i32* %j
  %__cmptemp4 = icmp slt i32 %j3, 5
  br i1 %__cmptemp4, label %__while5, label %__whilecontr

__while5:                                         ; preds = %__whilecond2
  %j6 = load i32, i32* %j
  %__addtmp = add i32 %j6, 1
  store i32 %__addtmp, i32* %j
  %j7 = load i32, i32* %j
  %__cmptemp8 = icmp slt i32 %j7, 3
  br i1 %__cmptemp8, label %__iftrue, label %__ifcontr

__iftrue:                                         ; preds = %__while5
  br label %__whilecond2
  br label %__ifcontr

__ifcontr:                                        ; preds = %__iftrue, %__while5
  %i9 = load i32, i32* %i
  %__multmp = mul i32 %i9, 5
  %j10 = load i32, i32* %j
  %__addtmp11 = add i32 %__multmp, %j10
  %__calltmp = call i32 @put(i32 %__addtmp11)
  br label %__whilecond2

__whilecontr:                                     ; preds = %__whilecond2
  %i12 = load i32, i32* %i
  %__addtmp13 = add i32 %i12, 1
  store i32 %__addtmp13, i32* %i
  %i14 = load i32, i32* %i
  %__cmptemp15 = icmp sgt i32 %i14, 1
  br i1 %__cmptemp15, label %__iftrue16, label %__ifcontr17

__iftrue16:                                       ; preds = %__whilecontr
  br label %__whilecontr18
  br label %__ifcontr17

__ifcontr17:                                      ; preds = %__iftrue16, %__whilecontr
  br label %__whilecond

__whilecontr18:                                   ; preds = %__iftrue16, %__whilecond
  ret i32 0
}