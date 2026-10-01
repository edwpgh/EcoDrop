#ifndef ASK_RT433_H
#define ASK_RT433_H

#include "stm32f0xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Configuration defaults (can be overridden in your project)
#ifndef _ASK_EDGE
#define _ASK_EDGE 120
#endif

#ifndef _ASK_MINIMUM_STAY_IN_LOW_STATE_TO_DETECT_NEW_FRAME_IN_MS
#define _ASK_MINIMUM_STAY_IN_LOW_STATE_TO_DETECT_NEW_FRAME_IN_MS 20
#endif

#ifndef _ASK_TIMEOUT_TO_DETECT_NEW_FRAME_IN_MS
#define _ASK_TIMEOUT_TO_DETECT_NEW_FRAME_IN_MS 200
#endif

#ifndef _ASK_HOLD_LAST_FRAME_IN_MS
#define _ASK_HOLD_LAST_FRAME_IN_MS 1000
#endif

#ifndef _ASK_MIN_DATA_BYTE
#define _ASK_MIN_DATA_BYTE 3
#endif

#ifndef _ASK_MAX_DATA_BYTE
#define _ASK_MAX_DATA_BYTE 8
#endif

// External timer used for measuring pulse lengths (must be configured by user)
extern TIM_HandleTypeDef _ASK_TIM;

typedef struct {
	GPIO_TypeDef *gpio;
	uint16_t pin;

	uint16_t dataRaw[_ASK_EDGE];
	uint16_t dataRawStart;
	uint16_t dataRawEnd;
	uint8_t index;

	uint8_t data[_ASK_MAX_DATA_BYTE];
	uint8_t dataLen;

	uint8_t dataLast[_ASK_MAX_DATA_BYTE];
	uint32_t dataTime;
	uint8_t dataAvailable;

	// timing / calibration
	uint16_t shortPulse;      // calibrated short pulse (timer ticks)
	uint16_t longPulse;       // calibrated long pulse (timer ticks)
	uint16_t thresholdPulse;  // threshold between short and long

	// bookkeeping
	uint32_t lastPinChangeTimeMs;
	uint16_t lastCNT;

	// hold/repeat detection
	uint8_t repeatCount;
	uint32_t firstRepeatTick;
	uint8_t holdState; // 0=no hold, 1=holding

	uint8_t newFrame;
	uint8_t endFrame;
} ask_t;

// API
void ask_init(ask_t *rf, GPIO_TypeDef *gpio, uint16_t pin);
void ask_callBackPinChange(ask_t *rf);
void ask_loop(ask_t *rf);

bool ask_available(ask_t *rf);
bool ask_read(ask_t *rf, uint8_t *code, uint8_t *codeLenInByte, uint16_t *syncTime_us);

bool ask_isHold(ask_t *rf);

// helper checks
int16_t ask_checkChannelLast4Bit(uint8_t *newCode, uint8_t *reference, uint8_t len);
int16_t ask_checkChannelLast8Bit(uint8_t *newCode, uint8_t *reference, uint8_t len);

#endif // ASK_RT433_H
