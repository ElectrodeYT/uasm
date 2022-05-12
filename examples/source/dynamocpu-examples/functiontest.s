.uasm 1.0
.segment code
.lbl start
.origin 0x00 # Code origin

call func1
call func3
call func2
jmp halt


.lbl func1
li a, 10
out a
return

.lbl func2
li a, 20
out a
return

.lbl func3
li a, 30
out a
return

.lbl halt
jmp halt