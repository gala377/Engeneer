%Foo = type { i64, float, %Foo* }

define i32 @main() {
entry:
  %b = alloca %Foo
  %a = alloca float
  store float 4.000000e+00, float* %a
  %a1 = load float, float* %a
  %__cast_tmp = fptosi float %a1 to i32
  call void @put(i32 %__cast_tmp)
  %__calltmp = call %Foo @test()
  store %Foo %__calltmp, %Foo* %b
  %__gep_adr = getelementptr %Foo, %Foo* %b, i32 0, i32 0
  %__gep_val = load i64, i64* %__gep_adr
  %__cast_tmp2 = trunc i64 %__gep_val to i32
  call void @put(i32 %__cast_tmp2)
  %__gep_adr3 = getelementptr %Foo, %Foo* %b, i32 0, i32 1
  %__gep_val4 = load float, float* %__gep_adr3
  %__addtmp = fadd float %__gep_val4, 5.000000e-01
  %__cast_tmp5 = fptosi float %__addtmp to i32
  call void @put(i32 %__cast_tmp5)
  %__gep_adr6 = getelementptr %Foo, %Foo* %b, i32 0, i32 2
  store %Foo* %b, %Foo** %__gep_adr6
  %__gep_adr7 = getelementptr %Foo, %Foo* %b, i32 0, i32 2
  %__gep_val8 = load %Foo*, %Foo** %__gep_adr7
  %__gep_adr9 = getelementptr %Foo, %Foo* %__gep_val8, i32 0, i32 0
  %__gep_val10 = load i64, i64* %__gep_adr9
  %__cast_tmp11 = trunc i64 %__gep_val10 to i32
  call void @put(i32 %__cast_tmp11)
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

define %Foo @test() {
entry:
  %a = alloca %Foo
  %__gep_adr = getelementptr %Foo, %Foo* %a, i32 0, i32 0
  store i64 1, i64* %__gep_adr
  %__gep_adr1 = getelementptr %Foo, %Foo* %a, i32 0, i32 1
  store float 2.050000e+01, float* %__gep_adr1
  %a2 = load %Foo, %Foo* %a
  ret %Foo %a2
}