# uasm - Universal Assembler

# Arguments

-i		input file
-o		output file (defaults to input file with file extension changed to .o)
-m		machine

# Set Instructions #inst syntax

Example instruction:
\#inst mov #, #:2:a:b:0000aabb

The first argument is the instruction as written, with the argument count following it. In the case that a register has to be encoded,
