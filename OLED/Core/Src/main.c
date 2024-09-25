/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LEFT_BUTTON_PIN GPIO_PIN_13
#define RIGHT_BUTTON_PIN GPIO_PIN_4
#define BUTTON_GPIO_PORT GPIOC
#define WAKEUP_BUTTON_PIN GPIO_PIN_5
#define WAKEUP_GPIO_PORT GPIOC
#define LED_PIN GPIO_PIN_14
#define LED_GPIO_PORT GPIOB
#define BUZZER_PIN GPIO_PIN_3
#define BUZZER_GPIO_PORT GPIOC

#define BALL_SIZE 4
#define PADDLE_WIDTH 40
#define PADDLE_HEIGHT 4

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
/* Definitions for GameTask */
osThreadId_t GameTaskHandle;
const osThreadAttr_t GameTask_attributes = {
  .name = "GameTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* Definitions for PaddleTask */
osThreadId_t PaddleTaskHandle;
const osThreadAttr_t PaddleTask_attributes = {
  .name = "PaddleTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

/* Definitions for GameOverTask */
osThreadId_t GameOverTaskHandle;
const osThreadAttr_t GameOverTask_attributes = {
  .name = "GameOverTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};


osSemaphoreId_t gameOverSemaphore;

/* USER CODE BEGIN PV */
bool game_over = true;  // Start in game over state
bool first_play = true; // Flag to track the first play
char score_str[20];
char highscore_str[25];
const uint8_t score_display_height = 16;
const uint8_t game_area_top = score_display_height + 2;

uint8_t ball_x = SSD1306_WIDTH / 2;
uint8_t ball_y = (SSD1306_HEIGHT - game_area_top) / 2 + game_area_top;
int8_t ball_vx = 1;
int8_t ball_vy = 1;

uint8_t paddle_x = (SSD1306_WIDTH - PADDLE_WIDTH) / 2;
const uint8_t paddle_y = SSD1306_HEIGHT - PADDLE_HEIGHT - 2;

int score = 0;
static int highscore = 0;
bool wasLeftButtonPressed = false;
bool wasRightButtonPressed = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
void StartGameTask(void *argument);
void StartPaddleTask(void *argument);
void StartDisplayTask(void *argument);
void StartGameOverTask(void *argument);
void LpmTask(void *argument);

/* USER CODE BEGIN PFP */
GPIO_PinState readPC13State();
GPIO_PinState readPC4State();
GPIO_PinState readPC5State();  // New function prototype
bool movePaddleLeft(uint8_t *paddle_x);
bool movePaddleRight(uint8_t *paddle_x, uint8_t paddle_width);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

GPIO_PinState readPC13State() {
    return HAL_GPIO_ReadPin(GPIOC, LEFT_BUTTON_PIN);
}

GPIO_PinState readPC4State() {
    return HAL_GPIO_ReadPin(GPIOC, RIGHT_BUTTON_PIN);
}

GPIO_PinState readPC5State() {  // New function implementation
    return HAL_GPIO_ReadPin(GPIOC, WAKEUP_BUTTON_PIN);
}

bool movePaddleLeft(uint8_t *paddle_x) {
    GPIO_PinState leftButtonState = readPC13State();
    if (leftButtonState == GPIO_PIN_SET && !wasLeftButtonPressed) {
        wasLeftButtonPressed = true;
        if (*paddle_x > 0) {
            *paddle_x -= 2;
        }
        return true;
    }
    if (leftButtonState == GPIO_PIN_RESET) {
        wasLeftButtonPressed = false;
    }
    return false;
}

bool movePaddleRight(uint8_t *paddle_x, uint8_t paddle_width) {
    GPIO_PinState rightButtonState = readPC4State();
    if (rightButtonState == GPIO_PIN_SET && !wasRightButtonPressed) {
        wasRightButtonPressed = true;
        if (*paddle_x + paddle_width < SSD1306_WIDTH) {
            *paddle_x += 2;
        }
        return true;
    }
    if (rightButtonState == GPIO_PIN_RESET) {
        wasRightButtonPressed = false;
    }
    return false;
}

void StartGameTask(void *argument) {
    while (1) {
        if (!game_over) {
            ball_x += ball_vx;
            ball_y += ball_vy;

            if (ball_y <= game_area_top) {
                ball_vy = -ball_vy;
            }

            if (ball_x <= 0 || ball_x >= SSD1306_WIDTH - BALL_SIZE) {
                ball_vx = -ball_vx;
            }

            if (ball_y + BALL_SIZE >= paddle_y && ball_x + BALL_SIZE >= paddle_x && ball_x <= paddle_x + PADDLE_WIDTH) {
                ball_vy = -ball_vy;
                score++;
            }

            if (ball_y >= SSD1306_HEIGHT - BALL_SIZE - PADDLE_HEIGHT) {
                game_over = true;
                osSemaphoreRelease(gameOverSemaphore);
            }

            osDelay(60);
        } else {
            osDelay(200);
        }
    }
}

void StartPaddleTask(void *argument) {
    while (1) {
        if (!game_over) {
            if (movePaddleLeft(&paddle_x)) {}
            if (movePaddleRight(&paddle_x, PADDLE_WIDTH)) {}
        }
        osDelay(50);
    }
}


void StartDisplayTask(void *argument) {
    while (1) {
        if (!game_over) {
            ssd1306_Fill(Black);

            ssd1306_FillRectangle(ball_x, ball_y, ball_x + BALL_SIZE, ball_y + BALL_SIZE, White);
            ssd1306_FillRectangle(paddle_x, paddle_y, paddle_x + PADDLE_WIDTH, paddle_y + PADDLE_HEIGHT, White);

            if (score > highscore) {
                        highscore = score;
                    }
            sprintf(score_str, "Score: %d", score);
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString(score_str, Font_6x8, White);

            sprintf(highscore_str, "Highscore: %d", highscore);
            ssd1306_SetCursor(0, 8);
            ssd1306_WriteString(highscore_str, Font_6x8, White);

            ssd1306_UpdateScreen();
        }

        osDelay(100);
    }
}

void StartGameOverTask(void *argument) {
    while (1) {
        osSemaphoreAcquire(gameOverSemaphore, osWaitForever);
        if (!first_play && game_over) {
                    // Display "Game Over" screen only after the first play
        	ssd1306_Fill(Black); // Set OLED display to black
        	ssd1306_UpdateScreen();
            ssd1306_SetCursor(10, SSD1306_HEIGHT / 2 - 20);
            ssd1306_WriteString("Game Over", Font_11x18, White);
            ssd1306_SetCursor(10, SSD1306_HEIGHT / 2);
            ssd1306_WriteString(score_str, Font_6x8, White);
            ssd1306_SetCursor(10, SSD1306_HEIGHT / 2 + 20);
            ssd1306_WriteString(highscore_str, Font_6x8, White);
            ssd1306_UpdateScreen();
            HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_SET);
            HAL_Delay(2000);
                     // Buzzer Off
            HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_PIN, GPIO_PIN_RESET);
            HAL_Delay(2000);
        }
        if (game_over) {
         ssd1306_Fill(Black); // Set OLED display to black
         HAL_Delay(2000);
         HAL_SuspendTick();
         HAL_PWREx_EnableLowPowerRunMode();
         HAL_ResumeTick();
         HAL_PWREx_DisableLowPowerRunMode();
         }
        ssd1306_Fill(Black);
        ssd1306_UpdateScreen();


        while (readPC5State() == GPIO_PIN_RESET) {
            osDelay(50);  // Wait for PC5 button press to restart the game
        }
        if (game_over && HAL_GPIO_ReadPin(WAKEUP_GPIO_PORT, WAKEUP_BUTTON_PIN) == GPIO_PIN_SET){
        ball_x = SSD1306_WIDTH / 2;
        ball_y = (SSD1306_HEIGHT - game_area_top) / 2 + game_area_top;
        ball_vx = 1;
        ball_vy = 1;
        paddle_x = (SSD1306_WIDTH - PADDLE_WIDTH) / 2;
        score = 0;
        game_over = false;
        first_play = false;  // Set first_play to false after the first game
        ssd1306_Fill(Black);
        ssd1306_UpdateScreen();
        }
    }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == WAKEUP_BUTTON_PIN) {
        if (game_over) {
            game_over = false;
            osSemaphoreRelease(gameOverSemaphore);
        }
    }
}


/* USER CODE END 0 */

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    ssd1306_Init();

    osKernelInitialize();
    GameTaskHandle = osThreadNew(StartGameTask, NULL, &GameTask_attributes);
    PaddleTaskHandle = osThreadNew(StartPaddleTask, NULL, &PaddleTask_attributes);
    DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);
    GameOverTaskHandle = osThreadNew(StartGameOverTask, NULL, &GameOverTask_attributes);
    gameOverSemaphore = osSemaphoreNew(1, 1, NULL);

    osKernelStart();

    while (1) {
        // Should never reach here
    }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        Error_Handler();
    }

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  // Use the correct macro here
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  // Use the correct macro here

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00702991;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LEFT_BUTTON_PIN | RIGHT_BUTTON_PIN | WAKEUP_BUTTON_PIN;  // Add WAKEUP_BUTTON_PIN
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BUZZER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_GPIO_PORT, &GPIO_InitStruct);

    // Set the buzzer pin low initially

}


void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
}
#endif /* USE_FULL_ASSERT */
