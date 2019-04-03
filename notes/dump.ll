define [128 x i32] @get_array() {
entry:
  %i = alloca i32
  %a = alloca [128 x i32]
  store i32 0, i32* %i
  br label %__whilecond

__whilecond:                                      ; preds = %__while, %entry
  %i1 = load i32, i32* %i
  %__cmptemp = icmp slt i32 %i1, 128
  br i1 %__cmptemp, label %__while, label %__whilecontr

__while:                                          ; preds = %__whilecond
  %i2 = load i32, i32* %i
  %__gep_adr = getelementptr [128 x i32], [128 x i32]* %a, i32 0, i32 %i2
  %i3 = load i32, i32* %i
  store i32 %i3, i32* %__gep_adr
  %i4 = load i32, i32* %i
  %__addtmp = add i32 %i4, 1
  store i32 %__addtmp, i32* %i
  br label %__whilecond

__whilecontr:                                     ; preds = %__whilecond
  %a5 = load [128 x i32], [128 x i32]* %a
  ret [128 x i32] %a5
}

define i32 @main() {
entry:
  %d = alloca i32*
  %a = alloca [128 x i32]
  %__calltmp = call [128 x i32] @get_array()
  store [128 x i32] %__calltmp, [128 x i32]* %a
  %__gep_adr = getelementptr [128 x i32], [128 x i32]* %a, i32 0, i32 1
  store i32* %__gep_adr, i32** %d
  %d1 = load i32*, i32** %d
  %__cast_tmp = bitcast i32* %d1 to [0 x i32]*
  call void @test([0 x i32]* %__cast_tmp)
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

define void @test([0 x i32]* %ptr) {
entry:
  %v = alloca i32
  %ptr1 = alloca [0 x i32]*
  store [0 x i32]* %ptr, [0 x i32]** %ptr1
  %ptr2 = load [0 x i32]*, [0 x i32]** %ptr1
  %__gep_adr = getelementptr [0 x i32], [0 x i32]* %ptr2, i32 0, i32 1
  %__gep_val = load i32, i32* %__gep_adr
  store i32 %__gep_val, i32* %v
  %v3 = load i32, i32* %v
  call void @put(i32 %v3)
  ret void
}