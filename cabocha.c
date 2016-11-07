#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#include "cabocha.h"
#include "php_cabocha.h"

#define PHP_CABOCHA_RES_NAME "cabocha"

static int le_cabocha;

static void php_cabocha_dtor(zend_resource *res)
{
    cabocha_t *cabocha = (cabocha_t *) res->ptr;
    if (cabocha) {
        cabocha_destroy(cabocha);
    }
}

static void zval_tree(cabocha_tree_t *tree, zval *zv);
static void zval_tree_chunks(cabocha_tree_t *tree, zval *zv);
static void zval_tree_tokens(cabocha_tree_t *tree, zval *chunks, zval *zv);
static void zval_chunk(cabocha_chunk_t *chunk, zval *zv);
static void zval_token(cabocha_token_t *token, zval *chunk, zval *zv);
static void zval_feature_list(char **feature_list, size_t feature_list_size, zval *zv);
static zval **build_chunk_entries(zval *chunks, cabocha_tree_t *tree);
static cabocha_t *fetch_cabocha(zval *res);

static void tree_zval(zval *zv, cabocha_tree_t *tree);
static void chunk_zval(zval *zv, cabocha_chunk_t *chunk);
static void token_zval(zval *zv, cabocha_tree_t *tree, cabocha_token_t *token, size_t *ci);

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_cabocha_new, 0, 0, 0)
    ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cabocha_destroy, 0, 0, 1)
    ZEND_ARG_INFO(0, cabocha)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cabocha_parse, 0, 0, 2)
    ZEND_ARG_INFO(0, cabocha)
    ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cabocha_parse_tostr, 0, 0, 2)
    ZEND_ARG_INFO(0, cabocha)
    ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cabocha_parse_sentence, 0, 0, 1)
    ZEND_ARG_INFO(0, arg)
    ZEND_ARG_INFO(0, opt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cabocha_parse_sentence_tostr, 0, 0, 1)
    ZEND_ARG_INFO(0, arg)
    ZEND_ARG_INFO(0, opt)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cabocha_tree_read, 0, 0, 2)
    ZEND_ARG_INFO(0, arg)
    ZEND_ARG_INFO(0, input_layer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cabocha_tree_tostr, 0, 0, 2)
    ZEND_ARG_INFO(0, tree)
    ZEND_ARG_INFO(0, format)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ proto cabocha cabocha_new(string arg)
 */
PHP_FUNCTION(cabocha_new)
{
    char *arg = NULL;
    size_t arg_len;

    cabocha_t *cabocha;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &arg, &arg_len) == FAILURE) {
        RETURN_FALSE;
    }

    cabocha = arg ? cabocha_new2(arg) : cabocha_new(0, NULL);

    RETURN_RES(zend_register_resource(cabocha, le_cabocha));
}
/* }}} */

/* {{{ proto void cabocha_destroy(cabocha cabocha)
 */
PHP_FUNCTION(cabocha_destroy)
{
    zval *res;
    cabocha_t *cabocha;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res) == FAILURE) {
        return;
    }

    if ((cabocha = fetch_cabocha(res)) == NULL) {
        return;
    }

    zend_list_close(Z_RES_P(res));
}
/* }}} */

/* {{{ proto array cabocha_parse(cabocha cabocha, string arg)
 */
PHP_FUNCTION(cabocha_parse)
{
    zval *res;
    cabocha_t *cabocha;

    char *arg = NULL;
    size_t arg_len;

    cabocha_tree_t *tree;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &res, &arg, &arg_len) == FAILURE) {
        RETURN_FALSE;
    }

    if ((cabocha = fetch_cabocha(res)) == NULL) {
        return;
    }

    tree = cabocha_sparse_totree(cabocha, arg);
    zval_tree(tree, return_value);
}
/* }}} */

/* {{{ proto string cabocha_parse_tostr(cabocha cabocha, string arg)
 */
