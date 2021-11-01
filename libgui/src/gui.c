#include "gui.h"

#include <stdbool.h>
#include <stdio.h>

#include <pthread.h>

#include <utils.h>
#include <definitions.h>
#include <zz_event.h>

static bool exit = false;

static int some_number = 0;

pthread_mutex_t exitLock;

// Private prototypes
void print_text(zz_event_list_t *event);
void get_square_ready(zz_event_list_t *event);

// Public implementation
void gui_init(void)
{
    pthread_mutex_init(&exitLock, NULL);

    if (zz_event_create_queue(GUI_EVENT_QUEUE))
    {
        fprintf(stderr, "Unbale to create GUI event queue.\n");
        return;
    }

    zz_event_register_event_type_callback(
        GUI_EVENT_QUEUE, EVENT_GUI_PRINT_TEXT, &print_text);

    zz_event_register_event_type_callback(
        GUI_EVENT_QUEUE, EVENT_GUI_GET_SQUARE_READY, &get_square_ready);
}

void gui_deinit(void)
{
    zz_event_delete_queue(GUI_EVENT_QUEUE);
    pthread_mutex_destroy(&exitLock);
}

void *gui_main_loop(void *data)
{
    (void)data;
    pthread_mutex_lock(&exitLock);
    exit = false;
    pthread_mutex_unlock(&exitLock);

    uint64_t start_time = getTicksMs();

    while (!exit)
    {
        zz_event_process_events(GUI_EVENT_QUEUE, NULL);
        uint64_t elapsed_time = getTicksMs() - start_time;
        if (elapsed_time >= 500)
        {
            for (int i = 0; i < 5; i++)
            {
                int err = zz_event_create_event_in_queue(
                    MAINAPP_EVENT_QUEUE, EVENT_MAINAPP_GET_SQUARE,
                    ZZ_EVENT_DATA_TYPE_SIGNED_INT, &some_number, sizeof(int), NULL);
                if (err)
                {
                    fprintf(stderr, "Unable to create get square event.");
                }
                some_number = (some_number + 1) % 5;
            }
            start_time = getTicksMs();
        }

        msleep(100);
    }

    return NULL;
}

void gui_finalize(void)
{
    pthread_mutex_lock(&exitLock);
    exit = true;
    pthread_mutex_unlock(&exitLock);
}

// Private implementation
void print_text(zz_event_list_t *event)
{
    if (event->data_type == ZZ_EVENT_DATA_TYPE_STRING)
    {
        printf("Text received: [%s]\n", (char *)(event->data));
    }
    else
    {
        fprintf(stderr, "Wrong data type for this event\n");
    }
    fflush(stdout);
}

void get_square_ready(zz_event_list_t *event)
{
    if (event->event_type == EVENT_GUI_GET_SQUARE_READY &&
        event->data_type == ZZ_EVENT_DATA_TYPE_SIGNED_INT)
    {
        printf("Square: %d\n", *(int *)(event->data));
    }
}
