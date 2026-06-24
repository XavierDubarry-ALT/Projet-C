#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "wc.h"

void count(FILE *f, struct Counting *pcmp, dict_t *dict)
{
  pcmp->lcount = 0;
  pcmp->wcount = 0;
  pcmp->ccount = 0L;

  char *line = NULL;
  size_t len = 0;
  int *compteur;

  while (getline(&line, &len, f) != -1)
  {
    pcmp->lcount++;
    char *mot = strtok(line, " \t\n");
    while (mot != NULL)
    {
      pcmp->wcount++;
      pcmp->ccount = pcmp->ccount + strlen(mot) + 1; // le plus un est pour un espace après chaque mot

      if (dict != NULL)
      {
        if (dict_contains(dict, mot, strlen(mot)) == DICT_OK)
        {
          size_t val_len;
          dict_get_value(dict, mot, strlen(mot), (const void **)&compteur, &val_len);
          int val = *compteur + 1;
          dict_add(dict, mot, strlen(mot), &val, sizeof(int));
        }
        else
        {
          int val = 1;
          dict_add(dict, mot, strlen(mot), &val, sizeof(int));
        }
      }
      mot = strtok(NULL, " \t\n");
    }
    pcmp->ccount--; // pour enlever l'espace en trop qui est compté, lorsque l'on fait un retour à la ligne
  }
  free(line);
  pcmp->ltotal += pcmp->lcount;
  pcmp->wtotal += pcmp->wcount;
  pcmp->ctotal += pcmp->ccount;
}
