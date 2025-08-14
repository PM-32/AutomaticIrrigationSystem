/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "keyboard.h"
#include "OLED.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define SWAP(A, B) { A ^= B; B = A ^ B; A ^= B; }
#define ABS(A) (((A) < 0) ? ((-1) * (A)) : (A))

// Адрес RTC DS3231 (I2C)
#define DS3231_ADDRESS 0xD0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

/* USER CODE BEGIN PV */

//--------------------------------------------------------
//	Настройка текущей даты, времени, дня недели
//--------------------------------------------------------

// Текущие цифры на дисплее
// при настройке даты
uint8_t NumberSetDate[6] = {0};
char* ptr_NumberSetDate = (char*) &NumberSetDate;

// Текущие цифры на дисплее
// при настройке времени
uint8_t NumberSetTime[6] = {0};
char* ptr_NumberSetTime = (char*) &NumberSetTime;

// День недели для настройки
uint8_t NumberSetDayOfWeek = 1;

// Текущий разряд и цифра на
// дисплее при настройке даты
uint8_t CurrentDigitSetDate = 0;
uint8_t CurrentNumberSetDate = 0;

// Текущий разряд и цифра на
// дисплее при настройке времени
uint8_t CurrentDigitSetTime = 0;
uint8_t CurrentNumberSetTime = 0;

// Счётчики для организации
// входа/выхода в настройки
// даты, дня недели, времени
uint8_t cnt_SetDate = 0;
uint8_t cnt_SetDayOfWeek = 0;
uint8_t cnt_SetTime = 0;

// Флаги начала установки
// даты, дня недели, времени
uint8_t f_StSetDate = 0;
uint8_t f_StSetDayOfWeek = 0;
uint8_t f_StSetTime = 0;

// Для определения того,
// что дата установлена
uint8_t new_CondSetDate = 0;
uint8_t old_CondSetDate = 0;
uint8_t f_DateIsSet = 0;

// Для определения того,
// что день недели установлен
uint8_t new_CondSetDayOfWeek = 0;
uint8_t old_CondSetDayOfWeek = 0;
uint8_t f_DayOfWeekIsSet = 0;

// Для определения того,
// что время установлено
uint8_t new_CondSetTime = 0;
uint8_t old_CondSetTime = 0;
uint8_t f_TimeIsSet = 0;

//--------------------------------------------------------
//	Настройка полива
//--------------------------------------------------------

// Текущие цифры на дисплее
// при настройке времени начала полива
uint8_t NumberSetTimeWater[6] = {0};
char* ptr_NumberSetTimeWater = (char*) &NumberSetTimeWater;

// Текущие цифры на дисплее
// при настройке продолжительности полива
uint8_t NumberSetDurationWater[4] = {0};
char* ptr_NumberSetDurationWater = (char*) NumberSetDurationWater;

// Дни полива для настройки
uint8_t WaterDays = 0;

// Текущий разряд и цифра на
// дисплее при настройке начала
// времени полива
uint8_t CurrentDigitSetTimeWater = 0;
uint8_t CurrentNumberSetTimeWater = 0;

// Текущий разряд и цифра на
// дисплее при настройке
// продолжительности полива
uint8_t CurrentDigitSetDurationWater = 0;
uint8_t CurrentNumberSetDurationWater = 0;

// Текущий разряд на дисплее
// при настройке дней полива
uint8_t CurrentDigitSetDaysWater = 0;

// Счётчики для организации
// входа/выхода в настройки
// времени начала полива,
// продолжительности полива,
// дней полива
uint8_t cnt_SetTimeWater = 0;
uint8_t cnt_SetDurationWater = 0;
uint8_t cnt_SetDaysWater = 0;

// Флаги начала установки
// времени начала полива,
// продолжительности полива,
// дней полива
uint8_t f_StSetTimeWater = 0;
uint8_t f_StSetDurationWater = 0;
uint8_t f_StSetDaysWater = 0;

// Для определения того,
// что время полива установлено
uint8_t new_CondSetTimeWater = 0;
uint8_t old_CondSetTimeWater = 0;
uint8_t f_TimeIsSetWater = 0;

// Для определения того,
// что продолжительность
// полива установлена
uint8_t new_CondSetDurationWater = 0;
uint8_t old_CondSetDurationWater = 0;
uint8_t f_TimeIsSetDurationWater = 0;

// Для определения того,
// что дни полива установлены
uint8_t new_CondSetDaysWater = 0;
uint8_t old_CondSetDaysWater = 0;
uint8_t f_DaysWaterIsSet = 0;

// Флаг того, что настройки полива
// были изменены и записаны на flash
uint8_t f_WateringParamsChanged = 0;

// Определение продолжительности
// полива в секундах
uint16_t SecondsWatering = 0;

// Флаг начала полива
uint8_t f_StartWatering = 0;

// Флаг того, что сегодня нужен полив
uint8_t f_WaterDay = 0;

//--------------------------------------------------------
//	Настройка автоотключения экрана
//--------------------------------------------------------

// Текущие цифры на дисплее
// при настройке времени автоотключения экрана
uint8_t NumberSetScreenOff[4] = {0};
char* ptr_NumberSetScreenOff = (char*) &NumberSetScreenOff;

// Текущий разряд и цифра на
// дисплее при настройке времени
// автоотключения экрана
uint8_t CurrentDigitSetScreenOff = 0;
uint8_t CurrentNumberSetScreenOff = 0;

// Счётчик для организации
// входа/выхода в настройки
// времени автоотключения экрана
uint8_t cnt_SetScreenOff = 0;

// Флаг начала установки
// времени автоотключения экрана
uint8_t f_StSetScreenOff = 0;

// Для определения того,
// что время автоотключения
// экрана установлено
uint8_t new_CondSetScreenOff = 0;
uint8_t old_CondSetScreenOff = 0;
uint8_t f_ScreenOffIsSet = 0;

// Флаг того, что настройки автоотключения
// экрана были изменены и записаны на flash
uint8_t f_ScreenOffChanged = 0;

// Время автоотключения экрана
// в секундах при установке
uint16_t SecCheck = 0;

// Время автоотключения экрана
// (спящего режима) в секундах
uint16_t SecondsSleepMode_ARR = 0;

//--------------------------------------------------------
//	Настройка датчика уровня воды
//--------------------------------------------------------

// Флаг того, что настройка состояния датчика
// была изменена и записана на flash
uint8_t f_WaterLevelSensorStateChanged = 0;

//--------------------------------------------------------
//	Работа с кадрами (экран)
//--------------------------------------------------------

// Номер кадра и позиции курсора,
// с которых был выполнен переход на кадр 22
// (сообщение о сбросе настроек)
uint8_t previous_frame_reset = 0;
uint8_t previous_pos_cursor_reset = 0;

// Номер кадра и позиции курсора,
// с которых был выполнен переход на кадр 10
// (сообщение о сохранении настроек)
uint8_t previous_frame_set = 0;
uint8_t previous_pos_cursor_set = 0;

// Флаг того, что пользователь прочитал кадр 28
uint8_t f_NotificationViewed28 = 0;

// Номер предыдущего кадра
uint8_t prev_frame = 0;

// Положение курсора
// на предыдущем кадре
uint8_t prev_pos_cursor = 0;

// Номер текущего кадра
uint8_t current_frame = 0;

// Номер строки, в которой
// расположен курсор для
// выбора пункта меню
uint8_t pos_cursor = 0;

// Для определения ширины строки
uint8_t str_lenght = 0;

// Флаги блокировки
// перемещения курсора
uint8_t f_BlockDown = 0;
uint8_t f_BlockUp = 0;

// Флаг запрета на отрисовку курсора
// (используется в информационных кадрах)
uint8_t f_NoDrawCursor = 0;

/*	Логика переключения между пунктами меню.
 *
 * 	Массивы poss_ix_XX описывают возможные позиции курсора на текущем кадре.
 * 	Рассмотрим кадр poss_ix_4. Он содержит следующие значения {2, 0, 1, 2, 0}.
 * 	Весь экран разбит на 5 строк - столько же элементов имеет массив.
 * 	Пункты меню могут быть занимать несколько строк, если название пункта
 * 	не помещается в одну строку. Курсор, указывающий на текущий пункт меню,
 * 	должен полностью "покрывать" пункт меню. Таким образом, для отрисовки курсора
 * 	нужно знать, где начинается пункт меню и сколько строк он занимает.
 *	Значение 2 элемента poss_ix_4[0] означает, что в первой строке
 *	расположен один из пунктов меню, причём он занимает 2 строки на экране.
 *	Иначе говоря, ненулевое значение элемента массива указывает на то,
 *	что в данной строке начинается пункт меню, а само значение определяет
 *	ширину курсора (количество строк, которое занимает пункт меню).
 *	Нулевое значение указывает на то, что в данной строчке нет пункта меню.
 */

// Инфо кадр вместо кадра 1
uint8_t info_frame_0[5] = {0, 0, 0, 0, 1};

// Возможные позиции курсора на кадрах
uint8_t poss_ix_0[5] = {2, 0, 1, 2, 0};
uint8_t poss_ix_1[5] = {2, 0, 2, 0, 1};
uint8_t poss_ix_2[5] = {0, 0, 0, 1, 0};
uint8_t poss_ix_3[5] = {0, 0, 0, 1, 0};
uint8_t poss_ix_4[5] = {2, 0, 1, 2, 0};
uint8_t poss_ix_5[5] = {2, 0, 1, 1, 0};
uint8_t poss_ix_6[5] = {1, 1, 1, 0, 0};
uint8_t poss_ix_7[5] = {0, 0, 1, 1, 0};
uint8_t poss_ix_8[5] = {0, 0, 1, 1, 0};
uint8_t poss_ix_10[5] = {0, 0, 1, 0, 0};
uint8_t poss_ix_12[5] = {0, 0, 1, 1, 0};
uint8_t poss_ix_13[5] = {0, 0, 1, 1, 1};
uint8_t poss_ix_14[5] = {0, 0, 1, 1, 1};
uint8_t poss_ix_15[5] = {2, 0, 2, 0, 1};
uint8_t poss_ix_16[5] = {0, 0, 0, 0, 1};
uint8_t poss_ix_17[5] = {0, 0, 0, 1, 1};
uint8_t poss_ix_18[5] = {2, 0, 2, 0, 1};
uint8_t poss_ix_19[5] = {0, 0, 0, 1, 1};
uint8_t poss_ix_20[5] = {2, 0, 2, 0, 1};
uint8_t poss_ix_21[5] = {2, 0, 2, 0, 1};
uint8_t poss_ix_22[5] = {0, 0, 1, 0, 0};
uint8_t poss_ix_23[5] = {0, 0, 0, 1, 1};
uint8_t poss_ix_24[5] = {1, 0, 0, 0, 0};
uint8_t poss_ix_25[5] = {0, 0, 0, 0, 1};
uint8_t poss_ix_26[5] = {0, 0, 1, 0, 0};
uint8_t poss_ix_27[5] = {0, 0, 1, 0, 0};
uint8_t poss_ix_28[5] = {0, 0, 0, 0, 1};
uint8_t poss_ix_29[5] = {0, 0, 0, 0, 1};
uint8_t poss_ix_30[5] = {0, 0, 1, 0, 0};
uint8_t poss_ix_31[5] = {0, 0, 0, 0, 1};
uint8_t poss_ix_32[5] = {0, 0, 0, 0, 1};
uint8_t poss_ix_33[5] = {2, 0, 1, 0, 0};

// Текущий массив с
// возможными позициями курсора
uint8_t poss_ix[5] = {0};

// Номер нового кадра
uint8_t new_frame = 0;

// Номер старого кадра
uint8_t old_frame = 0;

// Флаг изменения кадра
// (0 - кадр не менялся,
//  1 - кадр изменился)
uint8_t f_FrameChanged = 0;

//--------------------------------------------------------
//	Структуры для параметров (для чтения и для записи)
//--------------------------------------------------------
/*
 * Структуры, содержащие слово Set в названии,
 * используются для записи. Соответственно,
 * структуры без данного слова используются для чтения.
 */

// Структура часов
typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t dayofweek;
	uint8_t dayofmonth;
	uint8_t month;
	uint8_t year;
} TIME;

TIME time;
TIME timeSetDate;

// Указатели на структуры часов
char* ptr_timeSetDate = (char*) &timeSetDate;
char* ptr_time = (char*) &time;


// Структура полива
typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t seconds_watering;
	uint8_t minutes_watering;
	uint8_t daysofweek;
	uint16_t bytes_alignment;
	uint32_t crc32;
} WATERING;

WATERING WateringParams;
WATERING WateringParamsSet;

// Указатели на структуры полива
char* ptr_WateringParams = (char*) &WateringParams;
char* ptr_WateringParamsSet = (char*) &WateringParamsSet;


// Структура времени
// автоотключения экрана
typedef struct
{
	uint8_t seconds_screen_off;
	uint8_t minutes_screen_off;
	uint16_t bytes_alignment;
	uint32_t crc32;
} AUTO_OFF;

AUTO_OFF ScreenOff;
AUTO_OFF ScreenOffSet;

// Указатели на структуры автоотключения экрана
char* ptr_ScreenOff = (char*) &ScreenOff;
char* ptr_ScreenOffSet = (char*) &ScreenOffSet;


// Структура состояния
// датчика уровня воды
typedef struct
{
	uint32_t f_IsWaterLevelSensorOn;
	uint32_t crc;
} WaterLevelSensor;

WaterLevelSensor WaterLevelSensorState;

// Указатель на структуру
// состояния датчика уровня воды
char* ptr_WaterLevelSensorState = (char*) &WaterLevelSensorState;

//--------------------------------------------------------
//	"Спящий" режим
//--------------------------------------------------------

// Флаг того, что был выполнен
// выход из "спящего" режима
uint8_t f_SleepModeExit = 0;

// Флаг блокировки нажатия кнопки
// (первое нажатие кнопки после
//  выхода из "спящего" режима
//  не должно выполнять никаких действий)
uint8_t f_BlockButtons = 0;

// Флаг для фиксирования
// второго нажатия на кнопки
// при выходе из "спящего" режима
uint8_t f_SecondPress = 0;

// Флаг начала ожидания
// второго нажатия на кнопки
// при выходе из "спящего" режима
uint8_t f_WaitSecondPress = 0;

//--------------------------------------------------------
//	Управление помпой
//--------------------------------------------------------

// Включение/выключение помпы
#define PumpOn() HAL_GPIO_WritePin(Pump2_GPIO_Port, Pump2_Pin, GPIO_PIN_RESET)
#define PumpOff() HAL_GPIO_WritePin(Pump2_GPIO_Port, Pump2_Pin, GPIO_PIN_SET)

//--------------------------------------------------------
//	АЦП
//--------------------------------------------------------

// Размеры массивов для хранения измерений АЦП
#define AdcArraySize 16

// Массив для хранения текущих преобразований АЦП:
// [0] - датчик уровня воды, [1] - датчик протечки.
uint16_t adc_data[2] = {0};

// Массивы для хранения результатов преобразований АЦП
uint16_t adc_result_ch0[AdcArraySize] = {0};
uint16_t adc_result_ch1[AdcArraySize] = {0};

// Номер текущего преобразования АЦП
uint8_t adc_index = 0;

// Среднее значение результатов АЦП
uint16_t mean_adc_ch0 = 0;
uint16_t mean_adc_ch1 = 0;

// Флаг того, что массивы
// с измерениями АЦП заполнены
uint8_t f_AdcFull = 0;

// Флаг начала включения
// индикации уровня воды
uint8_t f_StartAnalyzingAdcData = 0;

//--------------------------------------------------------
//	Уровень воды в резервуаре (датчик уровня воды)
//--------------------------------------------------------

// Флаг необходимости
// моргать тремя светодиодами
uint8_t f_LED_Blink = 0;

// Флаг сброса светодиодов при
// переходе в диапазон NoWater
// (нужно для синхронизации
// светодиодов при переходе из LowLevel)
uint8_t f_ResetLED = 0;

// Возможные диапазоны уровня воды
enum WaterState
{
	NoWater = 0,
	LowLevel,
	MidLevel,
	HighLevel
};

// Текущий уровень воды
// (по умолчанию воды в резервуаре мало)
enum WaterState WaterLevel = LowLevel;

// Для определения изменения уровня воды
uint8_t new_WaterLevel = 0;
uint8_t old_WaterLevel = 0;

// Флаг того, что уровень
// воды изменился на NoWater
uint8_t f_WaterLevelChangedNoWater = 0;

//--------------------------------------------------------
//	Уровень воды в корпусе (датчик протечки)
//--------------------------------------------------------

// Флаг протечки воды
uint8_t f_IsWaterLeak = 0;

// Флаг начала проверки протечки воды
uint8_t f_StartCheckLeak = 0;

//--------------------------------------------------------
//	Другое
//--------------------------------------------------------

// Ошибки при настройке параметров:
// 0 - нет ошибок
// 1 - ошибка ввода даты (при настройке текущей даты)
// 2 - ошибка ввода времени (при настройке текущего времени)
// 3 - ошибка ввода времени (при настройке времени полива)
// 4 - ошибка ввода времени (при настройке продолжительности полива)
// 5 - установка ошибки ввода дней полива
// 6 - установка ошибки ввода времени автоотключения экрана
uint8_t set_fails = 0;

// Флаг наличия фатальных ошибок
uint8_t f_FatalErrors = 0;

// Переменная с состояниями кнопок
uint32_t keys = 0;

// Флаг ошибки при работе с Micro SD
uint8_t f_ErrorSD = 0;

// Файлы для хранения параметров полива,
// автоотключения экрана,
// состояния датчика уровня воды
FIL file_WateringParams, file_ScreenOff, file_WaterLevelSensorState;

// Изображение спящего смайлика
static const uint8_t ImageSleepMode[512] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x7f, 0xf4, 0x0e, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xff, 0xff, 0x1c, 0x00, 0x00,
    0x00, 0x00, 0x0f, 0x80, 0x0f, 0xd8, 0x00, 0x00,
    0x00, 0x00, 0x3e, 0x00, 0x01, 0xb0, 0xff, 0xc0,
    0x00, 0x00, 0xf8, 0x00, 0x54, 0x7c, 0xff, 0x80,
    0x00, 0x01, 0xe0, 0x00, 0x7c, 0x6e, 0x07, 0xc0,
    0x00, 0x03, 0x80, 0x00, 0x08, 0xc0, 0x17, 0x00,
    0x00, 0x07, 0x00, 0x00, 0x19, 0xff, 0x8f, 0x00,
    0x00, 0x0e, 0x00, 0x00, 0x30, 0xfd, 0xce, 0x00,
    0x00, 0x1c, 0x00, 0x00, 0x60, 0x00, 0xdc, 0x00,
    0x00, 0x38, 0x00, 0x00, 0x68, 0x00, 0x38, 0x00,
    0x00, 0x30, 0x00, 0x00, 0x5c, 0x00, 0x78, 0x00,
    0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00,
    0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
    0x00, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0,
    0x00, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0,
    0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
    0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x03, 0x00, 0x80, 0x00, 0x20, 0x04, 0x03, 0x00,
    0x03, 0x01, 0xe0, 0xf0, 0x3c, 0xbc, 0x03, 0x00,
    0x03, 0x00, 0x7f, 0xe0, 0x3f, 0xf8, 0x03, 0x00,
    0x03, 0x00, 0x7f, 0xe0, 0x0f, 0xf0, 0x03, 0x00,
    0x03, 0x00, 0x3f, 0x80, 0x0f, 0xe0, 0x03, 0x00,
    0x03, 0x00, 0x0a, 0x00, 0x00, 0x80, 0x03, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
    0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00,
    0x01, 0xc0, 0x00, 0x07, 0x80, 0x00, 0x0c, 0x00,
    0x00, 0xc0, 0x00, 0x0f, 0xc0, 0x00, 0x0c, 0x00,
    0x00, 0xe0, 0x00, 0x1f, 0xe0, 0x00, 0x1c, 0x00,
    0x00, 0x60, 0x00, 0x1f, 0xe0, 0x00, 0x18, 0x00,
    0x00, 0x70, 0x00, 0x1f, 0xe0, 0x00, 0x38, 0x00,
    0x00, 0x30, 0x00, 0x0f, 0xe0, 0x00, 0x70, 0x00,
    0x00, 0x18, 0x00, 0x0f, 0xc0, 0x00, 0x60, 0x00,
    0x00, 0x1c, 0x00, 0x07, 0x00, 0x00, 0xe0, 0x00,
    0x00, 0x0e, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00,
    0x00, 0x07, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00,
    0x00, 0x03, 0xc0, 0x00, 0x00, 0x0f, 0x00, 0x00,
    0x00, 0x01, 0xe0, 0x00, 0x00, 0x1e, 0x00, 0x00,
    0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 0x00, 0x00,
    0x00, 0x00, 0x3e, 0x00, 0x03, 0xf0, 0x00, 0x00,
    0x00, 0x00, 0x0f, 0xe0, 0x1f, 0xc0, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Изображение грустного смайлика
static const uint8_t ImageSad[350] =
{
    0x00, 0x00, 0x0b, 0xf4, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7e, 0xaf, 0x80, 0x00, 0x00,
    0x00, 0x03, 0xc0, 0x01, 0xe0, 0x00, 0x00,
    0x00, 0x0e, 0x00, 0x00, 0x3c, 0x00, 0x00,
    0x00, 0x18, 0x00, 0x00, 0x0c, 0x00, 0x00,
    0x00, 0x70, 0x00, 0x00, 0x03, 0x80, 0x00,
    0x00, 0xa0, 0x00, 0x00, 0x02, 0x80, 0x00,
    0x01, 0xc0, 0x00, 0x00, 0x00, 0xe0, 0x00,
    0x03, 0x00, 0x10, 0x00, 0x00, 0x60, 0x00,
    0x02, 0x00, 0x40, 0x08, 0x00, 0x30, 0x00,
    0x06, 0x00, 0x30, 0x0e, 0x00, 0x18, 0x00,
    0x0c, 0x00, 0x70, 0x06, 0x00, 0x0c, 0x00,
    0x18, 0x01, 0xe0, 0x0f, 0x80, 0x06, 0x00,
    0x10, 0x05, 0xc0, 0x03, 0xc0, 0x06, 0x00,
    0x30, 0x2f, 0x80, 0x01, 0xfa, 0x02, 0x00,
    0x28, 0x7f, 0x00, 0x00, 0x7c, 0x03, 0x00,
    0x20, 0x38, 0x20, 0x01, 0x9c, 0x01, 0x00,
    0x60, 0x01, 0xe0, 0x07, 0xc0, 0x03, 0x80,
    0x40, 0x01, 0xf0, 0x03, 0xe0, 0x00, 0x80,
    0x60, 0x03, 0xf0, 0x07, 0xe0, 0x01, 0x80,
    0x40, 0x03, 0xf8, 0x0f, 0xe0, 0x00, 0x80,
    0xc0, 0x03, 0xf0, 0x07, 0xe0, 0x00, 0xc0,
    0x80, 0x01, 0xf0, 0x07, 0xe0, 0x00, 0xc0,
    0xc0, 0x01, 0xf0, 0x03, 0xc0, 0x00, 0xc0,
    0xc0, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0,
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
    0x60, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x20, 0x00, 0x02, 0xd0, 0x00, 0x01, 0x00,
    0x30, 0x00, 0x1f, 0xfc, 0x00, 0x07, 0x00,
    0x18, 0x00, 0x5f, 0xff, 0x40, 0x02, 0x00,
    0x18, 0x00, 0x7a, 0x07, 0x80, 0x06, 0x00,
    0x08, 0x00, 0xf0, 0x01, 0xe0, 0x14, 0x00,
    0x0c, 0x00, 0xe0, 0x01, 0xe0, 0x4c, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x71, 0x18, 0x00,
    0x03, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00,
    0x03, 0x80, 0x00, 0x00, 0x00, 0x50, 0x00,
    0x00, 0xc0, 0x00, 0x00, 0x00, 0xe0, 0x00,
    0x00, 0x60, 0x00, 0x00, 0x03, 0x80, 0x00,
    0x00, 0x78, 0x00, 0x00, 0x05, 0x00, 0x00,
    0x00, 0x1c, 0x00, 0x00, 0x1e, 0x00, 0x00,
    0x00, 0x07, 0x80, 0x00, 0x70, 0x00, 0x00,
    0x00, 0x01, 0xf4, 0x05, 0xd0, 0x00, 0x00,
    0x00, 0x00, 0x5f, 0xff, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0b, 0xe8, 0x00, 0x00, 0x00
};

extern volatile uint8_t f_SleepMode;
extern volatile uint16_t SecondsSleepMode_CNT;
extern volatile uint8_t f_DrawFrameSleepMode;
extern volatile uint16_t SecondsStartSleepMode_CNT;
extern volatile uint16_t SecondsStartWatering_CNT;
extern volatile uint8_t f_AdcEndOfConversion;
extern volatile uint8_t f_FinishCheckLeak;

//--------------------------------------------------------


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_CRC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM5_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//============================================================================
//	Micro SD
//============================================================================

// Чтение параметров с MicroSD
uint8_t SD_ReadData(uint8_t num_data)
{
	// Название файла
	char* file_name;

	// Объект файла
	FIL file;

	// Статусы операций
	// при работе с Micro SD
	FRESULT status;

	// Указатель на структуру для чтения
	char* ptr_struct;

	// Размер структуры в байтах
	uint8_t size = 0;

	// Количество прочитанных байт
	UINT bytes_read;

	// Рассчитанное значение CRC32
	uint32_t crc = 0;

	// Количество слов (по 32 байта) для расчёта CRC32
	uint8_t crc_num_words = 0;

	// Указатель на структуру для сравнения CRC32
	uint32_t* ptr_struct_32;


	if (num_data == 1)
	{
		file_name = "WateringParams.txt";
		file = file_WateringParams;
		ptr_struct = ptr_WateringParams;
		ptr_struct_32 = (uint32_t*) ptr_WateringParams;
		size = sizeof(WateringParams);
		crc_num_words = 2;
	}

	else
	if (num_data == 2)
	{
		file_name = "ScreenOff.txt";
		file = file_ScreenOff;
		ptr_struct = ptr_ScreenOff;
		ptr_struct_32 = (uint32_t*) ptr_ScreenOff;
		size = sizeof(ScreenOff);
		crc_num_words = 1;
	}

	else
	if (num_data == 3)
	{
		file_name = "WaterLevelSensorState.txt";
		file = file_WaterLevelSensorState;
		ptr_struct = ptr_WaterLevelSensorState;
		ptr_struct_32 = (uint32_t*) ptr_WaterLevelSensorState;
		size = sizeof(WaterLevelSensorState);
		crc_num_words = 1;
	}

	else
		return 1;

	// Открытие файла на чтение
	status = f_open(&file, file_name, FA_READ);

	if (status != FR_OK)
	{
		f_close(&file);
		f_ErrorSD = 2;
		return 2;
	}

	// Чтение файла в структуру
	status = f_read(&file, ptr_struct, size, &bytes_read);

	// Закрытие файла
	f_close(&file);

	if (status != FR_OK)
	{
		f_ErrorSD = 3;
		return 3;
	}

	// Расчёт CRC32
	crc = HAL_CRC_Calculate(&hcrc, (uint32_t*) ptr_struct, crc_num_words);

	// Если CRC32 не сходятся
	if (crc != (*(ptr_struct_32 + crc_num_words)))
	{
		// Очистка структуры
		memset(ptr_struct, 0, size);
		return 4;
	}

	return 0;
}


// Запись параметров на MicroSD
uint8_t SD_WriteData(uint8_t num_data)
{
	// Название файла
	char* file_name;

	// Объект файла
	FIL file;

	// Статусы операций
	// при работе с Micro SD
	FRESULT status;

	// Указатель на структуру для записи
	char* ptr_struct;

	// Размер структуры в байтах
	uint8_t size = 0;

	// Количество записанных байт
	UINT bytes_wrote;

	// Количество слов (по 32 байта) для расчёта CRC32
	uint8_t crc_num_words = 0;

	// Указатель на структуру для расчёта CRC32
	uint32_t* ptr_struct_32;

	// Смещение поля bytes_alignment
	// относительно адреса начала структуры
	uint8_t bytes_alignment_shift = 0;


	if (num_data == 1)
	{
		file_name = "WateringParams.txt";
		file = file_WateringParams;
		ptr_struct = ptr_WateringParamsSet;
		ptr_struct_32 = (uint32_t*) ptr_WateringParamsSet;
		size = sizeof(WateringParamsSet);
		crc_num_words = 2;
		bytes_alignment_shift = 6;
	}

	else
	if (num_data == 2)
	{
		file_name = "ScreenOff.txt";
		file = file_ScreenOff;
		ptr_struct = ptr_ScreenOffSet;
		ptr_struct_32 = (uint32_t*) ptr_ScreenOffSet;
		size = sizeof(ScreenOffSet);
		crc_num_words = 1;
		bytes_alignment_shift = 2;
	}

	else
	if (num_data == 3)
	{
		file_name = "WaterLevelSensorState.txt";
		file = file_WaterLevelSensorState;
		ptr_struct = ptr_WaterLevelSensorState;
		ptr_struct_32 = (uint32_t*) ptr_WaterLevelSensorState;
		size = sizeof(WaterLevelSensorState);
		crc_num_words = 1;
	}

	else
		return 1;

	// У структуры WaterLevelSensorState
	// нет поля bytes_alignment_shift
	if (num_data != 3)
	{
		// Очистка поля bytes_alignment структуры
		memset(ptr_struct + bytes_alignment_shift, 0, 2);
	}

	// Расчёт CRC32 и заполнение поля crc32 структуры
	*(ptr_struct_32 + crc_num_words) = HAL_CRC_Calculate(&hcrc, (uint32_t*) ptr_struct, crc_num_words);

	// Открытие файла на запись
	status = f_open(&file, file_name, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);

	if (status != FR_OK)
	{
		f_close(&file);
		f_ErrorSD = 4;
		return 2;
	}

	// Запись структуры в файл
	status = f_write(&file, ptr_struct, size, &bytes_wrote);

	// Закрытие файла
	f_close(&file);

	if (status != FR_OK)
	{
		f_ErrorSD = 5;
		return 3;
	}

	return 0;
}


