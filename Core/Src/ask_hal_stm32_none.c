
#include "ask_hal.h"
#include "main.h"

//ask_t   ask433;

//##################################################################################
//#define TX433_PIN           GPIO_PIN_6
//#define TX433_GPIO          GPIOC
extern TIM_HandleTypeDef htim6;  // config in cubemx, 1 us tick, use 32 bit timer
//##################################################################################
//inline void ask_write_pin_433(bool data)
//{
//    HAL_GPIO_WritePin(TX433_GPIO, TX433_PIN, (GPIO_PinState)data);
//}
//##################################################################################
inline bool ask_read_pin_433(void)
{
    return HAL_GPIO_ReadPin(RX433_GPIO, RX433_PIN);
}
//##################################################################################
inline uint16_t ask_micros_433(void)
{
    return htim6.Instance->CNT;
}
//##################################################################################
void ask_init_rx433(void)
{
    //  config gpio and timer in cubemx        
    HAL_TIM_Base_Start(&htim6);
}
//##################################################################################
//void ask_init_tx433(void)
//{
//    //  config in cubemx
//}
//##################################################################################
inline void ask_delay_ms_433(uint32_t delay_ms)
{
    HAL_Delay(delay_ms);
}
//##################################################################################
inline void ask_delay_us_433(uint32_t delay_us)
{
    uint32_t time = htim6.Instance->CNT;
    while (htim6.Instance->CNT - time < delay_us);
}
//##################################################################################
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//  if (GPIO_Pin == RX433_PIN)
//  {
//      ask_pinchange_callback(&ask433);
//  }
//}
