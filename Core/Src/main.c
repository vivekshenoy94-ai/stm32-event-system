/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h"
#include "event_queue.h"
#include "stdbool.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* Defines IDLE and BLINK States */
typedef enum{
	LED_IDLE,
	LED_BLINK
}LEDState_t;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define OUTPUT_GROUP  GPIOA
#define LED           GPIO_PIN_5
#define UART_BUFFER_SIZE 32
#define NUM_BUTTON 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
/* Variable to hold the button attributes */
static button_t button[NUM_BUTTON];
/* Variable to hold the LED States */
static LEDState_t  LEDState;
/* Variable to indicate the setting of LED toggle once */
static uint8_t toggle_once_flag=0;
/* Variable to indicate the setting of LED toggle twice */
static uint8_t toggle_twice_flag=0;
/* Variable to indicate the setting of LED toggle twice */
static uint8_t set_ON_flag = 0 ;
/* Variable to indicate the setting of LED toggle twice */
static uint8_t set_OFF_flag = 0 ;
/* Queue that holds the events from the button based on FIFO */
QueueHandle_t eventQueue;
/* UART Buffer */
char uartBuffer[UART_BUFFER_SIZE];
/* UART Index */
volatile uint8_t uartIndex = 0;
/* UART Command Complete */
volatile bool commandReady=false;
/* Holds the UART Received Character */
static char receivedChar;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
static void app_led_output(void);
static void app_process_events(Event_t event);
static Event_t parse_uart_command(char *cmd);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/************************************************************ISR Driven:BEGIN************************************************************/

/************************************************************
  * @brief  HAL Callback for the EXTI Trigger
  *         1. Performs Button state transitions.
  *         2. Starts the timer based on the Button state.
  *         3. Single and Double click Evaluated.
  * @retval void
  ***********************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	for(int i =0;i<NUM_BUTTON;i++){
		if(GPIO_Pin == button[i].pin)
		{
			if(HAL_GPIO_ReadPin(button[i].port,button[i].pin)==BUTTON_ACTIVE)
			{
				button_handle_press(&button[i]);

			}
			else
			{
				button_handle_release(&button[i]);
			}
		}

	}
}

/************************************************************
  * @brief  HAL Callback when the timer is elapsed which was started
  *         in HAL_GPIO_EXTI_Callback
  *         -> Increments the respective counters based on the button state accordingly 
  * @retval void
   ***********************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		for(int i =0;i<NUM_BUTTON;i++)
		{
			button_tick(&button[i]);
		}
	}

}

/************************************************************
  * @brief  UART Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  *
  ***********************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	    uint8_t ch = receivedChar;

        //Command Completed
        if((ch=='\n') || (ch=='\r'))
        {
        	if(uartIndex >0)
        	{
        		uartBuffer[uartIndex] = '\0';
        		commandReady = true;
        	}
        	uartIndex=0;
        }
        else
		{
        	if(uartIndex<UART_BUFFER_SIZE-1)
        	{
        		uartBuffer[uartIndex++]=ch;
        	}
        	else
        	{
        		//Buffer Overflow -> reset
        		uartIndex =0;
        	}
		}

	    HAL_UART_Receive_IT(&huart2,(uint8_t*)&receivedChar,1);
}

/************************************************************ISR Driven:END************************************************************/
/************************************************************RTOS Task Functions:BEGIN*************************************************/
/************************************************************
  * @brief  Event Task that performs button event and UART string processing by
  * enqueueing and dequeueing the events in order of FIFO
  *
  * @retval void
   ***********************************************************/
void EventTask(void* argument)
{
	Event_t event;
	TickType_t lastWakeTime = xTaskGetTickCount();

	while(1)
	{
        /* Send the UART string into the queue */
		if(commandReady)
		{
			commandReady = false;
			event = parse_uart_command(uartBuffer);

			if(event!=EVENT_NONE)
			{
				xQueueSend(eventQueue,&event,0);
			}
		}

		while(xQueueReceive(eventQueue,&event,0)==pdTRUE)
		{
			app_process_events(event);
		}

		for(int i =0;i<NUM_BUTTON;i++)
		{

			button_process(&button[i]);

		}

		vTaskDelayUntil(&lastWakeTime,pdMS_TO_TICKS(10));
	}
}
/************************************************************
  * @brief  Processes the Event to trigger the LED output
  *
  * @retval void
   ***********************************************************/

void LEDTask(void* argument)
{
	TickType_t lastWakeTime = xTaskGetTickCount();
	while(1)
	{
		app_led_output();
		vTaskDelayUntil(&lastWakeTime,pdMS_TO_TICKS(50));
	}
}


/************************************************************
 * @brief  Sets the LED State based on the  Button Events
 *         SINGLE CLICK : LED_TOG_ONCE
 *         DOUBLE CLICK : LED_TOG_TWICE
 *         LONG CLICK : Keep the LED in BLINK state
 * @retval void
 ***********************************************************/
