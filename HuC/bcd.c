void
bcd_convert_8bit(unsigned char  num,
                 char          *dest)
{
  dest[0] = (num / 100);
  dest[1] = ((num % 100) / 10);
  dest[2] = (num % 10);
}

void
bcd_convert_16bit(unsigned int  num,
                  char         *dest)
{
  dest[0] = (num / 10000);
  dest[1] = ((num % 10000) / 1000);
  dest[2] = ((num % 1000) / 100);
  dest[3] = ((num % 100) / 10);
  dest[4] = (num % 10);
}
