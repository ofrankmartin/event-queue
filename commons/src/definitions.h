#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <stdint.h>

/// Event queue processed by the mainapp
#define MAINAPP_EVENT_QUEUE 1
/// Event queue processed by the GUI
#define GUI_EVENT_QUEUE 2

/// GUI events
typedef enum
{
    /// Event for printing a text in the stdout
    EVENT_GUI_PRINT_TEXT = 1,
    EVENT_GUI_GET_SQUARE_READY = 2,
} gui_events_t;

/// Mainapp events
typedef enum
{
    /// Event to finalize the mainapp
    EVENT_MAINAPP_QUIT_APP = 1,
    EVENT_MAINAPP_GET_SQUARE = 2,
} mainapp_events_t;

#endif // __DEFINITIONS_H__