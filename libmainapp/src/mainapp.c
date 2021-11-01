#include "mainapp.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <utils.h>
#include <definitions.h>
#include <zz_event.h>

static bool exit = false;

pthread_mutex_t exitLock;

// Private prototypes
void quit_mainapp(zz_event_list_t *event);
void get_square(zz_event_list_t *event);

// Public implementation
void mainapp_init(void)
{
    pthread_mutex_init(&exitLock, NULL);

    if (zz_event_create_queue(MAINAPP_EVENT_QUEUE))
    {
        fprintf(stderr, "Unbale to create event queue.\n");
        return;
    }
    zz_event_register_event_type_callback(
        MAINAPP_EVENT_QUEUE, EVENT_MAINAPP_QUIT_APP, &quit_mainapp);

    zz_event_register_event_type_callback(
        MAINAPP_EVENT_QUEUE, EVENT_MAINAPP_GET_SQUARE, &get_square);
}

void mainapp_deinit(void)
{
    zz_event_delete_queue(MAINAPP_EVENT_QUEUE);
    pthread_mutex_destroy(&exitLock);
}

void* mainapp_main_loop(void* data)
{
    (void) data;
    pthread_mutex_lock(&exitLock);
    exit = false;
    pthread_mutex_unlock(&exitLock);

    uint64_t start_time = getTicksMs();

    while (!exit)
    {
        zz_event_process_events(MAINAPP_EVENT_QUEUE, NULL);
        uint64_t elapsed_time = getTicksMs() - start_time;
        if (elapsed_time > 1500)
        {
            char msg[] = "Hello from mainapp";
            zz_event_list_t *event = NULL;
            int err = zz_event_create_event_in_queue(
                GUI_EVENT_QUEUE, EVENT_GUI_PRINT_TEXT, 
                ZZ_EVENT_DATA_TYPE_STRING, (void*)msg, strlen(msg) + 1, event);

            if (err)
            {
                fprintf(stderr, "Mainapp fail!\n");
                break;
            }
            start_time = getTicksMs();
        }
        msleep(100);
    }

    return NULL;
}

void mainapp_finalize(void)
{
    pthread_mutex_lock(&exitLock);
    exit = true;
    pthread_mutex_unlock(&exitLock);
}

// Private implementation
void quit_mainapp(zz_event_list_t *event)
{
    if (event->event_type == EVENT_MAINAPP_QUIT_APP)
    {
        mainapp_finalize();
    }
}

void get_square(zz_event_list_t *event)
{
    if (event->event_type == EVENT_MAINAPP_GET_SQUARE && 
        event->data_type == ZZ_EVENT_DATA_TYPE_SIGNED_INT) 
    {
        // Ideally the precision of the int should be handled by the data_size
        // and then create a int8_t, int16_t, int32_t, ... , accordingly.
        int ret = *(int*)(event->data);
        ret *= ret;
        int err = zz_event_create_event_in_queue(
            GUI_EVENT_QUEUE, EVENT_GUI_GET_SQUARE_READY,
            ZZ_EVENT_DATA_TYPE_SIGNED_INT, (void *)&ret, sizeof(int), NULL);

        if (err)
        {
            fprintf(stderr, "Unable to create get square ready event.\n");
        }
    }
}
