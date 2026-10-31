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
#include <ask_rt433.h>
#include "FLASH_PAGE.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
typedef enum{
	idle,
	learning,
	learned
}learning_state_t;
learning_state_t ask_learning_state = idle;
uint8_t remote_pressed = false;
uint32_t ask_code_in_flash;
ask_t rf433;
uint8_t code[5];

uint8_t Test = 0; /////// For Debug

typedef enum{
	waiting_For_Start,
	button_Clicked,
	playing_Game,
	difficulty_setting,
	turn_setting,
}state_Of_Game_t;
state_Of_Game_t game_State = waiting_For_Start;

uint8_t timer = 0;
uint8_t hold_timer_cnt = 0, not_hold_timer_cnt = 0;
uint8_t Test,Test_B,Test_C;
uint8_t coin;
uint8_t remote_is_hold;
uint8_t remote_start_hold;
uint8_t a_hold;
uint8_t b_hold;
uint8_t c_hold;
uint8_t a_hold_timer_cnt = 0;
uint8_t b_hold_timer_cnt = 0;
uint8_t c_hold_timer_cnt = 0;
uint8_t A_is_pressed;

uint32_t remote_duration_hold = 0 ;

uint32_t a_hold_start_time = 0;
uint8_t a_hold_executed = 0;


uint8_t  a_pressed = 0, a_done = 0;
uint32_t a_start = 0, a_last = 0;

uint8_t  b_pressed = 0, b_done = 0;
uint32_t b_start = 0, b_last = 0;

uint8_t  c_pressed = 0, c_done = 0;
uint32_t c_start = 0, c_last = 0;






/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN)
//{
//	if (GPIO_PIN == RX433_PIN)
//	{
//		ask_pinchange_callback(&ask433);
//	}
//}
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN)
//{
//    if (GPIO_PIN == RX433_PIN)
//    {
//        ask_pinchange_callback(&rf433);
//    }
//}

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//    if (GPIO_Pin == ASK_IN_SIG_Pin)
//    {
//        ask_callBackPinChange(&rf433);
//    }
//}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	 if (GPIO_Pin == ASK_IN_SIG_Pin)
	    {
//	        Test = !Test;

	        ask_callBackPinChange(&rf433);

//	        if (rf433.index > 0)
//	        {
//	            Test = 1;
//	        }
	    }
}


void check_And_Learn_Ask()
{

	ask_loop(&rf433);
	if (ask_available(&rf433))
	{

		ask_read(&rf433, code, NULL, NULL);
		if((ask_code_in_flash & 0x0000FFFF) == (code[0] | (code[1] << 8)))
		{
		    HAL_GPIO_WritePin(MCU_LED_GPIO_Port, MCU_LED_Pin, 1);

		    if((code[2] & 0x0F) == 0x01)    // A
		    {
		        coin = 1;
		        if(a_pressed == 0)
		        {
		            a_pressed = 1;
		            a_start = HAL_GetTick();
		        }
		        a_last = HAL_GetTick();
		    }
		    else if((code[2] & 0x0F) == 0x02)    // B
		    {
		        if(b_pressed == 0)
		        {
		            b_pressed = 1;
		            b_start = HAL_GetTick();
		        }
		        b_last = HAL_GetTick();
		    }
		    else if((code[2] & 0x0F) == 0x04)    // C
		    {
		        if(c_pressed == 0)
		        {
		            c_pressed = 1;
		            c_start = HAL_GetTick();
		        }
		        c_last = HAL_GetTick();
		    }
		}
	}


	else
	{
		HAL_GPIO_WritePin(MCU_LED_GPIO_Port, MCU_LED_Pin, 0);
	}

	if(ask_learning_state == idle)	// Learn Procedure ...
	{
		if(!HAL_GPIO_ReadPin(MCU_BTN_GPIO_Port, MCU_BTN_Pin))
		{
			timer=0;
			ask_learning_state = learning;
		}
	}
	else if(ask_learning_state == learning)
	{
		if(timer==35)// wait for 5 seconds
		{
			// ask code is valid, so save it ...
			ask_code_in_flash = code[0] | (code[1] << 8) | (0x00 << 16);
			blinking();
			Flash_Write_Data(0x08007000, &ask_code_in_flash, 1);
			ask_learning_state = learned;
		}

		if(HAL_GPIO_ReadPin(MCU_BTN_GPIO_Port, MCU_BTN_Pin))	// BTN is released, learning procedure failed
			ask_learning_state = idle;

	}
	else // learned
	{
		// wait for BTN to release
		if(HAL_GPIO_ReadPin(MCU_BTN_GPIO_Port, MCU_BTN_Pin))
			ask_learning_state = idle;
	}
}


