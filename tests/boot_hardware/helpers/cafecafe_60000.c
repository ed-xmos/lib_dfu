// Copyright (c) 2020, XMOS Ltd, All rights reserved
int main(void)
{
  *((unsigned*)0x60000) = 0xCAFECAFE;
  return 0;
}
