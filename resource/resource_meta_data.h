#ifndef RESOURCE_META_STRUCT_H
#define RESOURCE_META_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(lmice_resource_meta_data_d)
struct lmice_meta_data_s;
struct lmice_meta_data_array_s;
typedef struct lmice_meta_data_s lmice_meta_data_t;
typedef struct lmice_meta_data_array_s lmice_meta_data_array_t;
#endif


int lmice_create_meta_array(     lmice_meta_data_t   **meta_array);
int lmice_destory_meta_array(    lmice_meta_data_t   *meta_array);
int lmice_terminate_meta_array(  lmice_meta_data_t   *meta_array);

int lmice_append_meta_data(lmice_meta_data_array_t* meta_array, lmice_meta_data_t* meta_data);
int lmice_remove_meta_data(lmice_meta_data_array_t* meta_array, lmice_meta_data_t* meta_data);

#ifdef __cplusplus
}
#endif

#endif /** RESOURCE_META_STRUCT_H */
