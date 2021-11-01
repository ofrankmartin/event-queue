#include "zz_event.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>

typedef struct event_queue_item_t
{
    zz_event_queue_t queue;
    pthread_mutex_t mtx;
} event_queue_item_t;

static event_queue_item_t event_queues[ZZ_EVENT_EVENT_MAX_EVENT_QUEUE];

static pthread_mutex_t queue_list_mtx;

// Private prototypes
int32_t get_next_free_queue(void);

event_queue_item_t *get_queue_item_by_id(
    int32_t queue_id);

int delete_queue(
    event_queue_item_t *queue_item);

int delete_event_callback_list(
        zz_event_callback_list_t **event_callback_list);

int delete_event_list(
        zz_event_list_t **event_list);

int remove_callback_from_list(
    zz_event_callback_list_t *event_list,
    uint32_t event_type);

zz_event_callback_list_t *get_event_callback(
    zz_event_queue_t *queue,
    uint32_t event_type);

int add_event_to_queue(
    int32_t queue_id,
    zz_event_list_t *event);

// Public implementation
int zz_event_init(void)
{
    pthread_mutex_init(&queue_list_mtx, NULL);

    for (int i = 0; i < ZZ_EVENT_EVENT_MAX_EVENT_QUEUE; i++)
    {
        event_queues[i].queue.event_callback_list = NULL;
        event_queues[i].queue.event_list = NULL;
        event_queues[i].queue.id = ZZ_EVENT_QUEUE_ID_UNSET;

        pthread_mutex_init(&event_queues[i].mtx, NULL);
    }

    return 0;
}

int zz_event_deinit(void)
{
    for (int i = 0; i < ZZ_EVENT_EVENT_MAX_EVENT_QUEUE; i++)
    {
        delete_queue(&(event_queues[i]));
        pthread_mutex_destroy(&event_queues[i].mtx);
    }

    pthread_mutex_destroy(&queue_list_mtx);

    return 0;
}

int zz_event_create_queue(
    int32_t queue_id)
{
    if (queue_id < 0)
    {
        fprintf(stderr, "Queue Id must be >= 0. Queue Id: %d", queue_id);
        return 1;
    }

    pthread_mutex_lock(&queue_list_mtx);
    event_queue_item_t *queue_item = get_queue_item_by_id(queue_id);
    if (queue_item)
    {
        pthread_mutex_unlock(&queue_list_mtx);
        fprintf(stderr, "Queue id %d already exists.\n", queue_id);
        return 1;
    }

    int32_t free_queue = get_next_free_queue();
    if (free_queue < 0)
    {
        pthread_mutex_unlock(&queue_list_mtx);
        fprintf(stderr, "No available slots for new event queues.\n");
        return 1;
    }

    event_queues[free_queue].queue.id = queue_id;

    pthread_mutex_unlock(&queue_list_mtx);

    return 0;
}

int zz_event_delete_queue(
    int32_t queue_id)
{
    pthread_mutex_lock(&queue_list_mtx);
    event_queue_item_t *queue_item = get_queue_item_by_id(queue_id);
    if (queue_item)
    {
        delete_queue(queue_item);
    }
    else
    {
        pthread_mutex_unlock(&queue_list_mtx);
        fprintf(stderr, "Event queue <%d> not found.\n", queue_id);
        return 1;
    }

    pthread_mutex_unlock(&queue_list_mtx);

    return 0;
}

