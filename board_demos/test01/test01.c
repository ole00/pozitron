#include <stddef.h>
#include "stm32f10x.h"

//Function prototype - implemented in test01.s
unsigned int test_asm(unsigned int a, unsigned int b, unsigned int c);


int calculate(int a, int b) {
	return a + b;
}

int main(void)
{
  int i = 0;

#ifndef HELLO_WORLD
#error HELLO_WORLD is not defined?
#endif

  i = calculate(i, 6);
  test_asm(i, 2 * i, i + 3);

  while (1)
  {
	i++;
  }
}

