#include <stdint.h>
#ifndef DICO_H
#define DICO_H
typedef struct dict_struct dict_t;

typedef enum {
    DICT_NOK = 0,
    DICT_OK,
    DICT_VALUE_UPDATED,
    DICT_ERR_NOT_FOUND,
    DICT_ERR_MALLOC,
    DICT_REMOVE_OK
} dict_status_t;


// This is ridiculously low:
#define HASH_TABLE_DEFAULT_SIZE 512
//#define HASH_TABLE_DEFAULT_SIZE 100003

size_t dict_table_len(const dict_t *dict);

size_t dict_len(const dict_t* dict);

dict_t* dict_create(void);

uint64_t now_us(void);

dict_status_t dict_contains(const dict_t* dict, const void* key, size_t key_len);

void dict_destroy(dict_t* dict);

char** load_dataset(const char* filename, size_t* out_count);

dict_status_t dict_add(dict_t* dict, void* key, size_t key_len, void* value, size_t value_len);

dict_status_t dict_remove_key(dict_t *dict, const void *key, size_t key_len);

void dict_dump(dict_t *dict);

dict_status_t dict_get_value(const dict_t *dict, const void *key, size_t key_len,const void **value_ptr, size_t *value_len);

#endif


