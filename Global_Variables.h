#include "Configuration.h"
//=============================================================================
//============================ Глобальные переменные ==========================
//=============================================================================
#define MAX_DATA_SIZE 90

//  Массив байтов от SECU и флаг успешного получения данных
byte DataOk = 0;
byte Data[MAX_DATA_SIZE + 2] = {};

// Размер пакета данных
byte DataSize = 0;

// Смещение данных после расширения блока CE
byte DataShift = 0;

// Таймер для изменения яркости
unsigned long BrightTimer = 0;

// Режим подсветки, 0 - день, 1 - ночь
byte BrightMode = 0;

// Яркость подсветки дисплея, 0 - день, 1 - ночь, 2 - буфер для перехода
byte BrightLCD[3] = {};

// ШИМ подсветка приборной панели.
byte BrightPWM[3] = {};

// Таймер для блока отображения яркости
char BrightBoxState = 0;
// Таймеры для дисплея
unsigned long LCDTimer = 0;
// Таймер для бокса инверсии 
char AlarmBoxState = 0;

// Пройденное расстояние в км за время работы
float DIST = 0.0;       // (29-31)  Дистанция

// Израсходованно топлива за поездку в мл
float FuelBurnedRide = 0.0;

// Переменные расчета и для хранения пробега и израсходованного топлива (в мл)
unsigned long FuelTimer = 0;
float PrevFF_FRQ = 0.0;
// Суточные
float Distance = 0.0;
float FuelBurned = 0.0;
// Всего
float DistanceAll = 0.0;
float FuelBurnedAll = 0.0;

// Моточасы в секундах
unsigned long EngineHours = 0;
// Таймер для моточасов
unsigned long EngineTimer = 0;

// Время работы за поездку
unsigned long RideTimer = 0;

// Уровень топлива
float FuelTankLevel = -1.0;
// Статус лампы аварийного уровня топлива
byte FuelTankState = 0;

// Энкодер
#ifdef ENCODER_CONTROL
	char EncoderState = 0;
#endif
	
// Состояние кнопок (0 - не нажата, 1 - нажата, 2 - обработана)
byte ButtonState[4] = {}; // Ввер/внизх, влево/вправо
// Таймер для кнопки.
unsigned long ButtonTimer = 0;
// Номер активного экрана
byte ScreenMode = 0;
// Флаг смены экрана
char ScreenChange = 0;

// Номер экрана ошибок CE
#define LCD_MODE_CE 3

#ifdef SPEED_CHIME_PIN
	// Состояние колокольчика AE86
	byte SpeedChimeStatus = 0;
	// Таймер для колокольчика
	unsigned long SpeedChimeTimer = 0;
#endif

// Датчик температуры DS18B20
#ifdef TEMP_SENSOR_PIN
	byte TempSensorStatus = 0;
	int TempSensorValue = 0;
#endif

// Наличие ошибок CE
byte StatusCE = 0;
// Количество ошибок
#define MAX_CE_BITS_COUNT 24
byte CountCE[MAX_CE_BITS_COUNT] = {};
// Предыдущее состояние
uint32_t PrevCE = 0;

#ifdef TM1637_ENABLE
	TM1637Display Display7S(TM1637_CLK_PIN, TM1637_DIO_PIN); // 28 байт
	// Буфер для перехода яркость подсветки
	byte TM1637Bright = 0;
#endif

#ifdef DEBUG_MODE
	unsigned long ProgramTimer = 0;
#endif

// Спекцсимволы в пакете данных
#define FOBEGIN  0x40       // '@'  Начало исходящего пакета
#define FIOEND   0x0D       // '\r' Конец пакета
#define FESC     0x0A       // '\n' Символ подмены байта (FESC)
// Измененные байты, которые были в пакете и совпадали со сцец байтами
#define TFOBEGIN 0x82       // Измененный FOBEGIN
#define TFIOEND  0x83       // Измененный FIOEND
#define TFESC    0x84       // Измененный FESC
