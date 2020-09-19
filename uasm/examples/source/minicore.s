.uasm 1.0
.segment code
.lbl start

# NOP
.db 0x00

# Set pointer register to variable
ldil variable
ltp # Store link register in pointer

# Initialize acc to 1, dat to 0
ldia 1
ldid 0
.lbl loop-fib
# Fibonnaci loop
# a:  3
# d:  2
# var:2

str # Store a to variable
add
ldd
jp loop-fib # Repeat

.segment data
.lbl variable
.db 0x0