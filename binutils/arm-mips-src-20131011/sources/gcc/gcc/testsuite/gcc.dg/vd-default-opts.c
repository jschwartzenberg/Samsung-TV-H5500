/* { dg-do compile } */
/* { dg-options "-frecord-gcc-switches" } */

double f(double x)
{
  return x;
}

/* { dg-final { scan-assembler "-femit-struct-debug-reduced" } } */
/* { dg-final { scan-assembler "-gstrict-dwarf" } } */
/* { dg-final { scan-assembler "-gdwarf-2" } } */
