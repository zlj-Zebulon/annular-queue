/** 
 * @file         anqueue.c 
 * @brief        annular queue
 * @details
 * @author       zlj 
 * @date         2018/1/21 
 * @version      v3.0 
 * @par          Copyright (c):zlj
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anqueue.h"

#ifdef ANQUEUE_DEBUG
#define PRINTF(level, format, arg...) printf("["#level"] - func[%s] line[%d] : "format, __func__, __LINE__, ##arg)
#else
#define PRINTF(level, format, arg...)
#endif

#define PARA_CHECK(para, str)                       \
    do{                                             \
        if(para) {                                  \
            PRINTF(error, "%s", str);               \
            return -ANQUEUE_EINVAL;                 \
        }                                           \
    }while(0)

#define ANQUEUE_CLEAN(handle)                       \
    do{                                             \
        memset(handle, 0, sizeof(struct anqueue));  \
        handle->buffer = NULL;                      \
        handle->mode   = ANQUEUE_MODE_ABANDON;      \
        handle->status = ANQUEUE_STATUS_UNKNOWN;    \
    }while(0)

int anqueue_create(struct anqueue *handle, const struct anqueue_param *param)
{
    unsigned int length = 0;
    
    PARA_CHECK((NULL == handle), "queue create fail, the handle is null!\n");
    PARA_CHECK((NULL == param), "queue create fail, the param is null!\n");

    if(0 == param->length) {
        PRINTF(error, "queue param->length error!\n");
        return -ANQUEUE_ELENGTH;
    }
    
    if(0 == param->elem_size) {
        PRINTF(error, "queue param->elem_size error!\n");
        return -ANQUEUE_ESIZE;
    }
    
    length = param->length + 1;
    
#ifdef ANQUEUE_CUSTOM_MEMORY
    if(param->buffer_size < (length * param->elem_size)) {
        PRINTF(error, "queue param->buffer_size error(%u)!\n", param->buffer_size);
        return -ANQUEUE_ESIZE;        
    }
    
    if(NULL == param->buffer) {
        PRINTF(error, "queue create fail, the param->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }
#endif
    
    if(ANQUEUE_STATUS_CREATE == handle->status) {
        PRINTF(error, "queue has been created!\n");
        return -ANQUEUE_EHASCREATED;
    }
    
    ANQUEUE_CLEAN(handle);

#ifdef ANQUEUE_CUSTOM_MEMORY
    handle->buffer      = param->buffer;
    handle->buffer_size = param->buffer_size;
#else    
    handle->buffer = (unsigned char *)malloc(length * param->elem_size);
    if(NULL == handle->buffer) {
        PRINTF(error, "queue create fail, buffer malloc fail!(length:%u, elem_size:%u, mode:%d)\n", param->length, param->elem_size, (int)param->mode);
        return -ANQUEUE_ENOMEM;
    }
#endif

    (void)memset(handle->buffer, 0x00, length * param->elem_size);
    handle->length      = length;
    handle->elem_size   = param->elem_size;
    handle->mode        = param->mode;
    handle->status      = ANQUEUE_STATUS_CREATE;

    PRINTF(debug, "queue create success.(length:%u, elem_size:%u, mode:%d)\n", param->length, param->elem_size, (int)param->mode);

    return ANQUEUE_OK;
}

int anqueue_delete(struct anqueue *handle)
{
    PARA_CHECK((NULL == handle), "queue delete fail, the handle is null!\n");

    if(ANQUEUE_STATUS_CREATE != handle->status) {
        PRINTF(error, "queue delete fail, the queue not create!\n");
        return -ANQUEUE_ENOTCREATE;
    }

    if(NULL == handle->buffer) {
        PRINTF(error, "queue delete fail, the handle->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }

#ifndef ANQUEUE_CUSTOM_MEMORY
    free(handle->buffer);
#endif
    ANQUEUE_CLEAN(handle);

    PRINTF(debug, "queue delete success.\n");
    
    return ANQUEUE_OK;
}

int anqueue_push(struct anqueue *handle, const void *elem, unsigned int elem_size)
{
    int ret;
    unsigned char *addr;

    PARA_CHECK((NULL == handle), "queue push fail, the handle is null!\n");
    PARA_CHECK((NULL == elem), "queue push fail, the elem is null!\n");
    PARA_CHECK((0 == elem_size), "queue create fail, the elem_size is 0!\n");

    if(ANQUEUE_STATUS_CREATE != handle->status) {
        PRINTF(error, "queue push fail, the queue not create!\n");
        return -ANQUEUE_ENOTCREATE;
    }

    if(NULL == handle->buffer) {
        PRINTF(error, "queue push fail, the handle->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }

    if(handle->elem_size < elem_size) {
        PRINTF(error, "queue push fail, handle->elem_size < elem_size!(handle->elem_size:%u, elem_size:%u)\n", handle->elem_size, elem_size);
        return -ANQUEUE_ESIZE;
    }

    ret = anqueue_is_full(handle);
    if(ANQUEUE_OK == ret) {
        if(ANQUEUE_MODE_ABANDON == handle->mode) {
            PRINTF(error, "queue push fail, the queue is full!\n");
            return -ANQUEUE_EFULL;
        }
        else {
            if(ANQUEUE_MODE_COVER == handle->mode) {
                addr = handle->buffer + handle->head * handle->elem_size;
                (void)memset(addr, 0x00, handle->elem_size);
                handle->head = (handle->head + 1) % handle->length;
            }
        }
    }

    addr = handle->buffer + handle->tail * handle->elem_size;
    (void)memcpy(addr, elem, elem_size);
    handle->tail = (handle->tail + 1) % handle->length;

    PRINTF(debug, "queue push success.(head:%u, tail:%u)\n", handle->head, handle->tail);

    return ANQUEUE_OK;
}

int anqueue_front(struct anqueue *handle, void *elem, unsigned int elem_size)
{
    int ret;
    unsigned char *addr;

    PARA_CHECK((NULL == handle), "queue front fail, the handle is null!\n");
    PARA_CHECK((NULL == elem), "queue front fail, the elem is null!\n");
    PARA_CHECK((0 == elem_size), "queue create fail, the elem_size is 0!\n");

    if(ANQUEUE_STATUS_CREATE != handle->status) {
        PRINTF(error, "queue front fail, the queue not create!\n");
        return -ANQUEUE_ENOTCREATE;
    }

    if(NULL == handle->buffer) {
        PRINTF(error, "queue front fail, the handle->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }

    ret = anqueue_is_empty(handle);
    if(ANQUEUE_OK == ret) {
        PRINTF(error, "queue front fail, the queue is empty.\n");
        return -ANQUEUE_EFULL;
    }

    addr = handle->buffer + handle->head * handle->elem_size;
    elem_size = elem_size < handle->elem_size ? elem_size : handle->elem_size;
    (void)memcpy(elem, addr, elem_size);

    PRINTF(debug, "queue front success.(head:%u, tail:%u)\n", handle->head, handle->tail);

    return ANQUEUE_OK;
}

int anqueue_pop(struct anqueue *handle)
{
    int ret;
    unsigned char *addr;

    PARA_CHECK((NULL == handle), "queue pop fail, the handle is null!\n");
    
    if(ANQUEUE_STATUS_CREATE != handle->status) {
        PRINTF(error, "queue pop fail, the queue not create!\n");
        return -ANQUEUE_ENOTCREATE;
    }

    if(NULL == handle->buffer) {
        PRINTF(error, "queue pop fail, the handle->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }

    ret = anqueue_is_empty(handle);
    if(ANQUEUE_OK == ret) {
        PRINTF(error, "queue pop fail, the queue is empty.\n");
        return -ANQUEUE_EFULL;
    }

    addr = handle->buffer + handle->head * handle->elem_size;
    (void)memset(addr, 0x00, handle->elem_size);
    handle->head = (handle->head + 1) % handle->length;  

    PRINTF(debug, "queue pop success.(head:%u, tail:%u)\n", handle->head, handle->tail);

    return ANQUEUE_OK;
}

int anqueue_is_full(const struct anqueue *handle)
{
    PARA_CHECK((NULL == handle), "unknown, the handle is null!\n");

    if(ANQUEUE_STATUS_CREATE != handle->status) {
        PRINTF(error, "unknown, the queue not create!\n");
        return -ANQUEUE_ENOTCREATE;
    }

    if(NULL == handle->buffer) {
        PRINTF(error, "unknown, the handle->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }

    if(((handle->tail + 1) % handle->length) == handle->head) {
        PRINTF(debug, "queue is full.(head:%u, tail:%u)\n", handle->head, handle->tail);
        return ANQUEUE_OK;
    }

    PRINTF(error, "queue is not full!(head:%u, tail:%u)\n", handle->head, handle->tail);

    return -ANQUEUE_EFULL;
}

int anqueue_is_empty(const struct anqueue *handle)
{
    PARA_CHECK((NULL == handle), "unknown, the handle is null!\n");

    if(ANQUEUE_STATUS_CREATE != handle->status) {
        PRINTF(error, "unknown, the queue not create!\n");
        return -ANQUEUE_ENOTCREATE;
    }

    if(NULL == handle->buffer) {
        PRINTF(error, "unknown, the handle->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }

    if(handle->head == handle->tail) {
        PRINTF(debug, "queue is empty.(head:%u, tail:%u)\n", handle->head, handle->tail);
        return ANQUEUE_OK;
    }

    PRINTF(error, "queue is not empty!(head:%u, tail:%u)\n", handle->head, handle->tail);

    return -ANQUEUE_EEMPTY;
}

int anqueue_clean(struct anqueue *handle)
{
    PARA_CHECK((NULL == handle), "queue clean fail, the handle is null!\n");

    if(ANQUEUE_STATUS_CREATE != handle->status) {
        PRINTF(error, "queue clean fail, the queue not create!\n");
        return -ANQUEUE_ENOTCREATE;
    }

    if(NULL == handle->buffer) {
        PRINTF(error, "queue clean fail, the handle->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }

    handle->head = 0;
    handle->tail = 0;
    (void)memset(handle->buffer, 0x00, handle->length * handle->elem_size);

    PRINTF(debug, "queue clean success.\n");

    return ANQUEUE_OK;
}

int anqueue_get_length(const struct anqueue *handle, unsigned int *length)
{
    PARA_CHECK((NULL == handle), "queue get length fail, the handle is null!\n");
    PARA_CHECK((NULL == length), "queue get length fail, the length is null!\n");

    if(ANQUEUE_STATUS_CREATE != handle->status) {
        PRINTF(error, "queue get length fail, the queue not create!\n");
        return -ANQUEUE_ENOTCREATE;
    }

    if(NULL == handle->buffer) {
        PRINTF(error, "queue get length fail, the handle->buffer is null!\n");
        return -ANQUEUE_ENOMEM;
    }

    *length = (handle->tail + handle->length - handle->head) % handle->length;

    PRINTF(debug, "queue length is %u.\n", *length);

    return ANQUEUE_OK;
}
