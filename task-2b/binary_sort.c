#include <stdio.h>

int binary_search(int *A, int low, int high, int x)
{
  return -1;
}

void insertion_sort(int *A, int size)
{
  return;
}

int input(int *A)
{
  int size;

  if (fscanf(stdin, "%08x\n", &size) != 1)
  {
    return 0;
  }

  for (int i = 0; i < 10; i++)
  {
    if (fscanf(stdin, "%08x\n", A) != 1)
    {
      return size;
    }
    A++;
  }

  return size;
}

void output(int size, int *A)
{
  for (int i = 0; i < size; i++)
  {
    fprintf(stdout, "%08x\n", (unsigned int)A[i]);
  }
}

int main(void)
{
  int A[10];
  int size;

  size = input(A);
  insertion_sort(A, size);
  output(size, A);

  return 0;
}
