#include "ask_rt433.h"
#include <string.h>
#include "askConfig.h"

// The project must provide a TIM handle named _ASK_TIM (same as original code)
// e.g. TIM_HandleTypeDef htim2; TIM_HandleTypeDef _ASK_TIM = htim2;
extern TIM_HandleTypeDef _ASK_TIM;

// Internal helpers
static void calibrateTiming(ask_t *rf);
static void decodeFrame(ask_t *rf);

// Initialization
void ask_init(ask_t *rf, GPIO_TypeDef *gpio, uint16_t pin)
{
	if (rf == NULL) return;
	memset(rf, 0, sizeof(ask_t));
	rf->gpio = gpio;
	rf->pin = pin;
	rf->shortPulse = 0;
	rf->longPulse = 0;
	rf->thresholdPulse = 0;

	_ASK_TIM.Instance->ARR = 0xFFFF;
	rf->lastPinChangeTimeMs = HAL_GetTick();
	rf->lastCNT = _ASK_TIM.Instance->CNT;
	HAL_TIM_Base_Start(&_ASK_TIM);
}

// This function must be called from the EXTI IRQ handler when the input pin changes
void ask_callBackPinChange(ask_t *rf)
{
	if (rf == NULL) return;

	uint16_t nowCNT = _ASK_TIM.Instance->CNT;
	uint16_t delta = nowCNT - rf->lastCNT;

	// record timing when inside a frame
	if ((rf->newFrame == 1) && (rf->endFrame == 0))
	{
		if (rf->index < _ASK_EDGE)
			rf->dataRaw[rf->index++] = delta;

		// detect end frame: stayed low for minimum and now returned high
		if ((HAL_GetTick() - rf->lastPinChangeTimeMs > _ASK_MINIMUM_STAY_IN_LOW_STATE_TO_DETECT_NEW_FRAME_IN_MS) &&
				(HAL_GPIO_ReadPin(rf->gpio, rf->pin) == GPIO_PIN_SET))
		{
			rf->endFrame = 1;
			rf->dataRawEnd = delta;
		}
	}
	else if ((rf->newFrame == 0) && (HAL_GetTick() - rf->lastPinChangeTimeMs > _ASK_MINIMUM_STAY_IN_LOW_STATE_TO_DETECT_NEW_FRAME_IN_MS)
			&& (HAL_GPIO_ReadPin(rf->gpio, rf->pin) == GPIO_PIN_SET))
	{
		// start of frame
		rf->newFrame = 1;
		rf->dataRawStart = delta;
		rf->index = 0; // reset index at start
	}

	rf->lastCNT = nowCNT;
	rf->lastPinChangeTimeMs = HAL_GetTick();
}

// Called periodically in main loop
void ask_loop(ask_t *rf)
{
	if (rf == NULL) return;

	// timeout while receiving frame: discard
	if ((rf->newFrame == 1) && (rf->endFrame == 0) && (HAL_GetTick() - rf->lastPinChangeTimeMs > _ASK_TIMEOUT_TO_DETECT_NEW_FRAME_IN_MS))
	{
		rf->index = 0;
		rf->newFrame = 0;
		rf->endFrame = 0;
		return;
	}

	if (rf->endFrame == 1)
	{
		// decode the collected raw pulses
		decodeFrame(rf);

		if (rf->dataAvailable)
		{
			// compare with last data to detect repeat/hold
			if ((rf->dataLen > 0) && (memcmp(rf->dataLast, rf->data, rf->dataLen) == 0))
			{
				// repeated frame
				rf->repeatCount++;
				if (rf->repeatCount == 1)
					rf->firstRepeatTick = HAL_GetTick();

				if ((HAL_GetTick() - rf->firstRepeatTick) >= 300) // HOLD_THRESHOLD_MS
				{
					rf->holdState = 1;
				}
			}
			else
			{
				// new frame differs from last
				memcpy(rf->dataLast, rf->data, rf->dataLen);
				rf->repeatCount = 0;
				rf->firstRepeatTick = HAL_GetTick();
				rf->holdState = 0;
			}

			rf->dataTime = HAL_GetTick();
		}

		// reset for next
		rf->index = 0;
		rf->newFrame = 0;
		rf->endFrame = 0;
	}
}

bool ask_available(ask_t *rf)
{
	if (rf == NULL) return false;
	if (rf->dataAvailable == 1)
	{
		rf->dataAvailable = 0;
		return true;
	}
	else
	{
		if (HAL_GetTick() - rf->dataTime > _ASK_HOLD_LAST_FRAME_IN_MS)
			memset(rf->dataLast, 0, sizeof(rf->dataLast));
		return false;
	}
}

