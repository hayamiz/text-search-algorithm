#ifndef TEXT_SEARCH_H
#define TEXT_SEARCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <glib.h>

#include "SFMT/SFMT.h"

typedef struct option_rec {
    gint trial_num;
    gint keystr_len;
    gint searchstr_len;
    gint seed;
    gboolean existing_key;
    gboolean verbose;
    gboolean debug;
} option_t;


struct suffix_automaton_transition_rec;

typedef struct suffix_automaton_state_rec {
    gint id;
    gint end_pos; // end position of first occurrence.
    struct suffix_automaton_state_rec *suffix_link;
    gint num_children;
    struct suffix_automaton_transition_rec *children;
} sauto_state_t;

typedef struct suffix_automaton_transition_rec {
    gchar label;
    gboolean solid;
    sauto_state_t *state;
} sauto_tran_t;

typedef struct suffix_automaton_rec {
    gint num_states;
    sauto_state_t **states; // 0th state is the root
} sauto_t;

typedef struct suffix_array_rec {
    const gchar *str;
    gint str_len;
    gchar **array;
} sarray_t;

gint poor_search(const gchar *str, const gchar *key);

sauto_t *sauto_new(const gchar *string);
void sauto_delete(sauto_t *sauto);
gint sauto_search(sauto_t *sauto, const gchar *key);
void sauto_graphviz(FILE *out, sauto_t *sauto);

sarray_t *sarray_new(const gchar *string);
void sarray_delete(sarray_t *sarray);
gint sarray_search(sarray_t *sarray, const gchar *key);

#endif
