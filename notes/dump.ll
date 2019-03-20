define i32 @ftoi(float %f) {
entry:
  %i = alloca i32
  %f1 = alloca float
  store float %f, float* %f1
  %f2 = load float, float* %f1
  %__cast_tmp = fptosi float %f2 to i32
  store i32 %__cast_tmp, i32* %i
  %i3 = load i32, i32* %i
  ret i32 %i3
}

define float @itof(i32 %i) {
entry:
  %f = alloca float
  %i1 = alloca i32
  store i32 %i, i32* %i1
  %i2 = load i32, i32* %i1
  %__cast_tmp = sitofp i32 %i2 to float
  store float %__cast_tmp, float* %f
  %f3 = load float, float* %f
  ret float %f3
}

define i32 @main() {
entry:
  %i = alloca i32
  store i32 0, i32* %i
  br label %__whilecond

__whilecond:                                      ; preds = %__while, %entry
  %i1 = load i32, i32* %i
  %__cmptemp = icmp slt i32 %i1, 10
  br i1 %__cmptemp, label %__while, label %__whilecontr

__while:                                          ; preds = %__whilecond
  %i2 = load i32, i32* %i
  call void @put(i32 %i2)
  br label %__whilecond

__whilecontr:                                     ; preds = %__whilecond
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