#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dico.h"

int main(void)
{
    size_t word_count = 0;
    char **dataset = load_dataset("mots_17479.txt", &word_count);

    if (!dataset || word_count == 0)
    {
        fprintf(stderr, "Erreur: Impossible de charger le dataset.\n");
        return 1;
    }

    // Attention, arrivé là on peut avoir des doublons comptés dans word_count !
    dict_t *dico = dict_create();

    printf("\n=== DEBUT DU BENCHMARK ===\n");
    printf("Configuration: %d cases dans le tableau\n", HASH_TABLE_DEFAULT_SIZE);

    // ---------------------------------------------------------
    // TEST 1 : INSERTION
    // ---------------------------------------------------------
    uint64_t t0 = now_us();
    for (size_t i = 0; i < word_count; i++)
    {
        // On utilise i comme "valeur" (int) juste pour avoir une payload
        int val = (int)i;
        dict_add(dico, dataset[i], strlen(dataset[i]) + 1, &val, sizeof(int));
        // Sinon on peut aussi mettre NULL et 0 en derniers paramètres et
        // ça fait un set du pauvre.
    }
    size_t uniq_word_count = dict_len(dico); // ici on a le nombre de clés uniques
    uint64_t t1 = now_us();

    double total_insert_ms = (t1 - t0) / 1000.0;
    double ns_per_insert = ((double)(t1 - t0) * 1000.0) / word_count;

    printf("\n[INSERTION]\n");
    printf("Le dictionnaire contient %ld mots uniques parmi %ld mots dans le fichier\n",
           uniq_word_count, word_count);
    printf("Total temps : %.3f ms\n", total_insert_ms);
    printf("Moyenne     : %.2f ns / mot\n", ns_per_insert);



    // ---------------------------------------------------------
    // TEST 2 : RECHERCHE (POSITIVE - Cas où ça existe)
    // ---------------------------------------------------------
    t0 = now_us();
    int found_count = 0;
    for (size_t i = 0; i < word_count; i++)
    {
        if (dict_contains(dico, dataset[i], strlen(dataset[i]) + 1) == DICT_OK)
        {
            found_count++;
        }
    }
    t1 = now_us();

    double ns_per_find_hit = ((double)(t1 - t0) * 1000.0) / word_count;

    printf("\n[RECHERCHE (HIT) - Mots existants]\n");
    printf("Trouvés     : %d / %zu\n", found_count, word_count);
    printf("Moyenne     : %.2f ns / recherche\n", ns_per_find_hit);

    // ---------------------------------------------------------
    // TEST 3 : RECHERCHE (NEGATIVE - Cas où ça n'existe pas)
    // C'est souvent le pire cas pour les tables de hachage (parcours complet des collisions)
    // ---------------------------------------------------------
    t0 = now_us();
    int not_found_count = 0;
    char buffer[256];

    for (size_t i = 0; i < word_count; i++)
    {
        // On génère un mot qui n'existe surement pas (mot + "_X")
        snprintf(buffer, 256, "%s_X", dataset[i]);

        if (dict_contains(dico, buffer, strlen(buffer) + 1) != DICT_OK)
        {
            not_found_count++;
        }
    }
    t1 = now_us();

    double ns_per_find_miss = ((double)(t1 - t0) * 1000.0) / word_count;

    printf("\n[RECHERCHE (MISS) - Mots inexistants]\n");
    printf("Non trouvés : %d / %zu\n", not_found_count, word_count);
    printf("Moyenne     : %.2f ns / recherche\n", ns_per_find_miss);

    // ---------------------------------------------------------
    // ANALYSE RAPIDE
    // ---------------------------------------------------------
    printf("\n=== ANALYSE ===\n");
    printf("Facteur de charge (Load Factor) : %.2f\n", (float)word_count / HASH_TABLE_DEFAULT_SIZE);
    if ((float)word_count / HASH_TABLE_DEFAULT_SIZE > 5.0)
    {
        printf("ALERTE: Le facteur de charge est ÉLEVÉ.\n");
        printf("Cela signifie beaucoup de collisions (moyenne théorique de %.0f éléments par case).\n",
               (float)word_count / HASH_TABLE_DEFAULT_SIZE);
        printf("C'est pour ça que la recherche est lente !\n");
    }
    printf("\nTest intensif sur une clé unique\n");
    t0 = now_us();

    // On fait 10 MILLIONS de fois la même recherche
    for (int i = 0; i < 10000000; i++)
    {
        // On cherche toujours le même mot (ou on cycle sur un petit tableau)
        // pour que les données restent dans le Cache L1 du CPU.
        // Ainsi, on élimine le temps RAM, et on ne mesure que le temps CPU (Hash + Modulo).
        dict_contains(dico, dataset[uniq_word_count - 1], strlen(dataset[uniq_word_count - 1] + 1));
    }

    t1 = now_us();
    double ns_per_find = ((double)(t1 - t0) * 1000.0) / 10000000;
    printf("Durée par recherche: %.2f ns / recherche\n", ns_per_find);

    dict_status_t suppr = dict_remove_key(dico, "test1", strlen("test1")+1);
    if (suppr == DICT_REMOVE_OK)
    {
        printf("La clé test1 à été supprimée (status:%d)",suppr);
    }
    else
    {
        printf("La clé est introuvable dans le dictionnaire (status:%d)",suppr);
    }
    

    // Nettoyage
    dict_destroy(dico);
    for (size_t i = 0; i < word_count; i++)
        free(dataset[i]);
    free(dataset);

    return 0;
}