.uasm 1.0
.segment code
.lbl start
.origin 0x00 # Code origin

# Data test

# We test the lil instruction
lil 10
li a, 0
out a
swl a
out a

# First, we write 10 into data, overwrite a with something, read it back and output it
lil 0
li a, 10
mrd a
li a, 20
out a
mdr a
out a

# Next, with 10 in position 0, we write 30 into 2 and read that back
lil 1
li a, 30
mrd a
li a, 40
out a
mdr a
out a

# For this, we write 0, 1, 2, 3 into the data area, then read and out it in reverse order

# Zero link register
lil 0

# Set b = 1 so we can increment with add
li b, 1

# Write zero into data
li a, 0
mrd a
# Increment a and l
add a
swl a
add a
# Write 1 into data
mrd a

# 2
add a
swl a
add a
mrd a

#3
add a
swl a
add a
mrd a

# Now we can read data
mdr a
out a
sub a
swl a
mdr a
out a
sub a
swl a
mdr a
out a
sub a
swl a
mdr a
out a
.lbl halt
jmp halt