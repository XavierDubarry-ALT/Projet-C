#include <stdio.h>
#include <stdlib.h>

#define isspace(c) (c==' ' || c=='\t' || c=='\n' || c=='\f' || c=='\r')

long lcount;			/* Count of lines */
long wcount;			/* Count of words */
long ccount;			/* Count of characters */

long ltotal;			/* Total count of lines */
long wtotal;			/* Total count of words */
long ctotal;			/* Total count of characters */

void count(FILE* f);
//test
void count(FILE* f)
{
  register int c;
  register int word = 0;

  lcount = 0;
  wcount = 0;
  ccount = 0L;

  while ((c = getc(f)) != EOF) {
	ccount++;

	if (isspace(c)) {
		if (word) wcount++;
		word = 0;
	} else {
		word = 1;
	}

	if (c == '\n' || c == '\f') lcount++;
  }
  ltotal += lcount;
  wtotal += wcount;
  ctotal += ccount;
}
