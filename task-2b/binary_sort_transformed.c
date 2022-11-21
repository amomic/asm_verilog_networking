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
   int m = 0; //m
   a2 = m;

   size_t stacked_s1 = s1; //low
   t1 = s1;

   size_t stacked_s2 = s2; //high
   t2 = s2;

   size_t stacked_s3 = s3; //x
   t3 = s3;


   if(t1<=t2) goto input_continue;
   if(t3<=a0) goto if_continue;
   a0 = s1;
   return;

   input_continue:
   m = (s1+s2)/2;
   if(t3 != a0) goto input_if_continue;
   a0 = a2;
   return;

   if_continue:
   a0 = s1 +1;
   return;
    
   input_if_continue: 
    if (t3<a0) goto calculation_one;
    goto calculation_two; 

    calculation_one: //
    return;

    calculation_two: //
    return;

  size_t t6 = -1;
  a0 = t6;
  return;
}


// void insertion_sort(int* A, int size)
void insertion_sort()
{

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
    a1 = a1 + 4;
    // Decrement loop counter
    a0 = a0 - 1;
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