PHP_FUNCTION(cabocha_parse_tostr)
{
    zval *res;
    cabocha_t *cabocha;

    char *arg = NULL;
    size_t arg_len;

    char *str = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &res, &arg, &arg_len) == FAILURE) {
        RETURN_FALSE;
    }

    if ((cabocha = fetch_cabocha(res)) == NULL) {
        return;
    }

    str = cabocha_sparse_tostr(cabocha, arg);
    RETURN_STRING(str);
}
/* }}} */

/* {{{ proto array cabocha_parse_sentence(string arg, string opt = null)
 */
PHP_FUNCTION(cabocha_parse_sentence)
{
    char *arg = NULL;
    size_t arg_len;

    char *opt = NULL;
    size_t opt_len;

    cabocha_t *cabocha;
    cabocha_tree_t *tree;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|s", &arg, &arg_len, &opt, &opt_len) == FAILURE) {
        RETURN_FALSE;
    }

    cabocha = opt ? cabocha_new2(opt) : cabocha_new(0, NULL);

    tree = cabocha_sparse_totree(cabocha, arg);
    zval_tree(tree, return_value);

    cabocha_destroy(cabocha);
}
/* }}} */

/* {{{ proto string cabocha_parse_sentence_tostr(string arg, string opt = null)
 */
PHP_FUNCTION(cabocha_parse_sentence_tostr)
{
    char *arg = NULL;
    size_t arg_len;

    char *opt = NULL;
    size_t opt_len;

    cabocha_t *cabocha;
    char *str = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|s", &arg, &arg_len, &opt, &opt_len) == FAILURE) {
        RETURN_FALSE;
    }

    cabocha = opt ? cabocha_new2(opt) : cabocha_new(0, NULL);

    str = cabocha_sparse_tostr(cabocha, arg);
    RETVAL_STRING(str);

    cabocha_destroy(cabocha);
}
/* }}} */

/* {{{ proto array cabocha_tree_read(string arg, int input_layer)
 */
PHP_FUNCTION(cabocha_tree_read)
{
    char *arg = NULL;
    size_t arg_len;
    zend_long input_layer;

    cabocha_tree_t *tree;
    int res;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sl", &arg, &arg_len, &input_layer) == FAILURE) {
        RETURN_FALSE;
    }

    tree = cabocha_tree_new();
    res = cabocha_tree_read(tree, arg, arg_len, input_layer);
    if (res) {
        zval_tree(tree, return_value);
    } else {
        RETVAL_FALSE;
    }

    cabocha_tree_destroy(tree);
}
/* }}} */

/* {{{ proto string cabocha_tree_tostr(array tree, int format)
 */
PHP_FUNCTION(cabocha_tree_tostr)
{
    zval *arr;
    zend_long format;

    cabocha_tree_t *tree;
    char *str = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "al", &arr, &format) == FAILURE) {
        RETURN_FALSE;
    }

    tree = cabocha_tree_new();
    tree_zval(arr, tree);

    str = cabocha_tree_tostr(tree, format);
    RETVAL_STRING(str);

    cabocha_tree_destroy(tree);
}
/* }}} */

static cabocha_t *fetch_cabocha(zval *res)
{
    return (cabocha_t *) zend_fetch_resource(
        Z_RES_P(res),
        PHP_CABOCHA_RES_NAME,
        le_cabocha
    );
}

static void zval_tree(cabocha_tree_t *tree, zval *zv)
{
    zval chunks;
    zval tokens;

    array_init(zv);

    char *sentence = cabocha_tree_sentence(tree);
    add_assoc_string(zv, "sentence", sentence);

    zval_tree_chunks(tree, &chunks);
    add_assoc_zval(zv, "chunk", &chunks);

    zval_tree_tokens(tree, &chunks, &tokens);
    add_assoc_zval(zv, "token", &tokens);

    int charset = cabocha_tree_charset(tree);
    add_assoc_long(zv, "charset", charset);

    int posset = cabocha_tree_posset(tree);
    add_assoc_long(zv, "posset", posset);

    int output_layer = cabocha_tree_output_layer(tree);
    add_assoc_long(zv, "output_layer", output_layer);
}

