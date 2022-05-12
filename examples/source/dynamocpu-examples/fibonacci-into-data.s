.uasm 1.0
.segment code
.lbl start
.origin 0x00 # Code origin

# Fibonacci into data

# Fibonacci  technically requires 3 variables: t1, t2, and next
# t1 and t2 initialize to 0 and 1, while next is also 1 due to t1 + t2

# Zero b and l and set l' to 1
li b, 0
lil 0
lilp 1

# We set a to 1
li a, 1
out b # First number is 0, so we output it here

.lbl loop
# The Fibonacci algorithm looks like this:
# next = t1 + t2
# output next
# t2 = t1
# t1 = next

# However, due to the fact that we only have two registers, we have to get a bit freaky with this
# To calculate next, we need to add t1 and t2
# We put t2 in a, as it will not be needed anyway
add a # a = a + b

# If there was an overflow, we have calculated the highest fibonnaci we can do
jov halt

# Now a = next, b = t1
# out a

# We now need to write this to the data segment
# We have set L to the address, so we can just write it
mrd a

# We now need to increment L, and since L' is 1, we can swap them into A and B, increment A, and swap them back
swl a
swlp b
add a
swl a
swlp b

# We now swap a and b, so a = t1 and b = next
# However, since we need to now reassign t1 and t2, we can also say a = t2 and b = t1
swp

jmp loop

.lbl halt
jmp halt