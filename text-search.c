
#include "text-search.h"

static sauto_tran_t *
sauto_get_tran(sauto_state_t *parent, gchar label)
{
    gint idx;

    if (parent == NULL) return NULL;
    
    for(idx = 0;idx < parent->num_children;idx++){
        if (parent->children[idx].label == label) {
            return &parent->children[idx];
        }
    }

    return NULL;
}

static void
sauto_tran_add(sauto_state_t *from,
               sauto_state_t *to,
               gchar label, gboolean solid)
{
    gint idx;
    idx = from->num_children ++;
    from->children = g_realloc(from->children,
                               from->num_children * sizeof(sauto_tran_t));
    from->children[idx].solid = solid;
    from->children[idx].label = label;
    from->children[idx].state = to;
}

static sauto_state_t *
sauto_state_new(sauto_t *sauto)
{
    sauto_state_t *newstate;
    gint64 idx;

    newstate = g_malloc(sizeof(sauto_state_t));
    newstate->suffix_link = NULL;
    newstate->num_children = 0;
    newstate->children = NULL;

    idx = sauto->num_states ++;
    newstate->id = sauto->num_states;
    sauto->states = g_realloc(sauto->states,
                              sizeof(sauto_state_t *) * sauto->num_states);
    sauto->states[idx] = newstate;
    
    return newstate;
}

sauto_t *
sauto_new(const gchar *string)
{
    sauto_t *sauto;
    sauto_state_t *root;
    sauto_state_t *sink;
    sauto_state_t *newsink;
    sauto_state_t *newnode;
    sauto_state_t *w;
    sauto_state_t *v;
    sauto_tran_t  *v_edge;
    sauto_tran_t  *u;
    gint64 len;
    gint64 i;
    gint   j;
    gchar c;

    sauto = g_malloc(sizeof(sauto_t));
    sauto->num_states = 0;
    sauto->states = NULL;

    root = sauto_state_new(sauto);
    root->suffix_link = NULL;
    root->num_children = 0;
    root->children = NULL;

    len = strlen(string);
    
    sink = root;
    for(i = 0;i < len;i++){
        c = string[i];
        newsink = sauto_state_new(sauto);
        sauto_tran_add(sink, newsink, c, TRUE);
        w = sink->suffix_link;
        while (w != NULL && sauto_get_tran(w, c) == NULL) {
            sauto_tran_add(w, newsink, c, FALSE); // make shortcut
            w = w->suffix_link;
        }
        if (w == NULL) {
            newsink->suffix_link = root;
        } else {
            v_edge = sauto_get_tran(w, c);
            v = v_edge->state;
            
            if (v_edge->solid == TRUE) {
                newsink->suffix_link = v;
            } else {
                newnode = sauto_state_new(sauto);
                newnode->num_children = v->num_children;
                newnode->children = g_malloc(sizeof(sauto_tran_t) *
                                             newnode->num_children);
                for(j = 0;j < newnode->num_children;j++){
                    newnode->children[j] = v->children[j];
                    newnode->children[j].solid = FALSE;
                }
                v_edge->state = newnode;
                v_edge->solid = TRUE;
                newsink->suffix_link = newnode;
                newnode->suffix_link = v->suffix_link;
                v->suffix_link = newnode;
                w = w->suffix_link;
                while (w != NULL) {
                    u = NULL;
                    for(j = 0;j < w->num_children;j++){
                        if (w->children[j].state == v) {
                            u = &w->children[j];
                            break;
                        }
                    }
                    if (u == NULL || u->solid == FALSE) {
                        break;
                    }
                    u->state = newnode;
                    w = w->suffix_link;
                }
            }
        }
        sink = newsink;
    }

    return sauto;
}

void
sauto_delete(sauto_t *sauto)
{
    gint64 i;
    sauto_state_t *state;

    for(i = 0;i < sauto->num_states;i++){
        state = sauto->states[i];
        g_free(state->children);
        g_free(state);
    }

    g_free(sauto->states);
    g_free(sauto);

    return;
}

gboolean
sauto_find(sauto_t *sauto, const gchar *key)
{

    return FALSE;
}

gint
main(gint argc, gchar **argv)
{
    sauto_t *sauto;
    sauto_state_t *root;
    sauto_state_t *state;
    gint64 i;
    gint   j;

    sauto = sauto_new(argv[1]);
    root = sauto->states[0];
    g_print("digraph suffix_automaton {\n");
    g_print("graph [rankdir = LR];");
    for(i = 0;i < sauto->num_states;i++){
        state = sauto->states[i];
        for(j = 0;j < state->num_children;j++){
            if (state->children[j].solid == TRUE) {
                g_print("%lld -> %lld [label = \"%c\", style = bold, weight = 5]\n",
                        state->id, state->children[j].state->id,
                        state->children[j].label);
            } else {
                g_print("%lld -> %lld [label = \"%c\", weight = 1]\n",
                        state->id, state->children[j].state->id,
                        state->children[j].label);
            }
        }
    }
    g_print("}\n");
    sauto_delete(sauto);

    return 0;
}