// Чтение настроек,
// если они были изменены
void ReadFlashParams(void)
{
	 if (f_WateringParamsChanged)
	 {
		 SD_ReadData(1);
		 f_WateringParamsChanged = 0;
	 }

	 if (f_ScreenOffChanged)
	 {
		 SD_ReadData(2);

		 if (!f_SleepMode)
		 {
			 // Сброс секунд бездействия пользователя
			 SecondsSleepMode_CNT = 0;

			 // Автоотключение экрана было выключено
			 if ((!ScreenOff.minutes_screen_off) &&
				 (!ScreenOff.seconds_screen_off))
			 {
				// Сброс времени автоотключения экрана
				SecondsSleepMode_ARR = 0;

				// Сброс и выключение таймера 3
				HAL_TIM_Base_Stop_IT(&htim3);
				TIM3->CNT = 0;
			 }

			 else
			 {
				 // Обновление времени автоотключения экрана в секундах
				 SecondsSleepMode_ARR = ScreenOff.minutes_screen_off * 60 + ScreenOff.seconds_screen_off;

				 // Сброс и включение таймера 3
				 TIM3->CNT = 0;
				 HAL_TIM_Base_Start_IT(&htim3);
			 }
		 }

		 f_ScreenOffChanged = 0;
	 }

	 if (f_WaterLevelSensorStateChanged)
	 {
		 SD_ReadData(3);
		 f_WaterLevelSensorStateChanged = 0;
	 }
}


// Чтение настроек полива
// и времени автоотключения экрана
// при включении
void InitReadFlashParams(void)
{
	SD_ReadData(1);
	SD_ReadData(2);
	SD_ReadData(3);
}


//============================================================================
//	RTC DS3231
//============================================================================

// Преобразование чисел из десятичной системы в двоичную
uint8_t decToBcd(int val)
{
  return (uint8_t) ((val/10*16) + (val%10));
}


// Преобразование чисел из двоичной системы в десятичную
int bcdToDec(uint8_t val)
{
  return (int) ((val/16*10) + (val%16));
}


// Установка даты и дня недели
static void SetDateDS(uint8_t dow, uint8_t dom,
						 uint8_t month, uint8_t year)
{
	uint8_t set_time[4];

	set_time[0] = decToBcd(dow);
	set_time[1] = decToBcd(dom);
	set_time[2] = decToBcd(month);
	set_time[3] = decToBcd(year);

	HAL_I2C_Mem_Write(&hi2c2, DS3231_ADDRESS, 0x03, 1, set_time, 4, 1000);
}


// Установка времени
static void SetTimeDS(uint8_t sec, uint8_t min, uint8_t hour)
{
	uint8_t set_time[3];

	set_time[0] = decToBcd(sec);
	set_time[1] = decToBcd(min);
	set_time[2] = decToBcd(hour);

	HAL_I2C_Mem_Write(&hi2c2, DS3231_ADDRESS, 0x00, 1, set_time, 3, 1000);
}


// Чтение текущего времени с DS3231
void Get_Time(void)
{
	uint8_t get_time[7];

	HAL_I2C_Mem_Read(&hi2c2, DS3231_ADDRESS, 0x00, 1, get_time, 7, 1000);

	time.seconds = bcdToDec(get_time[0]);
	time.minutes = bcdToDec(get_time[1]);
	time.hour = bcdToDec(get_time[2]);
	time.dayofweek = bcdToDec(get_time[3]);
	time.dayofmonth = bcdToDec(get_time[4]);
	time.month = bcdToDec(get_time[5]);
	time.year = bcdToDec(get_time[6]);
}


// Чтение текущей температуры с DS3231
float Get_Temp(void)
{
	uint8_t temp[2];

	HAL_I2C_Mem_Read(&hi2c2, DS3231_ADDRESS, 0x11, 1, temp, 2, 1000);
	return ((temp[0])+(temp[1]>>6)/4.0);
}


// Увеличение скорости обновления температуры
void force_temp_conv(void)
{
	uint8_t status=0;
	uint8_t control=0;

	// Чтение status register
	HAL_I2C_Mem_Read(&hi2c2, DS3231_ADDRESS, 0x0F, 1, &status, 1, 100);

	if (!(status&0x04))
	{
		// Чтение control register
		HAL_I2C_Mem_Read(&hi2c2, DS3231_ADDRESS, 0x0E, 1, &control, 1, 100);

		// Запись в control register
		HAL_I2C_Mem_Write(&hi2c2, DS3231_ADDRESS, 0x0E, 1, (uint8_t *)(control|(0x20)), 1, 100);
	}
}


//============================================================================
//	Модифицированные процедуры из файла OLED.c
//============================================================================

// Модифицированная процедура отрисовки строки (русский)
void OLED_DrawStr_RuModified(const char *str, uint8_t x, uint8_t y, uint8_t mode)
{
	FontSet(Segoe_UI_Rus_10);
	OLED_DrawStr(str, x, y, mode);
}


// Модифицированная процедура отрисовки строки (английский)
void OLED_DrawStr_EnModified(const char *str, uint8_t x, uint8_t y, uint8_t mode)
{
	FontSet(Segoe_UI_Eng_10);
	OLED_DrawStr(str, x, y, mode);
}


// Модифицированная процедура отрисовки символа (ASCII)
void OLED_DrawChar_Modified(char c, uint8_t ix, uint8_t iy, uint8_t mode)
{
	FontSet(Segoe_UI_Eng_10);
	OLED_DrawChar(c, ix, iy, mode);
}


// Модифицированная процедура отрисовки линии
// (позволяет выбрать цвет линии через параметр mode)
void OLED_DrawLine_Modified(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t mode)
{
	uint16_t curr_dx = ABS(x1 - x0);
	uint16_t curr_dy = ABS(y1 - y0);
	uint8_t step = curr_dy > curr_dx;

	if (step)
	{
		SWAP(x0, y0);
		SWAP(x1, y1);
		SWAP(curr_dx, curr_dy);
	}

	if (x0 > x1)
	{
		SWAP(x0, x1);
		SWAP(y0, y1);
	}

	int16_t y_step = (y0 < y1) ? 1 : -1;
	int16_t err = curr_dx >> 1;

	for ( ; x0 <= x1; x0++)
	{
		if (step)	OLED_DrawPixelStatus(y0, x0, mode);
		else		OLED_DrawPixelStatus(x0, y0, mode);

		err -= curr_dy;
		if (err < 0)
		{
			y0 += y_step;
			err += curr_dx;
		}
	}
}


// Модифицированная процедура отрисовки треугольника с заполнением
// (позволяет выбрать цвет заполнения через параметр mode)
void OLED_DrawTriangleFill_Modified(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint8_t mode)
{
	int16_t delta_x = 0, delta_y = 0;
	int16_t	x = 0, y = 0, x_inc_1 = 0, x_inc_2 = 0, y_inc_1 = 0, y_inc_2 = 0;
	int16_t	den = 0, num = 0, num_add = 0, num_pixels = 0;

	delta_x = ABS(x2 - x1);
	delta_y = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1)	{	x_inc_1 = 1;	x_inc_2 = 1;	}
	else 			{	x_inc_1 = -1;	x_inc_2 = -1;	}

	if (y2 >= y1) 	{	y_inc_1 = 1;	y_inc_2 = 1;	}
	else 			{	y_inc_1 = -1;	y_inc_2 = -1;	}

	if (delta_x >= delta_y)
	{
		x_inc_1 = 0;
		y_inc_2 = 0;
		den = delta_x;
		num = delta_x / 2;
		num_add = delta_y;
		num_pixels = delta_x;
	}
	else
	{
		x_inc_2 = 0;
		y_inc_1 = 0;
		den = delta_y;
		num = delta_y / 2;
		num_add = delta_x;
		num_pixels = delta_y;
	}

	for (int16_t cur_pixel = 0; cur_pixel <= num_pixels; cur_pixel++)
	{
		OLED_DrawLine_Modified(x, y, x3, y3, mode);

		num += num_add;
		if (num >= den)
		{
			num -= den;
			x += x_inc_1;
			y += y_inc_1;
		}
		x += x_inc_2;
		y += y_inc_2;
	}
}


// Модифицированная процедура отрисовки круга без заполнения
// (позволяет выбрать цвет контура через параметр mode)
void OLED_DrawCircle_Modified(int16_t x0, int16_t y0, uint8_t r, uint8_t mode)
{
	int16_t x = 0;
	int16_t y = r;
	int16_t ddF_y = - (r << 1);
	int16_t ddF_x = 1;
	int16_t f = 1 - r;

	OLED_DrawPixelStatus(x0, y0 + r, mode);
	OLED_DrawPixelStatus(x0, y0 - r, mode);
	OLED_DrawPixelStatus(x0 + r, y0, mode);
	OLED_DrawPixelStatus(x0 - r, y0, mode);

	while(x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		OLED_DrawPixelStatus(x0 + x, y0 + y, mode);
		OLED_DrawPixelStatus(x0 - x, y0 + y, mode);
		OLED_DrawPixelStatus(x0 + x, y0 - y, mode);
		OLED_DrawPixelStatus(x0 - x, y0 - y, mode);
		OLED_DrawPixelStatus(x0 + y, y0 + x, mode);
		OLED_DrawPixelStatus(x0 - y, y0 + x, mode);
		OLED_DrawPixelStatus(x0 + y, y0 - x, mode);
		OLED_DrawPixelStatus(x0 - y, y0 - x, mode);
	}
}


// Модифицированная процедура отрисовки круга с заполнением
// (позволяет выбрать цвет заполнения через параметр mode)
void OLED_DrawCircleFill_Modified(int16_t x0, int16_t y0, uint8_t r, uint8_t mode)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	OLED_DrawPixel(x0, y0 + r);
	OLED_DrawPixel(x0, y0 - r);
	OLED_DrawPixel(x0 + r, y0);
	OLED_DrawPixel(x0 - r, y0);
	OLED_DrawLine(x0 - r, y0, x0 + r, y0);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		OLED_DrawLine_Modified(x0 - x, y0 + y, x0 + x, y0 + y, mode);
		OLED_DrawLine_Modified(x0 + x, y0 - y, x0 - x, y0 - y, mode);

		OLED_DrawLine_Modified(x0 + y, y0 + x, x0 - y, y0 + x, mode);
		OLED_DrawLine_Modified(x0 + y, y0 - x, x0 - y, y0 - x, mode);
	}
}


//============================================================================
//	Процедуры кадра
//============================================================================

// Отрисовка текущего дня недели
// (параметр mode определяет цвет)
void DrawDayOfWeek(uint8_t mode)
{
	switch(time.dayofweek)
	{
		// Понедельник
		case 1:
				OLED_DrawStr_RuModified("Gjytltkmybr", 12, 13 * 3, mode);
		break;


		// Вторник
		case 2:
				OLED_DrawStr_RuModified("Dnjhybr", 12, 13 * 3, mode);
		break;


		// Среда
		case 3:
				OLED_DrawStr_RuModified("Chtlf", 12, 13 * 3, mode);
		break;


		// Четверг
		case 4:
				OLED_DrawStr_RuModified("Xtndthu", 12, 13 * 3, mode);
		break;


		// Пятница
		case 5:
				OLED_DrawStr_RuModified("Gznybwf", 12, 13 * 3, mode);
		break;


		// Суббота
		case 6:
				OLED_DrawStr_RuModified("Ce,,jnf", 12, 13 * 3, mode);
		break;


		// Воскресенье
		case 7:
				OLED_DrawStr_RuModified("Djcrhtctymt", 12, 13 * 3, mode);
		break;


		default:
		break;
	}
}


// Отрисовка текущего времени
// (параметр mode определяет цвет)
void DrawTime(uint8_t mode)
{
	// Длина числа месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_1 = 0;

	// Длина номера месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_2 = 0;

	// Длина символа двоеточия в пикселях
	uint8_t char_length = OLED_GetWidthStr("/");

	// Смещение для нарисованных нулей
	uint8_t num_length_zero[3] = {0};


	// �?значально время установлено как 0.
	// Но формат времени предполагает, что должно быть отрисовано 2 нуля.
	// Поэтому если первая цифра не установлена,
	// то нужно дорисовать ноль самостоятельно
	if (time.hour < 10)
	{
		num_length_zero[0] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 12 + 40, 13 * 1, mode);
	}

	OLED_DrawNum((int16_t) time.hour, 12 + num_length_zero[0] + 40, 13 * 1, mode);
	num_length_1 = OLED_GetWidthNum(time.hour);

	OLED_DrawChar_Modified(':', 12 + num_length_zero[0] + num_length_1 + 2 + 40, 13 * 1, mode);

	if (time.minutes < 10)
	{
		num_length_zero[1] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 12 + num_length_zero[0] + num_length_1 + char_length + 40 , 13 * 1, mode);
	}

	OLED_DrawNum((int16_t) time.minutes, 12 + num_length_zero[0] + num_length_1 + char_length + num_length_zero[1] + 40, 13 * 1, mode);
	num_length_2 = OLED_GetWidthNum(time.minutes);

	OLED_DrawChar_Modified(':', 12 + num_length_1 + num_length_zero[0] + char_length + num_length_2 + num_length_zero[1] + 2 + 40, 13 * 1, mode);

	if (time.seconds < 10)
	{
		num_length_zero[2] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 12 + num_length_1 + num_length_zero[0] + char_length * 2 + num_length_2 + num_length_zero[1] + 40, 13 * 1, mode);
	}

	OLED_DrawNum((int16_t) time.seconds, 12 + num_length_1 + num_length_zero[0] + char_length * 2 + num_length_2 + num_length_zero[1] + num_length_zero[2] + 40, 13 * 1, mode);
}


// Отрисовка текущей даты
// (параметр mode определяет цвет)
void DrawDate(uint8_t mode)
{
	// Длина числа месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_1 = 0;

	// Длина номера месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_2 = 0;

	// Длина символа дроби в пикселях
	uint8_t char_length = OLED_GetWidthStr("/");

	// Смещение для нарисованных нулей
	uint8_t num_length_zero[3] = {0};


	// Изначально число месяца установлено как 0.
	// Но формат даты предполагает, что должно быть отрисовано 2 нуля.
	// Поэтому если первая цифра не установлена,
	// то нужно дорисовать ноль самостоятельно
	if (time.dayofmonth < 10)
	{
		num_length_zero[0] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 12 + 40, 13 * 0, mode);
	}

	OLED_DrawNum((int16_t) time.dayofmonth, 12 + num_length_zero[0] + 40, 13 * 0, mode);
	num_length_1 = OLED_GetWidthNum(time.dayofmonth);

	OLED_DrawChar('/', 12 + num_length_zero[0] + num_length_1 + 40, 13 * 0, mode);

	if (time.month < 10)
	{
		num_length_zero[1] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 12 + num_length_zero[0] + num_length_1 + char_length + 40, 13 * 0, mode);
	}

	OLED_DrawNum((int16_t) time.month, 12 + num_length_zero[0] + num_length_1 + char_length + num_length_zero[1] + 40, 13 * 0, mode);
	num_length_2 = OLED_GetWidthNum(time.month);

	OLED_DrawChar('/', 12 + num_length_1 + num_length_zero[0] + char_length + num_length_2 + num_length_zero[1] + 40, 13 * 0, mode);

	if (time.year < 10)
	{
		num_length_zero[2] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 12 + num_length_1 + num_length_zero[0] + char_length * 2 + num_length_2 + num_length_zero[1] + 40, 13 * 0, mode);
	}

	OLED_DrawNum((int16_t) time.year, 12 + num_length_1 + num_length_zero[0] + char_length * 2 + num_length_2 + num_length_zero[1] + num_length_zero[2] + 40, 13 * 0, mode);
}


// Сборка даты при настройке
void BuildSetDate(void)
{
	timeSetDate.dayofmonth = NumberSetDate[0] * 10 + NumberSetDate[1];
	timeSetDate.month = NumberSetDate[2] * 10 + NumberSetDate[3];
	timeSetDate.year = NumberSetDate[4] * 10 + NumberSetDate[5];
}


// Отрисовка даты при настройке
// (параметр mode определяет цвет)
void DrawSetDate(uint8_t mode)
{
	// Длина числа месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_1 = 0;

	// Длина номера месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_2 = 0;

	// Длина символа дроби в пикселях
	uint8_t char_length = OLED_GetWidthStr("/");

	// Смещение для нарисованных нулей
	uint8_t num_length_zero[3] = {0};


	// �?значально число месяца установлено как 0.
	// Но формат даты предполагает, что должно быть отрисовано 2 нуля.
	// Поэтому если первая цифра не установлена,
	// то нужно дорисовать ноль самостоятельно
	if (timeSetDate.dayofmonth < 10)
	{
		num_length_zero[0] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 16, 13 * 2, mode);
	}

	OLED_DrawNum((int16_t) timeSetDate.dayofmonth, 16 + num_length_zero[0], 13 * 2, mode);
	num_length_1 = OLED_GetWidthNum(timeSetDate.dayofmonth);

	OLED_DrawChar('/', 16 + num_length_zero[0] + num_length_1, 13 * 2, mode);

	if (timeSetDate.month < 10)
	{
		num_length_zero[1] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 16 + num_length_zero[0] + num_length_1 + char_length, 13 * 2, mode);
	}

	OLED_DrawNum((int16_t) timeSetDate.month, 16 + num_length_zero[0] + num_length_1 + char_length + num_length_zero[1], 13 * 2, mode);
	num_length_2 = OLED_GetWidthNum(timeSetDate.month);

	OLED_DrawChar('/', 16 + num_length_1 + num_length_zero[0] + char_length + num_length_2 + num_length_zero[1], 13 * 2, mode);

	if (timeSetDate.year < 10)
	{
		num_length_zero[2] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 16 + num_length_1 + num_length_zero[0] + char_length * 2 + num_length_2 + num_length_zero[1], 13 * 2, mode);
	}

	OLED_DrawNum((int16_t) timeSetDate.year, 16 + num_length_1 + num_length_zero[0] + char_length * 2 + num_length_2 + num_length_zero[1] + num_length_zero[2], 13 * 2, mode);
}


// Настройка даты
void SetDate(void)
{
	// Сбор текущей даты
	BuildSetDate();

	// Вывод даты на экран
	DrawSetDate(1);
}


// Сборка дня недели при настройке
void BuildSetDayOfWeek(void)
{
	timeSetDate.dayofweek = NumberSetDayOfWeek;
}


// Отрисовка дня недели при установке
// (параметр mode определяет цвет)
void DrawSetDayOfWeek(uint8_t mode)
{
	switch(timeSetDate.dayofweek)
	{
		// Понедельник
		case 1:
				OLED_DrawStr_RuModified("Gjytltkmybr", 12, 13 * 2, mode);
		break;


		// Вторник
		case 2:
				OLED_DrawStr_RuModified("Dnjhybr", 12, 13 * 2, mode);
		break;


		// Среда
		case 3:
				OLED_DrawStr_RuModified("Chtlf", 12, 13 * 2, mode);
		break;


		// Четверг
		case 4:
				OLED_DrawStr_RuModified("Xtndthu", 12, 13 * 2, mode);
		break;


		// Пятница
		case 5:
				OLED_DrawStr_RuModified("Gznybwf", 12, 13 * 2, mode);
		break;


		// Суббота
		case 6:
				OLED_DrawStr_RuModified("Ce,,jnf", 12, 13 * 2, mode);
		break;


		// Воскресенье
		case 7:
				OLED_DrawStr_RuModified("Djcrhtctymt", 12, 13 * 2, mode);
		break;


		default:
		break;
	}
}


// Настройка дня недели
void SetDayOfWeek(void)
{
	// Сборка дня
	// недели при настройке
	BuildSetDayOfWeek();

	// Вывод дня недели на экран
	DrawSetDayOfWeek(1);
}


// Сбор времени при настройке
void BuildSetTime(void)
{
	switch(current_frame)
	{
		// Кадр настройки
		// времени начала полива
		case 7:

			WateringParamsSet.hour = NumberSetTimeWater[0] * 10 + NumberSetTimeWater[1];
			WateringParamsSet.minutes = NumberSetTimeWater[2] * 10 + NumberSetTimeWater[3];
			WateringParamsSet.seconds = NumberSetTimeWater[4] * 10 + NumberSetTimeWater[5];

		break;

		// Кадр настройки
		// продолжительности полива
		case 8:

			WateringParamsSet.minutes_watering = NumberSetDurationWater[0] * 10 + NumberSetDurationWater[1];
			WateringParamsSet.seconds_watering = NumberSetDurationWater[2] * 10 + NumberSetDurationWater[3];

		break;

		// Кадр настройки времени
		case 14:

			timeSetDate.hour = NumberSetTime[0] * 10 + NumberSetTime[1];
			timeSetDate.minutes = NumberSetTime[2] * 10 + NumberSetTime[3];
			timeSetDate.seconds = NumberSetTime[4] * 10 + NumberSetTime[5];

		break;

		default:
		break;
	}
}


// Отрисовка времени при настройке
// (параметр ptr_struct - указатель на структуру
// timeSetDate или WateringParamsSet)
void DrawSetTime(char* ptr_struct, uint8_t mode)
{
	// Длина числа месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_1 = 0;

	// Длина номера месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_2 = 0;

	// Длина символа в пикселях
	uint8_t char_length = OLED_GetWidthStr("/");

	// Смещение для нарисованных нулей
	uint8_t num_length_zero[3] = {0};

	// Массив, хранящий значения:
	// [0] - секунд, [1] - минут,
	// [2] - часов (для кадров 7, 14)
	uint8_t temp_arr[3] = {0};


	// Копирование данных в массив temp_arr
	memcpy((char*) &temp_arr, ptr_struct, 3);

	// Изначально время установлено как 0.
	// Но формат времени предполагает, что должно быть отрисовано 2 нуля.
	// Поэтому если первая цифра не установлена,
	// то нужно дорисовать ноль самостоятельно
	if (temp_arr[2] < 10)
	{
		num_length_zero[0] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 16, 13 * 2, mode);
	}

	OLED_DrawNum((int16_t) temp_arr[2], 16 + num_length_zero[0], 13 * 2, mode);
	num_length_1 = OLED_GetWidthNum(temp_arr[2]);

	OLED_DrawChar_Modified(':', 16 + num_length_zero[0] + num_length_1 + 2, 13 * 2, mode);

	if (temp_arr[1] < 10)
	{
		num_length_zero[1] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 16 + num_length_zero[0] + num_length_1 + char_length, 13 * 2, mode);
	}

	OLED_DrawNum((int16_t) temp_arr[1], 16 + num_length_zero[0] + num_length_1 + char_length + num_length_zero[1], 13 * 2, mode);
	num_length_2 = OLED_GetWidthNum(temp_arr[1]);

	OLED_DrawChar_Modified(':', 16 + num_length_1 + num_length_zero[0] + char_length + num_length_2 + num_length_zero[1] + 2, 13 * 2, mode);

	if (temp_arr[0] < 10)
	{
		num_length_zero[2] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 16 + num_length_1 + num_length_zero[0] + char_length * 2 + num_length_2 + num_length_zero[1], 13 * 2, mode);
	}

	OLED_DrawNum((int16_t) temp_arr[0], 16 + num_length_1 + num_length_zero[0] + char_length * 2 + num_length_2 + num_length_zero[1] + num_length_zero[2], 13 * 2, mode);
}


// Отрисовка продолжительности полива при настройке
void DrawSetDurationWater(char* ptr_struct, uint8_t mode)
{
	// Длина числа месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_1 = 0;

	// Длина номера месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_2 = 0;

	// Смещение для нарисованных нулей
	uint8_t num_length_zero[2] = {0};

	// Массив, хранящий значения:
	// [0] - секунд, [1] - минут (для кадров 8)
	uint8_t temp_arr[2] = {0};


	// Копирование данных в массив temp_arr
	memcpy((char*) &temp_arr, ptr_struct, 2);

	// Изначально время установлено как 0.
	// Но формат времени предполагает, что должно быть отрисовано 2 нуля.
	// Поэтому если первая цифра не установлена,
	// то нужно дорисовать ноль самостоятельно
	if (temp_arr[1] < 10)
	{
		num_length_zero[0] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 16, 13 * 2, mode);
	}

	OLED_DrawNum((int16_t) temp_arr[1], 16 + num_length_zero[0], 13 * 2, mode);
	num_length_1 = OLED_GetWidthNum(temp_arr[1]);

	OLED_DrawStr_RuModified("vby", 16 + num_length_zero[0] + num_length_1 + 2, 13 * 2, mode);

	if (temp_arr[0] < 10)
	{
		num_length_zero[1] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 3 + 16 + num_length_zero[0] + num_length_1 + OLED_GetWidthStr("vby") + 2 * 2, 13 * 2, mode);
	}

	OLED_DrawNum((int16_t) temp_arr[0], 3 + 16 + num_length_zero[0] + num_length_1 + OLED_GetWidthStr("vby") + 2 * 2 + num_length_zero[1], 13 * 2, mode);
	num_length_2 = OLED_GetWidthNum(temp_arr[0]);

	OLED_DrawStr_RuModified("ctr", 3 + 16 + num_length_zero[0] + num_length_1 + OLED_GetWidthStr("vby") + 2 * 3 + num_length_zero[1] + num_length_2, 13 * 2, mode);
}


// Настройка времени
void SetTimeSMH(void)
{
	// Сбор текущего времени
	BuildSetTime();

	switch(current_frame)
	{
		// Кадр настройки
		// времени начала полива
		case 7:

			// Вывод времени на экран
			DrawSetTime(ptr_WateringParamsSet, 1);

		break;

		// Кадр настройки
		// продолжительности полива
		case 8:

			// Вывод времени на экран (без часов)
			DrawSetDurationWater(ptr_WateringParamsSet + 3, 1);

		break;

		// Кадр настройки времени
		case 14:

			// Вывод времени на экран
			DrawSetTime(ptr_timeSetDate, 1);

		break;
	}
}


// Сбор текущего времени
// автоотключения экрана
void BuildSetScreenAutoOff(void)
{
	ScreenOffSet.minutes_screen_off = NumberSetScreenOff[0] * 10 + NumberSetScreenOff[1];
	ScreenOffSet.seconds_screen_off = NumberSetScreenOff[2] * 10 + NumberSetScreenOff[3];
}


// Отрисовка времени при установке времени автоотключения экрана
// (параметр ptr_struct - указатель на структуру)
void DrawSetScreenOff(char* ptr_struct, uint8_t mode)
{
	// Длина числа месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_1 = 0;

	// Длина номера месяца в пикселях
	// (для расчёта смещения)
	uint8_t num_length_2 = 0;

	// Смещение для нарисованных нулей
	uint8_t num_length_zero[2] = {0};

	// Массив, хранящий значения:
	// [0] - секунд, [1] - минут (для кадра 23)
	uint8_t temp_arr[2] = {0};


	// Копирование данных в массив temp_arr
	memcpy((char*) &temp_arr, ptr_struct, 2);

	// Изначально время установлено как 0.
	// Но формат времени предполагает, что должно быть отрисовано 2 нуля.
	// Поэтому если первая цифра не установлена,
	// то нужно дорисовать ноль самостоятельно
	if (temp_arr[1] < 10)
	{
		num_length_zero[0] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 16, 13 * 3, mode);
	}

	OLED_DrawNum((int16_t) temp_arr[1], 16 + num_length_zero[0], 13 * 3, mode);
	num_length_1 = OLED_GetWidthNum(temp_arr[1]);

	OLED_DrawStr_RuModified("vby", 16 + num_length_zero[0] + num_length_1 + 2, 13 * 3, mode);

	if (temp_arr[0] < 10)
	{
		num_length_zero[1] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 3 + 16 + num_length_zero[0] + num_length_1 + OLED_GetWidthStr("vby") + 2 * 2, 13 * 3, mode);
	}

	OLED_DrawNum((int16_t) temp_arr[0], 3 + 16 + num_length_zero[0] + num_length_1 + OLED_GetWidthStr("vby") + 2 * 2 + num_length_zero[1], 13 * 3, mode);
	num_length_2 = OLED_GetWidthNum(temp_arr[0]);

	OLED_DrawStr_RuModified("ctr", 3 + 16 + num_length_zero[0] + num_length_1 + OLED_GetWidthStr("vby") + 2 * 3 + num_length_zero[1] + num_length_2, 13 * 3, mode);
}


// Настройка времени автоотключения экрана
void SetScreenOff(void)
{
	// Сбор текущего времени
	// автоотключения экрана
	BuildSetScreenAutoOff();

	// Вывод времени автоотключения на экран (без часов)
	DrawSetScreenOff(ptr_ScreenOffSet, 1);
}


// Определение времени
// автоотключения экрана при включении
void InitScreenOff(void)
{
	// Автоотключение экрана
	// выключено или не настроено
	if ((!ScreenOff.minutes_screen_off) &&
        (!ScreenOff.seconds_screen_off))
	{
		// Выключение таймера 3
		HAL_TIM_Base_Stop_IT(&htim3);
	}

	// Если время автоотключения экрана установлено
	else
	{
		// Время автоотключения экрана в секундах
		SecondsSleepMode_ARR = ScreenOff.minutes_screen_off * 60 + ScreenOff.seconds_screen_off;

		// Включение таймера 3
		HAL_TIM_Base_Start_IT(&htim3);
	}
}


