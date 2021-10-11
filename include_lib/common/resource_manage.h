#ifndef __RESOURCE_MANAGE_H__
#define __RESOURCE_MANAGE_H__

#include "linked_list.h"

#define SUPPORT_MAX_MANAGE   5

typedef struct _mutex_resource {
    linked_item_t item;
    void (*apply_response)(void *priv);
    void (*release_request)(void *priv);
    int priority;
    void *priv;
} mutex_resource_t;

typedef struct resource_manage {
    linked_list_t mutex_header;
    mutex_resource_t current;
    volatile u32 resource_schdule_cnt ;
} resource_manage_t;


bool is_res_manage_empty();
mutex_resource_t *get_resource_addr();
mutex_resource_t *mutex_find_resource(char *resource);
tbool mutex_resource_apply(char *resource, int prio, void (*apply_response)(void *priv), void (*release_request)(void *priv), void *priv);
tbool mutex_resource_release(char *resource);
void resource_manage_schedule();
void resource_manage_init(void *res, void *res_tb);
void resource_manage_init_app();
bool is_cur_resource(char *resource);


#endif

