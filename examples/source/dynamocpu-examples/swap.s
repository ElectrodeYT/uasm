.uasm 1.0
.segment code
.lbl start
.origin 0x00 # Code origin

# Swap test

li a, 10
li b, 20
out a
out b
swp
out a
out b

.lbl halt
jmp halt