// Сбор текущих дней полива при настройке
void BuildSetDaysWater(void)
{
	WateringParamsSet.daysofweek = WaterDays;
}


// Отрисовка дней полива при настройке
void DrawSetDaysWater(uint8_t mode)
{
	uint8_t i = 0;
	uint8_t mask = 0x01;

	/*
	 * 	Логика отрисовки:
	 *  Для отображения выбранных дней полива
	 *  на дисплее отрисовываются круги:
	 *
	 *  - если день выбран для полива,
	 *    то круг должен быть заполнен;
	 *
	 *  - если день не выбран для полива,
	 *    то круг должен быть пустым
	 *    (отрисовывается только контур круга).
	 *
	 *	Дни полива хранятся в переменной WaterDays
	 *	типа uint8_t. На каждый день отводится один бит
	 *	(0 - день не выбран для полива,
	 *	 1 - день выбран для полива).
	 *	 Нумерация дней по возрастанию битов, т.е.
	 *	 бит 0 - понедельник ... бит 6 - воскресенье.
	 */

	for (i = 0; i < 7; i++)
	{
		if (i != 0)
			mask <<= 1;

		// Если текущий день выбран для полива,
		// то отрисовывается заполненный круг
		if (WateringParamsSet.daysofweek & mask)
		{
			switch(mask)
			{
				// Понедельник
				case 0x01:
							OLED_DrawCircleFill_Modified(8 + 12, 13 * 3.5, 3, mode);
				break;

				// Вторник
				case 0x02:
							OLED_DrawCircleFill_Modified(6 + 12 + OLED_GetWidthStr("Gy") + 3, 13 * 3.5, 3, mode);
				break;

				// Среда
				case 0x04:
							OLED_DrawCircleFill_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 2 + OLED_GetWidthStr("Dn"), 13 * 3.5, 3, mode);
				break;

				// Четверг
				case 0x08:
							OLED_DrawCircleFill_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 3 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch"), 13 * 3.5, 3, mode);
				break;

				// Пятница
				case 0x10:
							OLED_DrawCircleFill_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 4 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn"), 13 * 3.5, 3, mode);
				break;

				// Суббота
				case 0x20:
							OLED_DrawCircleFill_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 5 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn"), 13 * 3.5, 3, mode);
				break;

				// Воскресенье
				case 0x40:
							OLED_DrawCircleFill_Modified(7 + 12 + OLED_GetWidthStr("Gy") + 3 * 6 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn") + OLED_GetWidthStr("C,"), 13 * 3.5, 3, mode);
				break;
			}
		}

		// Если текущий день не выбран для полива,
		// то отрисовывается пустой круг
		else
		{
			switch(mask)
			{
				// Понедельник
				case 0x01:
							OLED_DrawCircle_Modified(8 + 12, 13 * 3.5, 3, mode);
				break;

				// Вторник
				case 0x02:
							OLED_DrawCircle_Modified(6 + 12 + OLED_GetWidthStr("Gy") + 3, 13 * 3.5, 3, mode);
				break;

				// Среда
				case 0x04:
							OLED_DrawCircle_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 2 + OLED_GetWidthStr("Dn"), 13 * 3.5, 3, mode);
				break;

				// Четверг
				case 0x08:
							OLED_DrawCircle_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 3 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch"), 13 * 3.5, 3, mode);
				break;

				// Пятница
				case 0x10:
							OLED_DrawCircle_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 4 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn"), 13 * 3.5, 3, mode);
				break;

				// Суббота
				case 0x20:
							OLED_DrawCircle_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 5 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn"), 13 * 3.5, 3, mode);
				break;

				// Воскресенье
				case 0x40:
							OLED_DrawCircle_Modified(7 + 12 + OLED_GetWidthStr("Gy") + 3 * 6 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn") + OLED_GetWidthStr("C,"), 13 * 3.5, 3, mode);
				break;
			}
		}
	}
}


// Настройка дней полива
void SetDaysWater(void)
{
	// Сбор текущих дней полива
	BuildSetDaysWater();

	// Вывод дней полива на экран
	DrawSetDaysWater(1);
}


// Отрисовка текущих параметров
// полива на кадре 1
void DrawWateringParamsPt1(void)
{
	// Смещение для нарисованных нулей
	uint8_t num_length_zero[5] = {0};


	// Время начала полива
	if (WateringParams.hour < 10)
	{
		num_length_zero[0] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 10 + 12 + OLED_GetWidthStr("gjkbdf") + 1, 13 * 1, 1);
	}

	OLED_DrawNum(WateringParams.hour, 10 + 12 + OLED_GetWidthStr("gjkbdf") + 1 + num_length_zero[0], 13 * 1, 1);
	OLED_DrawChar_Modified(':', 10 + 12 + OLED_GetWidthStr("gjkbdf") + OLED_GetWidthNum(WateringParams.hour) + 2 + num_length_zero[0], 13 * 1, 1);

	if (WateringParams.minutes < 10)
	{
		num_length_zero[1] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 10 + 12 + OLED_GetWidthStr("gjkbdf") + OLED_GetWidthNum(WateringParams.hour) + 2 + num_length_zero[0] + 3, 13 * 1, 1);
	}

	OLED_DrawNum(WateringParams.minutes, 10 + 12 + OLED_GetWidthStr("gjkbdf") + OLED_GetWidthNum(WateringParams.hour) + 2 + num_length_zero[0] + 3 + num_length_zero[1], 13 * 1, 1);
	OLED_DrawChar_Modified(':', 10 + 12 + OLED_GetWidthStr("gjkbdf") + OLED_GetWidthNum(WateringParams.hour) + 2 + num_length_zero[0] + 3 + num_length_zero[1] + 2 + OLED_GetWidthNum(WateringParams.minutes) - 1, 13 * 1, 1);

	if (WateringParams.seconds < 10)
	{
		num_length_zero[2] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 10 + 12 + OLED_GetWidthStr("gjkbdf") + OLED_GetWidthNum(WateringParams.hour) + 2 + num_length_zero[0] + 3 + num_length_zero[1] + 2 + OLED_GetWidthNum(WateringParams.minutes) + 2, 13 * 1, 1);
	}

	OLED_DrawNum(WateringParams.seconds, 10 + 12 + OLED_GetWidthStr("gjkbdf") + OLED_GetWidthNum(WateringParams.hour) + 2 + num_length_zero[0] + 3 + num_length_zero[1] + 2 + OLED_GetWidthNum(WateringParams.minutes) + 2 + num_length_zero[2], 13 * 1, 1);


	// Продолжительность полива
	if (WateringParams.minutes_watering < 10)
	{
		num_length_zero[3] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 12, 13 * 4, 1);
	}

	OLED_DrawNum(WateringParams.minutes_watering, 12 + num_length_zero[3], 13 * 4, 1);
	OLED_DrawStr_RuModified("vby", 12 + num_length_zero[3] + OLED_GetWidthNum(WateringParams.minutes_watering) + 2, 13 * 4, 1);

	if (WateringParams.seconds_watering < 10)
	{
		num_length_zero[4] = OLED_GetWidthNum(0);
		OLED_DrawNum((int16_t) 0, 12 + num_length_zero[3] + OLED_GetWidthNum(WateringParams.minutes_watering) + 2 + OLED_GetWidthStr("vby") + 5, 13 * 4, 1);
	}

	OLED_DrawNum(WateringParams.seconds_watering, 12 + num_length_zero[3] + OLED_GetWidthNum(WateringParams.minutes_watering) + 2 + OLED_GetWidthStr("vby") + 5 + num_length_zero[4], 13 * 4, 1);
	OLED_DrawStr_RuModified("ctr", 12 + num_length_zero[3] + OLED_GetWidthNum(WateringParams.minutes_watering) + 2 + OLED_GetWidthStr("vby") + 5 + num_length_zero[4] + OLED_GetWidthNum(WateringParams.seconds_watering) + 2, 13 * 4, 1);
}