static void zval_tree_chunks(cabocha_tree_t *tree, zval *zv)
{
    size_t sz;
    int i;
    zval c;

    sz = cabocha_tree_chunk_size(tree);
    array_init_size(zv, sz);
    zend_hash_real_init(Z_ARRVAL_P(zv), 1);
    ZEND_HASH_FILL_PACKED(Z_ARRVAL_P(zv)) {
        for (i = 0; i < sz; ++i) {
            zval_chunk(cabocha_tree_chunk(tree, i), &c);
            ZEND_HASH_FILL_ADD(&c);
        }
    } ZEND_HASH_FILL_END();
}

static void zval_tree_tokens(cabocha_tree_t *tree, zval *chunks, zval *zv)
{
    size_t sz;
    zval **entries;
    int i;
    int j;
    zval t;
    cabocha_token_t *token;

    sz = cabocha_tree_token_size(tree);
    entries = build_chunk_entries(chunks, tree);
    j = 0;

    array_init_size(zv, sz);
    zend_hash_real_init(Z_ARRVAL_P(zv), 1);
    ZEND_HASH_FILL_PACKED(Z_ARRVAL_P(zv)) {
        for (i = 0; i < sz; ++i) {
            token = cabocha_tree_token(tree, i);
            zval_token(token, token->chunk ? entries[j++] : NULL, &t);
            ZEND_HASH_FILL_ADD(&t);
        }
    } ZEND_HASH_FILL_END();

    efree(entries);
}

static zval **build_chunk_entries(zval *chunks, cabocha_tree_t *tree)
{
    size_t sz;
    zval **entries;
    int i;
    zval *c;

    sz = cabocha_tree_chunk_size(tree);
    entries = ecalloc(sz, sizeof(zval *));
    ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(chunks), i, c) {
        entries[i] = c;
    } ZEND_HASH_FOREACH_END();

    return entries;
}

#define ADD_ASSOC_STRING(zval, key, val) \
    if ((val) == NULL) { \
        add_assoc_null(zval, key); \
    } else { \
        add_assoc_string(zval, key, val); \
    }

static void zval_chunk(cabocha_chunk_t *chunk, zval *zv)
{
    array_init(zv);

    add_assoc_long(zv, "link", chunk->link);
    add_assoc_long(zv, "head_pos", chunk->head_pos);
    add_assoc_long(zv, "func_pos", chunk->func_pos);
    add_assoc_long(zv, "token_size", chunk->token_size);
    add_assoc_long(zv, "token_pos", chunk->token_pos);
    add_assoc_double(zv, "score", chunk->score);
    ADD_ASSOC_STRING(zv, "additional_info", chunk->additional_info);
    add_assoc_long(zv, "feature_list_size", chunk->feature_list_size);

    zval_feature_list(chunk->feature_list, chunk->feature_list_size, zv);
}

static void zval_token(cabocha_token_t *token, zval *chunk, zval *zv)
{
    array_init(zv);

    ADD_ASSOC_STRING(zv, "surface", token->surface);
    ADD_ASSOC_STRING(zv, "normalized_surface", token->normalized_surface);
    ADD_ASSOC_STRING(zv, "feature", token->feature);
    add_assoc_long(zv, "feature_list_size", token->feature_list_size);
    ADD_ASSOC_STRING(zv, "ne", token->ne);
    ADD_ASSOC_STRING(zv, "additional_info", token->additional_info);

    if (chunk) {
        add_assoc_zval(zv, "chunk", chunk);
        Z_ADDREF_P(chunk);
    } else {
        add_assoc_null(zv, "chunk");
    }

    zval_feature_list(token->feature_list, token->feature_list_size, zv);
}

