
#include "text-search.h"

typedef int(*compare_fun_t)(const void *, const void *);

typedef struct bucket_rec {
    gint type_a;
    gint type_b;
    gint idx;
} bucket_t;

static int
strcmpr(const gchar **s1, const gchar **s2)
{
    return strcmp(*s1, *s2);
}

sarray_t *
sarray_new(const gchar *string)
{
    sarray_t *sarray;
    bucket_t *buckets;
    bucket_t *bucket;
    gchar **array;
    gchar c, c_;
    const gchar *cp;
    gint i;
    gint idx;
    
    sarray = g_malloc(sizeof(sarray_t));
    sarray->str = string;
    sarray->str_len = strlen(string);
    array = sarray->array = g_malloc(sizeof(gchar *) * sarray->str_len);
    memset(array, 0, sizeof(gchar *) * sarray->str_len);

    if (sarray->str_len == 0){
        return sarray;
    }

    // last bucket is the sentinel
    buckets = g_malloc(sizeof(bucket_t) * (128 + 1));
    memset(buckets, 0, sizeof(bucket_t) * (128 + 1));

    for(i = 0;i < sarray->str_len;i++){
        c = string[i]; c_ = string[i+1];
        if (c > c_) {
            buckets[(gint)c].type_a++;
        } else {
            buckets[(gint)c].type_b++;
        }
    }
    idx = 0;
    for(i = 0;i < 128 + 1;i++){
        buckets[i].idx = idx;
        idx += buckets[i].type_a + buckets[i].type_b;
    }
    for(i = 0;i < sarray->str_len;i++){
        c = string[i]; c_ = string[i+1];
        cp = &string[i];
        bucket = &buckets[(gint)c];
        if (c > c_){
            if (bucket->type_a == 1){
                array[bucket->idx] = (gchar *)cp;
            }
        } else {
            array[buckets[c + 1].idx - bucket->type_b] = (gchar *)cp;
            bucket->type_b--;
        }
    }
    for(i = 0;i < 128;i++){
        g_assert(buckets[i].type_b == 0);
        buckets[i].type_b = buckets[i+1].idx - buckets[i].idx - buckets[i].type_a;
        qsort(&array[buckets[i].idx + buckets[i].type_a],
              buckets[i].type_b, sizeof(gchar *), (compare_fun_t) strcmpr);
    }

    c = string[sarray->str_len - 1];
    cp = &string[sarray->str_len - 1];
    bucket = &buckets[(gint)c];
    g_assert(bucket->type_a > 0);
    idx = buckets[c + 1].idx - bucket->type_b - bucket->type_a;
    array[idx] = (gchar *)cp;
    bucket->type_a--;
    for(i = 0;i < sarray->str_len;i++){
        cp = array[i];
        if (cp == string || cp == NULL) {
            continue;
        } else if (*(cp - 1) > *cp){
            bucket = &buckets[(gint)*(cp - 1)];
            g_assert(bucket->type_a > 0);
            idx = buckets[*(cp - 1) + 1].idx - bucket->type_b - bucket->type_a;
            array[idx] = (gchar *)cp - 1;
            bucket->type_a--;
        }
    }

    g_free(buckets);
    return sarray;
}

void
sarray_delete(sarray_t *sarray)
{
    g_free(sarray->array);
    g_free(sarray);
}



gint
sarray_search(sarray_t *sarray, const gchar *key)
{
    gint upper;
    gint lower;
    gint next;
    gchar **array;
    gint key_len;

    key_len = strlen(key);
    array = sarray->array;

    upper = sarray->str_len - 1;
    lower = 0;

    while(TRUE){
        next = (upper - lower) / 2 + lower;
        if (next == lower){
            if (strncmp(array[lower], key, key_len) == 0){
                return (gint)(array[lower] - sarray->str);
            } else if (strncmp(array[upper], key, key_len) == 0){
                return (gint)(array[upper] - sarray->str);
            } else {
                return -1;
            }
        }
        if (*key < *array[next]){
            upper = next;
        } else if (*key > *array[next]) {
            lower = next;
        } else {
            if (strncmp(key, array[next], key_len) <= 0){
                upper = next;
            } else {
                lower = next;
            }
        }
    }
    return -1;
}

gint
sarray_size(sarray_t *sarray)
{
    gint ret;
    ret = sizeof(sarray_t);
    ret += sizeof(gchar *) * sarray->str_len;

    return ret;
}


