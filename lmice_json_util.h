#ifndef LMICE_JSON_UTIL_H
#define LMICE_JSON_UTIL_H

#include <jansson.h>

#ifdef __cplusplus
extern "C" {
#endif

#define json_object_get_string(root, obj, name, value) do {   \
    obj = json_object_get(root, name);  \
    value=NULL;   \
    if( json_is_string(obj) ) {   \
        value = json_string_value(obj); \
    }   \
    }while(0)

#define json_object_get_integer(root, obj, name, value) do {   \
    obj = json_object_get(root, name);  \
    value=0;   \
    if( json_is_integer(obj) ) {   \
        value = json_integer_value(obj); \
    }   \
    }while(0)

#ifdef __cplusplus
}
#endif

#endif /* LMICE_JSON_UTIL_H */
