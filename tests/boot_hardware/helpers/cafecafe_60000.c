int main(void)
{
  *((unsigned*)0x60000) = 0xCAFECAFE;
  return 0;
}
