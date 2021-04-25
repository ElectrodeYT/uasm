.uasm 1.0
.origin 0x200
.lbl start
mov v0, 0
fnt v0

.lbl loop
add v0, 8
mov v1, 20
draw v0, v1, 5
jmp loop
.skip 10 # skip 10 bytes
.db 0x41, 0x42, 0x43, 0x44 # test this