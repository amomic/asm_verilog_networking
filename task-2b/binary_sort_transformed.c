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
    size_t saved_s6 = s6;
    saved_s6 = 0; //m
    size_t saved_s7 = s7; //for division by shifting
    saved_s7 = 2;
    size_t saved_s8 =s8;
    size_t saved_s9 = s9;


    if(a2 <= a3) goto inside_first_if;
    goto first_else;

    inside_first_if: saved_s6 = a2+a3;
    saved_s6 = saved_s6 >> saved_s7;

    saved_s8 = a0+saved_s6;
    if(a4 != saved_s8) goto second_else;
    a0 = saved_s6;
    return;

    first_else: 
    saved_s9 = a0 + a2;
    if(a4>saved_s9) goto ret1;
    a0 = a2;
    return;

    ret1: a0 = a2+1;
    return;

    second_else: if(a4 < saved_s8) goto ret2;
    a2 = saved_s6+1;
    binary_search();
    a0 = a0;
    return;


    ret2: a3 = saved_s6 -1;
    binary_search();
    a0 = a0;
    return;

return -1;

}


// void insertion_sort(int* A, int size)
void insertion_sort()
{

    size_t saved_s1 = s1; // i
    size_t saved_s2 = s2; //x
    size_t saved_s3 = s3; //idx
    size_t saved_s4 = s4; //Aidx
    size_t saved_s5 = s5; //ai
    t0 = 0; //explicit counter j
    t1 = 0; // current index
    t2 = 2; // for shifting
    t4 = 0; // temp for storing Aj
    t5 = 0; // temp for Aidx
    t6 = 0; //temp for storing Ai
    goto for_loop;

    for_loop: if(t0<a0) goto inside_for;
    s1 = saved_s1;
    s2 = saved_s2;
    s3 = saved_s3;
    s4 = saved_s4;
    s5 = saved_s5;
    return;

    inside_for: t1 = t0;
    t1 = t1 << t2;

    t4 = a1 + t1;
    saved_s2 = *(int*)t4;

    a2 = 0;
    a3 = t1;
    a4 = saved_s2;
    binary_search();
    saved_s3 = a0;

    saved_s1 = t1;

    goto while_loop;

    while_loop: if(saved_s1>saved_s3) goto inside_while;
    goto lower_for;

    inside_while: saved_s1 = saved_s1 -1;
    t6 = a1 + saved_s1;
    saved_s5 = *(int*)t6;
    if(saved_s1<=saved_s3) goto lower_for;
    goto while_loop;

    lower_for:  t5 = a1 + saved_s3;
    t5 = saved_s2;
    goto for_loop;

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

