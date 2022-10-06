
# IRQ Handler
.org 0x00
    # Read Result
    LW a0, 0x7dc(zero)              # result = gcd.data
    SW a0, 0x7fc(x0)                # write(result);

    # Read data for the next
    LW a0, 0x7fc(x0)                # a = read();
    LW a1, 0x7fc(x0)                # b = read();   
    BNE a0, zero, cont_irq
    BNE a1, zero, cont_irq
    EBREAK

cont_irq:
    # Start next
    SW a0, 0x7d4(zero)              # Write A
    SW a1, 0x7d8(zero)              # Write B
    SW t1, 0x7d0(zero)              # gcd.ctrl = Start and enable IRQ
    MRET


# Implements the sine/cosine calculation loop as in task-2.
# The actual calculation is performed using the memory mapped gcd device.
#


# Start Address
.org 0x80
_start:             ADDI sp, zero, 0x700
                    JAL ra, main
                    EBREAK

###############################################################################
# void main(void)
main:               ADDI sp, sp, -4
                    SW ra, 0(sp)


# Read the input from stdin and abort if a zero valued was entered
                    LW a0, 0x7fc(x0)                # a = read();
                    LW a1, 0x7fc(x0)                # b = read();

                    BNE a0, zero, cont
                    BNE a1, zero, cont
                    JALR zero, 0(ra)                # Return

cont:
                    ADDI t1, zero, 9

                    SW a0, 0x7d4(zero)              # Write A
                    SW a1, 0x7d8(zero)              # Write B
                    SW t1, 0x7d0(zero)              # gcd.ctrl = Start and enable IRQ


loop:               JAL zero, loop                 # While 1


###############################################################################
# int32_t gcd(int32_t a, int32_t b)
gcd:                ADDI t1, zero, 1
                    ADDI t2, zero, 2
                    SW a0, 0x7d4(zero)              # Write A
                    SW a1, 0x7d8(zero)              # Write B
                    SW t1, 0x7d0(zero)              # gcd.ctrl = Start
gcd_poll:           LW a0, 0x7d0(zero)              # Read Busy
                    AND a0, a0, t2
                    BNE a0, zero, gcd_poll
                    LW a0, 0x7dc(zero)              # result = gcd.data
                    JALR zero, 0(ra)
