.uasm 1.0
.segment code
.lbl start
.origin 0x00 # Code origin

# Jump register test with data
# Zero link
li a, 0
swl a
# Read first byte in data
mdr a
cov
jreg a
.lbl pass_1
sov
jreg a
.lbl pass_2
# Increment link and read data
li a, 1
swl a
mdr a
cov
jreg a
.lbl pass_3
sov
jreg a

.seek 0x30
out a
jmp pass_1

.seek 0x50
out a
jmp pass_3

.seek 0x130
out a
jmp pass_2

.seek 0x150
out a
jmp halt

.lbl halt
jmp halt


.segment data
.db 0x30
.db 0x50