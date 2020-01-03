// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <print.h>

void test_c(void);
void test_xc(void);

int main(void)
{
  test_xc();
  test_c();
  printstr("PASS\n");
  return 0;
}