// Отрисовка текущих параметров
// полива на кадре 2
void DrawWateringParamsPt2(void)
{
	uint8_t i = 0;
	uint8_t mask = 0x01;

	/*
	 * 	Логика отрисовки:
	 *  Для отображения выбранных дней полива
	 *  на дисплее отрисовываются круги:
	 *
	 *  - если день выбран для полива,
	 *    то круг должен быть заполнен;
	 *
	 *  - если день не выбран для полива,
	 *    то круг должен быть пустым
	 *    (отрисовывается только контур круга).
	 *
	 *	Дни полива хранятся в переменной WaterDays
	 *	типа uint8_t. На каждый день отводится один бит
	 *	(0 - день не выбран для полива,
	 *	 1 - день выбран для полива).
	 *	 Нумерация дней по возрастанию битов, т.е.
	 *	 бит 0 - понедельник ... бит 6 - воскресенье.
	 */

	for (i = 0; i < 7; i++)
	{
		if (i != 0)
			mask <<= 1;

		// Если текущий день выбран для полива,
		// то отрисовывается заполненный круг
		if (WateringParams.daysofweek & mask)
		{
			switch(mask)
			{
				// Понедельник
				case 0x01:
							OLED_DrawCircleFill_Modified(8 + 12, 13 * 2.5, 3, 1);
				break;

				// Вторник
				case 0x02:
							OLED_DrawCircleFill_Modified(6 + 12 + OLED_GetWidthStr("Gy") + 3, 13 * 2.5, 3, 1);
				break;

				// Среда
				case 0x04:
							OLED_DrawCircleFill_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 2 + OLED_GetWidthStr("Dn"), 13 * 2.5, 3, 1);
				break;

				// Четверг
				case 0x08:
							OLED_DrawCircleFill_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 3 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch"), 13 * 2.5, 3, 1);
				break;

				// Пятница
				case 0x10:
							OLED_DrawCircleFill_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 4 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn"), 13 * 2.5, 3, 1);
				break;

				// Суббота
				case 0x20:
							OLED_DrawCircleFill_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 5 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn"), 13 * 2.5, 3, 1);
				break;

				// Воскресенье
				case 0x40:
							OLED_DrawCircleFill_Modified(7 + 12 + OLED_GetWidthStr("Gy") + 3 * 6 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn") + OLED_GetWidthStr("C,"), 13 * 2.5, 3, 1);
				break;
			}
		}

		// Если текущий день не выбран для полива,
		// то отрисовывается пустой круг
		else
		{
			switch(mask)
			{
				// Понедельник
				case 0x01:
							OLED_DrawCircle_Modified(8 + 12, 13 * 2.5, 3, 1);
				break;

				// Вторник
				case 0x02:
							OLED_DrawCircle_Modified(6 + 12 + OLED_GetWidthStr("Gy") + 3, 13 * 2.5, 3, 1);
				break;

				// Среда
				case 0x04:
							OLED_DrawCircle_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 2 + OLED_GetWidthStr("Dn"), 13 * 2.5, 3, 1);
				break;

				// Четверг
				case 0x08:
							OLED_DrawCircle_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 3 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch"), 13 * 2.5, 3, 1);
				break;

				// Пятница
				case 0x10:
							OLED_DrawCircle_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 4 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn"), 13 * 2.5, 3, 1);
				break;

				// Суббота
				case 0x20:
							OLED_DrawCircle_Modified(8 + 12 + OLED_GetWidthStr("Gy") + 3 * 5 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn"), 13 * 2.5, 3, 1);
				break;

				// Воскресенье
				case 0x40:
							OLED_DrawCircle_Modified(7 + 12 + OLED_GetWidthStr("Gy") + 3 * 6 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn") + OLED_GetWidthStr("C,"), 13 * 2.5, 3, 1);
				break;
			}
		}
	}
}


// Проверка того, что дата была установлена
void IsSetDateFinished(void)
{
	new_CondSetDate = f_StSetDate;

	if ((!new_CondSetDate) && (old_CondSetDate))
		f_DateIsSet = 1;

	old_CondSetDate = new_CondSetDate;
}


// Проверка того, что день недели был установлен
void IsSetDayOfWeekFinished(void)
{
	new_CondSetDayOfWeek = f_StSetDayOfWeek;

	if ((!new_CondSetDayOfWeek) && (old_CondSetDayOfWeek))
		f_DayOfWeekIsSet = 1;

	old_CondSetDayOfWeek = new_CondSetDayOfWeek;
}


// Проверка того, что время полива было установлено
void IsSetTimeWaterFinished(void)
{
	new_CondSetTimeWater = f_StSetTimeWater;

	if ((!new_CondSetTimeWater) && (old_CondSetTimeWater))
		f_TimeIsSetWater = 1;

	old_CondSetTimeWater = new_CondSetTimeWater;
}


// Проверка того, что продолжительность полива была установлена
void IsSetTimeDurationWaterFinished(void)
{
	new_CondSetDurationWater = f_StSetDurationWater;

	if ((!new_CondSetDurationWater) && (old_CondSetDurationWater))
		f_TimeIsSetDurationWater = 1;

	old_CondSetDurationWater = new_CondSetDurationWater;
}


// Проверка того, что дни полива были установлены
void IsSetTimeDaysWaterFinished(void)
{
	new_CondSetDaysWater = f_StSetDaysWater;

	if ((!new_CondSetDaysWater) && (old_CondSetDaysWater))
		f_DaysWaterIsSet = 1;

	old_CondSetDaysWater = new_CondSetDaysWater;
}


// Проверка того, что время было установлено
void IsSetTimeFinished(void)
{
	new_CondSetTime = f_StSetTime;

	if ((!new_CondSetTime) && (old_CondSetTime))
		f_TimeIsSet = 1;

	old_CondSetTime = new_CondSetTime;
}


// Проверка того, что время
// автоотключения экрана было установлено
void IsSetScreenOffFinished(void)
{
	new_CondSetScreenOff = f_StSetScreenOff;

	if ((!new_CondSetScreenOff) && (old_CondSetScreenOff))
		f_ScreenOffIsSet = 1;

	old_CondSetScreenOff = new_CondSetScreenOff;
}


// Экстренное завершение полива
void EmergencyTerminationOfWatering(void)
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

// Определение того, нужен ли сегодня полив
void IsTodayWateringDay(void)
{
	switch(time.dayofweek)
	{
		// Понедельник
		case 1:
				f_WaterDay = WateringParams.daysofweek & 0x01;
		break;

		// Вторник
		case 2:
				f_WaterDay = WateringParams.daysofweek & 0x02;
		break;

		// Среда
		case 3:
				f_WaterDay = WateringParams.daysofweek & 0x04;
		break;

		// Четверг
		case 4:
				f_WaterDay = WateringParams.daysofweek & 0x08;
		break;

		// Пятница
		case 5:
				f_WaterDay = WateringParams.daysofweek & 0x10;
		break;

		// Суббота
		case 6:
				f_WaterDay = WateringParams.daysofweek & 0x20;
		break;

		// Воскресенье
		case 7:
				f_WaterDay = WateringParams.daysofweek & 0x40;
		break;
	}
}


// Включение помпы (начало полива)
void IsTimeToStartWatering(void)
{
	// Определение того, нужен ли сегодня полив
	IsTodayWateringDay();

	// Если сегодня нужен полив
	if (f_WaterDay)
	{
		// Если наступило время полива
		if ((time.hour == WateringParams.hour) &&
			(time.minutes == WateringParams.minutes) &&
			(time.seconds == WateringParams.seconds))
		{
			// Если датчик уровня воды включен
			if (WaterLevelSensorState.f_IsWaterLevelSensorOn)
			{
				// Если в момент начала
				// полива в резервуаре есть вода
				if (WaterLevel != NoWater)
				{
					// Определение продолжительности полива в секундах
					SecondsWatering = WateringParams.minutes_watering * 60 +
									  WateringParams.seconds_watering;

					// Флаг начала полива
					f_StartWatering = 1;

					// Сброс и включение таймера 4
					TIM4->CNT = 0;
					HAL_TIM_Base_Start_IT(&htim4);

					// Включение помпы
					PumpOn();
				}

				// Если в момент начала
				// полива в резервуаре нет воды
				else
				{
					// Если не выбран кадр 28
					// (его смысл такой же как у 29,
					//  дублировать нет смысла)
					if (current_frame != 28)
					{
						if (current_frame != 29)
						{
							// Сохранение прошлого кадра
							prev_frame = current_frame;
							prev_pos_cursor = pos_cursor;
						}

						// Переход на кадр 29
						current_frame = 29;
						pos_cursor = 4;
					}
				}
			}

			// Если датчик уровня воды выключен
			else
			{
				// Определение продолжительности полива в секундах
				SecondsWatering = WateringParams.minutes_watering * 60 +
								  WateringParams.seconds_watering;

				// Флаг начала полива
				f_StartWatering = 1;

				// Сброс и включение таймера 4
				TIM4->CNT = 0;
				HAL_TIM_Base_Start_IT(&htim4);

				// Включение помпы
				PumpOn();
			}
		}
	}
}


// Формирование кадров для отрисовки
void BuildFrame(uint8_t frame_number)
{
	switch(frame_number)
	{
		// 0. Начальный кадр
		case 0:

				// Текущая позиция курсора
				switch(pos_cursor)
				{
					case 0:

						/*
						 *------------------------
						 * 			Кадр 0
						 *------------------------
						 *	> 	Текущие
						 *		параметры
						 *
						 *	 	Настройки
						 *
						 *	 	Немедленно
						 *	 	завершить полив
						 *------------------------
						 */

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1 - 3, 3, (int16_t) 13 * 1 + 3, 9, (int16_t) 13 * 1, 0);
						OLED_DrawStr_RuModified("Ntreobt", 12, 0, 0);
						OLED_DrawStr_RuModified("gfhfvtnhs", 12, 13, 0);

						OLED_DrawStr_RuModified("Yfcnhjqrb", 12, 13 * 2, 1);

						OLED_DrawStr_RuModified("Ytvtlktyyj", 12, 13 * 3, 1);
						OLED_DrawStr_RuModified("pfdthibnm", 12, 13 * 4, 1);
						OLED_DrawStr_RuModified("gjkbd", 12 + OLED_GetWidthStr("pfdthibnm") + 3, 13 * 4, 1);

					break;


					case 2:

						/*
						 *------------------------
						 * 			Кадр 0
						 *------------------------
						 *	 	Текущие
						 *		параметры
						 *
						 *	> 	Настройки
						 *
						 *		Немедленно
						 *	 	завершить полив
						 *------------------------
						 */

						OLED_DrawStr_RuModified("Ntreobt", 12, 0, 1);
						OLED_DrawStr_RuModified("gfhfvtnhs", 12, 13, 1);

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
						OLED_DrawStr_RuModified("Yfcnhjqrb", 12, 13 * 2, 0);

						OLED_DrawStr_RuModified("Ytvtlktyyj", 12, 13 * 3, 1);
						OLED_DrawStr_RuModified("pfdthibnm", 12, 13 * 4, 1);
						OLED_DrawStr_RuModified("gjkbd", 12 + OLED_GetWidthStr("pfdthibnm") + 3, 13 * 4, 1);

					break;


					case 3:

						/*
						 *------------------------
						 * 			Кадр 0
						 *------------------------
						 *	 	Текущие
						 *		параметры
						 *
						 *	 	Настройки
						 *
						 *	> 	Немедленно
						 *	 	завершить полив
						 *------------------------
						 */

						OLED_DrawStr_RuModified("Ntreobt", 12, 0, 1);
						OLED_DrawStr_RuModified("gfhfvtnhs", 12, 13, 1);

						OLED_DrawStr_RuModified("Yfcnhjqrb", 12, 13 * 2, 1);

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4 - 3, 3, (int16_t) 13 * 4 + 3, 9, (int16_t) 13 * 4, 0);
						OLED_DrawStr_RuModified("Ytvtlktyyj", 12, 13 * 3, 0);
						OLED_DrawStr_RuModified("pfdthibnm", 12, 13 * 4, 0);
						OLED_DrawStr_RuModified("gjkbd", 12 + OLED_GetWidthStr("pfdthibnm") + 3, 13 * 4, 0);

					break;


					default:
					break;
				}

		break;


		// 1. Текущие параметры полива
		case 1:

				switch(pos_cursor)
				{
					case 4:

						/*
						 *------------------------
						 * 			Кадр 1
						 *------------------------
						 *	Текущие
						 *	параметры полива
						 *
						 *		 	Время начала
						 *			полива:
						 *
						 *			Продолжитель-
						 *			ность полива:
						 *
						 *		>	no text
						 *========================
						 *			Частота полива:
						 *
						 *	 		Назад
						 *------------------------
						 */

						OLED_DrawCircleFill_Modified(6, 5, 2, 1);
						OLED_DrawStr_RuModified("Dhtvz", 12, 0, 1);
						str_lenght = OLED_GetWidthStr("Dhtvz");
						OLED_DrawStr_RuModified("yfxfkf", 12 + str_lenght + 3, 0, 1);
						OLED_DrawStr_RuModified("gjkbdf", 12, 13, 1);
						str_lenght = OLED_GetWidthStr("gjkbdf");
						OLED_DrawChar_Modified(':', 12 + str_lenght + 1, 13, 1);

						OLED_DrawCircleFill_Modified(6, 13 * 2 + 5, 2, 1);
						OLED_DrawStr_RuModified("Ghjljk;bntkm", 12, 13 * 2, 1);
						str_lenght = OLED_GetWidthStr("Ghjljk;bntkm");
						OLED_DrawChar_Modified('-', 12 + str_lenght + 1, 13 * 2, 1);
						OLED_DrawStr_RuModified("yjcnm", 12, 13 * 3, 1);
						str_lenght = OLED_GetWidthStr("yjcnm");
						OLED_DrawStr_RuModified("gjkbdf", 12 + str_lenght + 3, 13 * 3, 1);
						str_lenght = OLED_GetWidthStr("yjcnmgjkbdf");
						OLED_DrawChar_Modified(':', 12 + str_lenght + 3 + 1, 13 * 3, 1);

						// Отрисовка времени начала
						// и продолжительности полива
						DrawWateringParamsPt1();

					break;


					default:
					break;
				}

		break;

		// 2. Продолжение кадра 1
		case 2:

				switch(pos_cursor)
				{
					case 3:

						/*
						 *------------------------
						 * 		Кадр 2 (продолжение кадра 1)
						 *------------------------
						 *	Текущие
						 *	параметры полива
						 *
						 *		 	Время начала
						 *			полива:
						 *
						 *			Продолжитель-
						 *			ность полива:
						 *
						 *			no text
						 *========================
						 *			Дни	полива:
						 *			Пн Вт Ср Чт Пт Сб Вс
						 *
						 *	 	>	Назад
						 *------------------------
						 */

						OLED_DrawCircleFill_Modified(6, 5, 2, 1);
						OLED_DrawStr_RuModified("Lyb", 12, 13 * 0, 1);
						OLED_DrawStr_RuModified("gjkbdf", 12 + OLED_GetWidthStr("Lyb") + 3, 13 * 0, 1);
						OLED_DrawChar_Modified(':', 12 + OLED_GetWidthStr("Lyb") + OLED_GetWidthStr("gjkbdf") + 4, 13 * 0, 1);

						OLED_DrawStr_RuModified("Gy", 12, 13 * 1, 1);
						OLED_DrawStr_RuModified("Dn", 12 + OLED_GetWidthStr("Gy") + 4, 13 * 1, 1);
						OLED_DrawStr_RuModified("Ch", 12 + OLED_GetWidthStr("Gy") + 3 * 2 + OLED_GetWidthStr("Dn"), 13 * 1, 1);
						OLED_DrawStr_RuModified("Xn", 12 + OLED_GetWidthStr("Gy") + 3 * 3 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch"), 13 * 1, 1);
						OLED_DrawStr_RuModified("Gn", 12 + OLED_GetWidthStr("Gy") + 3 * 4 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn"), 13 * 1, 1);
						OLED_DrawStr_RuModified("C,", 12 + OLED_GetWidthStr("Gy") + 3 * 5 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn"), 13 * 1, 1);
						OLED_DrawStr_RuModified("Dc", 12 + OLED_GetWidthStr("Gy") + 3 * 6 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn") + OLED_GetWidthStr("C,"), 13 * 1, 1);

						// Отрисовка дней полива
						DrawWateringParamsPt2();

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);
						OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 3, 0);

					break;

					default:
					break;
				}
		break;


		// 3. Уровень воды в резервуаре
		case 3:
				switch(pos_cursor)
				{
					case 3:

						/*
						 *------------------------
						 * 			Кадр 3
						 *------------------------
						 *	Уровень воды
						 *	в резервуаре
						 *
						 *		Резервуар
						 *		заполнен на 100%
						 *
						 *		> 	Назад
						 *------------------------
						 */

						OLED_DrawStr_RuModified("Htpthdefh", 12, 0, 1);
						OLED_DrawStr_RuModified("pfgjkyty", 12, 13, 1);
						str_lenght = OLED_GetWidthStr("pfgjkyty");
						OLED_DrawStr_RuModified("yf", 12 + str_lenght + 3, 13, 1);

						// Если включена индикация
						// (если рассчитано значение mean_adc_value_ch0)
						if (f_StartAnalyzingAdcData)
						{
							switch(WaterLevel)
							{
								case NoWater:

									OLED_DrawChar_Modified('0', 12, 13 * 2, 1);
									OLED_DrawChar_Modified('%', 12 + OLED_GetWidthStr("0") + 1, 13 * 2, 1);

								break;

								case LowLevel:

									OLED_DrawChar_Modified('0', 12, 13 * 2, 1);
									OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("0") + 1, 13 * 2, 1);
									OLED_DrawStr_EnModified("33", 12 + OLED_GetWidthStr("0") + 1 + OLED_GetWidthStr("-") + 1, 13 * 2, 1);
									OLED_DrawChar_Modified('%', 12 + OLED_GetWidthStr("0") + 1 + OLED_GetWidthStr("-") + 1 + OLED_GetWidthStr("33") + 1, 13 * 2, 1);

								break;

								case MidLevel:

									OLED_DrawStr_EnModified("34", 12, 13 * 2, 1);
									OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("34") + 1, 13 * 2, 1);
									OLED_DrawStr_EnModified("67", 12 + OLED_GetWidthStr("34") + 1 + OLED_GetWidthStr("-") + 1, 13 * 2, 1);
									OLED_DrawChar_Modified('%', 12 + OLED_GetWidthStr("34") + 1 + OLED_GetWidthStr("-") + 1 + OLED_GetWidthStr("67") + 1, 13 * 2, 1);

								break;

								case HighLevel:

									OLED_DrawStr_EnModified("68", 12, 13 * 2, 1);
									OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("68") + 1, 13 * 2, 1);
									OLED_DrawStr_EnModified("100", 12 + OLED_GetWidthStr("68") + 1 + OLED_GetWidthStr("-") + 1, 13 * 2, 1);
									OLED_DrawChar_Modified('%', 12 + OLED_GetWidthStr("68") + 1 + OLED_GetWidthStr("-") + 1 + OLED_GetWidthStr("100") + 1, 13 * 2, 1);

								break;

								default:
								break;
							}
						}

						else
							OLED_DrawStr_EnModified("---%", 12 + str_lenght + OLED_GetWidthStr("yf") + 3 * 2, 13, 1);

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);
						OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 3, 0);

					break;

					default:
					break;
				}
		break;


		// 4. Настройки
		case 4:
				switch(pos_cursor)
				{
					case 0:

						/*
						 *------------------------
						 * 			Кадр 4
						 *------------------------
						 *	Настройки
						 *
						 *	>	Настройка
						 *		текущего времени
						 *
						 *		Настройка полива
						 *
						 *		Автоотключение
						 *		экрана
						 *========================
						 *		Сброс настроек
						 *		Назад
						 *------------------------
						 */

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1 - 3, 3, (int16_t) 13 * 1 + 3, 9, (int16_t) 13 * 1, 0);
						OLED_DrawStr_RuModified("Yfcnhjqrf", 12, 0, 0);
						OLED_DrawStr_RuModified("ntreotuj", 12, 13, 0);
						str_lenght = OLED_GetWidthStr("ntreotuj");
						OLED_DrawStr_RuModified("dhtvtyb", 12 + str_lenght + 3, 13, 0);

						OLED_DrawStr_RuModified("Yfcnhjqrf", 12, 13 * 2, 1);
						str_lenght = OLED_GetWidthStr("Yfcnhjqrf");
						OLED_DrawStr_RuModified("gjkbdf", 12 + str_lenght + 3, 13 * 2, 1);

						OLED_DrawStr_RuModified("Fdnjjnrk.xtybt", 12, 13 * 3, 1);
						OLED_DrawStr_RuModified("'rhfyf", 12, 13 * 4, 1);

					break;


					case 2:

						/*
						 *------------------------
						 * 			Кадр 4
						 *------------------------
						 *	Настройки
						 *
						 *		Настройка
						 *		текущего времени
						 *
						 *	>	Настройка полива
						 *
						 *		Автоотключение
						 *		экрана
						 *========================
						 *		Сброс настроек
						 *		Назад
						 *------------------------
						 */

						OLED_DrawStr_RuModified("Yfcnhjqrf", 12, 0, 1);
						OLED_DrawStr_RuModified("ntreotuj", 12, 13, 1);
						str_lenght = OLED_GetWidthStr("ntreotuj");
						OLED_DrawStr_RuModified("dhtvtyb", 12 + str_lenght + 3, 13, 1);

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
						OLED_DrawStr_RuModified("Yfcnhjqrf", 12, 13 * 2, 0);
						str_lenght = OLED_GetWidthStr("Yfcnhjqrf");
						OLED_DrawStr_RuModified("gjkbdf", 12 + str_lenght + 3, 13 * 2, 0);

						OLED_DrawStr_RuModified("Fdnjjnrk.xtybt", 12, 13 * 3, 1);
						OLED_DrawStr_RuModified("'rhfyf", 12, 13 * 4, 1);

					break;


					case 3:

						/*
						 *------------------------
						 * 			Кадр 4
						 *------------------------
						 *	Настройки
						 *
						 *		Настройка
						 *		текущего времени
						 *
						 *		Настройка полива
						 *
						 *	>	Автоотключение
						 *		экрана
						 *========================
						 *		Сброс настроек
						 *		Назад
						 *------------------------
						 */

						OLED_DrawStr_RuModified("Yfcnhjqrf", 12, 0, 1);
						OLED_DrawStr_RuModified("ntreotuj", 12, 13, 1);
						str_lenght = OLED_GetWidthStr("ntreotuj");
						OLED_DrawStr_RuModified("dhtvtyb", 12 + str_lenght + 3, 13, 1);

						OLED_DrawStr_RuModified("Yfcnhjqrf", 12, 13 * 2, 1);
						str_lenght = OLED_GetWidthStr("Yfcnhjqrf");
						OLED_DrawStr_RuModified("gjkbdf", 12 + str_lenght + 3, 13 * 2, 1);

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4 - 3, 3, (int16_t) 13 * 4 + 3, 9, (int16_t) 13 * 4, 0);
						OLED_DrawStr_RuModified("Fdnjjnrk.xtybt", 12, 13 * 3, 0);
						OLED_DrawStr_RuModified("'rhfyf", 12, 13 * 4, 0);

					break;

					default:
					break;
				}

		break;


		// 5. Продолжение кадра 4
		case 5:

			switch(pos_cursor)
			{
				case 0:

					/*
					*------------------------
					* 		Кадр 5
					*------------------------
					*	Настройки
					*
					*		Настройка
					*		текущего времени
					*
					*		Настройка полива
					*
					*		Автоотключение
					*		экрана
					*========================
					*	>	(Отключить/включить) датчик
					*		уровня воды
					*
					*		Сброс настроек
					*
					*		Назад
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1 - 3, 3, (int16_t) 13 * 1 + 3, 9, (int16_t) 13 * 1, 0);

					// Если датчик уровня воды стал включен
					if (WaterLevelSensorState.f_IsWaterLevelSensorOn)
					{
						OLED_DrawStr_RuModified("Jnrk.xbnm", 12, 13 * 0, 0);
						OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("Jnrk.xbnm") + 3, 13 * 0, 0);
					}

					// Если датчик воды стал выключен
					else
					{
						OLED_DrawStr_RuModified("Drk.xbnm", 12, 13 * 0, 0);
						OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("Drk.xbnm") + 3, 13 * 0, 0);
					}

					OLED_DrawStr_RuModified("ehjdyz", 12, 13 * 1, 0);
					OLED_DrawStr_RuModified("djls", 12 + OLED_GetWidthStr("ehjdyz") + 3, 13 * 1, 0);


					OLED_DrawStr_RuModified("C,hjc", 12, 13 * 2, 1);
					OLED_DrawStr_RuModified("yfcnhjtr", 12 + OLED_GetWidthStr("C,hjc") + 3, 13 * 2, 1);

					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 3, 1);

				break;


				case 2:

					/*
					*------------------------
					* 		Кадр 5
					*------------------------
					*	Настройки
					*
					*		Настройка
					*		текущего времени
					*
					*		Настройка полива
					*
					*		Автоотключение
					*		экрана
					*========================
					*		(Отключить/включить) датчик
					*		уровня воды
					*
					*	>	Сброс настроек
					*
					*		Назад
					*------------------------
					*/

					// Если датчик уровня воды стал включен
					if (WaterLevelSensorState.f_IsWaterLevelSensorOn)
					{
						OLED_DrawStr_RuModified("Jnrk.xbnm", 12, 13 * 0, 1);
						OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("Jnrk.xbnm") + 3, 13 * 0, 1);
					}

					// Если датчик воды стал выключен
					else
					{
						OLED_DrawStr_RuModified("Drk.xbnm", 12, 13 * 0, 1);
						OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("Drk.xbnm") + 3, 13 * 0, 1);
					}

					OLED_DrawStr_RuModified("ehjdyz", 12, 13 * 1, 1);
					OLED_DrawStr_RuModified("djls", 12 + OLED_GetWidthStr("ehjdyz") + 3, 13 * 1, 1);


					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
					OLED_DrawStr_RuModified("C,hjc", 12, 13 * 2, 0);
					OLED_DrawStr_RuModified("yfcnhjtr", 12 + OLED_GetWidthStr("C,hjc") + 3, 13 * 2, 0);

					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 3, 1);

				break;


				case 3:

					/*
					*------------------------
					* 		Кадр 5
					*------------------------
					*	Настройки
					*
					*		Настройка
					*		текущего времени
					*
					*		Настройка полива
					*
					*		Автоотключение
					*		экрана
					*========================
					*		(Отключить/включить) датчик
					*		уровня воды
					*
					*		Сброс настроек
					*
					*	>	Назад
					*------------------------
					*/

					// Если датчик уровня воды стал включен
					if (WaterLevelSensorState.f_IsWaterLevelSensorOn)
					{
						OLED_DrawStr_RuModified("Jnrk.xbnm", 12, 13 * 0, 1);
						OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("Jnrk.xbnm") + 3, 13 * 0, 1);
					}

					// Если датчик воды стал выключен
					else
					{
						OLED_DrawStr_RuModified("Drk.xbnm", 12, 13 * 0, 1);
						OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("Drk.xbnm") + 3, 13 * 0, 1);
					}

					OLED_DrawStr_RuModified("ehjdyz", 12, 13 * 1, 1);
					OLED_DrawStr_RuModified("djls", 12 + OLED_GetWidthStr("ehjdyz") + 3, 13 * 1, 1);


					OLED_DrawStr_RuModified("C,hjc", 12, 13 * 2, 1);
					OLED_DrawStr_RuModified("yfcnhjtr", 12 + OLED_GetWidthStr("C,hjc") + 3, 13 * 2, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);
					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 3, 0);

				break;


				default:
				break;
			}

		break;


		// 6. Настройка текущего времени
		case 6:

			switch(pos_cursor)
			{
				case 0:

					/*
					 *------------------------
					 * 			Кадр 6
					 *------------------------
					 *	 >	Дата
					 *
					 *	 	Время
					 *
					 *	 	Назад
					 *------------------------
					 */

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 0.5 - 3, 3, (int16_t) 13 * 0.5 + 3, 9, (int16_t) 13 * 0.5, 0);
					OLED_DrawStr_RuModified("Lfnf", 12, 0, 0);

					OLED_DrawStr_RuModified("Dhtvz", 12, 13, 1);

					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 2, 1);

				break;

				case 1:

					/*
					 *------------------------
					 * 			Кадр 6
					 *------------------------
					 *	 	Дата
					 *
					 *	 >	Время
					 *
					 *	 	Назад
					 *------------------------
					 */

					OLED_DrawStr_RuModified("Lfnf", 12, 0, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1.5 - 3, 3, (int16_t) 13 * 1.5 + 3, 9, (int16_t) 13 * 1.5, 0);
					OLED_DrawStr_RuModified("Dhtvz", 12, 13, 0);

					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 2, 1);

				break;

				case 2:

					/*
					 *------------------------
					 * 			Кадр 6
					 *------------------------
					 *	 	Дата
					 *
					 *	 	Время
					 *
					 *	 >	Назад
					 *------------------------
					 */

					OLED_DrawStr_RuModified("Lfnf", 12, 0, 1);

					OLED_DrawStr_RuModified("Dhtvz", 12, 13, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 2, 0);

				break;

				default:
				break;
			}

		break;


		// 7. Настройка полива
		case 7:

			// Установите время
			// начала полива
			OLED_DrawStr_RuModified("Ecnfyjdbnt", 6, 13 * 0, 1);
			OLED_DrawStr_RuModified("dhtvz", 6 + OLED_GetWidthStr("Ecnfyjdbnt") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("yfxfkf", 6, 13 * 1, 1);
			OLED_DrawStr_RuModified("gjkbdf", 6 + OLED_GetWidthStr("yfxfkf") + 3, 13 * 1, 1);

			switch(pos_cursor)
			{
				case 2:

					/*
					*------------------------
					* 		Кадр 7
					*------------------------
					*	Настройка полива
					*
					*	 	Установите время
					*	 	начала полива
					*	>	--:--:--
					*		Далее
					*------------------------
					*/

					// Началась установка
					// времени начала полива
					if (f_StSetTimeWater)
					{
						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 1);

						// Функция настройки времени
						SetTimeSMH();
					}

					else
					{
						// Если время начала
						// полива установлено
						if (f_TimeIsSetWater)
						{
							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
							DrawSetTime(ptr_WateringParamsSet, 0);
						}

						else
						{
							OLED_DrawStr_EnModified("--", 16, 13 * 2, 0);
							str_lenght = OLED_GetWidthStr("--");
							OLED_DrawChar_Modified(':', 16 + str_lenght + 4 * 1, 13 * 2, 0);
							OLED_DrawStr_EnModified("--", 16 + str_lenght + 4 * 2, 13 * 2, 0);
							OLED_DrawChar_Modified(':', 16 + str_lenght * 2 + 4 * 3, 13 * 2, 0);
							OLED_DrawStr_EnModified("--", 16 + str_lenght * 2 + 4 * 4, 13 * 2, 0);

							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
						}
					}

					// Проверка того, что
					// время полива было установлено
					IsSetTimeWaterFinished();

					OLED_DrawStr_RuModified("Lfktt", 12, 13 * 3, 1);

				break;


				case 3:

					/*
					*------------------------
					* 		Кадр 7
					*------------------------
					*	Настройка полива
					*
					*	 	Установите время
					*	 	начала полива
					*		--:--:--
					*	>	Далее
					*------------------------
					*/

					// Проверка того, что
					// время полива было установлено
					IsSetTimeWaterFinished();

					if (f_TimeIsSetWater)
						DrawSetTime(ptr_WateringParamsSet, 1);
					else
					{
						OLED_DrawStr_EnModified("--", 16, 13 * 2, 1);
						str_lenght = OLED_GetWidthStr("--");
						OLED_DrawChar_Modified(':', 16 + str_lenght + 4 * 1, 13 * 2, 1);
						OLED_DrawStr_EnModified("--", 16 + str_lenght + 4 * 2, 13 * 2, 1);
						OLED_DrawChar_Modified(':', 16 + str_lenght * 2 + 4 * 3, 13 * 2, 1);
						OLED_DrawStr_EnModified("--", 16 + str_lenght * 2 + 4 * 4, 13 * 2, 1);
					}

					OLED_DrawStr_RuModified("Lfktt", 12, 13 * 3, 0);
					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);

				break;


				default:
				break;
			}

		break;


		// 8. Настройка продолжительности полива
		case 8:

			OLED_DrawStr_RuModified("Ecnfyjdbnt", 6, 13 * 0, 1);
			OLED_DrawStr_RuModified("ghjljk", 6 + OLED_GetWidthStr("Ecnfyjdbnt") + 3, 13 * 0, 1);
			OLED_DrawChar_Modified('-', 6 + OLED_GetWidthStr("Ecnfyjdbnt") + OLED_GetWidthStr("ghjljk") + 3 + 1, 13 * 0, 1);

			OLED_DrawStr_RuModified(";bntkmyjcnm", 6, 13 * 1, 1);
			OLED_DrawStr_RuModified("gjkbdf", 6 + OLED_GetWidthStr(";bntkmyjcnm") + 3, 13 * 1, 1);

			switch(pos_cursor)
			{
				case 2:

					/*
					*------------------------
					* 		Кадр 8
					*------------------------
					*	Настройка полива
					*
					*	 	Установите продол-
					*	 	жительность полива
					*	>	--мин --сек
					*		Далее
					*------------------------
					 */

					// Началась установка
					// продолжительности полива
					if (f_StSetDurationWater)
					{
						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 1);

						// Функция настройки времени
						SetTimeSMH();
					}

					else
					{
						// Если время продолжительности
						// полива установлено
						if (f_TimeIsSetDurationWater)
						{
							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
							DrawSetDurationWater(ptr_WateringParamsSet + 3, 0);
						}

						else
						{
							OLED_DrawStr_EnModified("--", 16, 13 * 2, 0);
							OLED_DrawStr_RuModified("vby", 16 + OLED_GetWidthStr("--") + 2, 13 * 2, 0);
							OLED_DrawStr_EnModified("--", 3 + 16 + OLED_GetWidthStr("--") + OLED_GetWidthStr("vby") + 2 * 2, 13 * 2, 0);
							OLED_DrawStr_RuModified("ctr", 3 + 16 + OLED_GetWidthStr("--") * 2 + OLED_GetWidthStr("vby") + 2 * 3, 13 * 2, 0);

							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
						}
					}

					// Проверка того, что
					// продолжительность полива была установлена
					IsSetTimeDurationWaterFinished();

					OLED_DrawStr_RuModified("Lfktt", 12, 13 * 3, 1);

				break;


				case 3:

					/*
					*------------------------
					* 		Кадр 8
					*------------------------
					*	Настройка полива
					*
					*	 	Установите продол-
					*	 	жительность полива
					*		--мин --сек
					*	>	Далее
					*------------------------
					 */

					// Проверка того, что
					// продолжительность полива была установлена
					IsSetTimeDurationWaterFinished();

					if (f_TimeIsSetDurationWater)
						DrawSetDurationWater(ptr_WateringParamsSet + 3, 1);
					else
					{
						OLED_DrawStr_EnModified("--", 16, 13 * 2, 1);
						OLED_DrawStr_RuModified("vby", 16 + OLED_GetWidthStr("--") + 2, 13 * 2, 1);
						OLED_DrawStr_EnModified("--", 3 + 16 + OLED_GetWidthStr("--") + OLED_GetWidthStr("vby") + 2 * 2, 13 * 2, 1);
						OLED_DrawStr_RuModified("ctr", 3 + 16 + OLED_GetWidthStr("--") * 2 + OLED_GetWidthStr("vby") + 2 * 3, 13 * 2, 1);
					}

					OLED_DrawStr_RuModified("Lfktt", 12, 13 * 3, 0);
					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);

				break;


				default:
				break;
			}

		break;


		// 10. Кадр сохранения данных
		case 10:

			OLED_DrawStr_RuModified("Gfhfvtnhs", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("ecgtiyj", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("cj[hfytys", 12 + OLED_GetWidthStr("ecgtiyj") + 3, 13 * 1, 1);
			OLED_DrawChar_Modified('!', 12 + OLED_GetWidthStr("ecgtiyj") + 3 + OLED_GetWidthStr("cj[hfytys") + 1, 13 * 1, 1);

			switch(pos_cursor)
			{
				case 2:

					/*
					*------------------------
					* 		Кадр 10
					*------------------------
					*	Сохранение параметров
					*
					*	 	Параметры
					*	 	успешно сохранены!
					*
					*	 	Ок
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
					OLED_DrawStr_RuModified("Jr", 12, 13 * 2, 0);

				break;
			}

		break;


		// 11. Кадр подтверждения настроек
		// (если настройки не были сохранены на кадре 10)
		case 11:

			//...

		break;


		// 12. Кадр настройки даты
		case 12:

			// Информационная часть - отрисовывается
			// независимо от положения курсора

			// Установите дату
			// в формате дд/мм/гг
			OLED_DrawStr_RuModified("Ecnfyjdbnt lfne d", 6, 13 * 0, 1);
			OLED_DrawStr_RuModified("ajhvfnt ll", 6, 13 * 1, 1);
			str_lenght = OLED_GetWidthStr("ajhvfnt ll");
			OLED_DrawChar_Modified('/', 6 + str_lenght + 3, 13 * 1, 1);
			OLED_DrawStr_RuModified("vv", 6 + str_lenght + 8, 13 * 1, 1);
			OLED_DrawChar_Modified('/', 6 + str_lenght + 25, 13 * 1, 1);
			OLED_DrawStr_RuModified("uu", 6 + str_lenght + 30, 13 * 1, 1);

			switch(pos_cursor)
			{
				case 2:

						/*
						*------------------------
						* 		Кадр 12
						*------------------------
						*	Установите дату
						*	в формате дд/мм/гг
						*	> 	--/--/--
						*		Далее
						*------------------------
						*/

						// Началась установка даты
						if (f_StSetDate)
						{
							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 1);

							// Функция настройки даты
							SetDate();
						}

						else
						{
							// Если дата установлена
							if (f_DateIsSet)
							{
								OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
								DrawSetDate(0);
							}

							else
							{
								OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
								OLED_DrawStr_EnModified("--/--/--", 16, 13 * 2, 0);
							}
						}

						// Проверка того, что была
						// дата была установлена
						IsSetDateFinished();

						OLED_DrawStr_RuModified("Lfktt", 12, 13 * 3, 1);
				break;


				case 3:

						/*
						*------------------------
						* 		Кадр 12
						*------------------------
						*	Установите дату
						*	в формате дд/мм/гг
						*	 	--/--/--
						*	>	Далее
						*------------------------
						*/

						// Проверка того, что была
						// дата была установлена
						IsSetDateFinished();

						// Если дата установлена
						if (f_DateIsSet)
							DrawSetDate(1);
						else
							OLED_DrawStr_EnModified("--/--/--", 16, 13 * 2, 1);

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);
						OLED_DrawStr_RuModified("Lfktt", 12, 13 * 3, 0);
				break;
			}

		break;


		// 13. Кадр настройки дня недели
		case 13:

			// Информационная часть - отрисовывается
			// независимо от положения курсора

			// Установите день
			// недели
			OLED_DrawStr_RuModified("Ecnfyjdbnt ltym", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("ytltkb", 12, 13 * 1, 1);

			switch(pos_cursor)
			{
				/*
				*------------------------
				* 		Кадр 13
				*------------------------
				*	Установите день
				*	недели
				*	> 	----
				*		Сохранить
				*		Выйти
				*------------------------
				*/
				case 2:
						// Началась установка дня недели
						if (f_StSetDayOfWeek)
						{
							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 1);

							// Функция настройки дня недели
							SetDayOfWeek();
						}

						else
						{
							// Если день недели установлен
							if (f_DayOfWeekIsSet)
							{
								OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
								DrawSetDayOfWeek(0);
							}

							else
							{
								OLED_DrawStr_EnModified("----", 16, 13 * 2, 0);
								OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
							}
						}

						// Проверка того, что день
						// недели был установлен
						IsSetDayOfWeekFinished();

						OLED_DrawStr_RuModified("Cj[hfybnm", 12, 13 * 3, 1);

						OLED_DrawStr_RuModified("Dsqnb", 12, 13 * 4, 1);
				break;


				/*
				*------------------------
				* 		Кадр 13
				*------------------------
				*	Установите день
				*	недели
				*	 	----
				*	>	Сохранить
				*		Выйти
				*------------------------
				*/
				case 3:

						// Проверка того, что день
						// недели был установлен
						IsSetDayOfWeekFinished();

						if (f_DayOfWeekIsSet)
							DrawSetDayOfWeek(1);
						else
							OLED_DrawStr_EnModified("----", 16, 13 * 2, 1);

						OLED_DrawStr_RuModified("Cj[hfybnm", 12, 13 * 3, 0);
						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);

						OLED_DrawStr_RuModified("Dsqnb", 12, 13 * 4, 1);

				break;


				/*
				*------------------------
				* 		Кадр 13
				*------------------------
				*	Установите день
				*	недели
				*	 	----
				*		Сохранить
				*	>	Выйти
				*------------------------
				*/
				case 4:
						// Проверка того, что день
						// недели был установлен
						IsSetDayOfWeekFinished();

						if (f_DayOfWeekIsSet)
							DrawSetDayOfWeek(1);
						else
							OLED_DrawStr_EnModified("----", 16, 13 * 2, 1);

						OLED_DrawStr_RuModified("Cj[hfybnm", 12, 13 * 3, 1);

						OLED_DrawStr_RuModified("Dsqnb", 12, 13 * 4, 0);
						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
				break;


				default:
				break;
			}

		break;


		// 14. Кадр настройки времени
		case 14:

				// �?нформационная часть - отрисовывается
				// независимо от положения курсора

				// Установите
				// текущее время
				OLED_DrawStr_RuModified("Ecnfyjdbnt", 12, 13 * 0, 1);
				OLED_DrawStr_RuModified("ntreott dhtvz", 12, 13 * 1, 1);

				switch(pos_cursor)
				{
					case 2:

						/*
						*------------------------
						* 		Кадр 14
						*------------------------
						* 	Установите
						*	текущее время
						*
						*	>	--:--:--
						*		Сохранить
						*		Выйти
						*------------------------
						*/

						// Началась установка дня недели
						if (f_StSetTime)
						{
							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 1);

							// Функция настройки времени
							SetTimeSMH();
						}

						else
						{
							// Если день недели установлен
							if (f_TimeIsSet)
							{
								OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
								DrawSetTime(ptr_timeSetDate, 0);
							}

							else
							{
								OLED_DrawStr_EnModified("--", 16, 13 * 2, 0);
								str_lenght = OLED_GetWidthStr("--");
								OLED_DrawChar_Modified(':', 16 + str_lenght + 4 * 1, 13 * 2, 0);
								OLED_DrawStr_EnModified("--", 16 + str_lenght + 4 * 2, 13 * 2, 0);
								OLED_DrawChar_Modified(':', 16 + str_lenght * 2 + 4 * 3, 13 * 2, 0);
								OLED_DrawStr_EnModified("--", 16 + str_lenght * 2 + 4 * 4, 13 * 2, 0);

								OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
							}
						}

						// Проверка того, что
						// время было установлено
						IsSetTimeFinished();

						OLED_DrawStr_RuModified("Cj[hfybnm", 12, 13 * 3, 1);

						OLED_DrawStr_RuModified("Dsqnb", 12, 13 * 4, 1);

					break;


					case 3:

						/*
						*------------------------
						* 		Кадр 14
						*------------------------
						* 	Установите
						*	текущее время
						*
						*		--:--:--
						*	>	Сохранить
						*		Выйти
						*------------------------
						*/

						// Проверка того, что
						// время было установлено
						IsSetTimeFinished();

						if (f_TimeIsSet)
							DrawSetTime(ptr_timeSetDate, 1);
						else
						{
							OLED_DrawStr_EnModified("--", 16, 13 * 2, 1);
							str_lenght = OLED_GetWidthStr("--");
							OLED_DrawChar_Modified(':', 16 + str_lenght + 4 * 1, 13 * 2, 1);
							OLED_DrawStr_EnModified("--", 16 + str_lenght + 4 * 2, 13 * 2, 1);
							OLED_DrawChar_Modified(':', 16 + str_lenght * 2 + 4 * 3, 13 * 2, 1);
							OLED_DrawStr_EnModified("--", 16 + str_lenght * 2 + 4 * 4, 13 * 2, 1);
						}


						OLED_DrawStr_RuModified("Cj[hfybnm", 12, 13 * 3, 0);
						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);

						OLED_DrawStr_RuModified("Dsqnb", 12, 13 * 4, 1);

					break;


					case 4:

						/*
						*------------------------
						* 		Кадр 14
						*------------------------
						* 	Установите
						*	текущее время
						*
						*		--:--:--
						*		Сохранить
						*	>	Выйти
						*------------------------
						*/

						// Проверка того, что
						// время было установлено
						IsSetTimeFinished();

						if (f_TimeIsSet)
							DrawSetTime(ptr_timeSetDate, 1);
						else
						{
							OLED_DrawStr_EnModified("--", 16, 13 * 2, 1);
							str_lenght = OLED_GetWidthStr("--");
							OLED_DrawChar_Modified(':', 16 + str_lenght + 4 * 1, 13 * 2, 1);
							OLED_DrawStr_EnModified("--", 16 + str_lenght + 4 * 2, 13 * 2, 1);
							OLED_DrawChar_Modified(':', 16 + str_lenght * 2 + 4 * 3, 13 * 2, 1);
							OLED_DrawStr_EnModified("--", 16 + str_lenght * 2 + 4 * 4, 13 * 2, 1);
						}

						OLED_DrawStr_RuModified("Cj[hfybnm", 12, 13 * 3, 1);

						OLED_DrawStr_RuModified("Dsqnb", 12, 13 * 4, 0);
						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);

					break;


					default:
					break;
				}

		break;


		// 15. Кадр текущие параметры
		case 15:

			switch(pos_cursor)
			{
				case 0:

					/*
					*------------------------
					* 		Кадр 15-18
					*------------------------
					* 	>	Текущая
					* 		дата и время
					*
					* 		Текущие
					* 		параметры полива
					*
					* 		Уровень воды
					*------------------------
					* 		Уровень воды
					* 		в резервуаре
					*
					* 		Автоотключение
					* 		экрана
					*
					* 		Назад
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1 - 3, 3, (int16_t) 13 * 1 + 3, 9, (int16_t) 13 * 1, 0);
					OLED_DrawStr_RuModified("Ntreofz", 12, 13 * 0, 0);
					OLED_DrawStr_RuModified("lfnf", 12, 13 * 1, 0);
					str_lenght = OLED_GetWidthStr("lfnf");
					OLED_DrawStr_RuModified("b", 12 + str_lenght + 3, 13 * 1, 0);
					OLED_DrawStr_RuModified("dhtvz", 12 + str_lenght + 3 + 9, 13 * 1, 0);

					OLED_DrawStr_RuModified("Ntreobt", 12, 13 * 2, 1);
					OLED_DrawStr_RuModified("gfhfvtnhs", 12, 13 * 3, 1);
					str_lenght = OLED_GetWidthStr("gfhfvtnhs");
					OLED_DrawStr_RuModified("gjkbdf", 12 + str_lenght + 3, 13 * 3, 1);

					OLED_DrawStr_RuModified("Ehjdtym", 12, 13 * 4, 1);
					str_lenght = OLED_GetWidthStr("Ehjdtym");
					OLED_DrawStr_RuModified("djls", 12 + str_lenght + 3, 13 * 4, 1);

				break;


				case 2:

					/*
					*------------------------
					* 		Кадр 15-18
					*------------------------
					* 		Текущая
					* 		дата и время
					*
					* 	>	Текущие
					* 		параметры полива
					*
					* 		Уровень воды
					*------------------------
					* 		Уровень воды
					* 		в резервуаре
					*
					* 		Автоотключение
					* 		экрана
					*
					* 		Назад
					*------------------------
					*/

					OLED_DrawStr_RuModified("Ntreofz", 12, 13 * 0, 1);
					OLED_DrawStr_RuModified("lfnf", 12, 13 * 1, 1);
					str_lenght = OLED_GetWidthStr("lfnf");
					OLED_DrawStr_RuModified("b", 12 + str_lenght + 3, 13 * 1, 1);
					OLED_DrawStr_RuModified("dhtvz", 12 + str_lenght + 3 + 9, 13 * 1, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3 - 3, 3, (int16_t) 13 * 3 + 3, 9, (int16_t) 13 * 3, 0);
					OLED_DrawStr_RuModified("Ntreobt", 12, 13 * 2, 0);
					OLED_DrawStr_RuModified("gfhfvtnhs", 12, 13 * 3, 0);
					str_lenght = OLED_GetWidthStr("gfhfvtnhs");
					OLED_DrawStr_RuModified("gjkbdf", 12 + str_lenght + 3, 13 * 3, 0);

					OLED_DrawStr_RuModified("Ehjdtym", 12, 13 * 4, 1);
					str_lenght = OLED_GetWidthStr("Ehjdtym");
					OLED_DrawStr_RuModified("djls", 12 + str_lenght + 3, 13 * 4, 1);

				break;


				case 4:

					/*
					*------------------------
					* 		Кадр 15-18
					*------------------------
					* 		Текущая
					* 		дата и время
					*
					* 		Текущие
					* 		параметры полива
					*
					* 	>	Уровень воды
					*------------------------
					* 		Уровень воды
					* 		в резервуаре
					*
					* 		Автоотключение
					* 		экрана
					*
					* 		Назад
					*------------------------
					*/

					OLED_DrawStr_RuModified("Ntreofz", 12, 13 * 0, 1);
					OLED_DrawStr_RuModified("lfnf", 12, 13 * 1, 1);
					str_lenght = OLED_GetWidthStr("lfnf");
					OLED_DrawStr_RuModified("b", 12 + str_lenght + 3, 13 * 1, 1);
					OLED_DrawStr_RuModified("dhtvz", 12 + str_lenght + 3 + 9, 13 * 1, 1);

					OLED_DrawStr_RuModified("Ntreobt", 12, 13 * 2, 1);
					OLED_DrawStr_RuModified("gfhfvtnhs", 12, 13 * 3, 1);
					str_lenght = OLED_GetWidthStr("gfhfvtnhs");
					OLED_DrawStr_RuModified("gjkbdf", 12 + str_lenght + 3, 13 * 3, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
					OLED_DrawStr_RuModified("Ehjdtym", 12, 13 * 4, 0);
					str_lenght = OLED_GetWidthStr("Ehjdtym");
					OLED_DrawStr_RuModified("djls", 12 + str_lenght + 3, 13 * 4, 0);

				break;
			}

		break;


		// 16. Кадр текущая дата и время
		case 16:

			// Дата
			OLED_DrawCircleFill_Modified(6, 5, 2, 1);
			OLED_DrawStr_RuModified("Lfnf", 12, 13 * 0, 1);
			str_lenght = OLED_GetWidthStr("Lfnf");
			OLED_DrawChar_Modified(':', 12 + str_lenght, 13 * 0, 1);

			// Отрисовка текущей даты
			DrawDate(1);


			// Время
			OLED_DrawCircleFill_Modified(6, 13 * 1 + 5, 2, 1);
			OLED_DrawStr_RuModified("Dhtvz", 12, 13 * 1, 1);
			str_lenght = OLED_GetWidthStr("Dhtvz");
			OLED_DrawChar_Modified(':', 12 + str_lenght, 13 * 1, 1);

			// Отрисовка текущего времени
			DrawTime(1);


			// День недели
			OLED_DrawCircleFill_Modified(6, 13 * 2 + 5, 2, 1);
			OLED_DrawStr_RuModified("Ltym", 12, 13 * 2, 1);
			str_lenght = OLED_GetWidthStr("Ltym");
			OLED_DrawStr_RuModified("ytltkb", 12 + str_lenght + 3, 13 * 2, 1);
			OLED_DrawChar_Modified(':', 12 + str_lenght + 40, 13 * 2, 1);

			// Отрисовка текущего дня недели
			DrawDayOfWeek(1);


			switch(pos_cursor)
			{
				case 4:

					/*
					*------------------------
					* 		Кадр 16
					*------------------------
					* 	Дата: XX/XX/XX
					* 	Время: XX:XX:XX
					* 	День недели:
					* 	XXXX
					* 	Назад
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 4, 0);

				break;
			}

		break;


		// 17. Кадр ошибки введённых параметров
		case 17:

			// Информационная часть
			OLED_DrawStr_RuModified("Gfhfvtnhs", 3, 13 * 0, 1);
			str_lenght = OLED_GetWidthStr("Gfhfvtnhs");
			OLED_DrawStr_RuModified(",skb", 3 + str_lenght + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("ddtltys", 3, 13 * 1, 1);
			str_lenght = OLED_GetWidthStr("ddtltys");
			OLED_DrawStr_RuModified("ytrjhhtrnyj", 3 + str_lenght + 3, 13 * 1, 1);
			OLED_DrawStr_RuModified("b", 3, 13 * 2, 1);
			str_lenght = OLED_GetWidthStr("b");
			OLED_DrawStr_RuModified("yt", 3 + str_lenght + 3, 13 * 2, 1);
			OLED_DrawStr_RuModified(",elen", 3 + str_lenght + OLED_GetWidthStr("yt") + 3 * 2, 13 * 2, 1);
			OLED_DrawStr_RuModified("cj[hfytys", 3 + str_lenght + OLED_GetWidthStr("yt") + OLED_GetWidthStr(",elen") + 3 * 3, 13 * 2, 1);
			OLED_DrawChar_Modified('!', 3 + str_lenght + OLED_GetWidthStr("yt") + OLED_GetWidthStr(",elen") + OLED_GetWidthStr("cj[hfytys") + 3 * 4, 13 * 2, 1);

			switch(pos_cursor)
			{
				case 3:

					/*
					*------------------------
					* 		Кадр 17
					*------------------------
					* 		Параметры были
					* 		введены некорректно
					* 		и не будут сохранены!
					* 	>	Начать заново
					* 		Выйти
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);
					OLED_DrawStr_RuModified("Yfxfnm", 12, 13 * 3, 0);
					str_lenght = OLED_GetWidthStr("Yfxfnm");
					OLED_DrawStr_RuModified("pfyjdj", 12 + str_lenght + 3, 13 * 3, 0);

					OLED_DrawStr_RuModified("Dsqnb", 12, 13 * 4, 1);

				break;


				case 4:

					/*
					*------------------------
					* 		Кадр 17
					*------------------------
					* 		Параметры были
					* 		введены некорректно
					* 		и не будут сохранены!
					* 		Начать заново
					* 	>	Выйти
					*------------------------
					*/

					OLED_DrawStr_RuModified("Yfxfnm", 12, 13 * 3, 1);
					str_lenght = OLED_GetWidthStr("Yfxfnm");
					OLED_DrawStr_RuModified("pfyjdj", 12 + str_lenght + 3, 13 * 3, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
					OLED_DrawStr_RuModified("Dsqnb", 12, 13 * 4, 0);

				break;
			}

		break;


		// 18. Продолжение кадра 15
		case 18:

			switch(pos_cursor)
			{
				case 0:

					/*
					*------------------------
					* 		Кадр 15-18
					*------------------------
					* 		Текущая
					* 		дата и время
					*
					* 		Текущие
					* 		параметры полива
					*
					* 		Уровень воды
					*------------------------
					* 	>	Уровень воды
					* 		в резервуаре
					*
					* 		Автоотключение
					* 		экрана
					*
					* 		Назад
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1 - 3, 3, (int16_t) 13 * 1 + 3, 9, (int16_t) 13 * 1, 0);
					OLED_DrawStr_RuModified("Ehjdtym", 12, 13 * 0, 0);
					str_lenght = OLED_GetWidthStr("Ehjdtym");
					OLED_DrawStr_RuModified("djls", 12 + str_lenght + 3, 13 * 0, 0);
					OLED_DrawStr_RuModified("d", 12, 13 * 1, 0);
					str_lenght = OLED_GetWidthStr("d");
					OLED_DrawStr_RuModified("htpthdefht", 12 + str_lenght + 3, 13 * 1, 0);

					OLED_DrawStr_RuModified("Fdnjjnrk.xtybt", 12, 13 * 2, 1);
					OLED_DrawStr_RuModified("'rhfyf", 12, 13 * 3, 1);

					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 4, 1);

				break;


				case 2:

					/*
					*------------------------
					* 		Кадр 15-18
					*------------------------
					* 		Текущая
					* 		дата и время
					*
					* 		Текущие
					* 		параметры полива
					*
					* 		Уровень воды
					*------------------------
					* 		Уровень воды
					* 		в резервуаре
					*
					* 	>	Автоотключение
					* 		экрана
					*
					* 		Назад
					*------------------------
					*/

					OLED_DrawStr_RuModified("Ehjdtym", 12, 13 * 0, 1);
					str_lenght = OLED_GetWidthStr("Ehjdtym");
					OLED_DrawStr_RuModified("djls", 12 + str_lenght + 3, 13 * 0, 1);
					OLED_DrawStr_RuModified("d", 12, 13 * 1, 1);
					str_lenght = OLED_GetWidthStr("d");
					OLED_DrawStr_RuModified("htpthdefht", 12 + str_lenght + 3, 13 * 1, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3 - 3, 3, (int16_t) 13 * 3 + 3, 9, (int16_t) 13 * 3, 0);
					OLED_DrawStr_RuModified("Fdnjjnrk.xtybt", 12, 13 * 2, 0);
					OLED_DrawStr_RuModified("'rhfyf", 12, 13 * 3, 0);

					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 4, 1);

				break;


				case 4:

					/*
					*------------------------
					* 		Кадр 15-18
					*------------------------
					* 		Текущая
					* 		дата и время
					*
					* 		Текущие
					* 		параметры полива
					*
					* 		Уровень воды
					*------------------------
					* 		Уровень воды
					* 		в резервуаре
					*
					* 		Автоотключение
					* 		экрана
					*
					* 	>	Назад
					*------------------------
					*/

					OLED_DrawStr_RuModified("Ehjdtym", 12, 13 * 0, 1);
					str_lenght = OLED_GetWidthStr("Ehjdtym");
					OLED_DrawStr_RuModified("djls", 12 + str_lenght + 3, 13 * 0, 1);
					OLED_DrawStr_RuModified("d", 12, 13 * 1, 1);
					str_lenght = OLED_GetWidthStr("d");
					OLED_DrawStr_RuModified("htpthdefht", 12 + str_lenght + 3, 13 * 1, 1);

					OLED_DrawStr_RuModified("Fdnjjnrk.xtybt", 12, 13 * 2, 1);
					OLED_DrawStr_RuModified("'rhfyf", 12, 13 * 3, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 4, 0);

				break;
			}

		break;


		// 19. Настройка полива (3)
		case 19:

			OLED_DrawStr_RuModified("Ds,thbnt", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("lyb", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("gjkbdf", 12 + OLED_GetWidthStr("lyb") + 3, 13 * 1, 1);
			OLED_DrawChar_Modified(':', 12 + OLED_GetWidthStr("lyb") + OLED_GetWidthStr("gjkbdf") +  3 + 1, 13 * 1, 1);
			OLED_DrawStr_RuModified("Gy", 12, 13 * 2, 1);
			OLED_DrawStr_RuModified("Dn", 12 + OLED_GetWidthStr("Gy") + 4, 13 * 2, 1);
			OLED_DrawStr_RuModified("Ch", 12 + OLED_GetWidthStr("Gy") + 3 * 2 + OLED_GetWidthStr("Dn"), 13 * 2, 1);
			OLED_DrawStr_RuModified("Xn", 12 + OLED_GetWidthStr("Gy") + 3 * 3 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch"), 13 * 2, 1);
			OLED_DrawStr_RuModified("Gn", 12 + OLED_GetWidthStr("Gy") + 3 * 4 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn"), 13 * 2, 1);
			OLED_DrawStr_RuModified("C,", 12 + OLED_GetWidthStr("Gy") + 3 * 5 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn"), 13 * 2, 1);
			OLED_DrawStr_RuModified("Dc", 12 + OLED_GetWidthStr("Gy") + 3 * 6 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn") + OLED_GetWidthStr("C,"), 13 * 2, 1);

			switch(pos_cursor)
			{
				case 3:

					/*
					*------------------------
					* 		Кадр 19
					*------------------------
					*	Настройка полива
					*
					*	 	Выберите
					*	 	дни полива:
					*	 	Пн Вт Ср Чт Пт Сб Вс
					*	> 	0  0  0  0  0  0  0
					*		Завершить
					*------------------------
					*/

					// Началась установка дней полива
					if (f_StSetDaysWater)
					{
						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 1);

						// Функция настройки дней полива
						SetDaysWater();
					}

					else
					{
						// Если дни полива установлены
						if (f_DaysWaterIsSet)
						{
							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);
							DrawSetDaysWater(0);
						}

						else
						{
							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);

							OLED_DrawChar_Modified('-', 5 + 12, 13 * 3, 0);
							OLED_DrawChar_Modified('-', 3 + 12 + OLED_GetWidthStr("Gy") + 3, 13 * 3, 0);
							OLED_DrawChar_Modified('-', 2 + 12 + OLED_GetWidthStr("Gy") + 3 * 2 + OLED_GetWidthStr("Dn"), 13 * 3, 0);
							OLED_DrawChar_Modified('-', 3 + 12 + OLED_GetWidthStr("Gy") + 3 * 3 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch"), 13 * 3, 0);
							OLED_DrawChar_Modified('-', 2 + 12 + OLED_GetWidthStr("Gy") + 3 * 4 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn"), 13 * 3, 0);
							OLED_DrawChar_Modified('-', 2 + 12 + OLED_GetWidthStr("Gy") + 3 * 5 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn"), 13 * 3, 0);
							OLED_DrawChar_Modified('-', 5 + 12 + OLED_GetWidthStr("Gy") + 3 * 6 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn") + OLED_GetWidthStr("C,"), 13 * 3, 0);
						}
					}

					// Проверка того, что
					// дни полива были установлены
					IsSetTimeDaysWaterFinished();

					OLED_DrawStr_RuModified("Pfdthibnm", 12, 13 * 4, 1);

				break;


				case 4:

					/*
					*------------------------
					* 		Кадр 19
					*------------------------
					*	Настройка полива
					*
					*	 	Выберите
					*	 	дни полива
					*	 	Пн Вт Ср Чт Пт Сб Вс
					*	 	0  0  0  0  0  0  0
					*	>	Завершить
					*------------------------
					*/

					// Проверка того, что
					// дни полива были установлены
					IsSetTimeDaysWaterFinished();

					// Если дни полива установлены
					if (f_DaysWaterIsSet)
						DrawSetDaysWater(1);
					else
					{
						OLED_DrawChar_Modified('-', 5 + 12, 13 * 3, 1);
						OLED_DrawChar_Modified('-', 3 + 12 + OLED_GetWidthStr("Gy") + 3, 13 * 3, 1);
						OLED_DrawChar_Modified('-', 2 + 12 + OLED_GetWidthStr("Gy") + 3 * 2 + OLED_GetWidthStr("Dn"), 13 * 3, 1);
						OLED_DrawChar_Modified('-', 3 + 12 + OLED_GetWidthStr("Gy") + 3 * 3 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch"), 13 * 3, 1);
						OLED_DrawChar_Modified('-', 2 + 12 + OLED_GetWidthStr("Gy") + 3 * 4 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn"), 13 * 3, 1);
						OLED_DrawChar_Modified('-', 2 + 12 + OLED_GetWidthStr("Gy") + 3 * 5 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn"), 13 * 3, 1);
						OLED_DrawChar_Modified('-', 5 + 12 + OLED_GetWidthStr("Gy") + 3 * 6 + OLED_GetWidthStr("Dn") + OLED_GetWidthStr("Ch") + OLED_GetWidthStr("Xn") + OLED_GetWidthStr("Gn") + OLED_GetWidthStr("C,"), 13 * 3, 1);
					}

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
					OLED_DrawStr_RuModified("Pfdthibnm", 12, 13 * 4, 0);

				break;
			}

		break;


		// 20. Кадр сброса настроек
		case 20:

			switch(pos_cursor)
			{
				case 0:

						/*
						*------------------------
						* 		Кадр 20-21
						*------------------------
						*	Сброс настроек
						*
						*	 >	Сбросить
						*	 	настройки времени
						*
						*	 	Сбросить
						*	 	настройки полива
						*
						*	 	Сбросить авто-
						*===========================
						*	 	Сбросить авто-
						*	 	отключение экрана
						*
						*	 	Сбросить
						*	 	все настройки
						*
						*	 	Назад
						*------------------------
						*/

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1 - 3, 3, (int16_t) 13 * 1 + 3, 9, (int16_t) 13 * 1, 0);
						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 0, 0);
						OLED_DrawStr_RuModified("yfcnhjqrb", 12, 13 * 1, 0);
						OLED_DrawStr_RuModified("dhtvtyb", 12 + OLED_GetWidthStr("yfcnhjqrb") + 3, 13 * 1, 0);

						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 2, 1);
						OLED_DrawStr_RuModified("yfcnhjqrb", 12, 13 * 3, 1);
						OLED_DrawStr_RuModified("gjkbdf", 12 + OLED_GetWidthStr("yfcnhjqrb") + 3, 13 * 3, 1);

						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 4, 1);
						OLED_DrawStr_RuModified("fdnj", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 4, 1);
						OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("C,hjcbnm") + 3 + OLED_GetWidthStr("fdnj") + 1, 13 * 4, 1);

				break;


				case 2:

						/*
						*------------------------
						* 		Кадр 20-21
						*------------------------
						*	Сброс настроек
						*
						*	 	Сбросить
						*	 	настройки времени
						*
						*	 >	Сбросить
						*	 	настройки полива
						*
						*	 	Сбросить авто-
						*===========================
						*	 	Сбросить авто-
						*	 	отключение экрана
						*
						*	 	Сбросить
						*	 	все настройки
						*
						*	 	Назад
						*------------------------
						*/

						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 0, 1);
						OLED_DrawStr_RuModified("yfcnhjqrb", 12, 13 * 1, 1);
						OLED_DrawStr_RuModified("dhtvtyb", 12 + OLED_GetWidthStr("yfcnhjqrb") + 3, 13 * 1, 1);

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3 - 3, 3, (int16_t) 13 * 3 + 3, 9, (int16_t) 13 * 3, 0);
						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 2, 0);
						OLED_DrawStr_RuModified("yfcnhjqrb", 12, 13 * 3, 0);
						OLED_DrawStr_RuModified("gjkbdf", 12 + OLED_GetWidthStr("yfcnhjqrb") + 3, 13 * 3, 0);

						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 4, 1);
						OLED_DrawStr_RuModified("fdnj", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 4, 1);
						OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("C,hjcbnm") + 3 + OLED_GetWidthStr("fdnj") + 1, 13 * 4, 1);

				break;


				case 4:

						/*
						*------------------------
						* 		Кадр 20-21
						*------------------------
						*	Сброс настроек
						*
						*	 	Сбросить
						*	 	настройки времени
						*
						*	 	Сбросить
						*	 	настройки полива
						*
						*	 >	Сбросить авто-
						*===========================
						*	 	Сбросить авто-
						*	 	отключение экрана
						*
						*	 	Сбросить
						*	 	все настройки
						*
						*	 	Назад
						*------------------------
						*/

						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 0, 1);
						OLED_DrawStr_RuModified("yfcnhjqrb", 12, 13 * 1, 1);
						OLED_DrawStr_RuModified("dhtvtyb", 12 + OLED_GetWidthStr("yfcnhjqrb") + 3, 13 * 1, 1);

						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 2, 1);
						OLED_DrawStr_RuModified("yfcnhjqrb", 12, 13 * 3, 1);
						OLED_DrawStr_RuModified("gjkbdf", 12 + OLED_GetWidthStr("yfcnhjqrb") + 3, 13 * 3, 1);

						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
						OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 4, 0);
						OLED_DrawStr_RuModified("fdnj", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 4, 0);
						OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("C,hjcbnm") + 3 + OLED_GetWidthStr("fdnj") + 1, 13 * 4, 0);

				break;
			}

		break;

		// 21. Сброс настроек (2)
		case 21:

			switch(pos_cursor)
			{
				case 0:

					/*
					*------------------------
					* 		Кадр 20-21-33
					*------------------------
					*	Сброс настроек
					*
					*	 	Сбросить
					*	 	настройки времени
					*
					*	 	Сбросить
					*	 	настройки полива
					*
					*	 	Сбросить авто-
					*===========================
					*	> 	Сбросить авто-
					*	 	отключение экрана
					*
					*	 	Сбросить датчик
					*	 	уровня воды
					*
					*	 	Сбросить
					*===========================
					*		Сбросить
					*	 	все настройки
					*
					*	 	Назад
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1 - 3, 3, (int16_t) 13 * 1 + 3, 9, (int16_t) 13 * 1, 0);
					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 0, 0);
					OLED_DrawStr_RuModified("fdnj", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 0, 0);
					OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("C,hjcbnm") + 3 + OLED_GetWidthStr("fdnj") + 1, 13 * 0, 0);
					OLED_DrawStr_RuModified("jnrk.xtybt", 12, 13 * 1, 0);
					OLED_DrawStr_RuModified("'rhfyf", 12 + OLED_GetWidthStr("jnrk.xtybt") + 3, 13 * 1, 0);

					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 2, 1);
					OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 2, 1);
					OLED_DrawStr_RuModified("ehjdyz", 12, 13 * 3, 1);
					OLED_DrawStr_RuModified("djls", 12 + OLED_GetWidthStr("ehjdyz") + 3, 13 * 3, 1);

					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 4, 1);

				break;


				case 2:

					/*
					*------------------------
					* 		Кадр 20-21-33
					*------------------------
					*	Сброс настроек
					*
					*	 	Сбросить
					*	 	настройки времени
					*
					*	 	Сбросить
					*	 	настройки полива
					*
					*	 	Сбросить авто-
					*===========================
					*	 	Сбросить авто-
					*	 	отключение экрана
					*
					*	> 	Сбросить датчик
					*	 	уровня воды
					*
					*	 	Сбросить
					*===========================
					*		Сбросить
					*	 	все настройки
					*
					*	 	Назад
					*/

					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 0, 1);
					OLED_DrawStr_RuModified("fdnj", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 0, 1);
					OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("C,hjcbnm") + 3 + OLED_GetWidthStr("fdnj") + 1, 13 * 0, 1);
					OLED_DrawStr_RuModified("jnrk.xtybt", 12, 13 * 1, 1);
					OLED_DrawStr_RuModified("'rhfyf", 12 + OLED_GetWidthStr("jnrk.xtybt") + 3, 13 * 1, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3 - 3, 3, (int16_t) 13 * 3 + 3, 9, (int16_t) 13 * 3, 0);
					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 2, 0);
					OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 2, 0);
					OLED_DrawStr_RuModified("ehjdyz", 12, 13 * 3, 0);
					OLED_DrawStr_RuModified("djls", 12 + OLED_GetWidthStr("ehjdyz") + 3, 13 * 3, 0);

					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 4, 1);

				break;


				case 4:

					/*
					*------------------------
					* 		Кадр 20-21-33
					*------------------------
					*	Сброс настроек
					*
					*	 	Сбросить
					*	 	настройки времени
					*
					*	 	Сбросить
					*	 	настройки полива
					*
					*	 	Сбросить авто-
					*===========================
					*	 	Сбросить авто-
					*	 	отключение экрана
					*
					*	> 	Сбросить датчик
					*	 	уровня воды
					*
					*	 	Сбросить
					*===========================
					*		Сбросить
					*	 	все настройки
					*
					*	 	Назад
					*/

					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 0, 1);
					OLED_DrawStr_RuModified("fdnj", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 0, 1);
					OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("C,hjcbnm") + 3 + OLED_GetWidthStr("fdnj") + 1, 13 * 0, 1);
					OLED_DrawStr_RuModified("jnrk.xtybt", 12, 13 * 1, 1);
					OLED_DrawStr_RuModified("'rhfyf", 12 + OLED_GetWidthStr("jnrk.xtybt") + 3, 13 * 1, 1);

					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 2, 1);
					OLED_DrawStr_RuModified("lfnxbr", 12 + OLED_GetWidthStr("C,hjcbnm") + 3, 13 * 2, 1);
					OLED_DrawStr_RuModified("ehjdyz", 12, 13 * 3, 1);
					OLED_DrawStr_RuModified("djls", 12 + OLED_GetWidthStr("ehjdyz") + 3, 13 * 3, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 4, 0);

				break;
			}

		break;


		// 22. Сообщение об успешном
		// 	   сбросе текущих настроек
		case 22:

			OLED_DrawStr_RuModified("Gfhfvtnhs", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("ecgtiyj", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("c,hjitys", 12 + OLED_GetWidthStr("ecgtiyj") + 3, 13 * 1, 1);
			OLED_DrawChar_Modified('!', 12 + OLED_GetWidthStr("ecgtiyj") + 3 + OLED_GetWidthStr("c,hjitys") + 1, 13 * 1, 1);

			switch(pos_cursor)
			{
				case 2:

					/*
					*------------------------
					* 		Кадр 22
					*------------------------
					*	Сброс параметров
					*
					*	 	Параметры
					*	 	успешно сброшены!
					*
					*	> 	Ок
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
					OLED_DrawStr_RuModified("Jr", 12, 13 * 2, 0);

				break;
			}

		break;


		// 23. Настройка времени автоотключения экрана (1)
		case 23:

			OLED_DrawStr_RuModified("Ecnfyjdbnt", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("dhtvz", 12 + OLED_GetWidthStr("Ecnfyjdbnt") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("fdnjjnrk", 12, 13 * 1, 1);
			OLED_DrawChar_Modified('.', 12 + OLED_GetWidthStr("fdnjjnrk") + 1, 13 * 1, 1);
			OLED_DrawStr_RuModified("'rhfyf", 12 + OLED_GetWidthStr("fdnjjnrk") + 1 + OLED_GetWidthStr(".") + 3 + 10, 13 * 1, 1);
			OLED_DrawChar_Modified('(', 12, 13 * 2, 1);
			OLED_DrawStr_EnModified("30", 12 + OLED_GetWidthStr("(") + 2, 13 * 2, 1);
			OLED_DrawStr_RuModified("vby", 12 + OLED_GetWidthStr("(") + 2 + OLED_GetWidthStr("30") + 3, 13 * 2, 1);
			OLED_DrawChar_Modified('>', 12 + OLED_GetWidthStr("(") + 2 + OLED_GetWidthStr("30") + 3 + OLED_GetWidthStr("vby") + 3, 13 * 2, 1);
			OLED_DrawChar_Modified('t', 12 + OLED_GetWidthStr("(") + 2 + OLED_GetWidthStr("30") + 3 + OLED_GetWidthStr("vby") + 3 + OLED_GetWidthStr(">") + 4, 13 * 2, 1);
			OLED_DrawChar_Modified('>', 12 + OLED_GetWidthStr("(") + 2 + OLED_GetWidthStr("30") + 3 + OLED_GetWidthStr("vby") + 3 + OLED_GetWidthStr(">") + 3 + OLED_GetWidthStr("t") + 3, 13 * 2, 1);
			OLED_DrawStr_EnModified("30", 12 + OLED_GetWidthStr("(") + 2 + OLED_GetWidthStr("30") + 3 + OLED_GetWidthStr("vby") + 3 + OLED_GetWidthStr(">") + 3 + OLED_GetWidthStr("t") + 3 + OLED_GetWidthStr(">") + 3, 13 * 2, 1);
			OLED_DrawStr_RuModified("ctr", 12 + OLED_GetWidthStr("(") + 2 + OLED_GetWidthStr("30") + 3 + OLED_GetWidthStr("vby") + 3 + OLED_GetWidthStr(">") + 3 + OLED_GetWidthStr("t") + 3 + OLED_GetWidthStr(">") + 3 + OLED_GetWidthStr("30") + 3, 13 * 2, 1);
			OLED_DrawChar_Modified(')', 12 + OLED_GetWidthStr("(") + 2 + OLED_GetWidthStr("30") + 3 + OLED_GetWidthStr("vby") + 3 + OLED_GetWidthStr(">") + 3 + OLED_GetWidthStr("t") + 3 + OLED_GetWidthStr(">") + 3 + OLED_GetWidthStr("30") + 3 + OLED_GetWidthStr("ctr") - 10, 13 * 2, 1);

			switch(pos_cursor)
			{
				case 3:

					/*
					*------------------------
					* 		Кадр 23-24
					*------------------------
					*	Настройка времени
					*	автоотключения экрана
					*
					*	 	Установите время
					*	 	автооткл. экрана
					*	 	(31 мин > t > 29 сек)
					*	>	-- мин -- сек
					*		Сохранить
					*========================
					*		Назад
					*------------------------
					*/

					// Началась установка
					// времени автоотключения экрана
					if (f_StSetScreenOff)
					{
						OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 1);

						// Функция настройки времени
						// автоотключения экрана
						SetScreenOff();
					}

					else
					{
						// Если время автоотключения
						// экрана установлено
						if (f_ScreenOffIsSet)
						{
							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);
							DrawSetScreenOff(ptr_ScreenOffSet, 0);
						}

						else
						{
							OLED_DrawStr_EnModified("--", 16, 13 * 3, 0);
							OLED_DrawStr_RuModified("vby", 16 + OLED_GetWidthStr("--") + 2, 13 * 3, 0);
							OLED_DrawStr_EnModified("--", 3 + 16 + OLED_GetWidthStr("--") + OLED_GetWidthStr("vby") + 2 * 2, 13 * 3, 0);
							OLED_DrawStr_RuModified("ctr", 3 + 16 + OLED_GetWidthStr("--") * 2 + OLED_GetWidthStr("vby") + 2 * 3, 13 * 3, 0);

							OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 3.5 - 3, 3, (int16_t) 13 * 3.5 + 3, 9, (int16_t) 13 * 3.5, 0);
						}
					}

					// Проверка того, что время
					// автоотключения экрана было установлено
					IsSetScreenOffFinished();

					OLED_DrawStr_RuModified("Cj[hfybnm", 12, 13 * 4, 1);

				break;


				case 4:

					/*
					*------------------------
					* 		Кадр 23-24
					*------------------------
					*	Настройка времени
					*	автоотключения экрана
					*
					*	 	Установите время
					*	 	автоотключения
					*	 	экрана (< 30 мин)
					*		-- мин -- сек
					*	>	Сохранить
					*========================
					*		Назад
					*------------------------
					*/

					// Проверка того, что время
					// автоотключения экрана было установлено
					IsSetScreenOffFinished();

					if (f_ScreenOffIsSet)
						DrawSetScreenOff(ptr_ScreenOffSet, 1);
					else
					{
						OLED_DrawStr_EnModified("--", 16, 13 * 3, 1);
						OLED_DrawStr_RuModified("vby", 16 + OLED_GetWidthStr("--") + 2, 13 * 3, 1);
						OLED_DrawStr_EnModified("--", 3 + 16 + OLED_GetWidthStr("--") + OLED_GetWidthStr("vby") + 2 * 2, 13 * 3, 1);
						OLED_DrawStr_RuModified("ctr", 3 + 16 + OLED_GetWidthStr("--") * 2 + OLED_GetWidthStr("vby") + 2 * 3, 13 * 3, 1);
					}

					OLED_DrawStr_RuModified("Cj[hfybnm", 12, 13 * 4, 0);
					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);

				break;
			}

		break;


		// 24. Настройка времени автоотключения экрана (2)
		case 24:

			switch(pos_cursor)
			{
				case 0:

					/*
					*------------------------
					* 		Кадр 23-24
					*------------------------
					*	Настройка времени
					*	автоотключения экрана
					*
					*	 	Установите время
					*	 	автоотключения
					*	 	экрана (< 30 мин)
					*		-- мин -- сек
					*		Сохранить
					*========================
					*	>	Назад
					*------------------------
					*/

					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 0, 0);
					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 0.5 - 3, 3, (int16_t) 13 * 0.5 + 3, 9, (int16_t) 13 * 0.5, 0);

				break;
			}

		break;


		// 25. Текущее время автоотключения экрана
		case 25:

			OLED_DrawStr_RuModified("Ntreott", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("dhtvz", 12 + OLED_GetWidthStr("Ntreott") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("fdnjjnrk.xtybz", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("'rhfyf", 12, 13 * 2, 1);
			OLED_DrawChar_Modified(':', 12 + OLED_GetWidthStr("'rhfyf") + 1, 13 * 2, 1);

			if ((!ScreenOff.minutes_screen_off) &&
				(!ScreenOff.seconds_screen_off))
			{
				OLED_DrawStr_RuModified("Jnrk.xtyj", 12, 13 * 3, 1);
			}

			else
				DrawSetScreenOff(ptr_ScreenOff, 1);

			switch(pos_cursor)
			{
				case 4:

					/*
					*------------------------
					* 		Кадр 25
					*------------------------
					*	Автоотключение экрана
					*
					*		Текущее время
					*		автоотключения
					*		экрана:
					*		-- мин -- сек / отключено
					*	>	Назад
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 4, 0);

				break;
			}

		break;


		// 26. Сообщение успешного
		// экстренного завершения полива
		case 26:

			OLED_DrawStr_RuModified("Ntreobq", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("gjkbd", 12 + OLED_GetWidthStr("Ntreobq") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("ecgtiyj", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("pfdthity", 12 + OLED_GetWidthStr("ecgtiyj") + 3, 13 * 1, 1);
			OLED_DrawChar_Modified('.', 12 + OLED_GetWidthStr("ecgtiyj") + 3 + OLED_GetWidthStr("pfdthity") + 1, 13 * 1, 1);

			switch(pos_cursor)
			{
				case 2:

					/*
					*------------------------
					* 		Кадр 26
					*------------------------
					*	Немедленно завершить полив
					*
					*		Текущий полив
					*		успешно завершен.
					*
					*	>	Ок
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
					OLED_DrawStr_RuModified("Jr", 12, 13 * 2, 0);

				break;
			}

		break;


		// 27. Сообщение о том,
		// что полив не выполняется
		case 27:

			OLED_DrawStr_RuModified("Gjkbd", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("ctqxfc", 12 + OLED_GetWidthStr("Gjkbd") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("yt", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("dsgjkyztncz", 12 + OLED_GetWidthStr("yt") + 3, 13 * 1, 1);
			OLED_DrawChar_Modified('!', 12 + OLED_GetWidthStr("yt") + 3 + OLED_GetWidthStr("dsgjkyztncz") + 2, 13 * 1, 1);

			switch(pos_cursor)
			{
				case 2:

					/*
					*------------------------
					* 		Кадр 27
					*------------------------
					*	Немедленно завершить полив
					*
					*		Полив сейчас
					*		не выполняется!
					*
					*	>	Ок
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
					OLED_DrawStr_RuModified("Jr", 12, 13 * 2, 0);

				break;
			}

		break;


		// 28. Уведомление о необходимости
		// пополнить резервуар
		case 28:

			OLED_DrawStr_RuModified("Ytljcnfnjxyj", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("djls", 12 + OLED_GetWidthStr("Ytljcnfnjxyj") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("lkz", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("gjkbdf", 12 + OLED_GetWidthStr("lkz") + 3, 13 * 1, 1);
			OLED_DrawChar_Modified('.', 12 + OLED_GetWidthStr("lkz") + 3 + OLED_GetWidthStr("gjkbdf") + 1, 13 * 1, 1);
			OLED_DrawStr_RuModified("Nht,e", 12 + OLED_GetWidthStr("lkz") + 3 + OLED_GetWidthStr("gjkbdf") + 1 + OLED_GetWidthStr(".") + 2 + 9, 13 * 1, 1);
			OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("lkz") + 3 + OLED_GetWidthStr("gjkbdf") + 1 + OLED_GetWidthStr(".") + 2 + 9 + 16, 13 * 1, 1);
			OLED_DrawStr_RuModified("tncz", 12, 13 * 2, 1);
			OLED_DrawStr_RuModified("yfgjkybnm", 12 + OLED_GetWidthStr("tncz") + 3, 13 * 2, 1);
			OLED_DrawStr_RuModified("htpthdefh", 12, 13 * 3, 1);
			OLED_DrawChar_Modified('.', 12 + OLED_GetWidthStr("htpthdefh") + 1, 13 * 3, 1);

			switch(pos_cursor)
			{
				case 4:

					/*
					*------------------------
					* 		Кадр 28
					*------------------------
					*	Уведомление о необходимости
					*	пополнить резервуар
					*
					*		Недостаточно воды
					*		для полива. Требу-
					*		ется наполнить
					*		резервуар.
					*
					*	>	Ок
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
					OLED_DrawStr_RuModified("Jr", 12, 13 * 4, 0);

				break;
			}

		break;


		// 29. Сообщение о том, что полив не будет
		// выполнен из-за недостатка воды (1)
		case 29:

			/*
			*------------------------
			* 		Кадр 29-30
			*------------------------
			*
			*		Текущий полив не
			*		будет выполнен
			*		из-за недостатка
			*		воды в резервуа-
			*	>	ре.
			*========================
			*		Необходимо попол-
			*		нить резервуар.
			*
			*		Ок
			*------------------------
			*/

			OLED_DrawStr_RuModified("Ntreobq", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("gjkbd", 12 + OLED_GetWidthStr("Ntreobq") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("yt", 12 + OLED_GetWidthStr("Ntreobq") + 3 + OLED_GetWidthStr("gjkbd") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified(",eltn", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("dsgjkyty", 12 + OLED_GetWidthStr(",eltn") + 3, 13 * 1, 1);
			OLED_DrawStr_RuModified("bp", 12, 13 * 2, 1);
			OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("bp") + 1, 13 * 2, 1);
			OLED_DrawStr_RuModified("pf", 12 + OLED_GetWidthStr("bp") + 1 + OLED_GetWidthStr("-") + 1 - 2, 13 * 2, 1);
			OLED_DrawStr_RuModified("ytljcnfnrf", 12 + OLED_GetWidthStr("bp") + 1 + OLED_GetWidthStr("-") + 1 + OLED_GetWidthStr("pf") + 3, 13 * 2, 1);
			OLED_DrawStr_RuModified("djls", 12, 13 * 3, 1);
			OLED_DrawStr_RuModified("d", 12 + OLED_GetWidthStr("djls") + 3, 13 * 3, 1);
			OLED_DrawStr_RuModified("htpthdef", 12 + OLED_GetWidthStr("djls") + 3 + OLED_GetWidthStr("d") + 3, 13 * 3, 1);
			OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("djls") + 3 + OLED_GetWidthStr("d") + 3 + OLED_GetWidthStr("htpthdef") + 1, 13 * 3, 1);
			OLED_DrawStr_RuModified("ht", 12, 13 * 4, 1);
			OLED_DrawChar_Modified('.', 12 + OLED_GetWidthStr("ht") + 1, 13 * 4, 1);

		break;


		// 30. Сообщение о том, что полив не будет
		// выполнен из-за недостатка воды (2)
		case 30:

			OLED_DrawStr_RuModified("Ytj,[jlbvj", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("gjgjk", 12 + OLED_GetWidthStr("Ytj,[jlbvj") + 3, 13 * 0, 1);
			OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("Ytj,[jlbvj") + 3 + OLED_GetWidthStr("gjgjk") + 1, 13 * 0, 1);
			OLED_DrawStr_RuModified("ybnm", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("htpthdefh", 12 + OLED_GetWidthStr("ybnm") + 3, 13 * 1, 1);
			OLED_DrawChar_Modified('.', 12 + OLED_GetWidthStr("ybnm") + 3 + OLED_GetWidthStr("htpthdefh") + 1, 13 * 1, 1);

			switch(pos_cursor)
			{
				case 2:

					/*
					*------------------------
					* 		Кадр 29-30
					*------------------------
					*
					*		Текущий полив не
					*		будет выполнен
					*		из-за недостатка
					*		воды в резервуа-
					*		ре.
					*========================
					*		Необходимо попол-
					*		нить резервуар.
					*
					*	>	Ок
					*------------------------
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
					OLED_DrawStr_RuModified("Jr", 12, 13 * 2, 0);

				break;
			}

		break;

		// 31. Сообщение о том, что датчик
		// уровня воды будет выключен/включен (1)
		case 31:

			/*
			*------------------------
			*		Кадр 31
			*------------------------
			*	Сообщение о том, что датчик
			*	уровня воды будет выключен/включен
			*
			*		(Отключение/включение) датчика
			*		уровня воды приве-
			*		дёт к (отключению/включению)
			*		лампочек на панели
			*	>	управления.
			*------------------------
			*		Следить за уровнем
			*		воды в резервуаре
			*		(не) нужно будет
			*		вручную.
			*
			*		Ок
			*------------------------
			*/

			// Если датчик воды стал включен
			if (WaterLevelSensorState.f_IsWaterLevelSensorOn)
			{
				OLED_DrawStr_RuModified("Jnrk.xtybt", 12, 13 * 0, 1);
				OLED_DrawStr_RuModified("lfnxbrf", 12 + OLED_GetWidthStr("Jnrk.xtybt") + 3, 13 * 0, 1);

				OLED_DrawStr_RuModified("jnrk.xtyb.", 12 + OLED_GetWidthStr("ltn") + 3 + OLED_GetWidthStr("r") + 3, 13 * 2, 1);
			}

			// Если датчик воды стал отключен
			else
			{
				OLED_DrawStr_RuModified("Drk.xtybt", 12, 13 * 0, 1);
				OLED_DrawStr_RuModified("lfnxbrf", 12 + OLED_GetWidthStr("Drk.xtybt") + 3, 13 * 0, 1);

				OLED_DrawStr_RuModified("drk.xtyb.", 12 + OLED_GetWidthStr("ltn") + 3 + OLED_GetWidthStr("r") + 3, 13 * 2, 1);
			}

			OLED_DrawStr_RuModified("ehjdyz", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("djls", 12 + OLED_GetWidthStr("ehjdyz") + 3, 13 * 1, 1);
			OLED_DrawStr_RuModified("ghbdt", 12 + OLED_GetWidthStr("ehjdyz") + 3 + OLED_GetWidthStr("djls") + 3, 13 * 1, 1);
			OLED_DrawChar_Modified('-', 12 + OLED_GetWidthStr("ehjdyz") + 3 + OLED_GetWidthStr("djls") + 3 + OLED_GetWidthStr("ghbdt") + 1, 13 * 1, 1);
			OLED_DrawStr_RuModified("ltn", 12, 13 * 2, 1);
			OLED_DrawStr_RuModified("r", 12 + OLED_GetWidthStr("ltn") + 3, 13 * 2, 1);

			OLED_DrawStr_RuModified("kfvgjxtr", 12, 13 * 3, 1);
			OLED_DrawStr_RuModified("yf", 12 + OLED_GetWidthStr("kfvgjxtr") + 3, 13 * 3, 1);
			OLED_DrawStr_RuModified("gfytkb", 12 + OLED_GetWidthStr("kfvgjxtr") + 3 + OLED_GetWidthStr("yf") + 3, 13 * 3, 1);
			OLED_DrawStr_RuModified("eghfdktybz", 12, 13 * 4, 1);
			OLED_DrawChar_Modified('.', 12 + OLED_GetWidthStr("eghfdktybz") + 1, 13 * 4, 1);

		break;


		// 32. Сообщение о том, что датчик
		// уровня воды будет выключен/включен (2)
		case 32:

			/*
			*------------------------
			*		Кадр 32
			*------------------------
			*	Сообщение о том, что датчик
			*	уровня воды будет выключен/включен
			*
			*		(Отключение/включение) датчика
			*		уровня воды приве-
			*		дёт к (отключению/включению)
			*		лампочек на панели
			*		управления.
			*------------------------
			*		Следить за уровнем
			*		воды в резервуаре
			*		(не) нужно будет
			*		вручную.
			*
			*	>	Ок
			*------------------------
			*/

			// Если датчик уровня воды стал включен
			if (WaterLevelSensorState.f_IsWaterLevelSensorOn)
			{
				OLED_DrawStr_RuModified("ye;yj", 12, 13 * 2, 1);
				OLED_DrawStr_RuModified(",eltn", 12 + OLED_GetWidthStr("ye;yj") + 3, 13 * 2, 1);
			}

			// Если датчик уровня воды стал отключен
			else
			{
				OLED_DrawStr_RuModified("yt", 12, 13 * 2, 1);
				OLED_DrawStr_RuModified("ye;yj", 12 + OLED_GetWidthStr("yt") + 3, 13 * 2, 1);
				OLED_DrawStr_RuModified(",eltn", 12 + OLED_GetWidthStr("yt") + 3 + OLED_GetWidthStr("ye;yj") + 3, 13 * 2, 1);
			}

			OLED_DrawStr_RuModified("Cktlbnm", 12, 13 * 0, 1);
			OLED_DrawStr_RuModified("pf", 12 + OLED_GetWidthStr("Cktlbnm") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("ehjdytv", 12 + OLED_GetWidthStr("Cktlbnm") + 3 + OLED_GetWidthStr("pf") + 3, 13 * 0, 1);
			OLED_DrawStr_RuModified("djls", 12, 13 * 1, 1);
			OLED_DrawStr_RuModified("d", 12 + OLED_GetWidthStr("djls") + 3, 13 * 1, 1);
			OLED_DrawStr_RuModified("htpthdefht", 12 + OLED_GetWidthStr("djls") + 3 + OLED_GetWidthStr("d") + 3, 13 * 1, 1);
			OLED_DrawStr_RuModified("dhexye.", 12, 13 * 3, 1);
			OLED_DrawChar_Modified('.', 12 + OLED_GetWidthStr("dhexye.") + 1, 13 * 3, 1);

			OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 4.5 - 3, 3, (int16_t) 13 * 4.5 + 3, 9, (int16_t) 13 * 4.5, 0);
			OLED_DrawStr_RuModified("Jr", 12, 13 * 4, 0);

		break;


		// 33. Сброс настроек (3)
		case 33:

			switch(pos_cursor)
			{
				case 0:

					/*
					*------------------------
					* 		Кадр 20-21-33
					*------------------------
					*	Сброс настроек
					*
					*	 	Сбросить
					*	 	настройки времени
					*
					*	 	Сбросить
					*	 	настройки полива
					*
					*	 	Сбросить авто-
					*===========================
					*	 	Сбросить авто-
					*	 	отключение экрана
					*
					*	 	Сбросить датчик
					*	 	уровня воды
					*
					*	 	Сбросить
					*===========================
					*	>	Сбросить
					*	 	все настройки
					*
					*	 	Назад
					*/

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 1 - 3, 3, (int16_t) 13 * 1 + 3, 9, (int16_t) 13 * 1, 0);
					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 0, 0);
					OLED_DrawStr_RuModified("dct", 12, 13 * 1, 0);
					OLED_DrawStr_RuModified("yfcnhjqrb", 12 + OLED_GetWidthStr("dct") + 3, 13 * 1, 0);

					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 2, 1);

				break;

				case 2:

					/*
					*------------------------
					* 		Кадр 20-21-33
					*------------------------
					*	Сброс настроек
					*
					*	 	Сбросить
					*	 	настройки времени
					*
					*	 	Сбросить
					*	 	настройки полива
					*
					*	 	Сбросить авто-
					*===========================
					*	 	Сбросить авто-
					*	 	отключение экрана
					*
					*	 	Сбросить датчик
					*	 	уровня воды
					*
					*	 	Сбросить
					*===========================
					*	>	Сбросить
					*	 	все настройки
					*
					*	 	Назад
					*/

					OLED_DrawStr_RuModified("C,hjcbnm", 12, 13 * 0, 1);
					OLED_DrawStr_RuModified("dct", 12, 13 * 1, 1);
					OLED_DrawStr_RuModified("yfcnhjqrb", 12 + OLED_GetWidthStr("dct") + 3, 13 * 1, 1);

					OLED_DrawTriangleFill_Modified(3, (int16_t) 13 * 2.5 - 3, 3, (int16_t) 13 * 2.5 + 3, 9, (int16_t) 13 * 2.5, 0);
					OLED_DrawStr_RuModified("Yfpfl", 12, 13 * 2, 0);

				break;
			}

		break;


		default:
		break;
	}
}


// Проверка смены кадра
void IsFrameChanged(void)
{
	new_frame = current_frame;

	if (new_frame != old_frame)
		f_FrameChanged = 1;
	else
		f_FrameChanged = 0;

	old_frame = new_frame;
}

//============================================================================
//	Процедуры курсора
//============================================================================

// Проверка запрета на отрисовку курсора
void BlockDrawCursor(void)
{
	switch(current_frame)
	{
		case 1:
				f_NoDrawCursor = 1;
		break;


		case 7:
				if (pos_cursor == 2)
				{
					// В процессе настройки времени
					// курсор не должен отрисовываться
					if (f_StSetTimeWater)
						f_NoDrawCursor = 1;

					// После завершения настройки времени
					// курсор должен отрисовываться
					else
					if ((!f_StSetTimeWater) && (f_TimeIsSetWater))
						f_NoDrawCursor = 0;

					// Если настройка дня недели не началась,
					// то курсор отрисовывается
					else
						f_NoDrawCursor = 0;
				}
		break;


		case 8:
				if (pos_cursor == 2)
				{
					// В процессе настройки времени
					// курсор не должен отрисовываться
					if (f_StSetDurationWater)
						f_NoDrawCursor = 1;

					// После завершения настройки времени
					// курсор должен отрисовываться
					else
					if ((!f_StSetDurationWater) && (f_TimeIsSetDurationWater))
						f_NoDrawCursor = 0;

					// Если настройка дня недели не началась,
					// то курсор отрисовывается
					else
						f_NoDrawCursor = 0;
				}
		break;


		case 12:

				if (pos_cursor == 2)
				{
					// В процессе настройки даты
					// курсор не должен отрисовываться
					if (f_StSetDate)
						f_NoDrawCursor = 1;

					// После завершения настройки даты
					// курсор должен отрисовываться
					else
					if ((!f_StSetDate) && (f_DateIsSet))
						f_NoDrawCursor = 0;

					// Если настройка даты не началась,
					// то курсор отрисовывается
					else
						f_NoDrawCursor = 0;
				}

		break;


		case 13:
				if (pos_cursor == 2)
				{
					// В процессе настройки дня недели
					// курсор не должен отрисовываться
					if (f_StSetDayOfWeek)
						f_NoDrawCursor = 1;

					// После завершения настройки дня недели
					// курсор должен отрисовываться
					else
					if ((!f_StSetDayOfWeek) && (f_DayOfWeekIsSet))
						f_NoDrawCursor = 0;

					// Если настройка дня недели не началась,
					// то курсор отрисовывается
					else
						f_NoDrawCursor = 0;
				}
		break;


		case 14:
				if (pos_cursor == 2)
				{
					// В процессе настройки времени
					// курсор не должен отрисовываться
					if (f_StSetTime)
						f_NoDrawCursor = 1;

					// После завершения настройки времени
					// курсор должен отрисовываться
					else
					if ((!f_StSetTime) && (f_TimeIsSet))
						f_NoDrawCursor = 0;

					// Если настройка дня недели не началась,
					// то курсор отрисовывается
					else
						f_NoDrawCursor = 0;
				}
		break;


		case 19:
				if (pos_cursor == 3)
				{
					if (f_StSetDaysWater)
						f_NoDrawCursor = 1;
					else
					if ((!f_StSetDaysWater) && (f_DaysWaterIsSet))
						f_NoDrawCursor = 0;
					else
						f_NoDrawCursor = 0;
				}
		break;


		case 23:
				if (pos_cursor == 3)
				{
					if (f_StSetScreenOff)
						f_NoDrawCursor = 1;
					else
					if ((!f_StSetScreenOff) && (f_ScreenOffIsSet))
						f_NoDrawCursor = 0;
					else
						f_NoDrawCursor = 0;
				}
		break;


		case 29:
				f_NoDrawCursor = 1;
		break;


		case 31:
				f_NoDrawCursor = 1;
		break;


		default:
				f_NoDrawCursor = 0;
		break;
	}
}


// Отрисовка курсора на дисплее
void DrawCursor(void)
{
	// Проверка запрета на отрисовку курсора
	BlockDrawCursor();

	if (!f_NoDrawCursor)
	{
		// Отрисовка курсора на дисплее с учётом ширины курсора
		// (курсор становится уже, если достигает последней строчки.
		//  иначе одна из сторон прямоугольного курсора не видна)
		if ((pos_cursor + poss_ix[pos_cursor]) == 5)
			OLED_DrawRectangleFill(0, pos_cursor * 13, 125, (pos_cursor + poss_ix[pos_cursor]) * 13 - 2, 1);
		else
			OLED_DrawRectangleFill(0, pos_cursor * 13, 125, (pos_cursor + poss_ix[pos_cursor]) * 13 - 1, 1);
	}
}


// Определение возможных позиций
// курсора в текущем кадре
void CursorPossiblePositions(void)
{
	uint8_t i = 0;

	switch(current_frame)
	{
		case 0:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_0[0], sizeof(poss_ix));
		break;


		case 1:
				for (i = 0; i < 5; i++)
					poss_ix[i] = info_frame_0[4];
		break;


		case 2:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_2[0], sizeof(poss_ix));
		break;


		case 3:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_3[0], sizeof(poss_ix));
		break;


		case 4:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_4[0], sizeof(poss_ix));
		break;


		case 5:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_5[0], sizeof(poss_ix));
		break;


		case 6:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_6[0], sizeof(poss_ix));
		break;


		case 7:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_7[0], sizeof(poss_ix));
		break;


		case 8:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_8[0], sizeof(poss_ix));
		break;


		case 10:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_10[0], sizeof(poss_ix));
		break;


		case 12:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_12[0], sizeof(poss_ix));
		break;


		case 13:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_13[0], sizeof(poss_ix));
		break;


		case 14:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_14[0], sizeof(poss_ix));
		break;


		case 15:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_15[0], sizeof(poss_ix));
		break;


		case 16:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_16[0], sizeof(poss_ix));
		break;


		case 17:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_17[0], sizeof(poss_ix));
		break;


		case 18:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_18[0], sizeof(poss_ix));
		break;


		case 19:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_19[0], sizeof(poss_ix));
		break;


		case 20:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_20[0], sizeof(poss_ix));
		break;


		case 21:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_21[0], sizeof(poss_ix));
		break;


		case 22:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_22[0], sizeof(poss_ix));
		break;


		case 23:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_23[0], sizeof(poss_ix));
		break;


		case 24:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_24[0], sizeof(poss_ix));
		break;


		case 25:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_25[0], sizeof(poss_ix));
		break;


		case 26:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_26[0], sizeof(poss_ix));
		break;


		case 27:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_27[0], sizeof(poss_ix));
		break;


		case 28:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_28[0], sizeof(poss_ix));
		break;


		case 29:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_29[0], sizeof(poss_ix));
		break;


		case 30:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_30[0], sizeof(poss_ix));
		break;


		case 31:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_31[0], sizeof(poss_ix));
		break;


		case 32:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_32[0], sizeof(poss_ix));
		break;


		case 33:
				memcpy((char*) &poss_ix[0], (char*) &poss_ix_33[0], sizeof(poss_ix));
		break;


		default:
		break;
	}
}


