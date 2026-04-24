/*
 * button.c
 *
 *  Created on: Mar 31, 2026
 *      Author: vivek
 */


#include "button.h"


#define DEBOUNCE_THRESHOLD 2

extern QueueHandle_t eventQueue;
static uint8_t debugOverflowCounter=0;
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

	btn->debounce_time=0;
	btn->release_time=0;
	btn->press_time=0;
}
/************************************************************/

/************************************************************
  * @brief  Function to evaluate the button state during rising edge of the button press
  *
  * @retval void
 ***********************************************************/
void button_handle_press(button_t *btn)
{

	switch(btn->state)
	{
	/*Evaluate the First or second click to ensure that its not noise*/
	case BUTTON_IDLE:
		btn->state  = BUTTON_DEBOUNCE_FIRST;
		btn->debounce_time=0;
		break;
	case BUTTON_WAIT_DOUBLE:
		btn->state  = BUTTON_DEBOUNCE_SECOND;
		btn->debounce_time=0;
		break;
	default:
		break;
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


	switch(btn->state)
	{
	case BUTTON_FIRST_PRESS:

		/* Button pressed for 2s , then its considered long else wait for double press. */
		if(btn->press_time>200)
		{
			btn->event = EVENT_LONG_CLICK;
			btn->state = BUTTON_IDLE;
			button_queue_event_from_isr(btn->event);

		}
		else
		{
			btn->state = BUTTON_WAIT_DOUBLE;

		}
		btn->press_time=0;
		break;

	case BUTTON_SECOND_PRESS:

		/* Button pressed for 2s , then its considered long else double click */
		if(btn->press_time>200)
		{

			btn->event = EVENT_LONG_CLICK;

		}
		else
		{
			btn->event = EVENT_DOUBLE_CLICK;
		}
		btn->state = BUTTON_IDLE;
		btn->press_time = 0;
		button_queue_event_from_isr(btn->event);

	default:
		break;
	}


}
/************************************************************/

/************************************************************
  * @brief Function to queue the events in RTOS queue
  *
  * @retval void
 ***********************************************************/
void button_queue_event_from_isr(Event_t event)
{
	BaseType_t xHigherPrioirtyTaskWoken = pdFALSE;

	if(xQueueSendFromISR(eventQueue,&event,&xHigherPrioirtyTaskWoken)!=pdTRUE)
	{
		/*Handle the debug Overflow Counter*/
		debugOverflowCounter++;
	}
	portYIELD_FROM_ISR(xHigherPrioirtyTaskWoken);
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
	if((btn->state==BUTTON_WAIT_DOUBLE)&&((btn->release_time)>100))
	{
		btn->event = EVENT_SINGLE_CLICK;
		btn->state = BUTTON_IDLE;
		btn->release_time=0;
		button_queue_event_from_isr(btn->event);
	}

    /* Switch states are same after debounce time :
     *   then consider moving to next state
     */
	if((btn->state == BUTTON_DEBOUNCE_FIRST)||(btn->state == BUTTON_DEBOUNCE_SECOND))
	{
		if(btn->debounce_time >= DEBOUNCE_THRESHOLD)
		{
			/* Read the pin to be still pressed after debounce time*/
			if(HAL_GPIO_ReadPin(btn->port,btn->pin) == BUTTON_ACTIVE)
			{

				switch(btn->state)
				{
				case BUTTON_DEBOUNCE_FIRST:
					btn->state = BUTTON_FIRST_PRESS;
					break;
				case BUTTON_DEBOUNCE_SECOND:
					btn->state = BUTTON_SECOND_PRESS;
					break;
				default:
					break;
				}
			}
			else
			{
    /* Switch states are not same after debounce time :
     *   first press or second press is discarded accordingly
     */
				switch(btn->state)
				{
				case BUTTON_DEBOUNCE_FIRST:
					btn->state = BUTTON_IDLE;
					break;
				case BUTTON_DEBOUNCE_SECOND:
					btn->state = BUTTON_FIRST_PRESS;
					break;
				default:
					break;
				}
			}
			btn->debounce_time=0;
		}


	}


}
/************************************************************/

/************************************************************
  * @brief Pop function for the button event
  *
  * @retval Provides the Event
 ***********************************************************/
Event_t button_pop_event(button_t *btn)
{
	Event_t Ret_event=btn->event;
	btn->event = EVENT_NONE;
	return Ret_event;
}
/************************************************************/

/************************************************************
  * @brief Increment the respective counter based on the Button States
  *
  * @retval void
 ***********************************************************/
void button_tick(button_t *btn)
{
   switch(btn->state)
   {
   case BUTTON_DEBOUNCE_FIRST:
   case BUTTON_DEBOUNCE_SECOND:
	   btn->debounce_time++;
	   break;
   case BUTTON_FIRST_PRESS:
   case BUTTON_SECOND_PRESS:
	   btn->press_time++;
	   break;
   case BUTTON_WAIT_DOUBLE:
	   btn->release_time++;
	   break;

   default:
	   break;
   }

}

/************************************************************/
