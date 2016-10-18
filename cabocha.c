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
static cabocha_t *fetch_cabocha(zval *zv);

/* {{{ proto cabocha cabocha_new(string arg)
 */
PHP_FUNCTION(cabocha_new)
{
    char *arg = NULL;
    size_t arg_len;

    cabocha_t *cabocha;
    zend_resource *res;

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
    zval *zv;
    cabocha_t *cabocha;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zv) == FAILURE) {
        return;
    }

    if ((cabocha = fetch_cabocha(zv)) == NULL) {
        return;
    }

    zend_list_close(Z_RES_P(zv));
}
/* }}} */

/* {{{ proto array cabocha_parse(cabocha cabocha, string arg)
 */
PHP_FUNCTION(cabocha_parse)
{
    zval *zv;
    cabocha_t *cabocha;

    char *arg = NULL;
    size_t arg_len;

    cabocha_tree_t *tree;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &zv, &arg, &arg_len) == FAILURE) {
        RETURN_FALSE;
    }

    if ((cabocha = fetch_cabocha(zv)) == NULL) {
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
    zval *zv;
    cabocha_t *cabocha;

    char *arg = NULL;
    size_t arg_len;

    char *str = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &zv, &arg, &arg_len) == FAILURE) {
        RETURN_FALSE;
    }

    if ((cabocha = fetch_cabocha(zv)) == NULL) {
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

static cabocha_t *fetch_cabocha(zval *zv)
{
    return (cabocha_t *) zend_fetch_resource(
        Z_RES_P(zv),
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
    php_info_print_table_header(2, "cabocha support", "enabled");
    php_info_print_table_end();
}
/* }}} */

/* {{{ cabocha_functions[]
 */
const zend_function_entry cabocha_functions[] = {
    PHP_FE(cabocha_new, NULL)
    PHP_FE(cabocha_destroy, NULL)
    PHP_FE(cabocha_parse, NULL)
    PHP_FE(cabocha_parse_tostr, NULL)
    PHP_FE(cabocha_parse_sentence, NULL)
    PHP_FE(cabocha_parse_sentence_tostr, NULL)
    PHP_FE_END
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
