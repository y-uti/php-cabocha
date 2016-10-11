#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_cabocha.h"

#include "cabocha.h"

/* If you declare any globals in php_cabocha.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(cabocha)
*/

/* True global resources - no need for thread safety here */
static int le_cabocha;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("cabocha.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_cabocha_globals, cabocha_globals)
    STD_PHP_INI_ENTRY("cabocha.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_cabocha_globals, cabocha_globals)
PHP_INI_END()
*/
/* }}} */

static void zval_chunk(cabocha_chunk_t *chunk, zval *zv);
static void zval_token(cabocha_token_t *token, zval *zv);
static void zval_feature_list(char **feature_list, size_t feature_list_size, zval *zv);

/* {{{ proto array cabocha_parse(string arg, string opt = null)
 */
PHP_FUNCTION(cabocha_parse)
{
    char *arg = NULL;
    size_t arg_len;

    char *opt = NULL;
    size_t opt_len;

    cabocha_t *cabocha;
    cabocha_tree_t *tree;

    zval zv;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|s", &arg, &arg_len, &opt, &opt_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (opt == NULL) {
        cabocha = cabocha_new(0, NULL);
    } else {
        cabocha = cabocha_new2(opt);
    }

    tree = cabocha_sparse_totree(cabocha, arg);

    array_init(return_value);

    /* Add sentence to return_value */
    char *sentence = cabocha_tree_sentence(tree);
    add_assoc_string(return_value, "sentence", sentence);

    /* Add chunks to return_value */
    zval chunks;
    size_t chunk_size = cabocha_tree_chunk_size(tree);
    array_init_size(&chunks, chunk_size);
    zend_hash_real_init(Z_ARRVAL(chunks), 1);
    ZEND_HASH_FILL_PACKED(Z_ARRVAL(chunks)) {
        int i;
        for (i = 0; i < chunk_size; ++i) {
            cabocha_chunk_t *chunk = cabocha_tree_chunk(tree, i);
            zval_chunk(chunk, &zv);
            ZEND_HASH_FILL_ADD(&zv);
        }
    } ZEND_HASH_FILL_END();
    add_assoc_zval(return_value, "chunk", &chunks);

    /* Store chunk entries */
    zval **entries = calloc(chunk_size, sizeof(zval *));
    zval *entry;
    int k = 0;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(chunks), entry) {
        entries[k++] = entry;
    } ZEND_HASH_FOREACH_END();

    /* Add tokens to return_value */
    zval tokens;
    size_t token_size = cabocha_tree_token_size(tree);
    array_init_size(&tokens, token_size);
    zend_hash_real_init(Z_ARRVAL(tokens), 1);
    k = 0;
    ZEND_HASH_FILL_PACKED(Z_ARRVAL(tokens)) {
        int i;
        for (i = 0; i < token_size; ++i) {
          cabocha_token_t *token = cabocha_tree_token(tree, i);
          zval_token(token, &zv);
          if (token->chunk != NULL) {
            // assert(token->chunk == cabocha_tree_chunk(tree, k));
            add_assoc_zval(&zv, "chunk", entries[k]);
            Z_ADDREF_P(entries[k++]);
          } else {
            add_assoc_null(&zv, "chunk");
          }
          ZEND_HASH_FILL_ADD(&zv);
        }
    } ZEND_HASH_FILL_END();
    add_assoc_zval(return_value, "token", &tokens);

    /* Add charset to return_value */
    int charset = cabocha_tree_charset(tree);
    add_assoc_long(return_value, "charset", charset);

    /* Add posset to return_value */
    int posset = cabocha_tree_posset(tree);
    add_assoc_long(return_value, "posset", posset);

    /* Add output_layer to return_value */
    int output_layer = cabocha_tree_output_layer(tree);
    add_assoc_long(return_value, "output_layer", output_layer);

    free(entries);
    cabocha_destroy(cabocha);
}
/* }}} */

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

static void zval_token(cabocha_token_t *token, zval *zv)
{
    array_init(zv);

    ADD_ASSOC_STRING(zv, "surface", token->surface);
    ADD_ASSOC_STRING(zv, "normalized_surface", token->normalized_surface);
    ADD_ASSOC_STRING(zv, "feature", token->feature);
    add_assoc_long(zv, "feature_list_size", token->feature_list_size);
    ADD_ASSOC_STRING(zv, "ne", token->ne);
    ADD_ASSOC_STRING(zv, "additional_info", token->additional_info);

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

/* {{{ php_cabocha_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_cabocha_init_globals(zend_cabocha_globals *cabocha_globals)
{
    cabocha_globals->global_value = 0;
    cabocha_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(cabocha)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(cabocha)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(cabocha)
{
#if defined(COMPILE_DL_CABOCHA) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(cabocha)
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

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

/* {{{ cabocha_functions[]
 *
 * Every user visible function must have an entry in cabocha_functions[].
 */
const zend_function_entry cabocha_functions[] = {
    PHP_FE(cabocha_parse, NULL)
    PHP_FE_END  /* Must be the last line in cabocha_functions[] */
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
    PHP_RINIT(cabocha),     /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(cabocha), /* Replace with NULL if there's nothing to do at request end */
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
