#ifndef LMICE_EAL_SOCKET_H
#define LMICE_EAL_SOCKET_H

#include <stdint.h>

#include "eal/lmice_eal_common.h"

enum lmice_socket_data_e
{
    LMICE_SOCKET_DATA_LIST_SIZE = 32
};

struct lmice_socket_data_s
{
    int64_t inst_id;
    SOCKET nfd;
    SOCKADDR_STORAGE addr;
};
typedef struct lmice_socket_data_s lm_socket_dt;

void forceinline create_socket_data_list(lm_socket_dt** cl)
{
    *cl = (lm_socket_dt*)malloc( sizeof(lm_socket_dt)*LMICE_SOCKET_DATA_LIST_SIZE );
    memset(*cl, 0, sizeof(lm_socket_dt)*LMICE_SOCKET_DATA_LIST_SIZE );
}

void forceinline delete_socket_data_list(lm_socket_dt* cl)
{
    lm_hd_head_t* head = NULL;
    lm_socket_dt* next = NULL;
    do {
        head = (lm_hd_head_t*)cl;
        next = head->next;
        free(cl);
        cl = next;
    } while(cl != NULL);
}

int forceinline create_socket_data(lm_handle_data_t* cl, lm_handle_data_t** val)
{
    lm_hd_head_t* head = NULL;
    lm_handle_data_t* cur = NULL;
    size_t i = 0;

    do {
        head = (lm_hd_head_t*)cl;
        for( i= 1; i < LMICE_CLIENT_LIST_SIZE; ++i) {
            *cur = cl+i;
            if(cur->state == LMICE_HANDLE_DATA_NOTUSE) {
                cur->state = LMICE_HANDLE_DATA_USING;
                *val = cur;
                return 0;
            }
        }
        cl = head->next;
    } while(cl != NULL);

    create_socket_data_list(&cur);
    head->next = cur;
    *val = cur+1;
    return 0;
}

int forceinline delete_socket_data(lm_handle_data_t* cl, const lm_handle_data_t* val)
{
    lm_hd_head_t* head = NULL;
    lm_handle_data_t* cur = NULL;
    size_t i = 0;
    head = (lm_hd_head_t*)cl;
    do {
        for( i= 1; i < LMICE_CLIENT_LIST_SIZE; ++i) {
            *cur = cl+i;
            if(cur->state == LMICE_HANDLE_DATA_USING &&
                    cur == val) {
                cur->state = LMICE_HANDLE_DATA_NOTUSE;
                return 0;
            }
        }
        cl = head->next;
    } while(cl != NULL);

    return 1;
}


#endif /** LMICE_EAL_SOCKET_H */

