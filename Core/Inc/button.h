/*
 * button.h
 *
 *  Created on: Mar 31, 2026
 *      Author: vivek
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "stm32f4xx_hal.h"
#include "stm32f446xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/* Defines different Button States */
typedef enum{
	BUTTON_IDLE,
	BUTTON_DEBOUNCE_FIRST,
	BUTTON_FIRST_PRESS,
	BUTTON_WAIT_DOUBLE,
	BUTTON_DEBOUNCE_SECOND,
	BUTTON_SECOND_PRESS
} ButtonState_t;

/* Defines different Button and UART Events */
typedef enum{
	EVENT_NONE,
	EVENT_SINGLE_CLICK,
	EVENT_DOUBLE_CLICK,
	EVENT_LONG_CLICK,

	EVENT_UART_ON,
	EVENT_UART_OFF,
	EVENT_UART_BLINK,
	EVENT_UART_STOP
} Event_t;

/* Defines different Button properties */
typedef struct{
	GPIO_TypeDef *port;
	uint16_t pin;
	ButtonState_t state;
	Event_t event;
    uint16_t debounce_time;
	uint16_t press_time;
	uint16_t release_time;
}button_t;

#define BUTTON_ACTIVE GPIO_PIN_RESET

void button_init(button_t *btn,GPIO_TypeDef *port,uint16_t pin);
void button_handle_press(button_t *btn);
void button_handle_release(button_t *btn);
void button_process(button_t *btn);
void button_tick(button_t *btn);
Event_t button_pop_event(button_t *btn);
void button_queue_event_from_isr(Event_t event);
#endif /* INC_BUTTON_H_ */