bool ask_read(ask_t *rf, uint8_t *code, uint8_t *codeLenInByte, uint16_t *syncTime_us)
{
	if (rf == NULL) return false;
	bool isNew = false;
	if (code != NULL)
		memcpy(code, rf->data, rf->dataLen);
	if (codeLenInByte != NULL)
		*codeLenInByte = rf->dataLen;
	if (syncTime_us != NULL)
		*syncTime_us = rf->dataRawStart * 10; // assuming timer tick = 10us (adjust if different)

	if (memcmp(rf->dataLast, rf->data, rf->dataLen) != 0)
		isNew = true;

	memcpy(rf->dataLast, rf->data, rf->dataLen);
	rf->dataTime = HAL_GetTick();
	return isNew;
}

bool ask_isHold(ask_t *rf)
{
	if (rf == NULL) return false;
	return (rf->holdState == 1);
}

int16_t ask_checkChannelLast4Bit(uint8_t *newCode, uint8_t *reference, uint8_t len)
{
	if (len < 1) return -1;
	uint8_t maskNew[_ASK_MAX_DATA_BYTE];
	uint8_t maskRef[_ASK_MAX_DATA_BYTE];
	memset(maskNew, 0, sizeof(maskNew));
	memset(maskRef, 0, sizeof(maskRef));
	memcpy(maskNew, newCode, len);
	memcpy(maskRef, reference, len);
	maskNew[len - 1] &= 0xF0;
	maskRef[len - 1] &= 0xF0;
	if (memcmp(maskNew, maskRef, len) != 0) return -1;
	return newCode[len - 1] & 0x0F;
}

int16_t ask_checkChannelLast8Bit(uint8_t *newCode, uint8_t *reference, uint8_t len)
{
	if (len < 1) return -1;
	if (memcmp(newCode, reference, len - 1) != 0) return -1;
	return newCode[len - 1];
}

// -------------------- internal functions --------------------
static void calibrateTiming(ask_t *rf)
{
	if (rf == NULL) return;
	if (rf->index < 6) return; // not enough data

	uint32_t sumShort = 0, cntShort = 0;
	uint32_t sumLong = 0, cntLong = 0;

	for (uint8_t i = 0; i < rf->index; i++)
	{
		uint16_t v = rf->dataRaw[i];
		if (v > 800) continue; // likely SYNC or garbage
		if (v < 70)
		{
			sumShort += v; cntShort++;
		}
		else
		{
			sumLong += v; cntLong++;
		}
	}

	if (cntShort) rf->shortPulse = (uint16_t)(sumShort / cntShort);
	if (cntLong) rf->longPulse = (uint16_t)(sumLong / cntLong);

	if (rf->shortPulse == 0 && rf->longPulse > 0)
		rf->shortPulse = rf->longPulse / 3;
	if (rf->longPulse == 0 && rf->shortPulse > 0)
		rf->longPulse = rf->shortPulse * 3;

	if (rf->shortPulse && rf->longPulse)
		rf->thresholdPulse = (rf->shortPulse + rf->longPulse) / 2;
	else
		rf->thresholdPulse = rf->shortPulse + (rf->shortPulse / 2);
}

static void decodeFrame(ask_t *rf)
{
	if (rf == NULL) return;

	// do basic calibration
	calibrateTiming(rf);
	if (rf->thresholdPulse == 0) return;

	memset(rf->data, 0, sizeof(rf->data));
	int8_t bit = 7;
	uint8_t byte = 0;

	for (uint8_t i = 0; i + 1 < rf->index; i += 2)
	{
		uint16_t a = rf->dataRaw[i];
		uint16_t b = rf->dataRaw[i+1];

		// protect against sync/large values
		if (a > 800 || b > 800) break;

		bool aIsLong = (a >= rf->thresholdPulse);
		bool bIsLong = (b >= rf->thresholdPulse);

		if (aIsLong && !bIsLong)
		{
			// long + short => 1
			rf->data[byte] |= (1 << bit);
		}
		else if (!aIsLong && bIsLong)
		{
			// short + long => 0 (do nothing)
		}
		else
		{
			// ambiguous - use distance to prototypes
			uint32_t da = (a > rf->shortPulse) ? (a - rf->shortPulse) : (rf->shortPulse - a);
			uint32_t db = (b > rf->longPulse) ? (b - rf->longPulse) : (rf->longPulse - b);
			if (da < db)
			{
				// likely short+long => 0
			}
			else
			{
				rf->data[byte] |= (1 << bit);
			}
		}

		bit--;
		if (bit < 0)
		{
			bit = 7;
			byte++;
			if (byte >= sizeof(rf->data)) break;
		}
	}

	rf->dataLen = byte;
	if ((rf->dataLen >= _ASK_MIN_DATA_BYTE) && (rf->dataLen <= _ASK_MAX_DATA_BYTE))
		rf->dataAvailable = 1;
}