static void app_process_events(Event_t event)
{

		switch(event)
		{
		/*Toggle the LED if its a single click*/
		case EVENT_LONG_CLICK:
			LEDState = LED_BLINK;
			break;
			/*Blink the LED if its a long click*/
		case EVENT_SINGLE_CLICK:
			toggle_once_flag=1;
			LEDState = LED_IDLE;
			break;
			/*Toggle the LED twice with delay of 1s if its a double click*/
		case EVENT_DOUBLE_CLICK:
			toggle_twice_flag=1;
			LEDState = LED_IDLE;
			break;
			/*Turn ON the LED based on the UART Command*/
		case EVENT_UART_ON:
			set_ON_flag = 1;
			LEDState = LED_IDLE;
			break;
			/*Turn OFF the LED based on the UART Command*/
		case EVENT_UART_OFF:
			set_OFF_flag = 1;
			LEDState = LED_IDLE;
			break;
			/*Turn Blink the LED based on the UART Command*/
		case EVENT_UART_BLINK:
			LEDState = LED_BLINK;
			break;
			/*STOP the LED based on the UART Command*/
		case EVENT_UART_STOP:
			LEDState = LED_IDLE;
			break;
			/*There is no event so do nothing*/
		default:
			break;
		}

}
/************************************************************
 * @brief  Toggles/Blink the LED based on the Button State
 *         SINGLE CLICK : Toggle Once
 *         DOUBLE CLICK : Toggle twice
 *         LONG CLICK : Keep the LED in BLINK state
 * @retval void
  ***********************************************************/
static void app_led_output()
{
	static uint32_t last_toggle=0;
	static uint8_t toggle_counter =0;

	/*State Driven Behaviour*/
	if(LEDState == LED_BLINK){
		/*Blink => Toggle with delay(500ms) */
		if(HAL_GetTick()-last_toggle > 500)
		{
			HAL_GPIO_TogglePin(OUTPUT_GROUP,LED);
			last_toggle = HAL_GetTick();
		}
	}

	/*One time action */

	/*Toggle once*/
	if(toggle_once_flag)
	{
		HAL_GPIO_TogglePin(OUTPUT_GROUP,LED);
		toggle_once_flag=0;

	}

	/*Toggle twice*/
	if(toggle_twice_flag)
	{
		if(HAL_GetTick()-last_toggle > 1000)
		{
			HAL_GPIO_TogglePin(OUTPUT_GROUP,LED);
			last_toggle = HAL_GetTick();
			toggle_counter++;
		}
		if(toggle_counter>=2)
		{
			last_toggle=0;
			toggle_counter=0;
			toggle_twice_flag=0;
		}
	}

    /*Flag is set */
	if(set_ON_flag)
	{
		HAL_GPIO_WritePin(OUTPUT_GROUP, LED, GPIO_PIN_SET);
		set_ON_flag=0;
	}
	
	/*Flag is set */
	if(set_OFF_flag)
	{
		HAL_GPIO_WritePin(OUTPUT_GROUP, LED, GPIO_PIN_RESET);
		set_OFF_flag=0;
	}

}
/************************************************************
 * @brief  Parses the UART Command
 *         EVENT_UART_ON : Turn ON the LED
 *         EVENT_UART_OFF : Turn OFF the LED
 *         EVENT_UART_BLINK : Turn BLINK the LED
 *         EVENT_UART_STOP : Turn STOP the LED
 * @retval Event_t
  ***********************************************************/
Event_t parse_uart_command(char *cmd)
{

	if(strcmp(cmd,"ON")==0){
		return EVENT_UART_ON;
	}
	if(strcmp(cmd,"OFF")==0){
		return EVENT_UART_OFF;
	}
	if(strcmp(cmd,"BLINK")==0){
		return EVENT_UART_BLINK;
	}
	if(strcmp(cmd,"STOP")==0){
		return EVENT_UART_STOP;
	}

return EVENT_NONE;
}
/************************************************************RTOS Task Functions:END*************************************************/
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  HAL_UART_Receive_IT(&huart2,(uint8_t*)&receivedChar,1);

  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /*Initialize the buttons*/
  button_init(&button[0],GPIOC,GPIO_PIN_13);
  button_init(&button[1],GPIOC,GPIO_PIN_14);
  event_queue_init();

  /*start timer*/
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  eventQueue = xQueueCreate(10,sizeof(Event_t));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  xTaskCreate(EventTask,"EventTask",128,NULL,2,NULL);
  xTaskCreate(LEDTask,"LEDTask",128,NULL,1,NULL);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }

}

  /* USER CODE END 3 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8400-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 99;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/************************************************************OLD Queue Functions:BEGIN************************************************/
/************************************************************
 * @brief  Collects and processes the events for LED States
 * @retval void
 ***********************************************************/
//static void app_led_state_eval()
//{
//	uint8_t index=0;
//
//	while(index<NUM_BUTTON)
//	{
//		/*update the events in queue*/
//		index = app_queue_button_events(index);
//		/*process the queue event to set the LED state*/
//		app_process_events();
//	}
//}


/************************************************************
 * @brief  Collects the events into the queue for ordered processing
 * @retval void
 ***********************************************************/
//static uint8_t app_queue_button_events(uint8_t index)
//{
//    uint8_t stop_index=NUM_BUTTON;
//	app_event_t app_event;
//	ButtonEvent_t evt;
//	for(int i =index;i<NUM_BUTTON;i++)
//	{
//
//		button_process(&button[i]);
//
//		evt =  button_pop_event(&button[i]);
//        /*Load all the valid event to the queue*/
//		if(evt!=EVENT_NONE)
//		{
//
//			app_event.button_id = i;
//			app_event.event =evt;
//
//			if(!event_queue_push(&app_event))
//			{
//				/*Since processing rest of the button will not yield
//				 * any benefit since the queue is full, hence stop at current index
//				 */
//				stop_index = i;
//				break;
//			}
//		}
//	}
//	return stop_index;
//}
/************************************************************OLD Queue Functions:END************************************************/
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
