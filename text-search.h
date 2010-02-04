#ifndef TEXT_SEARCH_H
#define TEXT_SEARCH_H

#include <string.h>
#include <glib.h>

struct suffix_automaton_transition_rec;

typedef struct suffix_automaton_state_rec {
    gint64 id;
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
    gint64 num_states;
    sauto_state_t **states; // 0th state is the root
} sauto_t;


sauto_t *sauto_new(const gchar *string);
void sauto_delete(sauto_t *sauto);
gboolean sauto_find(sauto_t *sauto, const gchar *key);


#endif
