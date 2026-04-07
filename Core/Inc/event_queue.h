/*
 * event_queue.h
 *
 *  Created on: Apr 3, 2026
 *      Author: vivek
 */

#ifndef INC_EVENT_QUEUE_H_
#define INC_EVENT_QUEUE_H_

#include <stdint.h>

#define EVENT_QUEUE_SIZE 10


/* Defines different Button events */
typedef struct{
    uint8_t button_id;
    uint8_t event;
}app_event_t;


void event_queue_init(void);
uint8_t event_queue_push(app_event_t *event);
uint8_t event_queue_pop(app_event_t *event);

#endif /* INC_EVENT_QUEUE_H_ */
