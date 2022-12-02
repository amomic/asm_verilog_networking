#include <stdio.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
// RISC-V Register set
const size_t zero = 0;
size_t a0, a1;                      // fn args or return args
size_t a2, a3, a4, a5, a6, a7;      // fn args
size_t t0, t1, t2, t3, t4, t5, t6;  // temporaries
// Callee saved registers, must be stacked befor using it in a function!
size_t s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
//-----------------------------------------------------------------------------


// int binary_search(int* A, int low, int high, int x)
void binary_search()
{
    size_t saved_s1 = s1;
    size_t saved_s2 = s2;
    size_t saved_s3 = s3;
    size_t saved_s4 = s4;
    size_t saved_s5 = s5;
    size_t saved_s6 = s6;
    size_t saved_s7 = s7;
    size_t saved_s8 = s8;
    size_t saved_s9 = s9;
    size_t saved_s10 = s10;

    saved_s7 = a0;
    saved_s8 = a1;
    t2 = 0; //m
    t3 = -1; //shift

    if(saved_s1<= saved_s6) goto inside_first_if;
    goto first_else;

    inside_first_if: t2 = saved_s8+saved_s6;
    t2 = t2 >> 2;

    saved_s9 = a0+t2;
    if(saved_s2 != saved_s9) goto second_else;
    s1 = saved_s1;
    s2 = saved_s2;
    s3 = saved_s3;
    s4 = saved_s4;
    s5 = saved_s5;
    s6 = saved_s6;
    s7 = saved_s7;
    s8 = saved_s8;
    s9 = saved_s9;
    s10 = saved_s10;
    return;

    first_else: 
    saved_s10 = saved_s7+saved_s8;
    if(saved_s2>saved_s10) goto ret1;
    a0 = saved_s8;
    s1 = saved_s1;
    s2 = saved_s2;
    s3 = saved_s3;
    s4 = saved_s4;
    s5 = saved_s5;
    s6 = saved_s6;
    s7 = saved_s7;
    s8 = saved_s8;
    s9 = saved_s9;
    s10 = saved_s10;
    return;

    ret1: a0 = s8+1;
    s1 = saved_s1;
    s2 = saved_s2;
    s3 = saved_s3;
    s4 = saved_s4;
    s5 = saved_s5;
    s6 = saved_s6;
    s7 = saved_s7;
    s8 = saved_s8;
    s9 = saved_s9;
    s10 = saved_s10;
    return;

    second_else: if(saved_s2 < saved_s9) goto ret2;
    t2 = t2 +1;
    a0 = saved_s7;
    a1 = t2;
    a2 = saved_s6;
    a3 = saved_s2;
    binary_search();
    s1 = saved_s1;
    s2 = saved_s2;
    s3 = saved_s3;
    s4 = saved_s4;
    s5 = saved_s5;
    s6 = saved_s6;
    s7 = saved_s7;
    s8 = saved_s8;
    s9 = saved_s9;
    s10 = saved_s10;
    return;

    ret2:
    a0 = saved_s7;
    a1 = saved_s8;
    s11 = t2+t3;
    a2 = s11;
    a3 = saved_s2;
    binary_search();
    saved_s4 = a0;
    a0 = saved_s4;
    s1 = saved_s1;
    s2 = saved_s2;
    s3 = saved_s3;
    s4 = saved_s4;
    s5 = saved_s5;
    s6 = saved_s6;
    s7 = saved_s7;
    s8 = saved_s8;
    s9 = saved_s9;
    s10 = saved_s10;
    return;

return (void)-1;

}


// void insertion_sort(int* A, int size)
void insertion_sort()
{

    size_t saved_s1 = s1; // i
    size_t saved_s2 = s2; //x
    size_t saved_s3 = s3; //idx
    size_t saved_s4 = s4; //Aidx
    size_t saved_s5 = s5; //ai
    size_t saved_s6 = s6; //j
    t0 = 0; //explicit counter j
    t4 = 0; // temp for storing Aj
    t5 = 0; // temp for idx
    t6 = 0; //temp 
    goto for_loop;

    for_loop: if(t0<a0) goto inside_for;
    s1 = saved_s1;
    s2 = saved_s2;
    s3 = saved_s3;
    s4 = saved_s4;
    s5 = saved_s5;
    s6 = saved_s6;
    return;

    inside_for: t0 = t0+4;
    saved_s6 = t0;
    t4 = a1 + t0;
    saved_s2 = t4;

    a2 = 0;
    a3 = t1;
    a4 = saved_s2;
    binary_search();

    saved_s1 = t0;

    goto while_loop;

    while_loop: if(saved_s1>saved_s3) goto inside_while;
    goto lower_for;

    inside_while: saved_s1 = saved_s1 -1;
    t6 = a1 + saved_s1;
    saved_s5 = t6;
    if(saved_s5<=saved_s3) goto lower_for;
    goto while_loop;

    lower_for:  t5 = a1 + saved_s3;
    saved_s2 = t5;
    goto for_loop;


s1 = saved_s1;
s2 = saved_s2;
s3 = saved_s3;
s4 = saved_s4;
s5 = saved_s5;
s6 = saved_s6;
  return;
}


void input(void)
{
    // Read size
    t0 = a0; // Save a0
    a0 = fscanf(stdin, "%08x\n", (int*)&t1);
    t4 = 1;
    if (a0 == t4) goto input_continue;
    // Early exit
    a0 = 0;
    return;

input_continue:
    t4 = 1;
    t5 = 10;
input_loop_begin:
    if(t5 == 0) goto after_input_loop;
    a0 = fscanf(stdin, "%08x\n", (int*)&t2);
    if(a0 == t4) goto continue_read;
    // Exit, because read was not successful
    a0 = t1;
    return;
continue_read:
    *(int*)t0 = t2;
    // Pointer increment for next iteration
    t0 = t0 + 4;
    // Loop counter decrement
    t5 = t5 - 1;
    goto input_loop_begin;

after_input_loop:
    a0 = t1;
    return;
}


void output(void)
{
before_output_loop:
    if (a0 == 0) goto after_output_loop;

    fprintf(stdout, "%08x\n", (unsigned int)*(int*)a1);

    // Pointer increment for next iteration
    a1 = a1 + 4; //a1 is A[i]
    // Decrement loop counter
    a0 = a0 - 1; //ao is size
    goto before_output_loop;

after_output_loop:
    return;
}


int main(void)
{
  int A[10];
  int size;

  a0 = (size_t) A;
  input();
  size = a0;

  a0 = (size_t) A;
  a1 = size;
  // void insertion_sort(int* A, int size)
  insertion_sort();

  a0 = size;
  a1 = (size_t) A;
  output();

  return 0;
}