static void zval_feature_list(char **feature_list, size_t feature_list_size, zval *zv)
{
    zval features;

    array_init_size(&features, feature_list_size);
    zend_hash_real_init(Z_ARRVAL(features), 1);
    ZEND_HASH_FILL_PACKED(Z_ARRVAL(features)) {
        int j;
        for (j = 0; j < feature_list_size; ++j) {
            zval f;
            char *feature = feature_list[j];
            ZVAL_STRING(&f, feature);
            ZEND_HASH_FILL_ADD(&f);
        }
    } ZEND_HASH_FILL_END();
    add_assoc_zval(zv, "feature_list", &features);
}

#define HASH_LONG(zv, key, lval) { \
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(zv), key, sizeof(key) - 1); \
    lval = Z_LVAL_P(tmp); \
}
#define HASH_DOUBLE(zv, key, dval) { \
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(zv), key, sizeof(key) - 1); \
    dval = Z_DVAL_P(tmp); \
}
#define HASH_STR(zv, key, str, len) { \
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(zv), key, sizeof(key) - 1); \
    str = Z_STRVAL_P(tmp); \
    len = Z_STRLEN_P(tmp); \
}
#define HASH_ARR(zv, key, val) { \
    zval *tmp = zend_hash_str_find(Z_ARRVAL_P(zv), key, sizeof(key) - 1); \
    val = Z_ARRVAL_P(tmp); \
}
#define HASH_ISNULL(zv, key) \
    Z_ISNULL_P(zend_hash_str_find(Z_ARRVAL_P(zv), key, sizeof(key) - 1))

static void tree_zval(zval *zv, cabocha_tree_t *tree)
{
    zend_long l;
    char *str;
    size_t str_len;
    zend_array *arr;
    zval *val;

    cabocha_chunk_t *chunk;
    cabocha_token_t *token;
    size_t ci = 0;

    HASH_STR(zv, "sentence", str, str_len);
    cabocha_tree_set_sentence(tree, str, str_len);

    HASH_ARR(zv, "chunk", arr);
    ZEND_HASH_FOREACH_VAL(arr, val) {
        chunk = cabocha_tree_add_chunk(tree);
        chunk_zval(val, chunk);
    } ZEND_HASH_FOREACH_END();

    HASH_ARR(zv, "token", arr);
    ZEND_HASH_FOREACH_VAL(arr, val) {
        token = cabocha_tree_add_token(tree);
        token_zval(val, tree, token, &ci);
    } ZEND_HASH_FOREACH_END();

    HASH_LONG(zv, "charset", l);
    cabocha_tree_set_charset(tree, l);

    HASH_LONG(zv, "posset", l);
    cabocha_tree_set_posset(tree, l);

    HASH_LONG(zv, "output_layer", l);
    cabocha_tree_set_output_layer(tree, l);
}

static void chunk_zval(zval *zv, cabocha_chunk_t *chunk)
{
    zend_long l;
    double d;
    char *str;
    size_t str_len;

    HASH_LONG(zv, "link", l);
    chunk->link = l;

    HASH_LONG(zv, "head_pos", l);
    chunk->head_pos = l;

    HASH_LONG(zv, "func_pos", l);
    chunk->func_pos = l;

    HASH_LONG(zv, "token_size", l);
    chunk->token_size = l;

    HASH_LONG(zv, "token_pos", l);
    chunk->token_pos = l;

    HASH_DOUBLE(zv, "score", d);
    chunk->score = d;

    if (!HASH_ISNULL(zv, "additional_info")) {
        HASH_STR(zv, "additional_info", str, str_len);
        chunk->additional_info = strndup(str, str_len);
    }

    HASH_LONG(zv, "feature_list_size", l);
    chunk->feature_list_size = l;
}

