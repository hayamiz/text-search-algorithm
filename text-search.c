
#include "text-search.h"

gint
main(gint argc, gchar **argv)
{
    sauto_t *sauto;

    sauto = sauto_new(argv[1]);
    sauto_graphviz(stdout, sauto);
    sauto_delete(sauto);

    return 0;
}
