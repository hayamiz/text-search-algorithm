
#include "text-search.h"

gint
poor_search(const gchar *str, const gchar *key)
{
    gint i, j;
    gint str_len, key_len;

    str_len = strlen(str);
    key_len = strlen(key);

    for(i = 0;i <= str_len - key_len;i++) {
        for(j = 0;j < key_len;j++) {
            if (str[i + j] != key[j]) {
                goto next;
            }
        }
        return i;
    next:
        continue;
    }
    
    return -1;
}