static void token_zval(zval *zv, cabocha_tree_t *tree, cabocha_token_t *token, size_t *ci)
{
    zend_long l;
    char *str;
    size_t str_len;

    if (!HASH_ISNULL(zv, "surface")) {
        HASH_STR(zv, "surface", str, str_len);
        token->surface = strndup(str, str_len);
    }

    if (!HASH_ISNULL(zv, "normalized_surface")) {
        HASH_STR(zv, "normalized_surface", str, str_len);
        token->normalized_surface = strndup(str, str_len);
    }

    if (!HASH_ISNULL(zv, "feature")) {
        HASH_STR(zv, "feature", str, str_len);
        token->feature = strndup(str, str_len);
    }

    HASH_LONG(zv, "feature_list_size", l);
    token->feature_list_size = l;

    if (!HASH_ISNULL(zv, "ne")) {
        HASH_STR(zv, "ne", str, str_len);
        token->ne = strndup(str, str_len);
    }

    if (!HASH_ISNULL(zv, "additional_info")) {
        HASH_STR(zv, "additional_info", str, str_len);
        token->additional_info = strndup(str, str_len);
    }

    if (!HASH_ISNULL(zv, "chunk")) {
        token->chunk = cabocha_tree_chunk(tree, *ci);
        *ci = *ci + 1;
    }
}

#undef HASH_LONG
#undef HASH_DOUBLE
#undef HASH_STR
#undef HASH_ZVAL
#undef HASH_ISNULL

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(cabocha)
{
    le_cabocha = zend_register_list_destructors_ex(
        php_cabocha_dtor,
        NULL,
        PHP_CABOCHA_RES_NAME,
        module_number
    );

#define PHP_CABOCHA_EXPORT_CONSTANT(name) \
    REGISTER_LONG_CONSTANT(#name, name, CONST_CS | CONST_PERSISTENT)

    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_EUC_JP);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_CP932);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_UTF8);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_ASCII);

    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_IPA);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_JUMAN);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_UNIDIC);

    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_FORMAT_TREE);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_FORMAT_LATTICE);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_FORMAT_TREE_LATTICE);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_FORMAT_XML);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_FORMAT_CONLL);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_FORMAT_NONE);

    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_INPUT_RAW_SENTENCE);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_INPUT_POS);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_INPUT_CHUNK);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_INPUT_SELECTION);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_INPUT_DEP);

    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_OUTPUT_RAW_SENTENCE);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_OUTPUT_POS);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_OUTPUT_CHUNK);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_OUTPUT_SELECTION);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_OUTPUT_DEP);

    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_TRAIN_NE);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_TRAIN_CHUNK);
    PHP_CABOCHA_EXPORT_CONSTANT(CABOCHA_TRAIN_DEP);

#undef PHP_CABOCHA_EXPORT_CONSTANT

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(cabocha)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(cabocha)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "CaboCha support", "enabled");
    php_info_print_table_end();
}
/* }}} */

/* {{{ cabocha_functions[]
 */
const zend_function_entry cabocha_functions[] = {

#define PHP_FE_WITH_ARG_INFO(name) PHP_FE(name, arginfo_##name)

    PHP_FE_WITH_ARG_INFO(cabocha_new)
    PHP_FE_WITH_ARG_INFO(cabocha_destroy)
    PHP_FE_WITH_ARG_INFO(cabocha_parse)
    PHP_FE_WITH_ARG_INFO(cabocha_parse_tostr)
    PHP_FE_WITH_ARG_INFO(cabocha_parse_sentence)
    PHP_FE_WITH_ARG_INFO(cabocha_parse_sentence_tostr)
    PHP_FE_WITH_ARG_INFO(cabocha_tree_read)
    PHP_FE_WITH_ARG_INFO(cabocha_tree_tostr)
    PHP_FE_END

#undef PHP_FE_WITH_ARG_INFO

};
/* }}} */

/* {{{ cabocha_module_entry
 */
zend_module_entry cabocha_module_entry = {
    STANDARD_MODULE_HEADER,
    "cabocha",
    cabocha_functions,
    PHP_MINIT(cabocha),
    PHP_MSHUTDOWN(cabocha),
    NULL,
    NULL,
    PHP_MINFO(cabocha),
    PHP_CABOCHA_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CABOCHA
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(cabocha)
#endif
