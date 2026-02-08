/* USER CODE BEGIN Header */
/* USER CODE END Header */

#include "main.h"
#include "fonts.h"
#include "ssd1306.h"
#include "stdio.h"

/* Değişkenler ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1;

#define DHT22_PORT GPIOB
#define DHT22_PIN GPIO_PIN_14

uint8_t hum1, hum2, tempC1, tempC2, SUM, CHECK;
float temp_Celsius = 0;
float Humidity = 0;
char string[32];

/* --- Dişli Çark (Gear) Bitmap (32x32) --- */
const unsigned char gear_32x32 [] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x20, 0x04, 0x00,
    0x00, 0x40, 0x02, 0x00, 0x01, 0x0c, 0x30, 0x80, 0x03, 0x1c, 0x38, 0xc0, 0x02, 0x1c, 0x38, 0x40,
    0x04, 0x00, 0x00, 0x20, 0x0c, 0x00, 0x00, 0x30, 0x08, 0x0f, 0xf0, 0x10, 0x10, 0x10, 0x08, 0x08,
    0x10, 0x20, 0x04, 0x08, 0x20, 0x20, 0x04, 0x04, 0x20, 0x20, 0x04, 0x04, 0x20, 0x20, 0x04, 0x04,
    0x10, 0x20, 0x04, 0x08, 0x10, 0x10, 0x08, 0x08, 0x08, 0x0f, 0xf0, 0x10, 0x0c, 0x00, 0x00, 0x30,
    0x04, 0x00, 0x00, 0x20, 0x02, 0x1c, 0x38, 0x40, 0x03, 0x1c, 0x38, 0xc0, 0x01, 0x0c, 0x30, 0x80,
    0x00, 0x80, 0x01, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x18, 0x18, 0x00,
    0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* Prototipler ---------------------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);

/* Yardımcı Fonksiyonlar -----------------------------------------------*/
void microDelay (uint16_t delay) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
}

uint8_t DHT22_Start (void) {
    uint8_t Response = 0;
    GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
    GPIO_InitStructPrivate.Pin = DHT22_PIN;
    GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStructPrivate);

    HAL_GPIO_WritePin (DHT22_PORT, DHT22_PIN, 0);
    microDelay (1200);
    HAL_GPIO_WritePin (DHT22_PORT, DHT22_PIN, 1);
    microDelay (30);

    GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructPrivate.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStructPrivate);
    microDelay (40);

    if (!(HAL_GPIO_ReadPin (DHT22_PORT, DHT22_PIN))) {
        microDelay (80);
        if ((HAL_GPIO_ReadPin (DHT22_PORT, DHT22_PIN))) Response = 1;
    }
    uint32_t timeout = HAL_GetTick();
    while ((HAL_GPIO_ReadPin (DHT22_PORT, DHT22_PIN)) && (HAL_GetTick() - timeout < 2));
    return Response;
}

uint8_t DHT22_Read (void) {
    uint8_t x, y = 0;
    __disable_irq();
    for (x = 0; x < 8; x++) {
        uint32_t timeout = HAL_GetTick();
        while (!(HAL_GPIO_ReadPin (DHT22_PORT, DHT22_PIN)) && (HAL_GetTick() - timeout < 2));
        microDelay (40);
        if (!(HAL_GPIO_ReadPin (DHT22_PORT, DHT22_PIN))) y &= ~(1 << (7 - x));
        else y |= (1 << (7 - x));
        timeout = HAL_GetTick();
        while ((HAL_GPIO_ReadPin (DHT22_PORT, DHT22_PIN)) && (HAL_GetTick() - timeout < 2));
    }
    __enable_irq();
    return y;
}

/* Ana Program ---------------------------------------------------------*/
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM1_Init();
    HAL_TIM_Base_Start(&htim1);

    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(5, 15, gear_32x32, 32, 32, White);
    ssd1306_SetCursor(45, 12);
    ssd1306_WriteString("EGE EEE", Font_7x10, White);
    ssd1306_SetCursor(45, 27);
    ssd1306_WriteString("Monitoring", Font_7x10, White);
    ssd1306_SetCursor(45, 42);
    ssd1306_WriteString("v1.0-Safak", Font_7x10, White);
    ssd1306_UpdateScreen();
    HAL_Delay(3000);

    while (1) {
        if(DHT22_Start()) {
            hum1 = DHT22_Read();
            hum2 = DHT22_Read();
            tempC1 = DHT22_Read();
            tempC2 = DHT22_Read();
            SUM = DHT22_Read();

            if ((uint8_t)(hum1 + hum2 + tempC1 + tempC2) == SUM) {
                int16_t temp16 = (tempC1 << 8) | tempC2;
                temp_Celsius = (temp16 & 0x8000) ? (float)(temp16 & 0x7FFF) / -10.0f : (float)temp16 / 10.0f;
                Humidity = (float)((hum1 << 8) | hum2) / 10.0f;

                ssd1306_Fill(Black);
                ssd1306_SetCursor(5, 5);
                sprintf(string, "Nem: %.1f %%", Humidity);
                ssd1306_WriteString(string, Font_11x18, White);
                ssd1306_SetCursor(5, 30);
                sprintf(string, "Sic: %.1f C", temp_Celsius);
                ssd1306_WriteString(string, Font_11x18, White);
                ssd1306_UpdateScreen();
            }
        } else {
            ssd1306_SetCursor(0, 55);
            ssd1306_WriteString("Sensor Hatasi!", Font_7x10, White);
            ssd1306_UpdateScreen();
        }
        HAL_Delay(2000);
    }
}

/* Donanım Konfigürasyonları (Tanımlamalar) -----------------------------*/

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void MX_I2C1_Init(void) {
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  HAL_I2C_Init(&hi2c1);
}

static void MX_TIM1_Init(void) {
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim1);
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);
}

static void MX_GPIO_Init(void) {
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void) {
  __disable_irq();
  while (1) {}
}