// Установка начальных возможных
// позиций курсора (кадр №0)
void DefaultCursorPositions(void)
{
	short i = 0;
	uint8_t start_ix = 0;

	for (i = 4; i >= 0; i--)
	{
		poss_ix[i] = poss_ix_0[i];

		if (poss_ix[i])
			start_ix = i;
	}

	pos_cursor = start_ix;
}


// Установка новых возможных позиций курсора
void SetCursorPositions(void)
{
	// Проверка смены кадра
	IsFrameChanged();

	// Если кадр сменился,
	// то устанавливаются новые позиции курсора
	if (f_FrameChanged)
		CursorPossiblePositions();
}

// Проверка необходимости запрета
// перемещения курсора вниз
void BlockCursorMoveDown(void)
{
	switch(current_frame)
	{
		case 0:
				if (pos_cursor == 3)
					f_BlockDown = 1;
				else
					f_BlockDown = 0;
		break;


		case 2:
				if (pos_cursor == 2)
					f_BlockDown = 1;
				else
					f_BlockDown = 0;
		break;


		case 3:
				f_BlockDown = 1;
		break;


		case 5:
				if (pos_cursor == 3)
					f_BlockDown = 1;
				else
					f_BlockDown = 0;
		break;


		case 18:
				if (pos_cursor == 4)
					f_BlockDown = 1;
				else
					f_BlockDown = 0;
		break;


		case 22:
				f_BlockDown = 1;
		break;


		case 24:
				if (pos_cursor == 0)
					f_BlockDown = 1;
				else
					f_BlockDown = 0;
		break;


		case 25:
				f_BlockDown = 1;
		break;


		case 26:
				f_BlockDown = 1;
		break;


		case 27:
				f_BlockDown = 1;
		break;


		case 33:

			if (pos_cursor == 2)
				f_BlockDown = 1;
			else
				f_BlockDown = 0;

		break;


		default:
				f_BlockDown = 0;
		break;
	}
}


