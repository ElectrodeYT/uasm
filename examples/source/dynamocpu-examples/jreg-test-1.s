.uasm 1.0
.segment code
.lbl start
.origin 0x00 # Code origin

# Jump register test
# Should output 10 and 40

li a, 0x10
sov
jreg a
li a, 30
out a
jmp halt

.seek 0x10
li a, 20
out a
jmp halt

.seek 0x30
li a, 40
out a
jmp halt

.seek 0x110

li a, 10
out a
cov
li a, 0x30
jreg a

.lbl halt
jmp halt

