/** 
 * @file         anqueue.h
 * @brief        annular queue
 * @details
 * @author       zlj 
 * @date         2018/1/21 
 * @version      v3.0 
 * @par          Copyright (c):zlj
 */
 
#ifndef _ANQUEUE_H_
#define _ANQUEUE_H_

#define ANQUEUE_OK          0x00
#define ANQUEUE_EINVAL      0x01
#define ANQUEUE_ENOMEM      0x02
#define ANQUEUE_ENOTCREATE  0x03
#define ANQUEUE_EHASCREATED 0x04
#define ANQUEUE_EEMPTY      0x05
#define ANQUEUE_EFULL       0x06
#define ANQUEUE_ESIZE       0x07
#define ANQUEUE_ELENGTH     0x08

#define ANQUEUE_DEBUG
#define ANQUEUE_CUSTOM_MEMORY

enum anqueue_mode
{
    ANQUEUE_MODE_ABANDON,
    ANQUEUE_MODE_COVER
};

enum anqueue_status
{
    ANQUEUE_STATUS_UNKNOWN = 0x0000,
    ANQUEUE_STATUS_CREATE  = 0xA5C3
};

struct anqueue_param
{
    unsigned int length;        /*the queue length*/
    unsigned int elem_size;     /*the queue element size*/
    enum anqueue_mode mode;     /*the queue mode:cover mode and abandon mode*/
    
#ifdef ANQUEUE_CUSTOM_MEMORY
    unsigned int buffer_size;
    unsigned char *buffer;
#endif
};

struct anqueue
{
    unsigned int length;        /*the queue length*/
    unsigned int elem_size;     /*the queue element size*/
    enum anqueue_mode mode;     /*the queue mode:cover mode and abandon mode*/

#ifdef ANQUEUE_CUSTOM_MEMORY    
    unsigned int buffer_size;
#endif
    unsigned char *buffer;
    
    enum anqueue_status status;
    unsigned int head;
    unsigned int tail;
};

int anqueue_create      (struct anqueue *handle, const struct anqueue_param *param);
int anqueue_delete      (struct anqueue *handle);
int anqueue_push        (struct anqueue *handle, const void *elem, unsigned int elem_size);
int anqueue_front       (struct anqueue *handle, void *elem, unsigned int elem_size);
int anqueue_pop         (struct anqueue *handle);
int anqueue_clean       (struct anqueue *handle);
int anqueue_is_full     (const struct anqueue *handle);
int anqueue_is_empty    (const struct anqueue *handle);
int anqueue_get_length  (const struct anqueue *handle, unsigned int *length);

#endif