int zz_event_register_event_type_callback(
    int32_t queue_id,
    uint32_t event_type,
    zz_event_callback *callback)
{
    pthread_mutex_lock(&queue_list_mtx);
    event_queue_item_t *queue_item = get_queue_item_by_id(queue_id);
    if (queue_item)
    {
        pthread_mutex_lock(&queue_item->mtx);
        bool is_new_event_callback = false;
        zz_event_callback_list_t *event_callback = get_event_callback(&queue_item->queue, event_type);
        if (event_callback == NULL)
        {
            is_new_event_callback = true;
            event_callback = calloc(1, sizeof(zz_event_callback_list_t));
            event_callback->event_type = event_type;
            event_callback->prev = NULL;
            event_callback->next = NULL;
        }
        event_callback->callback = callback;

        if (is_new_event_callback)
        {
            // Add the event to the end of the event list
            zz_event_callback_list_t *curr_item = queue_item->queue.event_callback_list;
            if (curr_item)
            {
                while (curr_item->next)
                {
                    curr_item = curr_item->next;
                }
                curr_item->next = event_callback;
                curr_item->next->prev = curr_item;
            }
            else
            {
                queue_item->queue.event_callback_list = event_callback;
            }
        }
        pthread_mutex_unlock(&queue_item->mtx);
    }
    pthread_mutex_unlock(&queue_list_mtx);

    return 0;
}

int zz_event_remove_event_type_callback(
    int32_t queue_id,
    uint32_t event_type)
{
    pthread_mutex_lock(&queue_list_mtx);
    event_queue_item_t *queue_item = get_queue_item_by_id(queue_id);

    if (queue_item)
    {
        pthread_mutex_lock(&queue_item->mtx);
        remove_callback_from_list(queue_item->queue.event_callback_list, event_type);
        pthread_mutex_unlock(&queue_item->mtx);
    }
    else
    {
        pthread_mutex_unlock(&queue_list_mtx);
        fprintf(stderr, "Event queue <%d> not found.\n", queue_id);
        return 1;
    }

    pthread_mutex_unlock(&queue_list_mtx);

    return 0;
}

int zz_event_create_event_in_queue(
    int32_t queue_id,
    uint32_t event_type,
    zz_event_data_type_t data_type,
    void *data,
    uint32_t data_size,
    zz_event_list_t *event)
{
    pthread_t caller_thread = pthread_self();
    printf("Adding event to queue <%d> in thread <%lx>.\n", queue_id, caller_thread);

    if (event)
    {
        fprintf(stderr, "event must be null.\n");
        return 1;
    }

    event = calloc(1, sizeof(zz_event_list_t));

    event->event_type = event_type;
    event->data_type = data_type;
    event->data = calloc(1, data_size);
    memcpy(event->data, data, data_size);
    event->data_size = data_size;

    return add_event_to_queue(queue_id, event);
}

int zz_event_process_events(
    int32_t queue_id,
    int32_t *n_events)
{

    int32_t event_count = 0;
    event_queue_item_t *queue_item = get_queue_item_by_id(queue_id);
    if (queue_item)
    {
        zz_event_list_t *curr_event = queue_item->queue.event_list;
        if (curr_event)
        {
            pthread_t caller_thread = pthread_self();
            printf("Processing events from queue <%d> in thread <%lx>.\n", queue_id, caller_thread);
        }
        while (curr_event)
        {
            zz_event_callback_list_t *callback_item = get_event_callback(&queue_item->queue, curr_event->event_type);
            if (callback_item)
            {
                callback_item->callback(curr_event);
            }

            pthread_mutex_lock(&queue_item->mtx);
            zz_event_list_t *next_event = curr_event->next;
            curr_event->next = NULL;
            delete_event_list(&curr_event);
            curr_event = next_event;
            queue_item->queue.event_list = curr_event;
            pthread_mutex_unlock(&queue_item->mtx);

            event_count++;
        }
    }

    if (n_events)
    {
        *n_events = event_count;
    }

    return 0;
}

// Private implementations
int32_t get_next_free_queue(void)
{
    for (int i = 0; i < ZZ_EVENT_EVENT_MAX_EVENT_QUEUE; i++)
    {
        if (event_queues[i].queue.id == ZZ_EVENT_QUEUE_ID_UNSET)
        {
            return i;
        }
    }

    return -1;
}

