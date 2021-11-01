#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <signal.h>

#include <zz_event.h>
#include <gui.h>
#include <mainapp.h>
#include <utils.h>

#include <definitions.h>

void signal_handler(int sig);

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    signal(SIGINT, signal_handler);

    pthread_t tid;

    // Initializes the event module from the main thread
    zz_event_init();

    // mainapp and gui init initializes their event queues and handlers
    mainapp_init();
    gui_init();

    int err = 0;
    // the gui main loop runs in a separate thread while mainapp runs on main thread
    if ((err = pthread_create(&tid, NULL, &gui_main_loop, NULL)))
    {
        printf("Unable to create thread. Err: %d", err);
    }

    // Run the mainapp loop until gets quit signal
    mainapp_main_loop(NULL);

    // Exit the gui thread (gui_finalize is thread safe)
    gui_finalize();

    pthread_join(tid, NULL);

    // Cleanup the mainapp and gui modules
    mainapp_deinit();
    gui_deinit();
    zz_event_deinit();

    return EXIT_SUCCESS;
}

void signal_handler(int sig)
{
    if (sig == SIGINT)
    {
        printf("\nSIGINT caught. Requesting to quit.\n");
        signal(sig, SIG_IGN);
        int err = zz_event_create_event_in_queue(
            MAINAPP_EVENT_QUEUE, EVENT_MAINAPP_QUIT_APP,
            ZZ_EVENT_DATA_TYPE_UNDEFINED, NULL, 0, NULL);

        if (err)
        {
            fprintf(stderr, "GUI fail!");
            signal(sig, signal_handler);
        }
    }
}
