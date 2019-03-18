declare i32 @putchar(i32)

define void @put(i32 %v) {
entry:
  %v1 = alloca i32
  store i32 %v, i32* %v1
  %v2 = load i32, i32* %v1
  %__addtmp = add i32 65, %v2
  %__calltmp = call i32 @putchar(i32 %__addtmp)
  ret void
}

define i32 @main() {
entry:
  %c = alloca i32
  %b = alloca fp128
  %a = alloca i8
  store i8 0, i8* %a
  %a1 = load i8, i8* %a
  %__cast_tmp = sitofp i8 %a1 to fp128
  store fp128 %__cast_tmp, fp128* %b
  %b2 = load fp128, fp128* %b
  %__cast_tmp3 = fptosi fp128 %b2 to i32
  store i32 %__cast_tmp3, i32* %c
  %c4 = load i32, i32* %c
  call void @put(i32 %c4)
  ret i32 0
}