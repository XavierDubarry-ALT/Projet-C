/* wc - count lines, words and characters	Author: David Messer */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "wc.h"

void usage(void);

int main(int argc, char *argv[])
{

	struct Counting cmp = {0};
	struct Counting *pcmp = &cmp;
	int opt;
	int tflag;

	while ((opt = getopt(argc, argv, "lwcS")) != -1)
	{
		switch (opt)
		{
		case 'l':
			cmp.lflag++;
			break;
		case 'w':
			cmp.wflag++;
			break;
		case 'c':
			cmp.cflag++;
			break;
		case 'S':
			cmp.Sflag++;
			break;
		default:
			usage();
		}
	}

	/* If no flags are set, treat as wc -lwc. */
	if (!cmp.lflag && !cmp.wflag && !cmp.cflag)
	{
		cmp.lflag = 1;
		cmp.wflag = 1;
		cmp.cflag = 1;
	}

	/* Process files. */
	tflag = (argc - optind) >= 2; /* set if # files > 1 */

	/* Check to see if input comes from std input. */
	if (optind >= argc)
	{
		if (cmp.Sflag)
		{
			dict_t *dico = dict_create();
			count(stdin, pcmp, dico);
			dict_dump(dico);
			dict_destroy(dico);
		}
		else
		{
			count(stdin, pcmp, NULL);
		}
		if (cmp.lflag)
			printf(" %6ld", cmp.lcount);
		if (cmp.wflag)
			printf(" %6ld", cmp.wcount);
		if (cmp.cflag)
			printf(" %6ld", cmp.ccount);
		printf(" \n");
		fflush(stdout);
		exit(0);
	}

	/* There is an explicit list of files.  Loop on files. */
	while (argv[optind])
	{
		FILE *f;

		if ((f = fopen(argv[optind], "r")) == (FILE *)NULL)
		{
			fprintf(stderr, "wc: cannot open %s\n", argv[optind]);
		}
		else
		{
			if (cmp.Sflag)
			{
				dict_t *dico = dict_create();
				count(f, pcmp, dico);
				dict_dump(dico);
				printf("%-24s %6ld\n", "Nombre de mots unique:", dict_len(dico));
				dict_destroy(dico);
			}
			else
			{
				count(f, pcmp, NULL);
			}

			if (cmp.lflag)
				printf("%-24s %6ld\n", "Nombre de lignes:", cmp.lcount);
			if (cmp.wflag)
				printf("%-24s %6ld\n", "Nombre de mots:", cmp.wcount);
			if (cmp.cflag)
				printf("%-26s %6ld\n", "Nombre de charactères:", cmp.ccount);
			printf("%-25s %s\n", "Document lu:", argv[optind]);
			fclose(f);
		}
		optind++;
	}

	if (tflag)
	{
		if (cmp.lflag)
			printf(" %6ld", cmp.ltotal);
		if (cmp.wflag)
			printf(" %6ld", cmp.wtotal);
		if (cmp.cflag)
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
