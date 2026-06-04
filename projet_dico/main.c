/* wc - count lines, words and characters	Author: David Messer */
#include <stdio.h>
#include <stdlib.h>
#include "wc.h"

int lflag; /* Count lines */
int wflag; /* Count words */
int cflag; /* Count characters */

void usage(void);

int main(int argc, char *argv[])
{
	struct Counting cmp;
	struct Counting *pcmp = &cmp;
	int k;
	char *cp;
	int tflag, files;

	/* Get flags. */
	files = argc - 1;
	k = 1;
	cp = argv[1];
	if (argc > 1 && *cp++ == '-')
	{
		files--;
		k++; /* points to first file */
		while (*cp != 0)
		{
			switch (*cp)
			{
			case 'l':
				lflag++;
				break;
			case 'w':
				wflag++;
				break;
			case 'c':
				cflag++;
				break;
			default:
				usage();
			}
			cp++;
		}
	}

	/* If no flags are set, treat as wc -lwc. */
	if (!lflag && !wflag && !cflag)
	{
		lflag = 1;
		wflag = 1;
		cflag = 1;
	}

	/* Process files. */
	tflag = files >= 2; /* set if # files > 1 */

	/* Check to see if input comes from std input. */
	if (k >= argc)
	{
		count(stdin, pcmp);
		if (lflag)
			printf(" %6ld", cmp.lcount);
		if (wflag)
			printf(" %6ld", cmp.wcount);
		if (cflag)
			printf(" %6ld", cmp.ccount);
		printf(" \n");
		fflush(stdout);
		exit(0);
	}

	/* There is an explicit list of files.  Loop on files. */
	while (k < argc)
	{
		FILE *f;

		if ((f = fopen(argv[k], "r")) == (FILE *)NULL)
		{
			fprintf(stderr, "wc: cannot open %s\n", argv[k]);
		}
		else
		{
			count(f, pcmp);
			if (lflag)
				printf(" %6ld", cmp.lcount);
			if (wflag)
				printf(" %6ld", cmp.wcount);
			if (cflag)
				printf(" %6ld", cmp.ccount);
			printf(" %s\n", argv[k]);
			fclose(f);
		}
		k++;
	}

	if (tflag)
	{
		if (lflag)
			printf(" %6ld", cmp.ltotal);
		if (wflag)
			printf(" %6ld", cmp.wtotal);
		if (cflag)
			printf(" %6ld", cmp.ctotal);
		printf(" total\n");
	}
	fflush(stdout);
	exit(0);
}

void usage(void)
{
	fprintf(stderr, "Usage: wc [-lwc] [name ...]\n");
	exit(1);
}
