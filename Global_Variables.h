#include "Configuration.h"
//=============================================================================
//============================ Глобальные переменные ==========================
//=============================================================================
#define MAX_DATA_SIZE 86

//  Массив байтов от SECU и флаг успешного получения данных
byte DataOk = 0;
byte Data[MAX_DATA_SIZE + 2];

// Размер пакета данных
byte DataSize = 0;

// Смещение данных после расширения блока CE
byte DataShift = 0;
#define MAX_CE_BITS_COUNT 21

// Яркость подсветки дисплея
byte Bright;
// Таймеры для дисплея
unsigned long LCDTimer = 0;
// Таймер для бокса инверсии 
char BoxState = 0;

// В глобальных переменных только дистанция для использования в прерывании
float DIST = 0.0;       // (29-31)  Дистанция

// Переменные расчета и для хранения пробега и израсходованного топлива
unsigned long FuelTimer = 0;
float PrevFF_FRQ = 0.0;
// Суточные
float Distance = 0.0;
float FuelBurned = 0.0;
// Всего
float DistanceAll = 0.0;
float FuelBurnedAll = 0.0;

// Энкодер
char EncoderState = 0;
// Состояние кнопки (0 - не нажата, 1 - нажата, 2 - обработана)
byte ButtonState = 0;
// Таймер для кнопки.
byte ButtonTimer = 0;
// Номер активного экрана
byte LCDMode = 0;


// Состояние колокольчика AE86
byte SpeedChimeStatus = 0;
// Таймер для колокольчика
unsigned long SpeedChimeTimer = 0;

// Наличие ошибок CE
byte StatusCE = 0;
// Количество ошибок
byte CountCE[21];
// Предыдущее состояние
uint32_t PrevCE;

#ifdef TM1637_ENABLE
	TM1637Display Display7S(TM1637_CLK_PIN, TM1637_DIO_PIN);
#endif

// Спекцсимволы в пакете данных
#define FOBEGIN  0x40       // '@'  Начало исходящего пакета
#define FIOEND   0x0D       // '\r' Конец пакета
#define FESC     0x0A       // '\n' Символ подмены байта (FESC)
// Измененные байты, которые были в пакете и совпадали со сцец байтами
#define TFOBEGIN 0x82       // Измененный FOBEGIN
#define TFIOEND  0x83       // Измененный FIOEND
#define TFESC    0x84       // Измененный FESC