// Проверка необходимости запрета
// перемещения курсора вверх
void BlockCursorMoveUp(void)
{
	switch(current_frame)
	{
		case 0:
				if (pos_cursor == 0)
					f_BlockUp = 1;
				else
					f_BlockUp = 0;
		break;


		case 1:
				if (pos_cursor == 4)
					f_BlockUp = 1;
				else
					f_BlockUp = 0;
		break;


		case 3:
				f_BlockUp = 1;
		break;


		case 4:
				if (pos_cursor == 0)
					f_BlockUp = 1;
				else
					f_BlockUp = 0;
		break;


		case 15:
				if (pos_cursor == 0)
					f_BlockUp = 1;
				else
					f_BlockUp = 0;
		break;


		case 20:
				if (pos_cursor == 0)
					f_BlockUp = 1;
				else
					f_BlockUp = 0;
		break;


		case 22:
				f_BlockUp = 1;
		break;


		case 23:
				if (pos_cursor == 3)
					f_BlockUp = 1;
				else
					f_BlockUp = 0;
		break;


		case 25:
				f_BlockUp = 1;
		break;


		case 26:
				f_BlockUp = 1;
		break;


		case 27:
				f_BlockUp = 1;
		break;


		default:
				f_BlockUp = 0;
		break;
	}
}


//============================================================================
//	Обработка кнопок
//============================================================================

