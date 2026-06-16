#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dico.h"

/*
 * Ne *PAS* utiliser cette implémentation pour du code de production dans
 * vos entreprises ! Il s'agit d'une démonstration du concept de
 * set / dictionnaire, pas d'un code optimisé.
 *
 * Quelques pistes pour faire mieux:
 * 	=> NE PAS RÉINVENTER LA ROUE, quel que soit votre environnement
 * 	de développement il y a fort à parier qu'une implémentation
 * 	robuste des dictionnaires soit disponible dans une bibliothèque
 * 	quelconque
    => supprimer les modulos en utilisant une taille de table de
 * 	hash qui est une puissance de deux et faire h&(taille-1)
 * 	=> pour la partie dictionnaire, le chaînage simple entraîne
 * 	rapidement un effondrement des performances. Plus il y a de
 * 	collisions de hash, plus les listes chaînées s'allongent,
 * 	plus le temps d'insertion et de recherche augmente. Il faut
 * 	a minima ajouter une fonction pour redimensionner automatiquement
 * 	le dictionnaire quand il devient "trop rempli".
 ** 	=> le hash utilisé (FNV-1a) est déterministe, si la sécurité
 * 	est de mise (par exemple hasher des données venues de sources
 * 	potentiellement hostiles) cela veut dire qu'un attaquant peut
 * 	générer intentionnellement des collisions de hash qui vont
 * 	effondrer les performances. Sans passer sur un hash cryptographique
 * 	il faudrait au moins disposer d'un hash résistant aux collisions
 * 	avec une clé secrète pour "saler" le hash (on parle de salted-hash
 * 	en anglais). Voir par exemple SipHash utilisé en Python ou Rust.
 * 	=> éviter le chaînage simple des dict_entry et passer en adressage
 * 	ouvert (sondage linéaire par exemple) en gardant le redimensionnement.
 * 	En plus ça éviterait d'avoir un tableau de pointeurs dans dict_struct et on
 * 	pourrait se contenter d'un tableau de structures, en prime le next de
 * 	dict_entry devient inutile. Avec cette optimisation les dict_entrt d'un
 * 	dictionnaire de quelques milliers d'entrées tiendrait en entier dans le
 * 	cache L1 d'un microprocesseur moderne, alors qu'avec les listes chaînées il
 * 	y a 90% (chiffre Opif(tm) mais réaliste) de chances de tomber en
 * 	dehors du cache et devoir aller chercher en mémoire.
 * 	=> même avec les dict_entry réorganisées comme suggéré, le dictionnaire
 * 	éparpille les valeurs en mémoire avec le pointeur value, une implémentation
 * 	sérieuse utiliserait un seul gros bloc 	alloué une fois pour toutes sur
 * 	le tas. Cela évite les malloc() à chaque insertion, on rajoute simplement
 * 	les valeurs les unes à la suite des autres dans ce bloc mémoire. La
 * 	libération de la mémoire du dictionnaire devient aussi simplissime, et
 * 	on augmente les chances d'avoir des valeurs déjà présentes dans le cache
 * 	du microprocesseur.
 * 	=> le set est vraiment juste un dictionnaire sans value alors qu'en
 * 	pratique on peut faire des structures de données et de code optimisées,
 * 	par exemple pas besoin de value et value_len, et on peut utiliser des
 * 	optimisations agressives telles que le cuckoo hashing qui marchent
 * 	moins bien sur les dictionnaires
 * 	=> cela peut sembler anodin mais réduire la taille de la structure
 * 	dict_entry en enlevant un pointeur et un entier (value et value_len) pour
 * 	en faire un set_entry a un impact énorme sur le nombre d'entrées qu'on peut
 * 	avoir dans le cache L1 du processeur: hash, raw_key, raw_key_len font
 * 	4+8+8 = 20 octets donc 24 en mémoire. Si on ajoute value et value_len
 * 	on a 20 + 8 + 8 = 36 octetes arrondis à 40. Donc on peut faire tenir un set
 * 	66% plus gros qu'un dictionnaire dans le cache d'un microprocesseur avec
 * 	cette simple optimisation.
 * 	=> le set codé ici n'offre aucune fonction d'union ou d'intersection
 */

typedef struct dict_entry
{
    uint32_t hash;

    void *raw_key;
    size_t raw_key_len;

    void *value;
    size_t value_len;

    struct dict_entry *next;
} dict_entry_t;

struct dict_struct
{
    dict_entry_t **table;
    size_t table_len;
    size_t key_nb;
};

