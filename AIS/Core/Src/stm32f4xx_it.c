/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "keyboard.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Выключение помпы
#define PumpOff() HAL_GPIO_WritePin(Pump2_GPIO_Port, Pump2_Pin, GPIO_PIN_SET)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

// Счётчик секунд бездействия пользователя
volatile uint16_t SecondsSleepMode_CNT = 0;

// Установка флага перехода в "спящий" режим
volatile uint8_t f_SleepMode = 0;

// Счётчик секунд отрисовки кадра со смайликом
volatile uint16_t SecondsStartSleepMode_CNT = 0;

// Счётчик секунд с момента начала полива
volatile uint16_t SecondsStartWatering_CNT = 0;

// Счётчик секунд для моргания светодиодами
volatile uint16_t SecondsBlinkLED_CNT = 0;

// Счётчик секунд для проверки протечки помпы
volatile uint16_t SecondsPumpLeak_CNT = 0;

// Флаг необходимости отрисовки кадра со смайликом
volatile uint8_t f_DrawFrameSleepMode = 0;

// Флаг завершения преобразования АЦП
volatile uint8_t f_AdcEndOfConversion = 0;

// Флаг окончания проверки протечки помпы
volatile uint8_t f_FinishCheckLeak = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
/* USER CODE BEGIN EV */

extern uint16_t SecondsSleepMode_ARR;
extern uint16_t SecondsWatering;
extern uint8_t f_StartWatering;
extern uint8_t f_LED_Blink;
extern uint8_t f_StartCheckLeak;

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */

  // Функция опроса кнопок
  KEYB_Input_Keys();

  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */

  /* USER CODE END TIM4_IRQn 1 */
}

/**
  * @brief This function handles TIM5 global interrupt.
  */
void TIM5_IRQHandler(void)
{
  /* USER CODE BEGIN TIM5_IRQn 0 */

  /* USER CODE END TIM5_IRQn 0 */
  HAL_TIM_IRQHandler(&htim5);
  /* USER CODE BEGIN TIM5_IRQn 1 */

  /* USER CODE END TIM5_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream0 global interrupt.
  */
void DMA2_Stream0_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream0_IRQn 0 */

  /* USER CODE END DMA2_Stream0_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi1_rx);
  /* USER CODE BEGIN DMA2_Stream0_IRQn 1 */

  /* USER CODE END DMA2_Stream0_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream3 global interrupt.
  */
void DMA2_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream3_IRQn 0 */

  /* USER CODE END DMA2_Stream3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi1_tx);
  /* USER CODE BEGIN DMA2_Stream3_IRQn 1 */

  /* USER CODE END DMA2_Stream3_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream4 global interrupt.
  */
void DMA2_Stream4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream4_IRQn 0 */

  /* USER CODE END DMA2_Stream4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA2_Stream4_IRQn 1 */

  /* USER CODE END DMA2_Stream4_IRQn 1 */
}

/* USER CODE BEGIN 1 */

// Прерывание при переполнении таймера
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3)
	{
		// Если не установлен "спящий" режим
		if (!f_SleepMode)
		{
			// Текущее количество секунд
			// бездействия пользователя
			// (отсутствие нажатия на кнопки)
			SecondsSleepMode_CNT++;

			// Переход в "спящий" режим (выключение экрана)
			// по достижению заданного количества секунд
			if (SecondsSleepMode_CNT == SecondsSleepMode_ARR)
			{
				f_SleepMode = 1;

				// Флаг начала отрисовки кадра со смайликом
				f_DrawFrameSleepMode = 1;
			}
		}

		if (f_DrawFrameSleepMode)
		{
			SecondsStartSleepMode_CNT++;

			// Завершение отрисовки кадра
			// со смайликом в течение спустя 30 секунд
			// после перехода в "спящий" режим
			if (SecondsStartSleepMode_CNT == 30)
			{
				f_DrawFrameSleepMode = 0;
				SecondsStartSleepMode_CNT = 0;
			}
		}
	}


	if (htim->Instance == TIM4)
	{
		// Если установлен флаг начала полива
		if (f_StartWatering)
		{
			SecondsStartWatering_CNT++;

			// Выключение полива через
			// заданное количество секунд
			if (SecondsStartWatering_CNT == SecondsWatering)
			{
				// Выключение помпы
				PumpOff();

				// Сброс и выключение таймера 4
				HAL_TIM_Base_Stop_IT(&htim4);
				TIM4->CNT = 0;

				f_StartWatering = 0;
				SecondsStartWatering_CNT = 0;
				SecondsWatering = 0;
			}
		}
	}


	if (htim->Instance == TIM5)
	{
		// Если установлен флаг
		// необходимости моргания светодиодами
		if (f_LED_Blink)
		{
			SecondsBlinkLED_CNT++;

			// Моргание светодиодов раз в секунду
			if (SecondsBlinkLED_CNT == 1)
			{
				HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
				HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
				HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);

				SecondsBlinkLED_CNT = 0;
			}
		}

		// Если установлен флаг
		// начала проверки протечки
		if (f_StartCheckLeak)
		{
			SecondsPumpLeak_CNT++;

			// После отсчёта 60 секунд выполняется
			// повторная проверка протечки помпы
			if (SecondsPumpLeak_CNT == 60)
			{
				f_FinishCheckLeak = 1;
				SecondsPumpLeak_CNT = 0;
			}
		}
	}
}


// Если преобразование АЦП завершено
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	// Установка флага завершения преобразования АЦП
    if (hadc->Instance == ADC1)
       f_AdcEndOfConversion = 1;
}

/* USER CODE END 1 */
