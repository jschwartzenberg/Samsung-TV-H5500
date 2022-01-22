/* Check that without "fshort-wchar" wchar_t the same size as "int".  */
/* { dg-do compile } */

extern int i[sizeof (L'a')];
int i[sizeof (int)];
