
#include "text-search.h"

#define TIME(stmt) (                            \
        {                                       \
            GTimer *_timer;                     \
            gdouble _t;                         \
            _timer = g_timer_new();             \
            g_timer_start(_timer);              \
            (stmt);                             \
            g_timer_stop(_timer);               \
            _t = g_timer_elapsed(_timer, NULL); \
            g_timer_destroy(_timer);            \
            _t;                                 \
        })

static void print_help(void);

static guint32 *randnums;
static gint randnums_size;

const gchar *
generate_string(gint length)
{
    gchar *strbuf;
    gint i;

    if (length < get_min_array_size32()) {
        for(i = 0;i < length;i++){
            randnums[i] = rand();
        }
    } else {
        fill_array32(randnums, randnums_size);
    }

    strbuf = g_malloc(sizeof(gchar) * (length + 1));
    for(i = 0;i < length;i++){
        strbuf[i] = 'a' + randnums[i] % ('z' - 'a' + 1);
    }
    strbuf[length] = '\0';

    return strbuf;
}

const gchar *
generate_key(const gchar *str, gint length)
{
    gint str_len;
    gint idx;
    gchar *ret;

    if (str == NULL) {
        return generate_string(length);
    }

    str_len = strlen(str);
    idx = rand() % (str_len - length + 1);
    ret = g_malloc(sizeof(gchar) * (length + 1));
    memcpy((gpointer) ret, (gpointer) str, sizeof(gchar) * length);
    ret[length] = '\0';

    return ret;
}

void
parse_args(int argc, char **argv, option_t *option)
{
    // default values
    option->trial_num = 10;
    option->keystr_len = 8;
    option->searchstr_len = 1024;
    option->seed = time(NULL);
    option->existing_key = FALSE;
    option->verbose = FALSE;
    option->debug = FALSE;

    char c;
    while((c = getopt(argc, argv, "ht:m:n:evs:")) != -1) {
        switch(c) {
        case 't':
            option->trial_num = atoi(optarg);
            break;
        case 's':
            option->seed = atoi(optarg);
            break;
        case 'm':
            option->keystr_len = atoi(optarg);
            break;
        case 'n':
            option->searchstr_len = atoi(optarg);
            break;
        case 'e':
            option->existing_key = TRUE;
            break;
        case 'v':
            option->verbose = TRUE;
            break;
        case 'd':
            option->debug = TRUE;
            break;
        case 'h':
            goto argument_error;
            break;
        default:
            g_printerr("Unknown option -%c\n\n", c);
            goto argument_error;
        }
    }
    
    return;
argument_error:
    
    print_help();
    exit(1);
}

void
print_help(void)
{
    g_print("Usage: text-search [options]\n\
\n\
Options:\n\
	-m NUM		Length of key string (default: 8)\n\
	-n NUM		Length of search target string (default: 1024)\n\
	-e		Constraint: search for existing key\n\
	-t		The number of trial runs for each algorithm (deafult: 10)\n\
	-s		Seed of random number generator\n\
	-v		Verbose (show labels for each column of results)\n\
	-d		Debug print\n\
"
        );
}

static gdouble
average(gdouble *data, gint length)
{
    gint i;
    gdouble sum;
    sum = 0.0;
    for(i = 0;i < length;i++){
        sum += data[i];
    }

    return sum / length;
}

static gdouble
error(gdouble *data, gint length)
{
    gint i;
    gdouble avg;
    gdouble var;
    gdouble stddevi;

    avg = average(data, length);
    var = 0.0;
    for(i = 0;i < length;i++){
        var += (data[i] - avg) * (data[i] - avg);
    }
    var /= length;
    stddevi = sqrt(var);

    return stddevi / sqrt((gdouble) length);
}

gint
main(gint argc, gchar **argv)
{
    option_t option;
    const gchar *str;
    const gchar *key;
    gint i;

    gdouble *poor_search_ret;
    gdouble *sauto_index_ret;
    gdouble *sauto_search_ret;

    sauto_t *sauto;

    gint poor_pos;
    gint sauto_pos;

    parse_args(argc, argv, &option);
    init_gen_rand(option.seed);
    srand(option.seed);

    if (option.verbose == TRUE) {
        g_print("# "
                "searchstr_len\t"
                "keystr_len\t"
                "poor_search_avg(sec)\t"
                "poor_search_err(sec)\t"
                "sauto_index_avg(sec)\t"
                "sauto_index_err(sec)\t"
                "sauto_search_avg(sec)\t"
                "sauto_search_err(sec)\t"
                "\n");
    }

    randnums_size = option.searchstr_len;
    if (option.keystr_len > randnums_size) {
        return 0;
    }
    randnums_size += (4 - randnums_size % 4);
    g_assert(randnums_size >= option.searchstr_len);
    if (0 != posix_memalign((gpointer *)&randnums,
                            16, sizeof(guint32) * randnums_size)) {
        perror("cannot allocate memory");
        exit(errno);
    }
    
    poor_search_ret  = g_malloc(sizeof(gdouble) * option.trial_num);
    sauto_index_ret  = g_malloc(sizeof(gdouble) * option.trial_num);
    sauto_search_ret = g_malloc(sizeof(gdouble) * option.trial_num);

    for(i = 0;i < option.trial_num;i++) {
        str = generate_string(option.searchstr_len);
        key = generate_key((option.existing_key == TRUE?
                            str : NULL),
                           option.keystr_len);
        if (option.debug == TRUE) {
            if (option.searchstr_len > 10) {
                g_printerr("str: %.10s...\n", str);
            } else {
                g_printerr("str: %s\n", str);
            }
            if (option.keystr_len > 10) {
                g_printerr("key: %.10s...\n", key);
            } else {
                g_printerr("key: %s\n", key);
            }
        }
        sauto_index_ret[i]  = TIME(sauto = sauto_new(str));
        sauto_search_ret[i] = TIME(sauto_pos = sauto_search(sauto, key));
        poor_search_ret[i]  = TIME(poor_pos = poor_search(str, key));

        g_assert(poor_pos == sauto_pos);
        g_assert(option.existing_key == FALSE || poor_pos >= 0);
        
        sauto_delete(sauto);
        g_free((gpointer) str);
        g_free((gpointer) key);
    }
    
    g_print("%d\t%d\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t"
            "\n",
            option.searchstr_len, option.keystr_len,
            average(poor_search_ret, option.trial_num),
            error  (poor_search_ret, option.trial_num),
            average(sauto_index_ret, option.trial_num),
            error  (sauto_index_ret, option.trial_num),
            average(sauto_search_ret, option.trial_num),
            error  (sauto_search_ret, option.trial_num)
        );

    g_free(poor_search_ret);
    g_free(sauto_index_ret);
    g_free(sauto_search_ret);
    free(randnums);

    return 0;
}