event_queue_item_t *get_queue_item_by_id(int32_t queue_id)
{
    for (int i = 0; i < ZZ_EVENT_EVENT_MAX_EVENT_QUEUE; i++)
    {
        if (event_queues[i].queue.id == queue_id)
        {
            return &(event_queues[i]);
        }
    }

    return NULL;
}

int delete_queue(event_queue_item_t *queue_item)
{
    if (queue_item)
    {
        pthread_mutex_lock(&queue_item->mtx);
        queue_item->queue.id = ZZ_EVENT_QUEUE_ID_UNSET;
        delete_event_list(&queue_item->queue.event_list);
        delete_event_callback_list(&queue_item->queue.event_callback_list);
        queue_item->queue.event_callback_list = NULL;
        pthread_mutex_unlock(&queue_item->mtx);
    }

    return 0;
}

int delete_event_callback_list(zz_event_callback_list_t **event_callback_list)
{
    if (event_callback_list)
    {
        zz_event_callback_list_t *curr_item = *event_callback_list;

        if (curr_item)
        {
            do
            {
                curr_item->prev = NULL;
                zz_event_callback_list_t *next_item = curr_item->next;
                free(curr_item);
                curr_item = next_item;
            } while (curr_item);
        }

        *event_callback_list = NULL;
    }

    return 0;
}

int delete_event_list(zz_event_list_t **event_list)
{
    if (event_list)
    {
        zz_event_list_t *curr_item = *event_list;

        if (curr_item)
        {
            do
            {
                curr_item->prev = NULL;
                zz_event_list_t *next_item = curr_item->next;
                free(curr_item->data);
                curr_item->data_size = 0;
                curr_item->data_type = ZZ_EVENT_DATA_TYPE_UNDEFINED;
                curr_item->event_type = 0;
                curr_item->uuid = 0;
                free(curr_item);
                curr_item = next_item;
            } while (curr_item);
        }
        *event_list = NULL;
    }

    return 0;
}

int remove_callback_from_list(zz_event_callback_list_t *event_list, uint32_t event_type)
{
    if (event_list)
    {
        zz_event_callback_list_t *curr_item = event_list;
        do
        {
            zz_event_callback_list_t *next_item = curr_item->next;
            if (curr_item->event_type == event_type)
            {
                if (curr_item->prev)
                {
                    curr_item->prev->next = curr_item->next;
                    curr_item->prev = NULL;
                }
                if (curr_item->next)
                {
                    curr_item->next->prev = curr_item->prev;
                    curr_item->next = NULL;
                }
                delete_event_callback_list(&curr_item);
            }
            curr_item = next_item;
        } while (curr_item);
    }

    return 0;
}

zz_event_callback_list_t *get_event_callback(zz_event_queue_t *queue, uint32_t event_type)
{
    zz_event_callback_list_t *ret = NULL;
    zz_event_callback_list_t *curr_item = queue->event_callback_list;
    while (curr_item)
    {
        if (curr_item->event_type == event_type)
        {
            ret = curr_item;
            break;
        }
        curr_item = curr_item->next;
    }

    return ret;
}

int add_event_to_queue(
    int32_t queue_id,
    zz_event_list_t *event)
{
    pthread_mutex_lock(&queue_list_mtx);
    event_queue_item_t *queue_item = get_queue_item_by_id(queue_id);
    if (queue_item)
    {
        pthread_mutex_lock(&queue_item->mtx);
        if (queue_item->queue.event_list == NULL)
        {
            queue_item->queue.event_list = event;
        }
        else
        {
            zz_event_list_t *curr_item = queue_item->queue.event_list;
            while (curr_item->next)
            {
                curr_item = curr_item->next;
            }
            event->prev = curr_item;
            curr_item->next = event;
        }
        pthread_mutex_unlock(&queue_item->mtx);
    }
    else
    {
        pthread_mutex_unlock(&queue_list_mtx);
        fprintf(stderr, "Event queue <%d> not found.\n", queue_id);
        return 1;
    }

    pthread_mutex_unlock(&queue_list_mtx);

    return 0;
}
