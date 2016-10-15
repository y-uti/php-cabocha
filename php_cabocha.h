#ifndef PHP_CABOCHA_H
#define PHP_CABOCHA_H

extern zend_module_entry cabocha_module_entry;
#define phpext_cabocha_ptr &cabocha_module_entry

#define PHP_CABOCHA_VERSION "0.2.0"

#ifdef PHP_WIN32
#   define PHP_CABOCHA_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_CABOCHA_API __attribute__ ((visibility("default")))
#else
#   define PHP_CABOCHA_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define CABOCHA_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(cabocha, v)

#if defined(ZTS) && defined(COMPILE_DL_CABOCHA)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif /* PHP_CABOCHA_H */
