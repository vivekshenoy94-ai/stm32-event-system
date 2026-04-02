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

/* Defines different Button States */
typedef enum{
	BUTTON_IDLE,
	BUTTON_DEBOUNCING,
	BUTTON_FIRST_PRESS,
	BUTTON_WAIT_DOUBLE,
	BUTTON_SECOND_PRESS
} ButtonState_t;

/* Defines different Button Events */
typedef enum{
	EVENT_NONE,
	EVENT_SINGLE_CLICK,
	EVENT_DOUBLE_CLICK,
	EVENT_LONG_CLICK
} ButtonEvent_t;

/* Defines different Button properties */
typedef struct{
	GPIO_TypeDef *port;
	uint16_t pin;
	ButtonState_t state;
	ButtonEvent_t event;

	uint32_t press_start_time;
	uint32_t release_time;
	uint32_t press2_start_time;
}button_t;

void button_init(button_t *btn,GPIO_TypeDef *port,uint16_t pin);
void button_handle_press(button_t *btn);
void button_handle_release(button_t *btn);
void button_process(button_t *btn);
ButtonEvent_t button_pop_event(button_t *btn);

#endif /* INC_BUTTON_H_ */
