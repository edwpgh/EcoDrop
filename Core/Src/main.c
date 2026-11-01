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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <ask_rt433.h>
#include "FLASH_PAGE.h"
#include <WS2811_12.h>

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
	idle_before_Start,
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


uint16_t MOSFET_PINS[10] =
{
    GPIO_PIN_15, // MOSFET 1
    GPIO_PIN_3,  // MOSFET 2
    GPIO_PIN_4,  // MOSFET 3
    GPIO_PIN_5,  // MOSFET 4
    GPIO_PIN_6,  // MOSFET 5
    GPIO_PIN_8,  // MOSFET 6
    GPIO_PIN_9,  // MOSFET 7
    GPIO_PIN_13, // MOSFET 8
    GPIO_PIN_14, // MOSFET 9
    GPIO_PIN_15  // MOSFET 10
};

GPIO_TypeDef *MOSFET_PORTS[10] =
{
    GPIOA, GPIOB, GPIOB, GPIOB, GPIOB,
    GPIOB, GPIOB, GPIOC, GPIOC, GPIOC
};






/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
        if((HAL_GetTick() - a_last) > 500)
        {
            a_pressed = 0;
        }
        else if(a_done == 0 && (HAL_GetTick() - a_start) >= 4800)
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
            Test_B = 1;
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
            Test_C = 1;
            c_done = 1;
        }
    }
    else
    {
        c_done = 0;
    }
}


void shuffle(uint8_t *array, uint8_t size)
{
	for (int i = size - 1; i > 0; i--) {
		int j = rand() % (i + 1);

		int temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}

void MOSFET_Control(uint8_t number, uint8_t state)
{
    HAL_GPIO_WritePin(MOSFET_PORTS[number],MOSFET_PINS[number],state);
}


void ON_ALL_MOSFET(void)
{

	for(uint8_t i = 0; i < 10; i++)
	    {
	        MOSFET_Control(i, 1);
	    }

}
void OFF_ALL_MOSFET(void)
{
	for(uint8_t i = 0; i < 10; i++)
		    {
		        MOSFET_Control(i, 0);
		    }
}


void Random_Generate(uint8_t *array, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        array[i] = i;
    }

    shuffle(array, size);
}


void Start_Game(void)
{
    uint8_t order[10];
    Random_Generate(order, 10);
    for (uint8_t i = 0; i < 10; i++)
    {
        MOSFET_Control(order[i], GPIO_PIN_RESET);

        HAL_Delay(1000);
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
  MX_DMA_Init();
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
  game_State = idle_before_Start;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


	  check_And_Learn_Ask();
	  timer_Update();
	  for(int i = 0; i <10; i++)
	  {
	  Set_LED(i, 255, 255, 255);
	  WS2812_Send();
	  HAL_Delay(1000);
	  }

//	  if(game_State == idle_before_Start)
//	  {
//	  if(coin)
//	  {
//		 ON_ALL_MOSFET();
//		 game_State = waiting_For_Start;
//	  }
//	  if(!coin)
//		  OFF_ALL_MOSFET();
//	  }
//	  else if(game_State == waiting_For_Start)
//	  {
//		  check_a_hold_key();
//		  check_b_hold_key();
//		  check_c_hold_key();
//		  ON_ALL_MOSFET();
//		  if(Test == 1)
//		  {
//			  game_State = playing_Game;
//		  }
//
//	  }
//	  else if(game_State == playing_Game)
//	  {
//		  Start_Game();
//		  coin = 0;
//		  Test = 0;
//		  game_State = idle_before_Start;
//
//	  }
//
//
//
//
//
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
