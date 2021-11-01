#ifndef __ZZ_EVENT_H__
#define __ZZ_EVENT_H__

#include <stdint.h>

/// Queues with this ID are considered empty and free
#define ZZ_EVENT_QUEUE_ID_UNSET -1
/// Maximum number of simultaneous event queues
#define ZZ_EVENT_EVENT_MAX_EVENT_QUEUE 5

// Forward declaration for the event type
typedef struct zz_event_list_t zz_event_list_t;
// Forward declaration for the event callback list type
typedef struct zz_event_callback_list_t zz_event_callback_list_t;

typedef void(zz_event_callback)(zz_event_list_t *);

/// The event data types
typedef enum zz_event_data_type_t
{
    /// Use this type to carry null or void pointers
    ZZ_EVENT_DATA_TYPE_UNDEFINED = 0,
    /// Signed int data type. Use the data_size to know the precision
    ZZ_EVENT_DATA_TYPE_SIGNED_INT = 1,
    /// Unsigned int data type. Use the data_size to know the precision
    ZZ_EVENT_DATA_TYPE_UNSIGNED_INT = 2,
    /// String data type. Use data_size to know its size (strlen + null terminator)
    ZZ_EVENT_DATA_TYPE_STRING = 4,
} zz_event_data_type_t;

/**
 * @brief The event list type
 */
struct zz_event_list_t
{
    /// Unique event item identifier
    uint32_t uuid;
    /// The event type or category
    uint32_t event_type;
    /// The data type transported by the data pointer
    zz_event_data_type_t data_type;
    /// A pointer to the data to be transported
    void *data;
    /// The binary size of the data transported by the data pointer
    uint32_t data_size;
    /// The previous list item
    zz_event_list_t *prev;
    /// The next list item
    zz_event_list_t *next;
};

/**
 * @brief The event callback list type
 */
struct zz_event_callback_list_t
{
    /// The event type handled by this callback
    uint32_t event_type;
    /// The callback to process the event
    zz_event_callback *callback;
    /// The previous list item
    zz_event_callback_list_t *prev;
    /// The next list item
    zz_event_callback_list_t *next;
};

/**
 * @brief The event queue type
 */
typedef struct zz_event_queue_t
{
    /// Event queue id
    int32_t id;
    /// The event list to be processed
    zz_event_list_t *event_list;
    /// The event handlers callbacks
    zz_event_callback_list_t *event_callback_list;
} zz_event_queue_t;

/**
 * @brief Initializes the event module
 * 
 * This function initializes the data and the mutexes for the whole event 
 * module.
 * 
 * @return int 0 if success, error code otherwise.
 */
int zz_event_init(void);

/**
 * @brief Deinitialize the event module
 * 
 * @return int 0 if success, error code otherwise.
 */
int zz_event_deinit(void);

/**
 * @brief Create a queue object
 * 
 * @param queue_id [in] If provided a value >= 0, this will be used as the 
 * new queue id. If the value is < 0 or if there is already a queue created with
 * the same id, an error is returned.
 * @return int 0 if success, error code otherwise.
 */
int zz_event_create_queue(
    int32_t queue_id);

/**
 * @brief Resets a queue and delete its associated data
 * 
 * @param queue_id [in] the id of the queue to be deleted
 * @return int 0 if success, error code otherwise.
 */
int zz_event_delete_queue(
    int32_t queue_id);

/**
 * @brief Register and event callback for a given queue and event type
 * 
 * @param queue_id [in] The queue where this callback will be registered
 * @param event_type [in] The event type tied to this callback
 * @param callback [in] The event handler callback
 * @return int 0 if success, error code otherwise.
 */
int zz_event_register_event_type_callback(
    int32_t queue_id,
    uint32_t event_type,
    zz_event_callback *callback);

/**
 * @brief Removes the callback for a given event in a given queue
 * 
 * @param queue_id [in] Queue where the event is registered
 * @param event_type [in] The event type to be unregistered
 * @return int 0 if success, error code otherwise.
 */
int zz_event_remove_event_type_callback(
    int32_t queue_id,
    uint32_t event_type);

/**
 * @brief Create an event and append to a queue
 * 
 * @param queue_id [in] Queue where the event will be registered
 * @param event_type [in] The event type id
 * @param data_type [in] The data type carried by the data pointer
 * @param data [in] A pointer to the event data. 
 *             @remarks The data in this pointer is copied and handled by the 
 *             event module
 * @param data_size The binary size of the data
 * @param event [out] The new created event pointer
 * @return int 0 if success, error code otherwise.
 */
int zz_event_create_event_in_queue(
    int32_t queue_id,
    uint32_t event_type,
    zz_event_data_type_t data_type,
    void *data,
    uint32_t data_size,
    zz_event_list_t *event);

/**
 * @brief Process the events in a given queue
 * 
 * @param queue_id [in] the id of the queue to process
 * @param n_events [out] The number of events processed (optional)
 * @return int 0 if success, error code otherwise.
 */
int zz_event_process_events(
    int32_t queue_id,
    int32_t *n_events);

#endif // __ZZ_EVENT_H__