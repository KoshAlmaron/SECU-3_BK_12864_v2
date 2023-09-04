//=============================================================================
//========================= Выбор версии прошивки =============================
//=============================================================================

// Скорость порта для процессора 1284 - 115200,
// 							  для 644 - 57600
#define SERIAL_PORT_SPEED 115200

//=============================================================================
//======================== Назначение выводов =================================
//=============================================================================
/*

	Здесь можно записать свое распределение выводов для справки,
	а после настроить кнофигурацию ниже.

	0 - Прием данных от SECU
	1 - 
	2 - Speed Chime (опционально)
	3 - ШИМ управление подсветкой приборной панели
	4 - Выход управления питанием
	5 - Датчик температуры DS18B20
	6 - Вход для проверки состояния ДАДМ
	7 - Вход для проверки состояния замка зажигания
	8 - 
	9 - SPI DC
	10 - SPI CS
	11 - SPI MOSI
	12 - SPI MISO (Не используется, но переназначать нельзя)
	13 - SPI SCK

	A0 - Кнопка вправо (Encoder C)
	A1 - Кнопка вниз (Encoder B)
	A2 - Кнопка влево
	A3 - Кнопка вверх (Encoder A)
	A4 - Датчик EGT (опционально)
	A5 - Фоторезистор регулировки яркости

Как ШИМ выходы можно использовать только пины 3, 5, 6
и 9, 10, 11, если дисплей подключен не по SPI шине.


Назначение выводов с описанием "Опционально" можно закомментировать.
В таком случае данный функционал будет отключен и не будет занимать память,
а освободившийся вывод можно использовать для других целей.

*/

// Управление энкодером
//#define ENCODER_CONTROL
// Кнопки
#define BUTTON_UP_PIN A3		// Вверх (Encoder A)
#define BUTTON_DOWN_PIN A1		// Вниз (Encoder B)
#define BUTTON_RIGHT_PIN A0		// Вправо (Encoder C)
#define BUTTON_LEFT_PIN A2		// Влево (Не используется при управлении энкодером)

// Входы для проверки состояний
#define INT_IGN_PIN 7			// Замка зажигания
#define INT_LIGHT_PIN 8		    // Габаритов (Не используется при авто управлении яркостью)
#define INT_OIL_PIN 6			// Датчика аварийного давления масла (Опционально)

// Выход управления питанием
#define POWER_RELAY_PIN 4

// ШИМ Управление яркостью
// Для дисплеев с программной регулировкой яркости, например OLED дисплеи,
// строку назначения пина "#define LCD_BRIGHT_PIN" необходимо закомментировать.
//#define LCD_BRIGHT_PIN 5		// Подсветки экрана (Опционально)
//#define PWM_BRIGHT_PIN 3		// Приборной панели (Опционально)

// Авто управление яркостью, раскомментировать для включения (Опционально)
#define AUTO_BRIGHT_ENABLE
#define AUTO_BRIGHT_PIN A5		// Фоторезистор регулировки яркости

// SPI дисплей
#define SPI_DC_PIN 9
#define SPI_CS_PIN 10
//#define SPI_RS_PIN 4 			// Reset для OLED с чипом SH1106 (stasoks)

// Колокольчик AE86 (Опционально)
#define SPEED_CHIME_PIN 2

// Датчик EGT (Опционально)
// Если параметр определен, то данные будут выводится с него, а не с пакета SECU
//#define EGT_SENSOR_PIN A4

// Показывать сырые значения analogRead() для настройки
//#define EGT_SENSOR_RAW_VALUE

// Коэффициенты для расчета EGT
#define EGT_SENSOR_KF_A 1.1
#define EGT_SENSOR_KF_B 12

// Усреднение показаний
#define EGT_COUNT_AVG 100

// Датчик температуры DS18B20 (Опционально)
#define TEMP_SENSOR_PIN 5

// Сегментный дисплей TM1637
//#define TM1637_ENABLE
//#define TM1637_DIO_PIN A4
//#define TM1637_CLK_PIN 6

// Вывод сигнала малого уровня топлива (Опционально)
//#define FUEL_TANK_LOW_LEVEL_PIN 8

//=============================================================================
//============================= Настройки =====================================
//=============================================================================

// Число импульсов датчика скорости на 1 км
#define SPEED_SENSOR_COUNT 6000

// Усреднение показаний уровня топлива, 
// чем больше число, тем медленее меняются показания.
// Минимум 1
#define FUEL_TANK_LEVEL_AVG 20

// Скорость обновления основного и второго экрана (пауза между обновлениями в мс)
#define LCD_UPDATE_DELAY 300
// Скорость обновления экрана замера разгона
#define LCD_ACC_UPDATE_DELAY 250

// Включение анимации смены экрана
#define ANIMATION_ENABLE
// Скорость анимации смена экрана (пикселей за цикл)
#define ANIMATION_SPEED 10

// Количество обновлений экрана для бокса выделения параметра
#define ALARM_BOX_TIMER 3
// Количество обновлений экрана для блока отображения яркости
#define BRIGHT_BOX_TIMER 16

// Минимальная яркость подсветки дисплея,
// для обычного LCD дисплея с ШИМ подсветкой 
// при значении меньше 128 уже ничего не видно.
// Для OLED можно ставить 0.
#define LCD_BRIGHT_MIN 0
// Шаг регулировки яркости
#define LCD_BRIGHT_STEP 8

// Минимальная яркость подсветки приборной панели.
#define PWM_BRIGHT_MIN 0
// Шаг регулировки яркости
#define PWM_BRIGHT_STEP 8

// Время отображения экрана завершения в секундах.
#define FINISH_SCREEN_TIME 8

// Что выводить на дополнительный дисплей TM1637:
// 0 - Обороты,
// 1 - EGT,
// 2 - AFR,
// 3 - Загрузка форсунок,
// 4 - ДАД,
// 5 - Скорость.
#define TM1637_DATA_TYPE 0

// Интервал включения и время удержания колокольчика AE86
#define SPEED_CHIME_INTERVAL 1000
#define SPEED_CHIME_DELAY 380

//=============================================================================
//============ Лимиты показаний для срабатывания сигнализации =================
//=============================================================================

// Пороги значений для температуры ОЖ
#define WATER_TEMP_MIN -30.0
#define WATER_TEMP_MAX 96.0

// Пороги значений для температуры воздуха
#define AIR_TEMP_MIN -20.0
#define AIR_TEMP_MAX 50.0

// Порог значений для УДК
#define UDK_VOLT_MIN 0.01
#define UDK_VOLT_MAX 0.87

// Порог значений для ШДК
#define LAMBDA_AFR_MIN 12.0
#define LAMBDA_AFR_MAX 15.2

// Порог значений наддува
#define OVERBOOST_LIMIT 176.0

// Пороги значений для лямбда коррекции
#define LAMBDA_CORR_MIN -15.0
#define LAMBDA_CORR_MAX 15.0

// Пороги напряжения сети
#define BATT_VOLT_MIN 12.0
#define BATT_VOLT_MAX 14.5

// Порог значений оборотов
#define RPM_LIMIT 5500

// Резерв топлива
#define FUEL_TANK_MIN 10.0

// Порог значений температуры выхлопных газов
#define EGT_LIMIT 850

// Порог значений давления масла
#define OIL_PRESSURE_MIN 1.0
#define OIL_PRESSURE_MAX 5.0

// Лимит скорости
#define SPEED_CHIME_LIMIT 100