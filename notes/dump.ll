%Foo = type { double, [3 x i32] }

define i32 @main() {
entry:
  %i = alloca i32
  %f = alloca %Foo
  %__gep_adr = getelementptr %Foo, %Foo* %f, i32 0, i32 0
  store double 1.500000e+00, double* %__gep_adr
  store i32 0, i32* %i
  br label %__whilecond

__whilecond:                                      ; preds = %__while, %entry
  %i1 = load i32, i32* %i
  %__cmptemp = icmp slt i32 %i1, 3
  br i1 %__cmptemp, label %__while, label %__whilecontr

__while:                                          ; preds = %__whilecond
  %__gep_adr2 = getelementptr %Foo, %Foo* %f, i32 0, i32 1
  %i3 = load i32, i32* %i
  %__gep_adr4 = getelementptr [3 x i32], [3 x i32]* %__gep_adr2, i32 0, i32 %i3
  %i5 = load i32, i32* %i
  store i32 %i5, i32* %__gep_adr4
  %__gep_adr6 = getelementptr %Foo, %Foo* %f, i32 0, i32 1
  %i7 = load i32, i32* %i
  %__gep_adr8 = getelementptr [3 x i32], [3 x i32]* %__gep_adr6, i32 0, i32 %i7
  %__gep_val = load i32, i32* %__gep_adr8
  %__calltmp = call i32 @put(i32 %__gep_val)
  %i9 = load i32, i32* %i
  %__addtmp = add i32 %i9, 1
  store i32 %__addtmp, i32* %i
  br label %__whilecond

__whilecontr:                                     ; preds = %__whilecond
  ret i32 0
}

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