// FNV-1a 32 bits
uint32_t fnv1a_32(const void *data, size_t len)
{
    uint32_t hash = 0x811c9dc5;                 // offset_basis 32 bits
    unsigned char *ptr = (unsigned char *)data; // cast pour avancer 1 octet à la fois

    for (size_t i = 0; i < len; i++)
    {
        hash ^= ptr[i];     // XOR avec l'octet courant
        hash *= 0x01000193; // multiplication par FNV_prime 32 bits
    }

    return hash;
}

uint64_t now_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

dict_t *dict_create(void)
{
    dict_t *dict = calloc(1, sizeof(*dict));

    if (!dict)
        return NULL;

    dict->table = calloc(HASH_TABLE_DEFAULT_SIZE, sizeof(dict_entry_t *));

    if (!dict->table)
    {
        free(dict);
        return NULL;
    }

    dict->table_len = HASH_TABLE_DEFAULT_SIZE;
    dict->key_nb = 0;
    return dict;
}

void dict_entry_destroy(dict_entry_t *entry)
{
    free(entry->value);
    free(entry->raw_key);
    free(entry);
}

void dict_destroy(dict_t *dict)
{
    for (size_t i = 0; i < dict->table_len; i++)
    {
        dict_entry_t *cur = dict->table[i];
        while (cur)
        {
            dict_entry_t *tmp = cur->next;
            dict_entry_destroy(cur);
            cur = tmp;
        }
    }
    free(dict->table);
    free(dict);
}

dict_status_t internal_dict_equal_key(const dict_entry_t *a, uint32_t hash, const void *raw_key, size_t raw_key_len)
{
    if (a->hash == hash && a->raw_key_len == raw_key_len && !memcmp(a->raw_key, raw_key, raw_key_len))
        return DICT_OK;
    return DICT_NOK;
}

size_t dict_len(const dict_t *dict)
{
    return dict->key_nb;
}

static dict_entry_t **internal_dict_find_entry_ptr(const dict_t *dict, const void *raw_key,
                                                   size_t raw_key_len, uint32_t hash)
{
    dict_entry_t **ptr = &(dict->table[hash % dict->table_len]);
    while (*ptr)
    {
        if (internal_dict_equal_key(*ptr, hash, raw_key, raw_key_len) == DICT_OK)
        {
            return ptr;
        }
        ptr = &((*ptr)->next);
    }
    return ptr;
}

dict_status_t dict_contains(const dict_t *dict, const void *key, size_t key_len)
{
    uint32_t h = fnv1a_32(key, key_len);
    const dict_entry_t *ret = *internal_dict_find_entry_ptr(dict, key, key_len, h);
    if (!ret)
        return DICT_ERR_NOT_FOUND;
    return DICT_OK;
}

dict_status_t dict_get_value(const dict_t *dict, const void *key, size_t key_len,
                             const void **value_ptr, size_t *value_len)
{
    uint32_t h = fnv1a_32(key, key_len);
    const dict_entry_t *ret = *internal_dict_find_entry_ptr(dict, key, key_len, h);
    if (!ret)
    {
        *value_ptr = NULL;
        *value_len = 0;
        return DICT_ERR_NOT_FOUND;
    }
    *value_ptr = ret->value;
    *value_len = ret->value_len;
    return DICT_OK;
}

static void *internal_helper_copy_or_null(const void *src, size_t len)
{
    if (!src || len == 0)
        return NULL;
    void *dest = malloc(len);
    if (dest)
        memcpy(dest, src, len);
    return dest;
}

dict_status_t dict_add(dict_t *dict, void *key, size_t key_len, void *value, size_t value_len)
{
    // if data is NULL or data_len is zero no value will be associated
    // with the key, call this a poor man's set
    uint32_t h = fnv1a_32(key, key_len);
    dict_entry_t **entry_ptr = internal_dict_find_entry_ptr(dict, key, key_len, h);
    if (*entry_ptr)
    { // we already know this key, update it
        void *new_val = internal_helper_copy_or_null(value, value_len);

        if (value && value_len && !new_val)
            return DICT_ERR_MALLOC;

        free((*entry_ptr)->value);
        (*entry_ptr)->value = new_val;
        (*entry_ptr)->value_len = value_len;
        return DICT_VALUE_UPDATED;
    }
    // If we're here we either have a collision or it's a new key
    // In both cases we have to create a new dict_entry

    dict_entry_t *new_node;
    new_node = calloc(1, sizeof(*new_node));

    if (!new_node)
        return DICT_ERR_MALLOC;

    new_node->raw_key = internal_helper_copy_or_null(key, key_len);
    new_node->value = internal_helper_copy_or_null(value, value_len);

    if (!new_node->raw_key || (value_len > 0 && !new_node->value))
    {
        free((void *)new_node->raw_key);
        free(new_node->value);
        free(new_node);
        return DICT_ERR_MALLOC;
    }

    new_node->hash = h;
    new_node->raw_key_len = key_len;
    new_node->value_len = value_len;

    // Append new data at the end of the list
    *entry_ptr = new_node;

    // Update dict length counter
    dict->key_nb++;
    return DICT_OK;
}

