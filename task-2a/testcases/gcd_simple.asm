# Implements the GCD calculation loop as in task-2a.
# The actual calculation is performed using the memory mapped gcd device.
#
.org 0x80
_start:             ADDI sp, zero, 0x700
                    JAL ra, main
                    EBREAK

###############################################################################
# void main(void)
main:               ADDI sp, sp, -4
                    SW ra, 0(sp)

# Read the input from stdin and abort if a zero valud was entered
main_loop:          LW a0, 0x7fc(x0)                # a = read();
                    LW a1, 0x7fc(x0)                # b = read();

                    BNE a0, zero, cont
                    BNE a1, zero, cont
                    JAL zero, main_return

cont:
                    JAL ra, gcd
                    SW a0, 0x7fc(x0)                # write(result);
                    JAL zero, main_loop

main_return:        LW ra, 0(sp)
                    ADDI sp, sp, 4
                    JALR zero, 0(ra)

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
