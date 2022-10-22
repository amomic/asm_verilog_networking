#include <stdio.h>

int binary_search(int *A, int low, int high, int x)
{
  int m = 0;
  if (low <= high)
  {
    m = (low + high) / 2;

    if (x == A[m])
    {
      return m;
    }
    else
    {
      if (x < A[m])
      {
        return binary_search(A, low, m - 1, x);
      }
      else
      {
        return binary_search(A, m + 1, high, x);
      }
    }
  }
  else
  {
    if (x > A[low])
    {
      return low + 1;
    }
    else
    {
      return low;
    }
  }

  return -1;
}

void insertion_sort(int *A, int size)
{
  int i = 0;
  int x = 0;
  int idx = 0;

  for (int j = 1; j < size; j++)
  {
    x = A[j];

    idx = binary_search(A, 0, j, x);
    i = j;

    while (i > idx)
    {
      A[i] = A[i - 1];
      i--;
    }

    A[idx] = x;
  }

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
