//=============================================================================
//========================= Выбор версии прошивки =============================
//=============================================================================

// Скорость порта для процессора 1284 - 115200,
// 							  для 644 - 57600
#define SERIAL_PORT_SPEED 115200

//=============================================================================
//======================== Назначение выводов =================================
//=============================================================================

// Превывание на отключение питания
#define POWER_PIN 2
// Управление яркостью подсветки ШИМ
#define BRIGHT_PIN 3
// Колокольчик AE86
#define SPEED_CHIME_PIN 4
// Энкодер
#define ENCODER_PIN_A 5
#define ENCODER_PIN_B 7
#define ENCODER_PIN_C 6
// Пины для TM1637
#define TM1637_DIO_PIN 8
#define TM1637_CLK_PIN 9


//=============================================================================
//============================= Настройки =====================================
//=============================================================================

// Число импульсов датчика скорости на 1 км
#define SPEED_SENSOR_COUNT 6000

// Скорость обновления основного и второго экрана (пауза между обновлениями в мс)
#define LCD_UPDATE_DELAY 400
// Скорость обновления экрана замера разгона
#define LCD_ACC_UPDATE_DELAY 250

// Яркость подсветки
// Минимальная
#define MIN_BRIGHT 128
// По умолчанию
#define STD_BRIGHT 180
// Шаг регулировки яркости
#define BRIGHT_STEP 4

// Раскомментировать для отображения оборотов на дополнительном экране TM1637.
//#define TM1637_ENABLE
// Яркость свечения экрана, 0 - минимальная, 7 - макимальная.  
#define TM1637_BRIGHTNESS 4

// Что выводить на дополнительный дисплей:
// 0 - Обороты,
// 1 - EGT,
// 2 - AFR,
// 3 - Загрузка форсунок,
// 4 - ДАД.
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
#define LAMBDA_AFR_MIN 13.0
#define LAMBDA_AFR_MAX 15.0

// Порог значений наддува
#define OVERBOOST_LIMIT 150.0

// Пороги значений для лямбда коррекции
#define LAMBDA_CORR_MIN -15.0
#define LAMBDA_CORR_MAX 15.0

// Пороги напряжения сети
#define BATT_VOLT_MIN 12.0
#define BATT_VOLT_MAX 14.5

// Порог значений оборотов
#define RPM_LIMIT 5500

// Резерв топлива
#define FUEL_TANK_MIN 10

// Порог значений температуры выхлопных газов
#define EGT_LIMIT 850

// Порог значений давления масла
#define OIL_PRESSURE_MIN 1.0
#define OIL_PRESSURE_MAX 5.0

// Лимит скорости
#define SPEED_CHIME_LIMIT 100
