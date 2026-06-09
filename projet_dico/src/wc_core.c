#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "wc.h"

void count(FILE *f, struct Counting *pcmp)
{
  int c;
  int word = 0;

  pcmp->lcount = 0;
  pcmp->wcount = 0;
  pcmp->ccount = 0L;

  while ((c = getc(f)) != EOF)
  {
    pcmp->ccount++;

    if (isspace(c))
    {
      if (word)
        pcmp->wcount++;
      word = 0;
    }
    else
    {
      word = 1;
    }

    if (c == '\n' || c == '\f')
      pcmp->lcount++;
  }
  pcmp->ltotal += pcmp->lcount;
  pcmp->wtotal += pcmp->wcount;
  pcmp->ctotal += pcmp->ccount;
}
