#ifndef __MAINAPP_H__
#define __MAINAPP_H__

/**
 * @brief Initializes the mainapp module
 * 
 */
void mainapp_init(void);

/**
 * @brief Deinitializes the mainapp module
 * 
 */
void mainapp_deinit(void);

/**
 * @brief Main application loop
 * 
 * Runs until it is requested to finalize
 */
void* mainapp_main_loop(void* data);

/**
 * @brief Finalize the main loop
 * 
 */
void mainapp_finalize(void);

#endif // __MAINAPP_H__