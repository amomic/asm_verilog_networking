# adding-two-constants example
# (no symbols, no jumps, no I/O)
# LW = load word
# ADD = addition
# SW = store word
# EBREAK = halt CPU

.org 0x80
    LW x1, 0x100(x0)
    LW x2, 0x104(x0)
    ADD x3, x1, x2
    SW x3, 0x108(x0)
    # Write results to stdout
    SW x3, 0x7fc(x0)
    EBREAK


.org 0x100
    .word 42
    .word 13
