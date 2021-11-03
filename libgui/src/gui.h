#ifndef __GUI_H__
#define __GUI_H__

/**
 * @brief Initializes the gui module
 * 
 */
void gui_init(void);

/**
 * @brief Deinitializes the gui module
 * 
 */
void gui_deinit(void);

/**
 * @brief GUI loop
 * 
 * Runs until it is requested to finalize
 */
void* gui_main_loop(void* data);

/**
 * @brief Finalize the main loop
 * 
 */
void gui_finalize(void);

#endif // __GUI_H__
