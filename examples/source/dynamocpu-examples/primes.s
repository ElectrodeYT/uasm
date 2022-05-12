.uasm 1.0
.segment code
.lbl start
.origin 0x00 # Code origin

# Sieve of eratosthenes
# First we create the list in data memory
# We have a max of 255, so we create a list of 2-25 in data
li a, 255
li b, 1
lil 255
.lbl data_create_loop
# A and L are the same, so we are effectively saving the value of
# L into L
mrd a
sub a
swl a
sub a
jnz a, data_create_loop

# Beginning at two, we now mark out any multiple of the next non-zero number by setting it to zero
lil 2
li a, 2
.lbl begin_loop
out a
# We have to be a bit careful; we never use data position 0 and 1 so we can use that as a "register"
# To move a -> b we write a to 0, and read it into b
# Since L and A are the same, we can then swap a and al and read it into a
lil 0
mrd a
mdr b
swl a
mdr a

# Now A, B and L are all the same (the prime)

# With this value in 0, we can now add and set it to 0 until we overflow
.lbl mark_out_loop
add a # We go to the next multiple
jov mark_out_loop_break # If we overflowed, that means we got to the end of the list
swl a # We move the next multiple to the linker registers
li a, 0 # We set A to 0
mrd a # Now we write 0 to the next multiple
swl a # We move the next multiple to the a register
jmp mark_out_loop

.lbl mark_out_loop_break
# The numbers have been marked out, we now need to find the next non-zero number
# For this we restore the number out of data pos 0
lil 0
li b, 1
mdr a

.lbl find_next_loop
add a # Get the next value
jov halt # We overflowed on finding the next number, we cant calculate any more
swl a # Swap to the link register
mdr a # Read it
jnz a, begin_loop # If the value we read was not zero, we jump to the top
swl a # It was zero, get the link register again
jmp find_next_loop


.lbl halt
jmp halt

.segment data