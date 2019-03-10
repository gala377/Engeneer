
declare i32 @putchar(i32)

define i32 @print_hello() {
entry:
  %__calltmp = call i32 @putchar(i32 72)
  %__calltmp1 = call i32 @putchar(i32 101)
  %__calltmp2 = call i32 @putchar(i32 108)
  %__calltmp3 = call i32 @putchar(i32 108)
  %__calltmp4 = call i32 @putchar(i32 111)
  ret i32 0
}

define i32 @print_space() {
entry:
  %__calltmp = call i32 @putchar(i32 32)
  ret i32 0
}

define i32 @print_world() {
entry:
  %__calltmp = call i32 @putchar(i32 87)
  %__calltmp1 = call i32 @putchar(i32 111)
  %__calltmp2 = call i32 @putchar(i32 114)
  %__calltmp3 = call i32 @putchar(i32 108)
  %__calltmp4 = call i32 @putchar(i32 100)
  ret i32 0
}

define i32 @cond_hello(i32 %a) {
entry:
  %__cmptemp = icmp sgt i32 %a, 5
  br i1 %__cmptemp, label %__then, label %__ifcont

__then:                                           ; preds = %entry
  %__calltmp = call i32 @print_hello()
  br label %__ifcont

__ifcont:                                         ; preds = %__then, %entry
  %iftmp = phi i32 [ %__calltmp, %__then ]
  %__calltmp1 = call i32 @print_world()
  ret i32 0
}

define i32 @if_else_hello(i32 %a) {
entry:
  %__cmptemp = icmp sgt i32 %a, 5
  br i1 %__cmptemp, label %__then, label %__else

__then:                                           ; preds = %entry
  %__calltmp = call i32 @print_hello()
  br label %__ifcont

__else:                                           ; preds = %entry
  %__calltmp1 = call i32 @print_world()
  br label %__ifcont

__ifcont:                                         ; preds = %__else, %__then
  %iftmp = phi i32 [ %__calltmp, %__then ], [ %__calltmp1, %__else ]
  ret i32 0
}

define i32 @main() {
entry:
  %__calltmp = call i32 @if_else_hello(i32 2)
  ret i32 0
}