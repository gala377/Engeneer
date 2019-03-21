define i32 @main() {
entry:
  %i = alloca i32
  %a = alloca [4 x i32]
  store i32 0, i32* %i
  br label %__whilecond

__whilecond:                                      ; preds = %__while, %entry
  %i1 = load i32, i32* %i
  %__cmptemp = icmp slt i32 %i1, 4
  br i1 %__cmptemp, label %__while, label %__whilecontr

__while:                                          ; preds = %__whilecond
  %i2 = load i32, i32* %i
  %__gep_adr = getelementptr [4 x i32], [4 x i32]* %a, i32 0, i32 %i2
  %i3 = load i32, i32* %i
  %__addtmp = sub i32 3, %i3
  store i32 %__addtmp, i32* %__gep_adr
  %i4 = load i32, i32* %i
  %__addtmp5 = add i32 %i4, 1
  store i32 %__addtmp5, i32* %i
  br label %__whilecond

__whilecontr:                                     ; preds = %__whilecond
  store i32 0, i32* %i
  br label %__whilecond6

__whilecond6:                                     ; preds = %__while9, %__whilecontr
  %i7 = load i32, i32* %i
  %__cmptemp8 = icmp slt i32 %i7, 4
  br i1 %__cmptemp8, label %__while9, label %__whilecontr14

__while9:                                         ; preds = %__whilecond6
  %i10 = load i32, i32* %i
  %__gep_adr11 = getelementptr [4 x i32], [4 x i32]* %a, i32 0, i32 %i10
  %__gep_val = load i32, i32* %__gep_adr11
  call void @put(i32 %__gep_val)
  %i12 = load i32, i32* %i
  %__addtmp13 = add i32 %i12, 1
  store i32 %__addtmp13, i32* %i
  br label %__whilecond6

__whilecontr14:                                   ; preds = %__whilecond6
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

declare i32 @putchar(i32)