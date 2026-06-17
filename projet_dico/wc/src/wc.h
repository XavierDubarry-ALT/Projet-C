struct Counting
{
    long lcount; /* Count of lines */
    long wcount; /* Count of words */
    long ccount; /* Count of characters */

    long ltotal; /* Total count of lines */
    long wtotal; /* Total count of words */
    long ctotal; /* Total count of characters */

    int lflag; /* Count lines */
    int wflag; /* Count words */
    int cflag; /* Count characters */
};

void count(FILE *f, struct Counting *pcmp);