// ---------------------------------------------------------------------------
// PARTIE BENCHMARK
// ---------------------------------------------------------------------------

// Helper pour charger tout le fichier en mémoire AVANT de lancer le chrono
// Cela évite de mesurer la vitesse du disque dur au lieu de l'algo
char **load_dataset(const char *filename, size_t *out_count)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        perror(filename);
        return NULL;
    }

    // On alloue large pour pas s'embêter avec des reallocs complexes dans le bench
    // Disons max 100k mots pour ce TP
    size_t capacity = 100000;
    char **words = malloc(capacity * sizeof(char *));
    size_t count = 0;

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, f) != -1)
    {
        if (count >= capacity)
            break;

        // Nettoyage \n
        line[strcspn(line, "\r\n")] = 0;

        // Copie propre pour stocker dans notre tableau
        words[count] = strdup(line);
        count++;
    }

    free(line);
    fclose(f);
    *out_count = count;
    printf("[INFO] %zu mots chargés en mémoire depuis %s\n", count, filename);
    return words;
}

/*
int main(void) {
    dict_t* set = dict_create();
    dict_t* dico = dict_create();

    FILE* f = fopen("mots_17479.txt", "r");
    if (!f) { perror("mots_17479.txt"); return 1; }

    char *line = NULL;
    size_t len = 0; // taille du buffer, getline s'en charge

    int line_nb = 1;
    while (getline(&line, &len, f) != -1) {
        // line contient la ligne complète, '\n' inclus
    size_t word_len = strcspn(line, "\r\n");
    // on remplace \n par \0 (pour pas casser printf, sinon OSEF)
    line[word_len] = '\0';
    // on veut hasher toute la chaîne \0 compris

    // ici on construit un vrai dictionnaire clé -> valeur
    dict_add(dico, line, word_len + 1, &line_nb, sizeof(int));

    // là on fait un set, donc juste des clés
    dict_add(set, line, word_len + 1, NULL, 0);
    line_nb++;
    }

    printf("Le dictionnaire contient %ld clés\n", dict_len(dico));
    printf("Le set contient %ld clés\n", dict_len(set));

    char to_find[]="aplatissaient";

    // un essai avec le set
    printf("Recherche dans un set simple\n");
    uint64_t t0 = now_us();
    dict_status_t found = dict_contains(set, to_find, strlen(to_find) + 1);
    uint64_t t1 = now_us();
    printf("durée = %llu us\n",
        (unsigned long long)(t1 - t0));
    if(found == DICT_OK){
        printf("%s est dans la liste de mots\n", to_find);
    }
    else{
        printf("%s n'est PAS dans la liste de mots\n", to_find);
    }

    // le même avec une recherche de clé dans le dictionnaire
    printf("Recherche dans un dictionnaire\n");
    char to_find2[]="qdsfjhqskjlhfk";
    t0 = now_us();
    found = dict_contains(dico, to_find2, strlen(to_find2) + 1);
    t1 = now_us();
    printf("durée = %llu us\n",
        (unsigned long long)(t1 - t0));
    if(found == DICT_OK){
        printf("%s est dans la liste de mots\n", to_find2);
    }
    else{
        printf("%s n'est PAS dans la liste de mots\n", to_find2);
    }


    printf("Recherche clé/valeur dans un dictionnaire\n");
    char to_find3[]="impliquassent";
    const int* value;
    size_t value_len;
    t0 = now_us();
    found = dict_get_value(dico, to_find3, strlen(to_find3) + 1, (const void**)&value, &value_len);
    t1 = now_us();
    printf("durée = %llu us\n",
        (unsigned long long)(t1 - t0));
    if(found == DICT_OK){
        printf("%s est dans la liste de mots à la ligne %d\n", to_find3, *(int*)value);
    }
    else{
        printf("%s n'est PAS dans la liste de mots\n", to_find3);
    }

    dict_destroy(dico);
    free(line);
    fclose(f);
    return 0;
}
*/