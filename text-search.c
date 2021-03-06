
#include "text-search.h"

#define TIME(stmt) (                            \
        {                                       \
            GTimer *_timer;                     \
            gdouble _t;                         \
            _timer = g_timer_new();             \
            g_timer_start(_timer);              \
            stmt;                               \
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
    memcpy((gpointer) ret, (gpointer) str + idx, sizeof(gchar) * length);
    ret[length] = '\0';

    return ret;
}

gint
parse_args(int argc, char **argv, option_t *option)
{
    // default values
    option->action = SEARCH;
    option->trial_num = 10;
    option->search_num = 1000;
    option->keystr_len = 32;
    option->searchstr_len = 1024;
    option->seed = time(NULL);
    option->existing_key = FALSE;
    option->verbose = FALSE;
    option->debug = FALSE;

    char c;
    while((c = getopt(argc, argv, "a:ht:T:m:n:evs:")) != -1) {
        switch(c) {
        case 'a':
            if (strcmp(optarg, "check-size") == 0){
                option->action = CHECK_SIZE;
            } else if (strcmp(optarg, "check-tran") == 0){
                option->action = CHECK_TRAN;
            } else if (strcmp(optarg, "check-tran-hist") == 0){
                option->action = CHECK_TRAN_HIST;
            } else if (strcmp(optarg, "graphviz") == 0){
                option->action = GRAPHVIZ;
            }
            break;
        case 't':
            option->trial_num = atoi(optarg);
            break;
        case 'T':
            option->search_num = atoi(optarg);
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
    
    return optind;
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
	-a ACTION	Action: search(default), check-size, check-tran, check-tran-hist, graphviz\n\
	-m NUM		Length of key string (default: 32)\n\
	-n NUM		Length of search target string (default: 1024)\n\
	-e		Constraint: search for existing key\n\
	-t		The number of trial runs for each algorithm (deafult: 10)\n\
	-T		The number of searches for each trial (deafult: 1000)\n\
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

void
action_search(option_t *option)
{
    const gchar *str;
    gint i;
    gint j;

    gdouble *poor_search_ret;
    gdouble *sauto_index_ret;
    gdouble *sauto_search_ret;
    gdouble *sarray_index_ret;
    gdouble *sarray_search_ret;

    sauto_t *sauto;
    sarray_t *sarray;

    const gchar **keys;
    gint         *poor_pos;
    gint         *sauto_pos;
    gint         *sarray_pos;

    if (option->verbose == TRUE) {
        g_print("# "
                "searchstr_len\t"
                "keystr_len\t"
                "poor_search_avg(sec)\t"
                "poor_search_err(sec)\t"
                "sauto_index_avg(sec)\t"
                "sauto_index_err(sec)\t"
                "sauto_search_avg(sec)\t"
                "sauto_search_err(sec)\t"
                "sarray_index_avg(sec)\t"
                "sarray_index_err(sec)\t"
                "sarray_search_avg(sec)\t"
                "sarray_search_err(sec)\t"
                "\n");
    }

    if (option->searchstr_len == 0) {
        return;
    }

    poor_search_ret  = g_malloc(sizeof(gdouble) * option->trial_num);
    sauto_index_ret  = g_malloc(sizeof(gdouble) * option->trial_num);
    sauto_search_ret = g_malloc(sizeof(gdouble) * option->trial_num);
    sarray_index_ret  = g_malloc(sizeof(gdouble) * option->trial_num);
    sarray_search_ret = g_malloc(sizeof(gdouble) * option->trial_num);

    keys = g_malloc(sizeof(gchar *) * option->search_num);
    poor_pos = g_malloc(sizeof(gint) * option->search_num);
    sauto_pos = g_malloc(sizeof(gint) * option->search_num);
    sarray_pos = g_malloc(sizeof(gint) * option->search_num);

    for(i = 0;i < option->trial_num;i++) {
        str = generate_string(option->searchstr_len);
        for(j = 0;j < option->search_num;j++){
            keys[j] = generate_key((option->existing_key == TRUE?
                                    str : NULL),
                                   option->keystr_len);
        }
        if (option->debug == TRUE) {
            if (option->searchstr_len > 10) {
                g_printerr("str: %.10s...\n", str);
            } else {
                g_printerr("str: %s\n", str);
            }
        }
        poor_search_ret[i] = TIME(
            for(j = 0;j < option->search_num;j++){
                poor_pos[j] = poor_search(str, keys[j]);
            });
        poor_search_ret[i] /= option->search_num;
        
        sauto_index_ret[i]  = TIME(sauto = sauto_new(str));
        sauto_search_ret[i] = TIME(
            for(j = 0;j < option->search_num;j++){
                sauto_pos[j] = sauto_search(sauto, keys[j]);
            });
        sauto_search_ret[i] /= option->search_num;

        sarray_index_ret[i]  = TIME(sarray = sarray_new(str));
        sarray_search_ret[i] = TIME(
            for(j = 0;j < option->search_num;j++){
                sarray_pos[j] = sarray_search(sarray, keys[j]);
            });
        sarray_search_ret[i] /= option->search_num;


        sauto_delete(sauto);
        sarray_delete(sarray);
        g_free((gpointer) str);
        for(j = 0;j < option->search_num;j++){
            g_free((gpointer) keys[j]);
            if (!(option->existing_key == FALSE || poor_pos >= 0)){
                g_printerr("Search failed\n");
                abort();
            }
            if (!(poor_pos[j] == sauto_pos[j] || poor_pos[j] == sarray_pos[j])){
                g_printerr("Result mismatch\n");
                abort();
            }
        }
    }
    
    g_print("%d\t%d\t%le\t%le\t%le\t%le\t%le\t%le\t%le\t%le\t%le\t%le\t"
            "\n",
            option->searchstr_len, option->keystr_len,
            average(poor_search_ret, option->trial_num),
            error  (poor_search_ret, option->trial_num),
            average(sauto_index_ret, option->trial_num),
            error  (sauto_index_ret, option->trial_num),
            average(sauto_search_ret, option->trial_num),
            error  (sauto_search_ret, option->trial_num),
            average(sarray_index_ret, option->trial_num),
            error  (sarray_index_ret, option->trial_num),
            average(sarray_search_ret, option->trial_num),
            error  (sarray_search_ret, option->trial_num)
        );

    g_free(poor_search_ret);
    g_free(sauto_index_ret);
    g_free(sauto_search_ret);
    g_free(sarray_index_ret);
    g_free(sarray_search_ret);
}

void
action_check_size(option_t *option)
{
    sauto_t *sauto;
    sarray_t *sarray;
    gint i;

    gdouble *sauto_size_ret;
    gdouble *sarray_size_ret;

    const gchar *str;

    sauto_size_ret = g_malloc(sizeof(gdouble) * option->trial_num);
    sarray_size_ret = g_malloc(sizeof(gdouble) * option->trial_num);
    
    if (option->verbose == TRUE) {
        g_print("# "
                "searchstr_len\t"
                "sauto_size(byte)\t"
                "sarray_size(byte)\t"
                "sauto_size_ratio\t"
                "sarray_size_ratio\t"
                "\n");
    }

    for(i = 0;i < option->trial_num;i++) {
        str = generate_string(option->searchstr_len);
        sauto = sauto_new(str);
        sarray = sarray_new(str);

        sauto_size_ret[i] = sauto_size(sauto);
        sarray_size_ret[i] = sarray_size(sarray);

        sauto_delete(sauto);
        sarray_delete(sarray);
    }

    g_print("%d\t%le\t%le\t%le\t%le\n",
            option->searchstr_len,
            average(sauto_size_ret, option->trial_num),
            average(sarray_size_ret, option->trial_num),
            average(sauto_size_ret, option->trial_num)
            / (sizeof(gchar) * option->searchstr_len),
            average(sarray_size_ret, option->trial_num)
            / (sizeof(gchar) * option->searchstr_len));

    g_free(sauto_size_ret);
    g_free(sarray_size_ret);
}

void
action_check_tran(option_t *option)
{
    sauto_t *sauto;
    gint i;

    gdouble *sauto_tran_ret;

    const gchar *str;

    sauto_tran_ret = g_malloc(sizeof(gdouble) * option->trial_num);
    
    if (option->verbose == TRUE) {
        g_print("# "
                "searchstr_len\t"
                "sauto_avg_tran\t"
                "\n");
    }

    for(i = 0;i < option->trial_num;i++) {
        str = generate_string(option->searchstr_len);
        sauto = sauto_new(str);
        sauto_tran_ret[i] = sauto_avg_tran(sauto);
        sauto_delete(sauto);
    }

    g_print("%d\t%le\n",
            option->searchstr_len,
            average(sauto_tran_ret, option->trial_num));

    g_free(sauto_tran_ret);
}

void
action_check_tran_hist(option_t *option)
{
    sauto_t *sauto;
    gint i;
    gint *sauto10_hist;
    gint *sauto20_hist;
    gint *sauto22_hist;

    gint sauto10_total;
    gint sauto20_total;
    gint sauto22_total;

    const gchar *str;

    free(randnums);
    randnums_size = 1 << 22;
    if (0 != posix_memalign((gpointer *)&randnums,
                            16, sizeof(guint32) * randnums_size)) {
        perror("cannot allocate memory");
        exit(errno);
    }

    if (option->verbose == TRUE) {
        g_print("# tran_num\tcount(N=2**10)\tcount(N=2**20)\tcount(N=2**22)"
                "\tfreq(N=2**10)\tfreq(N=2**20)\tfreq(N=2**22)"
                "\n");
    }

    str = generate_string(1 << 10);
    sauto = sauto_new(str);
    sauto10_total = sauto->num_states;
    sauto10_hist = sauto_tran_hist(sauto);
    sauto_delete(sauto);
    
    str = generate_string(1 << 20);
    sauto = sauto_new(str);
    sauto20_total = sauto->num_states;
    sauto20_hist = sauto_tran_hist(sauto);
    sauto_delete(sauto);
    
    str = generate_string(1 << 22);
    sauto = sauto_new(str);
    sauto22_total = sauto->num_states;
    sauto22_hist = sauto_tran_hist(sauto);
    sauto_delete(sauto);

    for(i = 1;i < 129;i++){
        g_print("%d\t%d\t%d\t%d\t%le\t%le\t%le\n",
                i, sauto10_hist[i], sauto20_hist[i], sauto22_hist[i],
                (gdouble)sauto10_hist[i] / sauto10_total,
                (gdouble)sauto20_hist[i] / sauto20_total,
                (gdouble)sauto22_hist[i] / sauto22_total);
    }

    g_free(sauto10_hist);
    g_free(sauto20_hist);
    g_free(sauto22_hist);
}

void
action_graphviz(option_t *option, gint idx, gchar **argv)
{
    sauto_t *sauto;
    sauto = sauto_new(argv[idx]);
    sauto_graphviz(stdout, sauto);
    sauto_delete(sauto);
}

gint
main(gint argc, gchar **argv)
{
    option_t option;
    gint argv_idx;

    argv_idx = parse_args(argc, argv, &option);
    init_gen_rand(option.seed);
    srand(option.seed);

    randnums_size = option.searchstr_len;
    if (option.keystr_len > randnums_size) {
        randnums_size = option.keystr_len;
    }
    randnums_size += (4 - randnums_size % 4);
    g_assert(randnums_size >= option.searchstr_len);
    if (0 != posix_memalign((gpointer *)&randnums,
                            16, sizeof(guint32) * randnums_size)) {
        perror("cannot allocate memory");
        exit(errno);
    }

    switch(option.action){
    case SEARCH:
        action_search(&option);
        break;
    case CHECK_SIZE:
        action_check_size(&option);
        break;
    case CHECK_TRAN:
        action_check_tran(&option);
        break;
    case CHECK_TRAN_HIST:
        action_check_tran_hist(&option);
        break;
    case GRAPHVIZ:
        action_graphviz(&option, argv_idx, argv);
        break;
    }

    free(randnums);

    return 0;
}
