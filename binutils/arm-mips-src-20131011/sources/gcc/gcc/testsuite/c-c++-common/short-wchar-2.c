/* { dg-do compile } */
/* { dg-options "-fshort-wchar" } */

extern int i[sizeof (L'a')];
int i[sizeof (short unsigned int)];
