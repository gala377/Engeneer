
define i64 @test(i64 %a, i64 %b) {
entry:
  %__addtmp = add i64 %a, %b
  ret i64 %__addtmp
}


define i64 @main() {
entry:
  %__calltmp = call i64 @test(i64 10, i64 20)
  ret i64 %__calltmp
}