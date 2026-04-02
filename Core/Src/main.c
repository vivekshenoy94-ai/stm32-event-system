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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum{
	LED_IDLE,
	LED_BLINK
}LEDState_t;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define INPUT_GROUP   GPIOC
#define BUTTON_SWITCH GPIO_PIN_13

#define OUTPUT_GROUP  GPIOA
#define LED           GPIO_PIN_5
#define BUTTON_ACTIVE GPIO_PIN_RESET
#define NUM_BUTTON 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Variable to hold the state of the button transitions*/
volatile ButtonState_t button_state =BUTTON_IDLE;
/* Variable to hold the Press event of Button*/
volatile ButtonEvent_t button_event =EVENT_NONE;
/* Variable to hold the start time when the button is first pressed*/
uint32_t press_start_time=0;
/* Variable to hold the start time when the button is first pressed*/
uint32_t press2_start_time=0;
/* Variable to hold the overall duration of the button press*/
uint32_t press_duration=0;
/* Variable to hold the overall duration of the button press*/
uint32_t press2_duration=0;
/* Variable to hold the overall duration of the button press*/
uint32_t release_time=0;
/* Variable to hold the button attributes */
button_t button[NUM_BUTTON];
/* Variable to hold the LED States */
LEDState_t  LEDState;
/* Variable to indicate the setting of LED toggle once */
static uint8_t toggle_once_flag=0;
/* Variable to indicate the setting of LED toggle twice */
static uint8_t toggle_twice_flag=0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
static void APP_LEDStateEval();
static void APP_LEDOutput();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
 * @brief  Sets the LED State based on the  Button State
 *         SINGLE CLICK : LED_TOG_ONCE
 *         DOUBLE CLICK : LED_TOG_TWICE
 *         LONG CLICK : Keep the LED in BLINK state
 * @retval void
  ***********************************************************/
static void APP_LEDStateEval()
{
	ButtonEvent_t Event_t;

	for(int i =0;i<NUM_BUTTON;i++)
	{

		button_process(&button[i]);

		Event_t =  button_pop_event(&button[i]);

		if(Event_t!=EVENT_NONE)
		{
			/*Toggle the LED if its a single click*/
			if(Event_t== EVENT_LONG_CLICK)
			{
				LEDState = LED_BLINK;

			}
			/*Blink the LED if its a long click*/
			else if(Event_t == EVENT_SINGLE_CLICK)
			{
				toggle_once_flag=1;
				LEDState = LED_IDLE;

			}
			/*Toggle the LED twice with delay of 1s if its a double click*/
			else if(Event_t == EVENT_DOUBLE_CLICK)
			{

				toggle_twice_flag=1;
				LEDState = LED_IDLE;

			}
			else
			{
				/*There is no event so do nothing*/
			}
		}
	}
}


/************************************************************
 * @brief  Toggles/Blink the LED based on the Button State
 *         SINGLE CLICK : Toggle Once
 *         DOUBLE CLICK : Toggle twice
 *         LONG CLICK : Keep the LED in BLINK state
 * @retval void
  ***********************************************************/
static void APP_LEDOutput()
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
		if(toggle_counter>1)
		{
			last_toggle=0;
			toggle_counter=0;
			toggle_twice_flag=0;
		}
	}



}
/* USER CODE END 0 */


/************************************************************
  * @brief  The application entry point.
  * @retval int
  ***********************************************************/
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  /*Initialize the button*/
  button_init(&button[0],GPIOC,GPIO_PIN_13);
  button_init(&button[1],GPIOC,GPIO_PIN_14);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	APP_LEDStateEval();
	APP_LEDOutput();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

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
  htim2.Init.Prescaler = 15999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 200;
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
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

///******************************To be considered in next upgrade#start********************************************************
//  * @brief  HAL Callback when the timer is elapsed which was started
//  *         in HAL_GPIO_EXTI_Callback
//  *         1. Resets the timer
//  *         2. Toggles the LED based on button state
//  * @retval void
//  */
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//	if(htim->Instance == TIM2)
//	{
//		/*stop timer*/
//		HAL_TIM_Base_Stop_IT(&htim2);
//
//		/*If the state is debounce then
//		 *  evaluate the pin to set the Button state accordingly*/
//		if(button_state == BUTTON_DEBOUNCING)
//		{
//			/* Read the pin to be still pressed after debounce time*/
//			if(HAL_GPIO_ReadPin(INPUT_GROUP,BUTTON_SWITCH) == BUTTON_ACTIVE)
//			{
//
//				button_state = BUTTON_FIRST_PRESS;
//				/*Capture the button press time*/
//				press_start_time = HAL_GetTick();
//			}
//			else
//			{
//				/*Ignore the event since it was not valid*/
//				button_state = BUTTON_IDLE;
//			}
//
//		}
//
//	}
//}
///******************************To be considered in next upgrade#end********************************************************