void blinking()
{
	for(int i=0;i<=5;i++)
	{
		HAL_GPIO_WritePin(MCU_LED_GPIO_Port, MCU_LED_Pin, 1);
		HAL_Delay(75);
		HAL_GPIO_WritePin(MCU_LED_GPIO_Port, MCU_LED_Pin, 0);
		HAL_Delay(75);
	}
}
void timer_Update()
{
	if(htim14.Instance->CNT>150)
	{
		htim14.Instance->CNT=0;
		timer++;
	}
}



void check_a_hold_key()
{
    if(a_pressed == 1)
    {
        if((HAL_GetTick() - a_last) > 500)    // دکمه رها شد
        {
            a_pressed = 0;
        }
        else if(a_done == 0 && (HAL_GetTick() - a_start) >= 5000)    // 5 ثانیه
        {
            Test = 1;
            a_done = 1;
        }
    }
    else
    {
        a_done = 0;
    }
}

void check_b_hold_key()
{
    if(b_pressed == 1)
    {
        if((HAL_GetTick() - b_last) > 500)
        {
            b_pressed = 0;
        }
        else if(b_done == 0 && (HAL_GetTick() - b_start) >= 5000)
        {
            Test_B = 1;    // اسم متغیرت را بگذار
            b_done = 1;
        }
    }
    else
    {
        b_done = 0;
    }
}

void check_c_hold_key()
{
    if(c_pressed == 1)
    {
        if((HAL_GetTick() - c_last) > 500)
        {
            c_pressed = 0;
        }
        else if(c_done == 0 && (HAL_GetTick() - c_start) >= 5000)
        {
            Test_C = 1;    // اسم متغیرت را بگذار
            c_done = 1;
        }
    }
    else
    {
        c_done = 0;
    }
}




//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//    if (GPIO_Pin == RX433_PIN)
//    {
//        HAL_GPIO_TogglePin(MCU_LED_GPIO_Port, MCU_LED_Pin);
//        ask_pinchange_callback(&ask433);
//    }
//}
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
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim14);
  HAL_TIM_Base_Start(&htim6);
  ask_init(&rf433,ASK_IN_SIG_GPIO_Port,ASK_IN_SIG_Pin);
  Flash_Read_Data(0x08007000, &ask_code_in_flash, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	  HAL_GPIO_TogglePin(Mosfet_1_GPIO_Port, Mosfet_1_Pin);
//	  HAL_Delay(100);
	  check_And_Learn_Ask();
	  timer_Update();

	      check_a_hold_key();
	      check_b_hold_key();
	      check_c_hold_key();

	  if(Test == 1)
	  {
		  HAL_Delay(2000);
		  Test = 0;
	  }
	  if(Test_B == 1)
	  {
		  HAL_Delay(2000);
		  Test_B = 0;
	  }
	  if(Test_C == 1)
	  {
		  HAL_Delay(2000);
		  Test_C = 0;
	  }



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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 479;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 47999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 65535;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Mosfet_8_Pin|Mosfet_9_Pin|Mosfet_10_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MCU_LED_Pin|Mosfet_2_Pin|Mosfet_3_Pin|Mosfet_4_Pin
                          |Mosfet_5_Pin|Mosfet_6_Pin|Mosfet_7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Mosfet_1_GPIO_Port, Mosfet_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Mosfet_8_Pin Mosfet_9_Pin Mosfet_10_Pin */
  GPIO_InitStruct.Pin = Mosfet_8_Pin|Mosfet_9_Pin|Mosfet_10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : MCU_BTN_Pin */
  GPIO_InitStruct.Pin = MCU_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(MCU_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : MCU_LED_Pin */
  GPIO_InitStruct.Pin = MCU_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MCU_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ASK_IN_SIG_Pin */
  GPIO_InitStruct.Pin = ASK_IN_SIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ASK_IN_SIG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Mosfet_1_Pin */
  GPIO_InitStruct.Pin = Mosfet_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Mosfet_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Mosfet_2_Pin Mosfet_3_Pin Mosfet_4_Pin Mosfet_5_Pin
                           Mosfet_6_Pin Mosfet_7_Pin */
  GPIO_InitStruct.Pin = Mosfet_2_Pin|Mosfet_3_Pin|Mosfet_4_Pin|Mosfet_5_Pin
                          |Mosfet_6_Pin|Mosfet_7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

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

#ifdef  USE_FULL_ASSERT
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