// Обработка нажатий на кнопки
void ButtonsPress(void)
{
	// Для циклов
	// (обязательно знаковое число)
	short i = 0;

	// Флаг остановки поиска
	// строки для курсора
	uint8_t f_stop_search = 0;


	// Проверка состояния кнопок
	keys = KEYB_Inkeys();


	// Если произошло нажатие на любую кнопку
	if ((keys & (1 << KEYB_DOWN)) ||
		(keys & (1 << KEYB_UP))	||
		(keys & (1 << KEYB_LEFT)) ||
		(keys & (1 << KEYB_RIGHT)) ||
		(keys & (1 << KEYB_OK)))
	{
		if (f_WaitSecondPress)
			f_SecondPress = 1;

		if (f_SleepMode)
		{
			// Сброс "спящего" режима
			f_SleepMode = 0;

			// Сброс отрисовки кадра со смайликом
			f_DrawFrameSleepMode = 0;
			SecondsStartSleepMode_CNT = 0;

			// Флаг выхода из "спящего" режима
			f_SleepModeExit = 1;

			// Ожидание второго нажатия
			// для разблокировки кнопок
			// при выходе из "спящего" режима
			f_WaitSecondPress = 1;
		}

		// Сброс отсчёта секунд
		// бездействия пользователя
		SecondsSleepMode_CNT = 0;
		TIM3->CNT = 0;
	}

	// Если был выход из "спящего" режима
	if (f_SleepModeExit)
	{
		// Произошло второе нажатие
		if (f_SecondPress)
		{
			// Разблокировка кнопок
			f_BlockButtons = 0;

			// Сброс флагов
			f_SleepModeExit = 0;
			f_SecondPress = 0;
			f_WaitSecondPress = 0;
		}

		// Произошло первое нажатие
		else
			// Блокировка кнопок
			f_BlockButtons = 1;
	}


	// Если нажата кнопка вниз
	if (keys & (1 << KEYB_DOWN))
	{
		if (!f_BlockButtons)
		{
			// Проверка необходимости запрета
			// перемещения курсора вниз
			BlockCursorMoveDown();

			// Переключение с кадра 1 на 2
			// (с переносом курсора)
			if ((current_frame == 1) && (pos_cursor == 4))
			{
				current_frame = 2;
				pos_cursor = 3;
			}

			// Переключение с кадра 4 на 5
			else
			if ((current_frame == 4) && (pos_cursor == 3))
			{
				current_frame = 5;
				pos_cursor = 0;
			}

			// Уменьшение цифры при установке
			// времени начала полива
			else
			if ((current_frame == 7) && (f_StSetTimeWater))
			{
				if (CurrentNumberSetTimeWater > 0)
					CurrentNumberSetTimeWater--;

				NumberSetTimeWater[CurrentDigitSetTimeWater] = CurrentNumberSetTimeWater;
			}

			// Уменьшение цифры при установке
			// продолжительности полива
			else
			if ((current_frame == 8) && (f_StSetDurationWater))
			{
				if (CurrentNumberSetDurationWater > 0)
					CurrentNumberSetDurationWater--;

				NumberSetDurationWater[CurrentDigitSetDurationWater] = CurrentNumberSetDurationWater;
			}

			// Уменьшение цифры при установке даты
			else
			if ((current_frame == 12) && (f_StSetDate))
			{
				if (CurrentNumberSetDate > 0)
					CurrentNumberSetDate--;

				NumberSetDate[CurrentDigitSetDate] = CurrentNumberSetDate;
			}

			// Уменьшение цифры (дня недели)
			// при установке дня недели
			else
			if ((current_frame == 13) && (f_StSetDayOfWeek))
			{
				if (NumberSetDayOfWeek > 1)
					NumberSetDayOfWeek--;
			}

			// Уменьшение цифры при установке времени
			else
			if ((current_frame == 14) && (f_StSetTime))
			{
				if (CurrentNumberSetTime > 0)
					CurrentNumberSetTime--;

				NumberSetTime[CurrentDigitSetTime] = CurrentNumberSetTime;
			}

			// Переключение с кадра 15 на 18
			else
			if ((current_frame == 15) && (pos_cursor == 4))
			{
				current_frame = 18;
				pos_cursor = 0;
			}

			// Сброс дня при настройке дней полива
			else
			if ((current_frame == 19) && (f_StSetDaysWater))
			{
				switch(CurrentDigitSetDaysWater)
				{
					case 0:
							WaterDays &= ~0x01;
					break;

					case 1:
							WaterDays &= ~0x02;
					break;

					case 2:
							WaterDays &= ~0x04;
					break;

					case 3:
							WaterDays &= ~0x08;
					break;

					case 4:
							WaterDays &= ~0x10;
					break;

					case 5:
							WaterDays &= ~0x20;
					break;

					case 6:
							WaterDays &= ~0x40;
					break;
				}
			}

			// Переключение с кадра 20 на 21
			else
			if ((current_frame == 20) && (pos_cursor == 4))
			{
				current_frame = 21;
				pos_cursor = 0;
			}

			// Уменьшение цифры при установке
			// времени автоотключения экрана
			else
			if ((current_frame == 23) && (f_StSetScreenOff))
			{
				if (CurrentNumberSetScreenOff > 0)
					CurrentNumberSetScreenOff--;

				NumberSetScreenOff[CurrentDigitSetScreenOff] = CurrentNumberSetScreenOff;
			}

			// Переключение с кадра 23 на 24
			else
			if ((current_frame == 23) && (pos_cursor == 4))
			{
				current_frame = 24;
				pos_cursor = 0;
			}

			// Переключение с кадра 29 на 30
			else
			if ((current_frame == 29) && (pos_cursor == 4))
			{
				current_frame = 30;
				pos_cursor = 2;
			}

			// Переключение с кадра 31 на 32
			else
			if ((current_frame == 31) && (pos_cursor == 4))
			{
				current_frame = 32;
				pos_cursor = 4;
			}

			// Переключение с кадра 21 на 33
			else
			if ((current_frame == 21) && (pos_cursor == 4))
			{
				current_frame = 33;
				pos_cursor = 0;
			}

			// Перемещение курсора вниз
			else
			if (!f_BlockDown)
			{
				// Если курсор внизу дисплея,
				// то ищем новое положение
				// курсора с первой строки дисплея
				if (pos_cursor == 4)
				{
					for (i = 0; i < 5; i++)
					{
						if (poss_ix[i])
						{
							pos_cursor = i;
							break;
						}
					}
				}


				// Если есть строки ниже,
				// то проверяем, можно ли
				// перенести на них курсор
				else
				{
					for (i = pos_cursor + 1; i < 5; i++)
					{
						if (poss_ix[i])
						{
							pos_cursor = i;
							f_stop_search = 1;
							break;
						}
					}

					// Если среди строк ниже не нашлось строк,
					// на которые можно перенести курсор, то
					// начинаем поиск с первой строки дисплея
					if (!f_stop_search)
					{
						for (i = 0; i < 5; i++)
						{
							if (poss_ix[i])
							{
								pos_cursor = i;
								break;
							}
						}
					}
				}
			}
		}
	}


	// Если нажата кнопка вверх
	if (keys & (1 << KEYB_UP))
	{
		if (!f_BlockButtons)
		{
			// Проверка необходимости запрета
			// перемещения курсора вверх
			BlockCursorMoveUp();

			// Переключение с кадра 2 на 1
			if ((current_frame == 2) && (pos_cursor == 3))
			{
				current_frame = 1;
				pos_cursor = 4;
			}

			// Переключение с кадра 5 на 4
			else
			if ((current_frame == 5) && (pos_cursor == 0))
			{
				current_frame = 4;
				pos_cursor = 3;
			}

			// Увеличение цифры при установке
			// времени начала полива
			else
			if ((current_frame == 7) && (f_StSetTimeWater))
			{
				if (CurrentNumberSetTimeWater < 9)
					CurrentNumberSetTimeWater++;

				NumberSetTimeWater[CurrentDigitSetTimeWater] = CurrentNumberSetTimeWater;


				// Ограничение ввода количества
				// часов, минут, секунд
				switch(CurrentDigitSetTimeWater)
				{
					// Ограничение для часов - не более 2-х десятков
					case 0:
							if (NumberSetTimeWater[CurrentDigitSetTimeWater] > 2)
							{
								CurrentNumberSetTimeWater = 2;
								NumberSetTimeWater[CurrentDigitSetTimeWater] = 2;
							}
					break;

					// Ограничение для минут - не более 5-ти десятков
					case 2:
							if (NumberSetTimeWater[CurrentDigitSetTimeWater] > 5)
							{
								CurrentNumberSetTimeWater = 5;
								NumberSetTimeWater[CurrentDigitSetTimeWater] = 5;
							}
					break;

					// Ограничение для секунд - не более 5-ти десятков
					case 4:
							if (NumberSetTimeWater[CurrentDigitSetTimeWater] > 5)
							{
								CurrentNumberSetTimeWater = 5;
								NumberSetTimeWater[CurrentDigitSetTimeWater] = 5;
							}
					break;

					default:
					break;
				}
			}

			// Увеличение цифры при установке продолжительности полива
			else
			if ((current_frame == 8) && (f_StSetDurationWater))
			{
				if (CurrentNumberSetDurationWater < 9)
					CurrentNumberSetDurationWater++;

				NumberSetDurationWater[CurrentDigitSetDurationWater] = CurrentNumberSetDurationWater;


				// Ограничение ввода количества
				// количества минут, секунд
				switch(CurrentDigitSetDurationWater)
				{
					// Ограничение для минут - не более 1-го десятка
					case 0:
							if (NumberSetDurationWater[CurrentDigitSetDurationWater] > 1)
							{
								CurrentNumberSetDurationWater = 1;
								NumberSetDurationWater[CurrentDigitSetDurationWater] = 1;
							}
					break;

					// Ограничение для секунд - не более 5-ти десятков
					case 2:
							if (NumberSetDurationWater[CurrentDigitSetDurationWater] > 5)
							{
								CurrentNumberSetDurationWater = 5;
								NumberSetDurationWater[CurrentDigitSetDurationWater] = 5;
							}
					break;

					default:
					break;
				}
			}

			// Увеличение цифры при установке даты
			else
			if ((current_frame == 12) && (f_StSetDate))
			{
				if (CurrentNumberSetDate < 9)
					CurrentNumberSetDate++;

				NumberSetDate[CurrentDigitSetDate] = CurrentNumberSetDate;


				// Ограничение ввода числа месяца
				// и номера месяца
				switch(CurrentDigitSetDate)
				{
					// Ограниечение для дней - не более 3-х десятков
					case 0:
							if (NumberSetDate[CurrentDigitSetDate] > 3)
							{
								CurrentNumberSetDate = 3;
								NumberSetDate[CurrentDigitSetDate] = 3;
							}
					break;

					// Ограничение для месяцев - не более 1-го десятка
					case 2:
							if (NumberSetDate[CurrentDigitSetDate] > 1)
							{
								CurrentNumberSetDate = 1;
								NumberSetDate[CurrentDigitSetDate] = 1;
							}
					break;

					default:
					break;
				}
			}

			// Увеличение цифры (дня недели)
			// при установке дня недели
			else
			if ((current_frame == 13) && (f_StSetDayOfWeek))
			{
				if (NumberSetDayOfWeek < 7)
					NumberSetDayOfWeek++;
			}

			// Увеличение цифры при установке времени
			else
			if ((current_frame == 14) && (f_StSetTime))
			{
				if (CurrentNumberSetTime < 9)
					CurrentNumberSetTime++;

				NumberSetTime[CurrentDigitSetTime] = CurrentNumberSetTime;


				// Ограничение ввода количества
				// часов, минут, секунд
				switch(CurrentDigitSetTime)
				{
					// Ограниечение для часов - не более 2-х десятков
					case 0:
							if (NumberSetTime[CurrentDigitSetTime] > 2)
							{
								CurrentNumberSetTime = 2;
								NumberSetTime[CurrentDigitSetTime] = 2;
							}
					break;

					// Ограничение для минут - не более 5-ти десятков
					case 2:
							if (NumberSetTime[CurrentDigitSetTime] > 5)
							{
								CurrentNumberSetTime = 5;
								NumberSetTime[CurrentDigitSetTime] = 5;
							}
					break;

					// Ограничение для секунд - не более 5-ти десятков
					case 4:
							if (NumberSetTime[CurrentDigitSetTime] > 5)
							{
								CurrentNumberSetTime = 5;
								NumberSetTime[CurrentDigitSetTime] = 5;
							}
					break;

					default:
					break;
				}
			}

			// Переключение с кадра 18 на 15
			else
			if ((current_frame == 18) && (pos_cursor == 0))
			{
				current_frame = 15;
				pos_cursor = 4;
			}

			// Установка дня при настройке дней полива
			else
			if ((current_frame == 19) && (f_StSetDaysWater))
			{
				switch(CurrentDigitSetDaysWater)
				{
					case 0:
							WaterDays |= 0x01;
					break;

					case 1:
							WaterDays |= 0x02;
					break;

					case 2:
							WaterDays |= 0x04;
					break;

					case 3:
							WaterDays |= 0x08;
					break;

					case 4:
							WaterDays |= 0x10;
					break;

					case 5:
							WaterDays |= 0x20;
					break;

					case 6:
							WaterDays |= 0x40;
					break;
				}
			}

			// Переключение с кадра 21 на 20
			else
			if ((current_frame == 21) && (pos_cursor == 0))
			{
				current_frame = 20;
				pos_cursor = 4;
			}

			// Увеличение цифры при установке
			// времени автоотключения
			else
			if ((current_frame == 23) && (f_StSetScreenOff))
			{
				if (CurrentNumberSetScreenOff < 9)
					CurrentNumberSetScreenOff++;

				NumberSetScreenOff[CurrentDigitSetScreenOff] = CurrentNumberSetScreenOff;


				// Ограничение ввода количества
				// количества минут, секунд
				switch(CurrentDigitSetScreenOff)
				{
					// Ограничение для минут - не более 3-го десятка
					case 0:
							if (NumberSetScreenOff[CurrentDigitSetScreenOff] > 3)
							{
								CurrentNumberSetScreenOff = 3;
								NumberSetScreenOff[CurrentDigitSetScreenOff] = 3;
							}
					break;

					// Ограничение для минут - не более 5-ти десятков
					case 2:
							if (NumberSetScreenOff[CurrentDigitSetScreenOff] > 5)
							{
								CurrentNumberSetScreenOff = 5;
								NumberSetScreenOff[CurrentDigitSetScreenOff] = 5;
							}
					break;

					default:
					break;
				}
			}

			// Переключение с кадра 24 на 23
			else
			if ((current_frame == 24) && (pos_cursor == 0))
			{
				current_frame = 23;
				pos_cursor = 4;
			}

			// Переключение с кадра 30 на 29
			else
			if ((current_frame == 30) && (pos_cursor == 2))
			{
				current_frame = 29;
				pos_cursor = 4;
			}

			// Переключение с кадра 32 на 31
			else
			if ((current_frame == 32) && (pos_cursor == 4))
			{
				current_frame = 31;
				pos_cursor = 4;
			}

			// Переключение с кадра 33 на 21
			else
			if ((current_frame == 33) && (pos_cursor == 0))
			{
				current_frame = 21;
				pos_cursor = 4;
			}

			// Перелючение курсора вверх
			else
			if (!f_BlockUp)
			{
				// Если курсор наверху дисплея,
				// то ищем новое положение
				// курсора с последней строки дисплея
				if (pos_cursor == 0)
				{
					for (i = 4; i >= 0; i--)
					{
						if (poss_ix[i])
						{
							pos_cursor = i;
							break;
						}
					}
				}

				// Если есть строки выше,
				// то проверяем, можно ли
				// перенести на них курсор
				else
				{
					for (i = pos_cursor - 1; i >= 0; i--)
					{
						if (poss_ix[i])
						{
							pos_cursor = i;
							f_stop_search = 1;
							break;
						}
					}

					// Если среди строк выше не нашлось строк,
					// на которые можно перенести курсор, то
					// начинаем поиск с последней строки дисплея
					if (!f_stop_search)
					{
						for (i = 4; i >= 0; i--)
						{
							if (poss_ix[i])
							{
								pos_cursor = i;
								break;
							}
						}
					}
				}
			}
		}
	}


	// Если нажата кнопка влево
	if (keys & (1 << KEYB_LEFT))
	{
		if (!f_BlockButtons)
		{
			// Смещение разряда при установке даты
			if ((current_frame == 12) && (f_StSetDate))
			{
				if (CurrentDigitSetDate > 0)
					CurrentDigitSetDate--;

				CurrentNumberSetDate = NumberSetDate[CurrentDigitSetDate];
			}

			else
			if ((current_frame == 7) && (f_StSetTimeWater))
			{
				if (CurrentDigitSetTimeWater > 0)
					CurrentDigitSetTimeWater--;

				CurrentNumberSetTimeWater = NumberSetTimeWater[CurrentDigitSetTimeWater];
			}

			else
			if ((current_frame == 8) && (f_StSetDurationWater))
			{
				if (CurrentDigitSetDurationWater > 0)
					CurrentDigitSetDurationWater--;

				CurrentNumberSetDurationWater = NumberSetDurationWater[CurrentDigitSetDurationWater];
			}

			else
			if ((current_frame == 14) && (f_StSetTime))
			{
				if (CurrentDigitSetTime > 0)
					CurrentDigitSetTime--;

				CurrentNumberSetTime = NumberSetTime[CurrentDigitSetTime];
			}

			else
			if ((current_frame == 19) && (f_StSetDaysWater))
			{
				if (CurrentDigitSetDaysWater > 0)
					CurrentDigitSetDaysWater--;
			}

			else
			if ((current_frame == 23) && (f_StSetScreenOff))
			{
				if (CurrentDigitSetScreenOff > 0)
					CurrentDigitSetScreenOff--;

				CurrentNumberSetScreenOff = NumberSetScreenOff[CurrentDigitSetScreenOff];
			}
		}
	}


	// Если нажата кнопка вправо
	if (keys & (1 << KEYB_RIGHT))
	{
		if (!f_BlockButtons)
		{
			// Смещение разряда при установке даты
			if ((current_frame == 12) && (f_StSetDate))
			{
				if (CurrentDigitSetDate < 5)
					CurrentDigitSetDate++;

				CurrentNumberSetDate = NumberSetDate[CurrentDigitSetDate];
			}

			else
			if ((current_frame == 7) && (f_StSetTimeWater))
			{
				if (CurrentDigitSetTimeWater < 5)
					CurrentDigitSetTimeWater++;

				CurrentNumberSetTimeWater = NumberSetTimeWater[CurrentDigitSetTimeWater];
			}

			else
			if ((current_frame == 8) && (f_StSetDurationWater))
			{
				if (CurrentDigitSetDurationWater < 3)
					CurrentDigitSetDurationWater++;

				CurrentNumberSetDurationWater = NumberSetDurationWater[CurrentDigitSetDurationWater];
			}

			else
			if ((current_frame == 14) && (f_StSetTime))
			{
				if (CurrentDigitSetTime < 5)
					CurrentDigitSetTime++;

				CurrentNumberSetTime = NumberSetTime[CurrentDigitSetTime];
			}

			else
			if ((current_frame == 19) && (f_StSetDaysWater))
			{
				if (CurrentDigitSetDaysWater < 6)
					CurrentDigitSetDaysWater++;
			}

			else
			if ((current_frame == 23) && (f_StSetScreenOff))
			{
				if (CurrentDigitSetScreenOff < 3)
					CurrentDigitSetScreenOff++;

				CurrentNumberSetScreenOff = NumberSetScreenOff[CurrentDigitSetScreenOff];
			}
		}
	}


	// Если нажата кнопка ок
	if (keys & (1 << KEYB_OK))
	{
		if (!f_BlockButtons)
		{
			switch(current_frame)
			{
				// 0. Начальный кадр
				case 0:

						// Текущие параметры
						if (pos_cursor == 0)
						{
							current_frame = 15;
							pos_cursor = 0;
						}

						// Настройки
						else
						if (pos_cursor == 2)
						{
							current_frame = 4;
							pos_cursor = 0;
						}

						// Немедленно завершить полив
						else
						if (pos_cursor == 3)
						{
							// Если начался полив
							if (f_StartWatering)
							{
								// Экстренное завершение полива
								EmergencyTerminationOfWatering();

								current_frame = 26;
								pos_cursor = 2;
							}

							// Полив не выполняется
							else
							{
								current_frame = 27;
								pos_cursor = 2;
							}
						}

				break;


				// 1. Текущие параметры полива
				case 1:
				break;


				// 2. Продолжение кадра 1
				case 2:
						if (pos_cursor == 3)
						{
							current_frame = 15;
							pos_cursor = 2;
						}
				break;


				// 3. Уровень воды в резервуаре
				case 3:
						if (pos_cursor == 3)
						{
							current_frame = 18;
							pos_cursor = 0;
						}
				break;


				// 4. Настройки
				case 4:
						if (pos_cursor == 0)
						{
							current_frame = 6;
							pos_cursor = 0;
						}

						else
						if (pos_cursor == 2)
						{
							current_frame = 7;
							pos_cursor = 2;
						}

						else
						if (pos_cursor == 3)
						{
							current_frame = 23;
							pos_cursor = 3;
						}
				break;


				// 5. Продолжение кадра 4
				case 5:
						// (Отключить/включить) датчик
						// уровня воды
						if (pos_cursor == 0)
						{
							current_frame = 31;
							pos_cursor = 4;
						}

						// Сброс настроек
						else
						if (pos_cursor == 2)
						{
							current_frame = 20;
							pos_cursor = 0;
						}

						// Назад
						else
						if (pos_cursor == 3)
						{
							current_frame = 0;
							pos_cursor =  2;
						}
				break;


				// 6. Настройка текущего времени
				case 6:
						if (pos_cursor == 0)
						{
							current_frame = 12;
							pos_cursor = 2;
						}

						else
						if (pos_cursor == 1)
						{
							current_frame = 14;
							pos_cursor = 2;
						}

						else
						if (pos_cursor == 2)
						{
							current_frame = 4;
							pos_cursor =  0;
						}
				break;


				// 7. Настройка полива
				case 7:
						// Установка времени
						if (pos_cursor == 2)
						{
							cnt_SetTimeWater++;

							if (cnt_SetTimeWater == 1)
							{
								f_StSetTimeWater = 1;
							}

							else
							if (cnt_SetTimeWater == 2)
							{
								f_StSetTimeWater = 0;
								cnt_SetTimeWater = 0;
								CurrentDigitSetTimeWater = 0;
								CurrentNumberSetTimeWater= 0;
							}
						}

						// Далее
						else
						if (pos_cursor == 3)
						{
							// Если время установлено
							if (f_TimeIsSetWater)
							{
								// Если значения введены корректно
								if ((WateringParamsSet.hour >= 0) && (WateringParamsSet.hour <= 23) &&
									(WateringParamsSet.minutes >= 0) && (WateringParamsSet.minutes <= 59) &&
									(WateringParamsSet.seconds >= 0) && (WateringParamsSet.seconds <= 59))
								{
									// Переход к кадру настройки
									// продолжительности полива
									current_frame = 8;
									pos_cursor = 2;
								}

								else
								{
									// Установка ошибки ввода времени
									set_fails = 3;

									// Сброс параметров
									// настройки времени
									f_StSetTimeWater = 0;
									cnt_SetTimeWater = 0;
									CurrentDigitSetTimeWater = 0;
									CurrentNumberSetTimeWater= 0;

									f_TimeIsSetWater = 0;

									// Сброс структуры WateringParamsSet
									// (очистка первых трёх байтов)
									// и массива NumberSetTimeWater
									memset(ptr_WateringParamsSet, 0, 3);
									memset(ptr_NumberSetTimeWater, 0, sizeof(NumberSetTimeWater)/sizeof(uint8_t));

									// Кадр ошибки введённых параметров
									current_frame = 17;
									pos_cursor = 3;
								}
							}
						}
				break;


				// 8. Продолжение кадра 7
				case 8:

					// Установка продолжительности полива
					if (pos_cursor == 2)
					{
						cnt_SetDurationWater++;

						if (cnt_SetDurationWater == 1)
						{
							f_StSetDurationWater = 1;
						}

						else
						if (cnt_SetDurationWater == 2)
						{
							f_StSetDurationWater = 0;
							cnt_SetDurationWater = 0;
							CurrentDigitSetDurationWater = 0;
							CurrentNumberSetDurationWater= 0;
						}
					}

					// Далее
					else
					if (pos_cursor == 3)
					{
						// Если время установлено
						if (f_TimeIsSetDurationWater)
						{
							// Если продолжительность полива равна нулю
							if ((WateringParamsSet.minutes_watering == 0) &&
								(WateringParamsSet.seconds_watering == 0))
							{
								// Установка ошибки ввода
								// продолжительности полива
								set_fails = 4;

								// Сброс параметров
								// настройки времени
								f_StSetTimeWater = 0;
								cnt_SetTimeWater = 0;
								CurrentDigitSetTimeWater = 0;
								CurrentNumberSetTimeWater= 0;

								f_TimeIsSetWater = 0;

								// Сброс структуры WateringParamsSet
								// (очистка первых трёх байтов)
								// и массива NumberSetTimeWater
								memset(ptr_WateringParamsSet, 0, 3);
								memset(ptr_NumberSetTimeWater, 0, sizeof(NumberSetTimeWater)/sizeof(uint8_t));

								// Сброс параметров
								// настройки времени
								f_StSetDurationWater = 0;
								cnt_SetDurationWater = 0;
								CurrentDigitSetDurationWater = 0;
								CurrentNumberSetDurationWater= 0;

								f_TimeIsSetDurationWater = 0;

								// Сброс структуры WateringParamsSet
								// и массива NumberSetTimeWater
								memset(ptr_WateringParamsSet + 3, 0, 2);
								memset(ptr_NumberSetDurationWater, 0, sizeof(NumberSetDurationWater)/sizeof(uint8_t));

								// Кадр ошибки введённых параметров
								current_frame = 17;
								pos_cursor = 3;
							}

							// Если значения введены корректно
							else
							if ((WateringParamsSet.minutes_watering >= 0) && (WateringParamsSet.minutes_watering <= 10) &&
								(WateringParamsSet.seconds_watering >= 0) && (WateringParamsSet.seconds_watering <= 59))
							{
								// Переход к кадру настройки
								// частоты полива
								current_frame = 19;
								pos_cursor = 3;
							}
						}
					}

				break;


				// 10. Кадр сохранения данных
				case 10:

					// Ок
					if (pos_cursor == 2)
					{
						// Переход на кадр, который вызвал переход на кадр 10
						current_frame = previous_frame_set;
						pos_cursor = previous_pos_cursor_set;

						// Сброс прошлого кадра
						previous_frame_set = 0;
						previous_pos_cursor_set = 0;
					}

				break;


				// 12. Кадр установки текущей даты
				case 12:

						// Переход к установке даты
						if (pos_cursor == 2)
						{
							cnt_SetDate++;

							// Начало настройки даты
							if (cnt_SetDate == 1)
							{
								f_StSetDate = 1;
							}

							// Конец настройки даты
							else
							if (cnt_SetDate == 2)
							{
								f_StSetDate = 0;
								cnt_SetDate = 0;
								CurrentDigitSetDate = 0;
								CurrentNumberSetDate= 0;
							}
						}

						else
						if (pos_cursor == 3)
						{
							current_frame = 13;
							pos_cursor = 2;
						}

				break;


				// 13. Кадр установки дня недели
				case 13:

						// Установка дня недели
						if (pos_cursor == 2)
						{
							cnt_SetDayOfWeek++;

							// Начало настройки дня недели
							if (cnt_SetDayOfWeek == 1)
							{
								f_StSetDayOfWeek = 1;
							}

							// Конец настройки дня недели
							else
							if (cnt_SetDayOfWeek == 2)
							{
								f_StSetDayOfWeek = 0;
								cnt_SetDayOfWeek = 0;
								NumberSetDayOfWeek = 1;
							}
						}

						// Сохранить (дату и день недели)
						else
						if (pos_cursor == 3)
						{
							// Если дата и день недели установлены
							if (f_DateIsSet && f_DayOfWeekIsSet)
							{
								// Если значения введены корректно
								if ((timeSetDate.dayofweek >= 1) && (timeSetDate.dayofweek <= 7) &&
									(timeSetDate.dayofmonth >= 1) && (timeSetDate.dayofmonth <= 31) &&
									(timeSetDate.month >= 1) && (timeSetDate.month <= 12))
								{
									// Отправка даты и дня недели на DS3231
									SetDateDS(timeSetDate.dayofweek, timeSetDate.dayofmonth,
												 timeSetDate.month, timeSetDate.year);

									previous_frame_set = 13;
									previous_pos_cursor_set = 3;

									// Переход на кадр 10 -
									// Сообщение об успешном сохранении
									current_frame = 10;
									pos_cursor = 2;
								}

								else
								{
									// Установка ошибки ввода даты
									set_fails = 1;

									// Сброс параметров настройки даты
									f_StSetDayOfWeek = 0;
									cnt_SetDayOfWeek = 0;
									NumberSetDayOfWeek = 1;

									f_DateIsSet = 0;
									f_DayOfWeekIsSet = 0;

									// Сброс структуры timeSetDate
									// (первые три байта очищать не нужно)
									// и массива NumberSetDate
									memset(ptr_timeSetDate + 3, 0, 4);
									memset(ptr_NumberSetDate, 0, sizeof(NumberSetDate)/sizeof(uint8_t));

									// Кадр ошибки введённых параметров
									current_frame = 17;
									pos_cursor = 3;
								}
							}
						}

						// Выйти
						else
						if (pos_cursor == 4)
						{
							// Сброс параметров настройки даты
							f_DateIsSet = 0;
							f_DayOfWeekIsSet = 0;

							// Сброс структуры timeSetDate
							// (первые три байта очищать не нужно)
							// и массива NumberSetDate
							memset(ptr_timeSetDate + 3, 0, 4);
							memset(ptr_NumberSetDate, 0, sizeof(NumberSetDate)/sizeof(uint8_t));

							// Переход на кадр 6
							current_frame = 6;
							pos_cursor = 0;
						}

				break;


				// 14. Кадр настройки времени
				case 14:

						// Установка времени
						if (pos_cursor == 2)
						{
							cnt_SetTime++;

							// Начало настройки дня недели
							if (cnt_SetTime == 1)
							{
								f_StSetTime = 1;
							}

							// Конец настройки дня недели
							else
							if (cnt_SetTime == 2)
							{
								f_StSetTime = 0;
								cnt_SetTime = 0;
								CurrentDigitSetTime = 0;
								CurrentNumberSetTime= 0;
							}
						}

						// Сохранить (время)
						else
						if (pos_cursor == 3)
						{
							// Если время установлено
							if (f_TimeIsSet)
							{
								// Если значения введены корректно
								if ((timeSetDate.hour >= 0) && (timeSetDate.hour <= 23) &&
									(timeSetDate.minutes >= 0) && (timeSetDate.minutes <= 59) &&
									(timeSetDate.seconds >= 0) && (timeSetDate.seconds <= 59))
								{
									// Отправка часов, минут, секунд на DS3231
									SetTimeDS(timeSetDate.seconds, timeSetDate.minutes, timeSetDate.hour);

									previous_frame_set = 14;
									previous_pos_cursor_set = 3;

									// Переход на кадр 10 -
									// Сообщение об успешном сохранении
									current_frame = 10;
									pos_cursor = 2;
								}

								else
								{
									// Установка ошибки ввода времени
									set_fails = 2;

									// Сброс параметров
									// настройки времени
									f_StSetTime = 0;
									cnt_SetTime = 0;
									CurrentDigitSetTime = 0;
									CurrentNumberSetTime= 0;

									f_TimeIsSet = 0;

									// Сброс структуры timeSetDate
									// (очистка первых трёх байтов)
									// и массива NumberSetTime
									memset(ptr_timeSetDate, 0, 3);
									memset(ptr_NumberSetTime, 0, sizeof(NumberSetTime)/sizeof(uint8_t));

									// Кадр ошибки введённых параметров
									current_frame = 17;
									pos_cursor = 3;
								}
							}
						}

						// Выйти
						else
						if (pos_cursor == 4)
						{
							// Сброс параметров
							// настройки времени
							f_TimeIsSet = 0;

							// Сброс структуры timeSetDate
							// (очистка первых трёх байтов)
							// и массива NumberSetTime
							memset(ptr_timeSetDate, 0, 3);
							memset(ptr_NumberSetTime, 0, sizeof(NumberSetTime)/sizeof(uint8_t));

							// Переход на кадр 6
							current_frame = 6;
							pos_cursor = 1;
						}

				break;


				// 15. Кадр текущие параметры
				case 15:

					// Текущая дата и время
					if (pos_cursor == 0)
					{
						current_frame = 16;
						pos_cursor = 4;
					}

					// Текущие параметры полива
					else
					if (pos_cursor == 2)
					{
						current_frame = 1;
						pos_cursor = 4;
					}

					// Уровень воды
					else
					if (pos_cursor == 4)
					{
						current_frame = 3;
						pos_cursor = 3;
					}

				break;


				// 16. Кадр текущая дата и время
				case 16:

					// Назад
					if (pos_cursor == 4)
					{
						current_frame = 15;
						pos_cursor = 0;
					}

				break;


				// 17. Кадр ошибки введённых параметров
				case 17:

					// Начать заново
					if (pos_cursor == 3)
					{
						// Переход на кадр настройки в
						// зависимости от того, какой
						// параметр был введён неправильно
						switch(set_fails)
						{
							// Ошибка ввода даты при настройке
							case 1:

								current_frame = 12;
								pos_cursor = 2;

								f_DateIsSet = 0;
								f_DayOfWeekIsSet = 0;

							break;


							// Ошибка ввода времени при настройке
							case 2:

								current_frame = 14;
								pos_cursor = 2;

								f_TimeIsSet = 0;

							break;


							// Ошибка ввода времени при
							// настройке времени начала полива
							case 3:

								current_frame = 7;
								pos_cursor = 2;

								f_TimeIsSetWater = 0;

							break;


							// Ошибка ввода времени при
							// настройке продолжительности полива
							// (полив не более 10 мин!)
							case 4:

								current_frame = 7;
								pos_cursor = 2;

								f_TimeIsSetWater = 0;

							break;


							// Ошибка ввода времени при
							// настройке дней полива
							// (нужно выбрать хотя бы один день)
							case 5:

								current_frame = 7;
								pos_cursor = 2;

								f_DaysWaterIsSet = 0;

							break;


							// Установка ошибки ввода
							// времени автоотключения экрана
							// (не более 30 мин 59 сек)
							case 6:

								current_frame = 23;
								pos_cursor = 3;

								f_ScreenOffIsSet = 0;

							break;


							default:
							break;
						}

						// Сброс ошибок
						set_fails = 0;
					}

					// Выйти
					else
					if (pos_cursor == 4)
					{
						// Переход на кадр в
						// зависимости от того,
						// какой параметр был введён
						switch(set_fails)
						{
							// Ошибка ввода даты при настройке
							case 1:

								current_frame = 6;
								pos_cursor = 0;

							break;


							// Ошибка ввода времени при настройке
							case 2:

								current_frame = 6;
								pos_cursor = 1;

							break;


							// Ошибка ввода времени при
							// настройке времени начала полива
							case 3:

								current_frame = 4;
								pos_cursor = 2;

							break;


							// Ошибка ввода времени при
							// настройке продолжительности полива
							// (полив не более 10 мин!)
							case 4:

								current_frame = 4;
								pos_cursor = 2;

							break;


							// Ошибка ввода времени при
							// настройке дней полива
							// (нужно выбрать хотя бы один день)
							case 5:

								current_frame = 4;
								pos_cursor = 2;

							break;


							// Установка ошибки ввода
							// времени автоотключения экрана
							// (не более 30 мин 59 сек)
							case 6:

								current_frame = 4;
								pos_cursor = 3;

							break;


							default:
							break;
						}

						// Сброс ошибок
						set_fails = 0;
					}

				break;


				// 18. Продолжение кадра 15
				case 18:

					// Уровень воды в резервуаре
					if (pos_cursor == 0)
					{
						current_frame = 3;
						pos_cursor = 3;
					}

					// Автоотключение экрана
					else
					if (pos_cursor == 2)
					{
						current_frame = 25;
						pos_cursor = 4;
					}

					// Назад
					else
					if (pos_cursor == 4)
					{
						current_frame = 0;
						pos_cursor = 0;
					}

				break;


				// 19. Настройка полива (3)
				case 19:

					// Установка дней полива
					if (pos_cursor == 3)
					{
						cnt_SetDaysWater++;

						if (cnt_SetDaysWater == 1)
						{
							f_StSetDaysWater = 1;
						}

						else
						if (cnt_SetDaysWater == 2)
						{
							f_StSetDaysWater = 0;
							cnt_SetDaysWater = 0;
							CurrentDigitSetDaysWater = 0;
						}
					}

					// Завершить
					else
					if (pos_cursor == 4)
					{
						// Если дни полива установлены
						if (f_TimeIsSetDurationWater)
						{
							// Если значения введены корректно
							// (установлен хотя бы один день полива)
							if (WateringParamsSet.daysofweek)
							{
								// Запись настроек полива на flash
								SD_WriteData(1);

								// Флаг того, что настройки полива
								// были изменены и записаны на flash
								f_WateringParamsChanged = 1;

								// Переход на другой кадр
								// (кадр настройки)
								previous_frame_set = 4;
								previous_pos_cursor_set = 2;

								// Переход на кадр 10 -
								// Сообщение об успешном сохранении
								current_frame = 10;
								pos_cursor = 2;
							}

							else
							{
								// Установка ошибки ввода
								// дней полива
								set_fails = 5;

								// Кадр ошибки введённых параметров
								current_frame = 17;
								pos_cursor = 3;
							}

							// Сброс параметров
							// настройки времени
							f_StSetTimeWater = 0;
							cnt_SetTimeWater = 0;
							CurrentDigitSetTimeWater = 0;
							CurrentNumberSetTimeWater= 0;

							f_TimeIsSetWater = 0;

							// Сброс структуры WateringParamsSet
							// (очистка первых трёх байтов)
							// и массива NumberSetTimeWater
							memset(ptr_WateringParamsSet, 0, 3);
							memset(ptr_NumberSetTimeWater, 0, sizeof(NumberSetTimeWater)/sizeof(uint8_t));

							// Сброс параметров
							// настройки времени
							f_StSetDurationWater = 0;
							cnt_SetDurationWater = 0;
							CurrentDigitSetDurationWater = 0;
							CurrentNumberSetDurationWater= 0;

							f_TimeIsSetDurationWater = 0;

							// Сброс структуры WateringParamsSet
							// и массива NumberSetTimeWater
							memset(ptr_WateringParamsSet + 3, 0, 2);
							memset(ptr_NumberSetDurationWater, 0, sizeof(NumberSetDurationWater)/sizeof(uint8_t));

							// Сброс параметров
							// настройки дней полива
							f_StSetDaysWater = 0;
							cnt_SetDaysWater = 0;
							CurrentDigitSetDaysWater = 0;

							f_DaysWaterIsSet = 0;

							// Сброс структуры WateringParamsSet
							memset(ptr_WateringParamsSet + 5, 0, 1);
							WaterDays = 0;
						}
					}

				break;


				// Сброс настроек (1)
				case 20:

					// Сбросить настройки
					// текущего времени
					if (pos_cursor == 0)
					{
						// Установка начальной даты
						// и дня недели (состояние сброса)
						SetDateDS(3, 1, 1, 25);

						// Установка начального времени
						// (состояние сброса)
						SetTimeDS(0, 0, 0);

						previous_frame_reset = 20;
						previous_pos_cursor_reset = 0;

						// Переход на кадр 22 -
						// Сообщение об успешном сбросе
						current_frame = 22;
						pos_cursor = 2;
					}

					// Сбросить настройки полива
					else
					if (pos_cursor == 2)
					{
						// Сброс текущих настроек полива
						memset(ptr_WateringParams, 0, sizeof(WateringParams)/sizeof(uint8_t));

						// Сохранение параметров
						SD_WriteData(1);

						// Флаг того, что настройки полива
						// были изменены и записаны на flash
						f_WateringParamsChanged = 1;

						previous_frame_reset = 20;
						previous_pos_cursor_reset = 2;

						// Переход на кадр 22 -
						// Сообщение об успешном сбросе
						current_frame = 22;
						pos_cursor = 2;
					}

					// Сбросить настройки
					// автоотключения экрана
					else
					if (pos_cursor == 4)
					{
						// Очистка текущего времени автоотключения экрана
						memset(ptr_ScreenOff, 0, sizeof(ScreenOff)/sizeof(uint8_t));

						// Сохранение параметров
						SD_WriteData(2);

						// Флаг того, что настройки
						// автоотключения экрана
						// были изменены и записаны на flash
						f_ScreenOffChanged = 1;

						previous_frame_reset = 20;
						previous_pos_cursor_reset = 4;

						// Переход на кадр 22 -
						// Сообщение об успешном сбросе
						current_frame = 22;
						pos_cursor = 2;
					}

				break;


				// Сброс настроек (2)
				case 21:

					// Сбросить настройки
					// автоотключения экрана
					if (pos_cursor == 0)
					{
						// Очистка текущего времени автоотключения экрана
						memset(ptr_ScreenOff, 0, sizeof(ScreenOff)/sizeof(uint8_t));

						// Сохранение параметров
						SD_WriteData(2);

						// Флаг того, что настройки
						// автоотключения экрана
						// были изменены и записаны на flash
						f_ScreenOffChanged = 1;

						previous_frame_reset = 21;
						previous_pos_cursor_reset = 0;

						// Переход на кадр 22 -
						// Сообщение об успешном сбросе
						current_frame = 22;
						pos_cursor = 2;
					}

					// Сбросить датчик
					// уровня воды
					else
					if (pos_cursor == 2)
					{
						// По умолчанию датчик уровня воды включен
						WaterLevelSensorState.f_IsWaterLevelSensorOn = 1;

						// Запись структуры WaterLevelSensorState на flash
						SD_WriteData(3);

						// Флаг того, что настройки состояния датчика
						// были изменены и записаны на flash
						f_WaterLevelSensorStateChanged = 1;

						previous_frame_reset = 21;
						previous_pos_cursor_reset = 2;

						// Переход на кадр 22 -
						// Сообщение об успешном сбросе
						current_frame = 22;
						pos_cursor = 2;
					}

					// Сбросить все настройки
					else
					if (pos_cursor == 4)
					{
						// Установка начальной даты
						// и дня недели (состояние сброса)
						SetDateDS(3, 1, 1, 25);

						// Установка начального времени
						// (состояние сброса)
						SetTimeDS(0, 0, 0);

						// Сброс текущих настроек полива
						memset(ptr_WateringParams, 0, sizeof(WateringParams)/sizeof(uint8_t));

						// Сохранение параметров полива
						SD_WriteData(1);

						// Флаг того, что настройки полива
						// были изменены и записаны на flash
						f_WateringParamsChanged = 1;

						// Очистка текущего времени автоотключения экрана
						memset(ptr_ScreenOff, 0, sizeof(ScreenOff)/sizeof(uint8_t));

						// Сохранение параметров
						SD_WriteData(2);

						// Флаг того, что настройки
						// автоотключения экрана
						// были изменены и записаны на flash
						f_ScreenOffChanged = 1;

						// По умолчанию датчик уровня воды включен
						WaterLevelSensorState.f_IsWaterLevelSensorOn = 1;

						// Запись структуры WaterLevelSensorState на flash
						SD_WriteData(3);

						// Флаг того, что настройки состояния датчика
						// были изменены и записаны на flash
						f_WaterLevelSensorStateChanged = 1;

						previous_frame_reset = 33;
						previous_pos_cursor_reset = 0;

						// Переход на кадр 22 -
						// Сообщение об успешном сбросе
						current_frame = 22;
						pos_cursor = 2;
					}

				break;


				// 22. Сообщение об успешном
				// сбросе настроек
				case 22:

					// Ок
					if (pos_cursor == 2)
					{
						// Переход на кадр, который вызвал переход на кадр 22
						current_frame = previous_frame_reset;
						pos_cursor = previous_pos_cursor_reset;

						// Сброс прошлого кадра
						previous_frame_reset = 0;
						previous_pos_cursor_reset = 0;
					}

				break;


				// 23. Настройка автоотключения экрана (1)
				case 23:

					// Установка времени
					// автоотключения экрана
					if (pos_cursor == 3)
					{
						cnt_SetScreenOff++;

						if (cnt_SetScreenOff == 1)
						{
							f_StSetScreenOff = 1;
						}

						else
						if (cnt_SetScreenOff == 2)
						{
							f_StSetScreenOff = 0;
							cnt_SetScreenOff = 0;
							CurrentDigitSetScreenOff = 0;
							CurrentNumberSetScreenOff= 0;
						}
					}

					// Сохранить
					else
					if (pos_cursor == 4)
					{
						// Если время автоотключения установлено
						if (f_ScreenOffIsSet)
						{
							// Установленное время автоотключения в секундах
							SecCheck = ScreenOffSet.minutes_screen_off * 60 + ScreenOffSet.seconds_screen_off;

							// Время автоотключения от 30 с до 30 мин (0 - отключить)
							if (((SecCheck >= 30) && (SecCheck <= 1800)) || (!SecCheck))
							{
								// Запись времени автоотключения
								// экрана на flash
								SD_WriteData(2);

								// Флаг того, что настройки автоотключения экрана
								// были изменены и записаны на flash
								f_ScreenOffChanged = 1;

								previous_frame_set = 23;
								previous_pos_cursor_set = 4;

								// Переход на кадр 10 -
								// Сообщение об успешном сохранении
								current_frame = 10;
								pos_cursor = 2;
							}

							else
							{
								// Установка ошибки ввода
								// времени автоотключения экрана
								set_fails = 6;

								// Кадр ошибки введённых параметров
								current_frame = 17;
								pos_cursor = 3;
							}

							// Сброс параметров
							// настройки времени
							// автоотключения экрана
							f_StSetScreenOff = 0;
							cnt_SetScreenOff = 0;
							CurrentDigitSetScreenOff = 0;
							CurrentNumberSetScreenOff = 0;

							f_ScreenOffIsSet = 0;

							// Сброс структуры ScreenOffSet
							// и массива NumberSetScreenOff
							memset(ptr_ScreenOffSet, 0, sizeof(ScreenOffSet)/sizeof(uint8_t));
							memset(ptr_NumberSetScreenOff, 0, sizeof(NumberSetScreenOff)/sizeof(uint8_t));
						}
					}

				break;


				// 24. Настройка автоотключения экрана (2)
				case 24:

					switch(pos_cursor)
					{
						// Назад
						case 0:

							current_frame = 4;
							pos_cursor = 3;

						break;
					}

				break;


				// 25. Текущее время автоотключения экрана
				case 25:

					switch(pos_cursor)
					{
						// Назад
						case 4:

							current_frame = 18;
							pos_cursor = 2;

						break;
					}

				break;


				// 26. Сообщение об успешном
				// экстренном завершении полива
				case 26:

					switch(pos_cursor)
					{
						case 2:

							current_frame = 0;
							pos_cursor = 3;

						break;
					}

				break;


				// 27. Сообщение о том,
				// что полив не выполняется
				case 27:

					switch(pos_cursor)
					{
						case 2:

							current_frame = 0;
							pos_cursor = 3;

						break;
					}

				break;


				// 28. Уведомление о необходимости
				// пополнить резервуар
				case 28:

					switch(pos_cursor)
					{
						case 4:

							// Пользователь просмотрел уведомление 28
							f_NotificationViewed28 = 1;

							// Переход на предыдущий кадр
							current_frame = prev_frame;
							pos_cursor = prev_pos_cursor;

						break;
					}

				break;


				// 30. Сообщение о том, что полив не будет
				// выполнен из-за недостатка воды (2)
				case 30:

					switch(pos_cursor)
					{
						case 2:

							// Переход на предыдущий кадр
							current_frame = prev_frame;
							pos_cursor = prev_pos_cursor;

						break;
					}

				break;

				// 32. Сообщение о том, что датчик
				// уровня воды будет выключен/включен (2)
				case 32:

					switch(pos_cursor)
					{
						case 4:

							// Смена значения флага
							// f_IsWaterLevelSensorOn
							if (WaterLevelSensorState.f_IsWaterLevelSensorOn)
								WaterLevelSensorState.f_IsWaterLevelSensorOn = 0;
							else
								WaterLevelSensorState.f_IsWaterLevelSensorOn = 1;

							// Запись структуры WaterLevelSensorState на flash
							SD_WriteData(3);

							// Флаг того, что настройки состояния датчика
							// были изменены и записаны на flash
							f_WaterLevelSensorStateChanged = 1;

							// Переход на кадр 5
							current_frame = 5;
							pos_cursor = 0;

						break;
					}

				break;


				// 33. Сброс настроек (3)
				case 33:

					// Сбросить все настройки
					if (pos_cursor == 0)
					{
						// Установка начальной даты
						// и дня недели (состояние сброса)
						SetDateDS(3, 1, 1, 25);

						// Установка начального времени
						// (состояние сброса)
						SetTimeDS(0, 0, 0);

						// Сброс текущих настроек полива
						memset(ptr_WateringParams, 0, sizeof(WateringParams)/sizeof(uint8_t));

						// Сохранение параметров полива
						SD_WriteData(1);

						// Флаг того, что настройки полива
						// были изменены и записаны на flash
						f_WateringParamsChanged = 1;

						// Очистка текущего времени автоотключения экрана
						memset(ptr_ScreenOff, 0, sizeof(ScreenOff)/sizeof(uint8_t));

						// Сохранение параметров
						SD_WriteData(2);

						// Флаг того, что настройки
						// автоотключения экрана
						// были изменены и записаны на flash
						f_ScreenOffChanged = 1;

						// По умолчанию датчик уровня воды включен
						WaterLevelSensorState.f_IsWaterLevelSensorOn = 1;

						// Запись структуры WaterLevelSensorState на flash
						SD_WriteData(3);

						// Флаг того, что настройки состояния датчика
						// были изменены и записаны на flash
						f_WaterLevelSensorStateChanged = 1;

						previous_frame_reset = 33;
						previous_pos_cursor_reset = 0;

						// Переход на кадр 22 -
						// Сообщение об успешном сбросе
						current_frame = 22;
						pos_cursor = 2;
					}

					// Назад
					else
					if (pos_cursor == 2)
					{
						current_frame = 5;
						pos_cursor = 2;
					}

				break;


				default:
				break;
			}
		}
	}
}


