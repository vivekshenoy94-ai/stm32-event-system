/*
 * button.c
 *
 *  Created on: Mar 31, 2026
 *      Author: vivek
 */


#include "button.h"

/************************************************************
  * @brief  Function to initialize the buttons
  *
  * @retval void
  ***********************************************************/
void button_init(button_t *btn,GPIO_TypeDef *port,uint16_t pin)
{
	btn->port = port;
	btn->pin = pin;

	btn->state = BUTTON_IDLE;
	btn->event = EVENT_NONE;

	btn->press2_start_time=0;
	btn->release_time=0;
	btn->press_start_time=0;
}
/************************************************************/

/************************************************************
  * @brief  Function to evaluate the button state during rising edge of the button press
  *
  * @retval void
 ***********************************************************/
void button_handle_press(button_t *btn)
{
	/*Trigger the timer only if the state is idle*/
	if(btn->state == BUTTON_IDLE)
	{

		btn->press_start_time = HAL_GetTick();

		btn->state  = BUTTON_FIRST_PRESS;
	}
	/*Capture the timestamp to check for seond press duration*/
	else if(btn->state == BUTTON_WAIT_DOUBLE)
	{
		btn->press2_start_time = HAL_GetTick();
		btn->state = BUTTON_SECOND_PRESS;
	}
	else
	{
		/*Do Nothing for any other state*/
	}
}
/************************************************************/

/************************************************************
  * @brief Function to evaluate the button state during falling edge of the button press
  *
  * @retval void
 ***********************************************************/
void button_handle_release(button_t *btn)
{
	/* Variable to hold the overall duration of the button press*/
	uint32_t press_duration=0;
	if(btn->state == BUTTON_FIRST_PRESS)
	{
		/*Calculate the duration*/
		press_duration = HAL_GetTick()-btn->press_start_time;

		/* Button pressed for 2s , then its considered long else short press. */
		if(press_duration>2000)
		{
			btn->event = EVENT_LONG_CLICK;
			btn->state = BUTTON_IDLE;
		}
		else
		{
			btn->release_time = HAL_GetTick();
			btn->state = BUTTON_WAIT_DOUBLE;

		}
		btn->press_start_time=0;

	}
	else if(btn->state == BUTTON_SECOND_PRESS)
	{
		/*Calculate the second click duration*/
		uint32_t press2_duration = HAL_GetTick()-btn->press2_start_time;

		/* Button pressed for 2s , then its considered long else double click */
		if(press2_duration>2000)
		{

			btn->event = EVENT_LONG_CLICK;

		}
		else
		{
			btn->event = EVENT_DOUBLE_CLICK;
		}
		btn->state = BUTTON_IDLE;
		btn->press2_start_time = 0;

	}
	else
	{

	}
}
/************************************************************/

/************************************************************
 * @brief Function to decide on the single click state of the button based on the button state and time elapsed.
 *
 * @retval void
 ***********************************************************/
void button_process(button_t *btn)
{

	/*STOP waiting for the second click after 1s and consider it to be single click*/
	if((btn->state==BUTTON_WAIT_DOUBLE)&&((HAL_GetTick()-btn->release_time)>1000))
	{
		btn->event = EVENT_SINGLE_CLICK;
		btn->state = BUTTON_IDLE;
		btn->release_time=0;
	}


}
/************************************************************/

/************************************************************
  * @brief Pop function for the button event
  *
  * @retval Provides the Event
 ***********************************************************/
ButtonEvent_t button_pop_event(button_t *btn)
{
	ButtonEvent_t Ret_event=btn->event;
	btn->event = EVENT_NONE;
	return Ret_event;
}
/************************************************************/