//============================================================================
//	Обработка ошибок
//============================================================================

// Отрисовка кадра ошибки при
// появлении критической ошибки
void DrawErrorFrame(void)
{
	// Отрисовка грустного смайлика
	OLED_DrawBitmap(ImageSad, 50, 50, 7, 7);

	// "Фатальная ошибка!"
	FontSet(Segoe_UI_Rus_10);
	OLED_DrawStr("Afnfkmyfz", 64, 13 * 1, 1);
	OLED_DrawStr("jib,rf", 64, 13 * 2, 1);
	OLED_DrawStr("!", 64 + OLED_GetWidthStr("jib,rf") + 2, 13 * 2, 1);

	// Если обнаружена протечка помпы
	if (f_IsWaterLeak)
	{
		// "Фатальная ошибка! (протечка помпы)"
		OLED_DrawChar_Modified('(', 64, 13 * 3, 1);
		FontSet(Segoe_UI_Rus_10);
		OLED_DrawStr("ghjntxrf", 64 + OLED_GetWidthStr("(") + 2, 13 * 3, 1);
		OLED_DrawStr("gjvgs", 64, 13 * 4, 1);
		OLED_DrawChar_Modified(')', 64 + OLED_GetWidthStr("gjvgs") + 2, 13 * 4, 1);
	}

	// Если обнаружена ошибка при работе с Micro SD
	else
	if (f_ErrorSD)
	{
		// "Фатальная ошибка! (Micro SD)"
		FontSet(Segoe_UI_Eng_10);
		OLED_DrawStr("(Micro", 64, 13 * 3, 1);
		OLED_DrawStr("SD)", 64 + OLED_GetWidthStr("(Micro") + 3, 13 * 3, 1);
	}
}


// Проверка наличия фатальных ошибок
void CheckFatalErrors(void)
{
	// Фиксирование фатальной ошибки
	// при появлянии протечки или
	//при ошибке в работе с micro sd
	if ((f_IsWaterLeak) || (f_ErrorSD))
		f_FatalErrors = 1;
	else
		f_FatalErrors = 0;
}


//============================================================================
//	АЦП
//============================================================================

// Проверка протечки помпы
void CheckWaterLeak(void)
{
	// При первом обнаружении утечки
	// запускается таймер на 60 секунд,
	// затем выполняется повторная
	// проверка протечки помпы
	if (mean_adc_ch1 <= 2000)
		f_StartCheckLeak = 1;

	if (f_FinishCheckLeak)
	{
		// Сброс флагов
		f_StartCheckLeak = 0;
		f_FinishCheckLeak = 0;

		// Повторная проверка
		if (mean_adc_ch1 <= 2000)
			f_IsWaterLeak = 1;
	}
}


// Управление светодиодами
// на основе уровня воды (индикация уровня воды)
void LED_Control(uint16_t adc_value)
{
	// Моргание тремя светодиодами при отсутствии воды
	if ((adc_value <= 4095) && (adc_value > 3000))
	{
		// Сброс светодиодов (синхронизация)
		if (!f_ResetLED)
		{
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);

			f_ResetLED = 1;
		}

		f_LED_Blink = 1;
		WaterLevel = NoWater;
	}

	else
	{
		f_LED_Blink = 0;
		f_ResetLED = 0;

		// Включение одного светодиода при небольшом уровне воды
		if ((adc_value <= 3000) && (adc_value > 1850))
		{
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
			WaterLevel = LowLevel;
		}

		// Включение двух светодиодов при умеренном уровне воды
		else
		if ((adc_value <= 1850) && (adc_value > 1750))
		{
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
			WaterLevel = MidLevel;
		}

		// Включение трёх светодиодов при большом количестве воды
		else
		if ((adc_value <= 1750) && (adc_value >= 0))
		{
			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
			WaterLevel = HighLevel;
		}
	}
}


// Проверка изменения уровня воды
void IsWaterLevelChanged(void)
{
	new_WaterLevel = WaterLevel;

	// Если было изменение уровня воды на NoWater
	if ((new_WaterLevel == NoWater) &&
		(old_WaterLevel == LowLevel))
		f_WaterLevelChangedNoWater = 1;

	old_WaterLevel = new_WaterLevel;
}


// Сохранение и обработка данных с АЦП
void ADC_DataProcessing(void)
{
	// Суммарное значение АЦП
	// для расчёта среднего значения
	uint32_t sum_adc_ch0 = 0;
	uint32_t sum_adc_ch1 = 0;

	uint8_t i = 0;

	// Если преобразование АЦП завершено
	if (f_AdcEndOfConversion)
	{
		// Сброс флага завершения преобразования АЦП
		f_AdcEndOfConversion = 0;

		//----------------------------------------------
		// Остановка АЦП, перенос данных в массивы
		//----------------------------------------------
		HAL_ADC_Stop_DMA(&hadc1);

		adc_result_ch0[adc_index] = adc_data[0];
		adc_result_ch1[adc_index] = adc_data[1];

		adc_index++;

		if (adc_index == AdcArraySize)
		{
			adc_index = 0;

			// Флаг того, что набрано
			// достаточно измерений АЦП
			f_AdcFull = 1;
		}

		//----------------------------------------------
		// Анализ данных
		//----------------------------------------------

		// Если набрано достаточно измерений АЦП
		if (f_AdcFull)
		{
			for (i = 0; i < AdcArraySize; i++)
			{
				sum_adc_ch0 += adc_result_ch0[i];
				sum_adc_ch1 += adc_result_ch1[i];
			}

			// Среднее значение АЦП (в отсчётах)
			mean_adc_ch0 = (uint16_t) (sum_adc_ch0 / AdcArraySize);
			mean_adc_ch1 = (uint16_t) (sum_adc_ch1 / AdcArraySize);

			// Очистка массивов
			memset((char*) &adc_result_ch0[0], 0, sizeof(adc_result_ch0));
			memset((char*) &adc_result_ch1[0], 0, sizeof(adc_result_ch1));

			f_AdcFull = 0;

			// Флаг начала анализа данных с АЦП
			f_StartAnalyzingAdcData = 1;
		}

		// Сброс флагов, если есть вода
		if (WaterLevel != NoWater)
		{
			f_WaterLevelChangedNoWater = 0;
			f_NotificationViewed28 = 0;
		}

		// Проверка изменения уровня воды
		IsWaterLevelChanged();

		if (f_StartAnalyzingAdcData)
		{
			// Если датчик уровня воды включен
			if (WaterLevelSensorState.f_IsWaterLevelSensorOn)
			{
				// Включение светодиодов
				// в зависимости от уровня воды
				LED_Control(mean_adc_ch0);

				// Если уровень воды в резервуаре
				// изменился на NoWater
				if (f_WaterLevelChangedNoWater)
				{
					// Если пользователь ранее
					// не видел уведомление 28
					if (!f_NotificationViewed28)
					{
						if (current_frame != 28)
						{
							// Сохранение текущего кадра
							prev_frame = current_frame;
							prev_pos_cursor = pos_cursor;
						}

						// 28. Уведомление о необходимости
						// пополнить резервуар
						current_frame = 28;
						pos_cursor = 4;
					}
				}
			}

			// Выключение всех светодиодов
			else
			{
				f_LED_Blink = 0;
				f_ResetLED = 0;

				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
			}

			// Проверка утечки воды
			CheckWaterLeak();
		}

		//----------------------------------------------
		// Запуск АЦП
		//----------------------------------------------
		adc_data[0] = 0;
		adc_data[1] = 0;

		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &adc_data, 2);
	}
}


//============================================================================
//	Другое
//============================================================================

// Монтирование логического диска
void MountDisk(void)
{
	if (f_mount(&USERFatFS, USERPath, 0) != FR_OK)
		f_ErrorSD = 1;
}


// Инициализация всего и
// определение начальных
// параметров при включении
void InitAll(void)
{
	// Один из двух выходов для управления помпой.
	// На этом выходе можно всегда поддерживать высокий уровень,
	// а вторым выходом непосредственно управлять помпой
	HAL_GPIO_WritePin(Pump1_GPIO_Port, Pump1_Pin, GPIO_PIN_SET);

	// Выключение помпы
	PumpOff();

	// Кнопки для опроса
	KEYB_Add_Button(Button_Left_GPIO_Port, Button_Left_Pin, KEYB_LEFT, KEYB_BUTTON_ACTIVE);
	KEYB_Add_Button(Button_Right_GPIO_Port, Button_Right_Pin, KEYB_RIGHT, KEYB_BUTTON_ACTIVE);
	KEYB_Add_Button(Button_Up_GPIO_Port, Button_Up_Pin, KEYB_UP, KEYB_BUTTON_ACTIVE);
	KEYB_Add_Button(Button_Down_GPIO_Port, Button_Down_Pin, KEYB_DOWN, KEYB_BUTTON_ACTIVE);
	KEYB_Add_Button(Button_Ok_GPIO_Port, Button_Ok_Pin, KEYB_OK, KEYB_BUTTON_ACTIVE);

	// Запуск таймера 2 (для обработки кнопок)
	TIM2->CR1 |= TIM_CR1_CEN;

	// Запуск таймера 5 (для управления светодиодами)
	HAL_TIM_Base_Start_IT(&htim5);

	// Инициализация дисплея
	OLED_Init(&hi2c1);

	// Установка начальных возможных
	// позиций курсора (кадр №0)
	DefaultCursorPositions();

	// Монтируем логический диск
	MountDisk();

	// Чтение настроек полива
	// и времени автоотключения экрана
	// при включении (при успешном монтировании диска)
	if (!f_ErrorSD)
		InitReadFlashParams();

	// Определение времени
	// автоотключения экрана
	// при включении
	InitScreenOff();

	// Включение АЦП
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &adc_data, 2);
}


// Отрисовка кадра со спящим смайликом
void DrawSleepFrame(void)
{
	// Отрисовка смайлика
	OLED_DrawBitmap(ImageSleepMode, 64, 64, 3, 0);

	// "Спящий режим"
	FontSet(Segoe_UI_Rus_12);
	OLED_DrawStr("Cgzobq", 64 + 2, 13 * 2, 1);
	OLED_DrawStr("ht;bv", 64 + 2 + 5, 13 * 3 + 2, 1);
}


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
  MX_ADC1_Init();
  MX_CRC_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */

  // Инициализация всего и
  // определение начальных
  // параметров при включении
  InitAll();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

	  // Проверка наличия критичесих ошибок
	  CheckFatalErrors();

	  // Если нет критических ошибок
	  if (!f_FatalErrors)
	  {
		  // Чтение настроек,
		  // если они были изменены
		  ReadFlashParams();

		  // Чтение текущего времени
		  Get_Time();

		  // Обработка данных с АЦП
		  ADC_DataProcessing();

		  // Обновление температуры
		  force_temp_conv();

		  // Очистка экрана
		  OLED_Clear(0);

		  // Обработка нажатий на кнопки
		  ButtonsPress();

		  // Если установлен "спящий" режим
		  if (f_SleepMode)
		  {
			  // Отрисовка кадра со смайликом, если после
			  // установки спящего режима не прошло 30 секунд
			  if (f_DrawFrameSleepMode)
				  DrawSleepFrame();
		  }

		  // Обычный режим
		  else
		  {
			  // Установка новых возможных позиций
			  // курсора, если кадр изменился
			  SetCursorPositions();

			  // Отрисовка курсора
			  DrawCursor();

			  // Построение текущего кадра
			  BuildFrame(current_frame);
		  }

		  // Включение помпы (начало полива)
		  IsTimeToStartWatering();
	  }

	  // Если есть критические ошибки
	  else
	  {
		  // Отключение помпы
		  PumpOff();

		  // Очистка экрана
		  OLED_Clear(0);

		  // Отрисовка кадра ошибки
		  DrawErrorFrame();
	  }

	  // Вывод следующего кадра
	  OLED_UpdateScreen();

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  htim2.Init.Prescaler = 999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 359;
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
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7199;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 7199;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 9999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 7199;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 9999;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SD_CS_Pin|LED1_Pin|LED2_Pin|LED3_Pin
                          |Pump2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Pump1_GPIO_Port, Pump1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SD_CS_Pin LED1_Pin LED2_Pin LED3_Pin
                           Pump2_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin|LED1_Pin|LED2_Pin|LED3_Pin
                          |Pump2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Pump1_Pin */
  GPIO_InitStruct.Pin = Pump1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Pump1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Button_Down_Pin Button_Right_Pin Button_Up_Pin Button_Ok_Pin */
  GPIO_InitStruct.Pin = Button_Down_Pin|Button_Right_Pin|Button_Up_Pin|Button_Ok_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : Button_Left_Pin */
  GPIO_InitStruct.Pin = Button_Left_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Button_Left_GPIO_Port, &GPIO_InitStruct);

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
