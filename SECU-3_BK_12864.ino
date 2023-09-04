/*
	2020-11-28
	2020-12-03
	2020-12-06
	2020-12-07
	2020-12-09 - Изменил размер пакета для версии 4.9
	2020-12-11 - При ХХ отображается только РХХ, 
						ДПДЗ отображается при положении газа более 2%.
					- Поменял шрифты температуры ОЖ и РХХ.
					- Добавил пороги для значений и их индикацию.
	2021-01-05 - Добавил возможность отображения AFR вместо напряжения.
					- Добавил пороги для напряжения.
					- Добавил возможность выбора числа импульсов датчика 
						скорости на 1 км.
	2021-02-19 - Добавил экран замера разгона 0-100.
	2021-04-25 - Изменил размер пакета для версии 4.9 от 14.04.2021
	2021-06-20 - Добавил сигнализацию превышения скорости (колокольчик AE86). 
	2021-07-27 - Добавил экран ошибок CE.
	2021-09-28 - Исправил размер пакета для версии 4.8. Вынес в настройки скорость порта.
	2021-10-03 - Добавил автоматическое переключение вывода АФР или напряжение УДК.
					- Добавил возможность вывода оборотов на семисегментный дисплей на TM1637.
					- Увеличил частоту обновления экрана.
					- Добавил в настройки скорость обновления экрана.
	2021-10-04 - Исправил баг с переполнением буфера незадействованных параметров,
						которые имели значение "7FFF".
					- Добавил ограничения значений для всех параметров.


	0 - Прием данных от SECU
	1 - 
	2 - Прерывание на отключение питания
	3 - ШИМ управление подсветкой дисплея
	4 - Speed Chime
	5 - Encoder A
	6 - Encoder C
	7 - Encoder B
	8 - TM1637 DIO
	9 - TM1637 CLK
	10 - SPI RS (CS)
	11 - SPI MOSI
	12 - 
	13 - SPI SCK

	A0 - 
	A1 - 
	A2 - 
	A3 - 
	A4 - 
	A5 - 

*/

#include <U8g2lib.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <TM1637Display.h>

// Раскомментировать один блок для нужной версии прошивки

// Прошивка v4.7
//#define DATA_ARRAY_SIZE 66
//#define V49_DATA_SHIFT 0
//#define CE_BITS_COUNT 16

// Прошивка v4.8
//#define DATA_ARRAY_SIZE 69
//#define V49_DATA_SHIFT 0
//#define CE_BITS_COUNT 16

// Прошивка v4.9 до 14.04.2021
//#define V49_DATA_SHIFT 2
//#define DATA_ARRAY_SIZE 75
//#define CE_BITS_COUNT 16

// Прошивка v4.9 с 14.04.2021
#define V49_DATA_SHIFT 2
#define DATA_ARRAY_SIZE 81
#define CE_BITS_COUNT 21

// Скорость порта для процессора 1284 - 115200,
// 							  для 644 - 57600
#define SERIAL_PORT_SPEED 115200

// Число импульсов датчика скорости на 1 км
#define SPEED_SENSOR_COUNT 6000
// Коэффициент для вычисления скорости и дистанции
#define M_PERIOD_DISTANCE 1000.0 / SPEED_SENSOR_COUNT

// Пороги значений для температуры ОЖ
#define WATER_TEMP_MIN -30.0
#define WATER_TEMP_MAX 96.0

// Пороги значений для температуры воздуха
#define AIR_TEMP_MIN -20.0
#define AIR_TEMP_MAX 50.0

// Порог значений для УДК
#define UDK_VOLT_MAX 0.87

// Раскомментировать для отображения оборотов на дополнительном экране TM1637.
//#define SHOW_RPM_TM1637
// Яркость свечения экрана, 0 - минимальная, 7 - макимальная.  
#define TM1637_BRIGHTNESS 4
// Пины для TM1637
#define TM1637_DIO 8
#define TM1637_CLK 9

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

// Скорость обновления экрана (пауза между обновлениями в мс)
#define LCD_UPDATE_DELAY 400

// Таймеры для дисплея
unsigned long LCDTimer = 0;
// Таймер для бокса инверсии 
char BoxState = 0;

//  Массив байтов от SECU и флаг успешного получения данных
//byte Data[DATA_ARRAY_SIZE + 2];
byte DataOk = 0;
byte Data[DATA_ARRAY_SIZE + 2];

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
#define ENCODER_PIN_A 5
#define ENCODER_PIN_B 7
#define ENCODER_PIN_C 6
char EncoderState = 0;
// Состояние кнопки (0 - не нажата, 1 - нажата, 2 - обработана)
byte ButtonState = 0;
// Таймер для кнопки.
byte ButtonTimer = 0;
// Номер активного экрана
byte LCDMode = 0;

// Яркость подсветки
byte Bright;
#define POWER_PIN 2
#define BRIGHT_PIN 3
#define MIN_BRIGHT 121
#define STD_BRIGHT 180

// Колокольчик AE86
#define SPEED_CHIME_PIN 4
// Лимит скорости
#define SPEED_CHIME_LIMIT 100
// Интервал включения и время удержания
#define SPEED_CHIME_INTERVAL 1000
#define SPEED_CHIME_DELAY 380
//Состояние колокольчика
byte SpeedChimeStatus = 0;
// Таймер для колокольчика
unsigned long SpeedChimeTimer = 0;

// Наличие ошибок CE
byte StatusCE = 0;
// Количество ошибок
byte CountCE[CE_BITS_COUNT];
// Предыдущее состояние
uint32_t PrevCE;

// Настройка LCD дисплея
U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, 10);

// Спекцсимволы в пакете данных
#define FOBEGIN  0x40       // '@'  Начало исходящего пакета
#define FIOEND   0x0D       // '\r' Конец пакета
#define FESC     0x0A       // '\n' Символ подмены байта (FESC)
// Измененные байты, которые были в пакете и совпадали со сцец байтами
#define TFOBEGIN 0x82       // Измененный FOBEGIN
#define TFIOEND  0x83       // Измененный FIOEND
#define TFESC    0x84       // Измененный FESC

void write_eeprom() {
	analogWrite(BRIGHT_PIN, 0);
	float Dst = Distance + DIST;
	float DstAll = DistanceAll + DIST;

	// Dst = 210;
	// DstAll = 2100;
	// FuelBurned = 15.6;
	// FuelBurnedAll = 164.1;

	for (byte i = 0; i < 4; i++ ) {
		byte *pValue = (byte*)&Dst;
		EEPROM.write(i, *(pValue + i)); 
	}
	for (byte i = 0; i < 4; i++ ) {
		byte *pValue = (byte*)&DstAll;
		EEPROM.write(i + 4, *(pValue + i)); 
	}
	for (byte i = 0; i < 4; i++ ) {
		byte *pValue = (byte*)&FuelBurned;
		EEPROM.write(i + 8, *(pValue + i)); 
	}
	for (byte i = 0; i < 4; i++ ) {
		byte *pValue = (byte*)&FuelBurnedAll;
		EEPROM.write(i + 12, *(pValue + i)); 
	}
	EEPROM.write(20, Bright); 

	analogWrite(BRIGHT_PIN, Bright);
	//Serial.println("EEPROM");
}

byte read_buff(byte b1) {
	if (b1 == FESC) {
		byte b2 = Serial.read();
		if (b2 == TFOBEGIN)
			return FOBEGIN;
		else if (b2 == TFIOEND)
			return FIOEND;
		else if (b2 == TFESC)
			return FESC;
		return 0;
	}
	else {
		return b1;
	}
}

void receive_data() {
	byte IncomingByte = 0;
	unsigned long Timer = millis();
	byte Receive = 0;
	byte N = 0;
	DataOk = 0;

	// Ждем символ начала пакета.
	while (millis() - Timer < 500) {
		if (Serial.available() > 0) {
			IncomingByte = Serial.read();
			if (IncomingByte == FOBEGIN) {
				Receive = 1;
				break;
			}
		}
		else {
			button_update();
			encoder_update();
		}
	}
	if (Receive) {
		Timer = millis();
		while (millis() - Timer < 500) {
			if (Serial.available() > 1) {
				IncomingByte = Serial.read();
				if (IncomingByte == FIOEND) {
					if (N == DATA_ARRAY_SIZE) {
						DataOk = 1;
					}
					break;
				}
				else {
					Data[N] = read_buff(IncomingByte);
					N = N + 1;
					if (N > DATA_ARRAY_SIZE) {
						break;
					}
				}
			}
			else {
				button_update();
				encoder_update();
			}
		}
	}
}

int build_int(byte i) {
	int Value = 0;
	byte *pValue = (byte*)&Value;
	*pValue = Data[i + 1];  
	*(pValue + 1) = Data[i];
	return Value;
}

// Расчет скорости ТС
float build_speed(byte i) {
	unsigned int Value = 0;
	byte* pValue = (byte*)&Value;
	*pValue = Data[i + 1];  
	*(pValue + 1) = Data[i];
	if (Value != 0 && Value != 65535) {
		float Period = (float) Value / 312500.0;
		float Speed = (float)  ((M_PERIOD_DISTANCE / Period) * 3600.0) * 0.001;
		Speed = constrain(Speed, 0, 333);
		return Speed;
	}
	else {
		return 0.0;
	}
}

// Расчет пройденного пути
float build_distance(byte i) {
	unsigned long Value = 0;
	byte* pValue = (byte*)&Value;
	*pValue = Data[i + 2]; 
	*(pValue + 1) = Data[i + 1];
	*(pValue + 2) = Data[i];

	float Dist = (float) (M_PERIOD_DISTANCE * Value) * 0.001;
	return Dist;
}

// Сборка данных из байтов от SECU
void build_data() {
	if (DataOk) {
		if (millis() - FuelTimer >= 500) {
			DIST = build_distance(29 + V49_DATA_SHIFT);

			// Расчет израсходованного топлива за интервал
			float FF_FRQ = (float) build_int(32 + V49_DATA_SHIFT) * 0.00390625;  // 256
			FF_FRQ = constrain(FF_FRQ, 0, 256);
			if (FuelTimer > 0) {
				float FFAVG = (float) (PrevFF_FRQ + FF_FRQ) * 0.1125; // Расход л/ч (3600 / 16000) / 2
				FuelBurned += (float) FFAVG * (millis() - FuelTimer) / 3600000;
				FuelBurnedAll += (float) FFAVG * (millis() - FuelTimer) / 3600000;
			}
			FuelTimer = millis();
			PrevFF_FRQ = FF_FRQ;
		}
	}
}

void button_update() {
	// ButtonState 0 - выкл, 1 - короткое включение, 2 - исполнено,
	//             3 - длинное включение, 5 - первый контакт.
	if (!digitalRead(ENCODER_PIN_C)) {
		if (ButtonState == 0) {
			ButtonState = 5;
			ButtonTimer = 0;
		}
	}
	else {
		if (ButtonState == 2) {
			if (ButtonTimer >= 1) {
				ButtonState = 0;
			}
		}
		else if (ButtonState == 5) {
			if (ButtonTimer >= 4) {
				ButtonState = 3;
			}
			else {
				ButtonState = 1;
			}
			ButtonTimer = 0;
		}
	}
}

void encoder_update() {
	boolean PinState[2];
	PinState[0] = digitalRead(ENCODER_PIN_A);
	PinState[1] = digitalRead(ENCODER_PIN_B);

	// Начальная позиция 11.
	if (PinState[0] && PinState[1]) {
		// Если переход на начальную позицию после шага 3,
		// то выполняем комманду.
		if (abs(EncoderState) == 3) {
			EncoderState = EncoderState + EncoderState/abs(EncoderState);
		}
	}
	// Первый шаг.
	if (EncoderState == 0) {
		// 10 +.
		if (PinState[0] && !PinState[1]) {EncoderState += 1;}
		// 01 -.
		if (!PinState[0] && PinState[1]) {EncoderState -= 1;}
	}
	// Второй шаг.
	if (abs(EncoderState) == 1) {
		// 00 *2.
		if (!PinState[0] && !PinState[1]) {EncoderState = EncoderState * 2;}  
	}
	// Третий шаг.
	if (EncoderState == 2) {
		// 01 +.
		if (!PinState[0] && PinState[1]) {EncoderState += 1;}  
	}
	if (EncoderState == -2) {
		// 10 -.
		if (PinState[0] && !PinState[1]) {EncoderState -= 1;}  
	}
}

void speed_chime() {
	float SPD = 0.0;
	SPD = build_speed(27 + V49_DATA_SHIFT);

	//SPD = 101;

	if (SpeedChimeStatus == 1) {
		if (millis() - SpeedChimeTimer >= SPEED_CHIME_DELAY) {
			digitalWrite(SPEED_CHIME_PIN, LOW);
			SpeedChimeTimer = millis();
			SpeedChimeStatus = 2;
		}
	}
	else if (SpeedChimeStatus == 2) {
		if (millis() - SpeedChimeTimer >= SPEED_CHIME_INTERVAL) {
			SpeedChimeStatus = 0;
		}
	}

	if (SpeedChimeStatus == 0) {
		if (SPD > SPEED_CHIME_LIMIT) {
			digitalWrite(SPEED_CHIME_PIN, HIGH);
			SpeedChimeTimer = millis();
			SpeedChimeStatus = 1;
		}
	}
}

// Массивы с пиктограммами
#define Trollface_width 81
#define Trollface_height 64
const unsigned char Trollface_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xf8, 0xff, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
  0xff, 0x01, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x03,
  0x1c, 0x00, 0x02, 0x80, 0x01, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00,
  0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x10, 0x00, 0x00, 0xe0,
  0x40, 0x00, 0x06, 0x00, 0x00, 0x00, 0x38, 0x00, 0xc0, 0x00, 0x00, 0x8c,
  0x00, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x30, 0x02,
  0x0c, 0x00, 0x00, 0x00, 0x06, 0xc0, 0x0f, 0x00, 0x00, 0xcc, 0x00, 0x1c,
  0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x20, 0x01, 0x18, 0x00,
  0x00, 0x00, 0x03, 0x02, 0x00, 0x80, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
  0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x80, 0x01,
  0x00, 0x00, 0x00, 0xf8, 0x1f, 0x00, 0x60, 0x00, 0x00, 0x80, 0x01, 0x60,
  0x00, 0x00, 0xde, 0x7f, 0x00, 0xe0, 0x00, 0x00, 0xc0, 0x00, 0xfc, 0x07,
  0x80, 0xc3, 0xff, 0x00, 0xc0, 0x01, 0x00, 0xe0, 0x00, 0xde, 0x1f, 0xc0,
  0xc1, 0xbf, 0x01, 0xc0, 0x03, 0x00, 0x70, 0x06, 0xfe, 0xbf, 0xc0, 0xf8,
  0xff, 0x11, 0x00, 0x07, 0x00, 0xb8, 0x00, 0x00, 0xff, 0xc0, 0x1f, 0x00,
  0x80, 0x03, 0x0c, 0x00, 0x5c, 0x00, 0x00, 0x70, 0x80, 0x07, 0x06, 0xf0,
  0x1f, 0x18, 0x00, 0xac, 0x11, 0x00, 0x30, 0x00, 0x00, 0x0e, 0x7c, 0x30,
  0x30, 0x00, 0x0c, 0xfc, 0x01, 0x30, 0x00, 0x00, 0xfc, 0x3f, 0x61, 0x20,
  0x00, 0x0c, 0x86, 0x1b, 0x30, 0x00, 0x00, 0xe0, 0x07, 0xc1, 0x60, 0x00,
  0x0c, 0x00, 0x1f, 0x38, 0x00, 0x00, 0x00, 0x80, 0x83, 0x60, 0x00, 0x0c,
  0x20, 0x00, 0x1c, 0x00, 0x03, 0x00, 0xe0, 0x87, 0x60, 0x00, 0x0c, 0x20,
  0x00, 0x07, 0x80, 0x0f, 0x00, 0x7c, 0x8f, 0x60, 0x00, 0x08, 0x31, 0x80,
  0x03, 0x00, 0x0c, 0x00, 0x1f, 0x9f, 0x60, 0x00, 0x18, 0x70, 0xc0, 0x07,
  0xf8, 0x0c, 0xe0, 0x07, 0xc3, 0x60, 0x00, 0xb0, 0x78, 0x90, 0x06, 0xf8,
  0x0c, 0xf8, 0x81, 0x43, 0x30, 0x00, 0x30, 0xf8, 0x00, 0x0c, 0x00, 0x06,
  0xff, 0xc1, 0x03, 0x32, 0x00, 0x60, 0xf8, 0x01, 0xf8, 0x00, 0xe0, 0x83,
  0xe1, 0x01, 0x18, 0x00, 0x60, 0xf8, 0x0f, 0x60, 0x00, 0x7c, 0x80, 0xff,
  0x00, 0x0c, 0x00, 0x40, 0xb8, 0x7f, 0x00, 0xe0, 0x3f, 0xc0, 0x7f, 0x00,
  0x06, 0x00, 0xc0, 0xb8, 0xf1, 0xff, 0xff, 0x30, 0xf0, 0x63, 0x00, 0x03,
  0x00, 0xc0, 0xb8, 0x31, 0xff, 0x0d, 0x30, 0xfe, 0x31, 0x80, 0x01, 0x00,
  0xc0, 0xbc, 0x19, 0x18, 0x0c, 0xf0, 0xff, 0x19, 0xc0, 0x00, 0x00, 0xc0,
  0xf8, 0x19, 0x18, 0x0c, 0xf8, 0x8f, 0x1d, 0xc0, 0x00, 0x00, 0xc0, 0xf8,
  0xff, 0xff, 0xff, 0xff, 0x81, 0x0f, 0x60, 0x00, 0x00, 0xc0, 0xf8, 0xff,
  0xff, 0xff, 0x7f, 0x00, 0x07, 0x60, 0x00, 0x00, 0xc0, 0xf8, 0xff, 0xff,
  0xff, 0x67, 0x80, 0x03, 0x20, 0x00, 0x00, 0xc0, 0xf8, 0xff, 0xff, 0xff,
  0x60, 0xc0, 0x01, 0x30, 0x00, 0x00, 0xc0, 0xf8, 0xff, 0xff, 0x1f, 0xc0,
  0xe0, 0x00, 0x30, 0x00, 0x00, 0xc0, 0xf0, 0xee, 0x3c, 0x10, 0xc0, 0x78,
  0x00, 0x18, 0x00, 0x00, 0xc0, 0xf0, 0xcd, 0x30, 0x30, 0x80, 0x1f, 0x00,
  0x0c, 0x00, 0x00, 0x40, 0xe0, 0xcc, 0x30, 0x30, 0x80, 0x0f, 0x00, 0x06,
  0x00, 0x00, 0x40, 0xe0, 0x9f, 0x21, 0x30, 0xf0, 0x01, 0x80, 0x03, 0x00,
  0x00, 0x60, 0xc0, 0xff, 0x21, 0xf0, 0x7f, 0x00, 0xc6, 0x01, 0x00, 0x00,
  0x60, 0x00, 0xfe, 0xff, 0xff, 0x07, 0x84, 0x71, 0x00, 0x00, 0x00, 0x60,
  0x00, 0xc0, 0xff, 0x1f, 0x00, 0x61, 0x3c, 0x00, 0x00, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x40, 0x0c, 0x0f, 0x00, 0x00, 0x00, 0x60, 0x80, 0x00,
  0x00, 0x00, 0x18, 0xc3, 0x03, 0x00, 0x00, 0x00, 0x60, 0x00, 0x03, 0x00,
  0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x60, 0x08, 0x18, 0x00, 0x00,
  0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00, 0x00, 0x00,
  0x0f, 0x00, 0x00, 0x00, 0x00, 0x60, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x20, 0x00, 0x00, 0xf0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x03, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0f, 0x00, 0xe0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xfc, 0xc1, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
  0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define Temp_width 8
#define Temp_height 15
const unsigned char Temp_bits[] PROGMEM = {
  0x1c, 0xf4, 0x14, 0x14, 0xf4, 0x14, 0x14, 0xf4, 0x14, 0x36, 0x63, 0x41,
  0x41, 0x63, 0x3e };

#define Cels_width 3
#define Cels_height 3
const unsigned char Cels_bits[] PROGMEM = {0x02, 0x05, 0x02};

#define Road_width 7
#define Road_height 16
const unsigned char Road_bits[] PROGMEM = {
	0x41, 0x49, 0x49, 0x49, 0x41, 0x49, 0x49, 0x49, 0x41, 0x49, 0x49, 0x49,
	0x41, 0x49, 0x49, 0x49 };

#define Angle_width 10
#define Angle_height 8
const unsigned char Angle_bits[] PROGMEM = {
	0x40, 0x00, 0x20, 0x00, 0x10, 0x00, 0x28, 0x00, 0x44, 0x00, 0x82, 0x00,
	0x81, 0x00, 0xff, 0x03 };

#define Map_width 9
#define Map_height 8
const unsigned char Map_bits[] PROGMEM = {
	0x38, 0x00, 0x54, 0x00, 0x92, 0x00, 0x82, 0x00, 0x44, 0x00, 0x38, 0x00,
	0x10, 0x00, 0xff, 0x01 };

#define Trottle_width 8
#define Trottle_height 12
const unsigned char Trottle_bits[] PROGMEM = {
	0x81, 0x81, 0x81, 0xc1, 0xa1, 0x99, 0x99, 0x85, 0x83, 0x81, 0x81, 0x81 };

#define IAC_width 8
#define IAC_height 12
const unsigned char IAC_bits[] PROGMEM = {
	0x18, 0x18, 0x3c, 0xff, 0x6a, 0x56, 0x6a, 0x56, 0x6a, 0x56, 0x6a, 0x7e };

#define AFC_width 10
#define AFC_height 15
const unsigned char AFC_bits[] PROGMEM = {
	0x10, 0x00, 0x30, 0x00, 0x78, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0xfe, 0x01,
	0xfe, 0x01, 0xff, 0x03, 0xff, 0x03, 0xf3, 0x03, 0xf3, 0x03, 0xf7, 0x03,
	0xe6, 0x01, 0xfc, 0x00, 0x78, 0x00 };

#define O2_width 11
#define O2_height 8
const unsigned char O2_bits[] PROGMEM = {
	0x1c, 0x00, 0x36, 0x00, 0x63, 0x00, 0x41, 0x07, 0x41, 0x04, 0x63, 0x07,
	0x36, 0x01, 0x1c, 0x07 };

#define LambdaCorr_width 11
#define LambdaCorr_height 8
const unsigned char LambdaCorr_bits[] PROGMEM = {
	0x8c, 0x01, 0xcc, 0x03, 0xec, 0x07, 0x8c, 0x01, 0x8c, 0x01, 0xbf, 0x01,
	0x9e, 0x01, 0x8c, 0x01 };

#define AirTemp_width 6
#define AirTemp_height 8
const unsigned char AirTemp_bits[] PROGMEM = {
	0x24, 0x0e, 0x2a, 0x0a, 0x2a, 0x11, 0x11, 0x0e };

#define Batt_width 11
#define Batt_height 8
const unsigned char Batt_bits[] PROGMEM = {
	0x8c, 0x01, 0xff, 0x07, 0x01, 0x04, 0x09, 0x04, 0xdd, 0x05, 0x09, 0x04,
	0x01, 0x04, 0xff, 0x07 };

#define Fuel_width 12
#define Fuel_height 15
const unsigned char Fuel_bits[] PROGMEM = {
	0xfe, 0x00, 0x82, 0x06, 0x82, 0x0c, 0x82, 0x08, 0x82, 0x0b, 0x82, 0x0a,
	0xfe, 0x0a, 0xfe, 0x0a, 0xfe, 0x0a, 0xfe, 0x0a, 0xfe, 0x0a, 0xfe, 0x0a,
	0xfe, 0x0e, 0xfe, 0x00, 0xff, 0x01 };

#define SPD_width 11
#define SPD_height 17
const unsigned char SPD_bits[] PROGMEM = {
	0x0e, 0x00, 0x11, 0x00, 0x01, 0x00, 0x0e, 0x00, 0x10, 0x00, 0xd1, 0x03,
	0x4e, 0x04, 0x40, 0x04, 0xc0, 0x03, 0x40, 0x00, 0x4f, 0x00, 0x51, 0x00,
	0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x0f, 0x00 };

#define MAP2_width 11
#define MAP2_height 17
const unsigned char MAP2_bits[] PROGMEM = {
	0xf8, 0x00, 0x04, 0x01, 0x82, 0x02, 0xc1, 0x04, 0x61, 0x04, 0x71, 0x04,
	0x21, 0x04, 0x01, 0x04, 0x02, 0x02, 0x04, 0x01, 0xf8, 0x00, 0x50, 0x00,
	0x50, 0x00, 0x50, 0x00, 0xdf, 0x07, 0x00, 0x00, 0xff, 0x07 };

#define Next_width 11
#define Next_height 7
const unsigned char Next_bits[] PROGMEM = {
   0x01, 0x04, 0x03, 0x06, 0x06, 0x03, 0x8c, 0x01, 0xd8, 0x00, 0x70, 0x00,
   0x20, 0x00 };

// Ошибки CE
const char CEItem_0[] PROGMEM =  "CKPS MALFUNCTION";
const char CEItem_1[] PROGMEM =  "EEPROM BROKEN   ";
const char CEItem_2[] PROGMEM =  "PROGRAM BROKEN  ";
const char CEItem_3[] PROGMEM =  "KSP CHIP FAILED ";
const char CEItem_4[] PROGMEM =  "KNOCK DETECTED  ";
const char CEItem_5[] PROGMEM =  "MAP SENSOR FAIL ";
const char CEItem_6[] PROGMEM =  "TEMP SENSOR FAIL";
const char CEItem_7[] PROGMEM =  "VOLT SENSOR FAIL";
const char CEItem_8[] PROGMEM =  "DWELL CONTROL   ";
const char CEItem_9[] PROGMEM =  "CAMS MALFUNCTION";
const char CEItem_10[] PROGMEM = "TPS SENSOR FAIL ";
const char CEItem_11[] PROGMEM = "ADD I1 SENSOR   ";
const char CEItem_12[] PROGMEM = "ADD I2 SENSOR   ";
const char CEItem_13[] PROGMEM = "ADD I3 SENSOR   ";
const char CEItem_14[] PROGMEM = "ADD I4 SENSOR   ";
const char CEItem_15[] PROGMEM = "SYS START       ";
const char CEItem_16[] PROGMEM = "ADD I5 SENSOR   ";
const char CEItem_17[] PROGMEM = "ADD I6 SENSOR   ";
const char CEItem_18[] PROGMEM = "ADD I7 SENSOR   ";
const char CEItem_19[] PROGMEM = "ADD I8 SENSOR   ";
const char CEItem_20[] PROGMEM = "ECU ERROR COUNT ";

const char* const CEItemsArray[] PROGMEM = {CEItem_0, CEItem_1
  , CEItem_2, CEItem_3, CEItem_4, CEItem_5, CEItem_6, CEItem_7
  , CEItem_8, CEItem_9, CEItem_10, CEItem_11, CEItem_12
  , CEItem_13, CEItem_14, CEItem_15, CEItem_16, CEItem_17, CEItem_18, CEItem_19, CEItem_20};

void draw_ff_fc(byte x, byte y) {
	float FF_FRQ = (float) build_int(32 + V49_DATA_SHIFT) * 0.00390625;  // 256
	FF_FRQ = constrain(FF_FRQ, 0, 256);
	float TPS = (float) Data[18] * 0.5;                     // 2
	float SPD = build_speed(27 + V49_DATA_SHIFT);
	float FF = 0.0;
	float FC = 0.0;

	// Режим отображения
	byte Mode = 1;

	// Мгновенный расход л/ч
	FF = (float) FF_FRQ * 0.225; // (3600 / 16000)
	// Мгновенный расход л/100км
	if (SPD > 3 && TPS > 3) {
	FC = min(99.9, (FF * 100.0) / SPD);
		// Режим отображения №2 - движение 
		Mode = 2; // Drive
	}
  
	char CharVal[5];
	byte H;
	byte L;

	u8g2.setFont(u8g2_font_haxrcorp4089_tr);
	H = u8g2.getAscent();
	u8g2.drawUTF8(x + 1, y + H + 2, "F");

	if (Mode == 1) {
		dtostrf(FF, 4, 1, CharVal);
		u8g2.drawUTF8(x + 1, y + H * 2 + 4, "F");
	}
	else if (Mode == 2) {
		dtostrf(FC, 4, 1, CharVal);
		u8g2.drawUTF8(x + 1, y + H * 2 + 4, "C");
	}

	u8g2.setFont(u8g2_font_helvB12_tn);
	H = u8g2.getAscent();
	L = u8g2.getUTF8Width(CharVal);
	if (CharVal[3] == '1') {L += 2;}
	u8g2.drawUTF8(x + 10 + (30 - L), y + 4 + H, CharVal);
}

void draw_water_temp(byte x, byte y, byte a=0) {
	float TEMP = (float) build_int(7) * 0.25;
	TEMP = constrain(TEMP, -99, 333);
	char CharVal[6];
	byte H;
	byte L;

	if (BoxState > 0) {
		if (TEMP < WATER_TEMP_MIN || TEMP > WATER_TEMP_MAX) {
			u8g2.drawBox(x, y + 1, 42 + a, 19);
		}
	}

	u8g2.drawXBMP(x + 1, y + 3, Temp_width, Temp_height, Temp_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(TEMP, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 34 - L + a, y + H + 5, CharVal);
	u8g2.drawXBMP(x + 34 + 1 + a, y + 5, Cels_width, Cels_height, Cels_bits);
}

void draw_distance(byte x, byte y) {
	char CharVal[6];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 1, y + 2, Road_width, Road_height, Road_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(Distance + DIST, 5, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 11 + (29 - L), y + H + 2, CharVal);

	dtostrf(DistanceAll + DIST, 5, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 11 + (29 - L), y + H * 2 + 5, CharVal);
}

void draw_map_airtemp(byte x, byte y) {
	float MAP = (float) build_int(3) * 0.015625;            // 64
	MAP = constrain(MAP, 0, 333);
	float AIRTEMP = (float) build_int(34 + V49_DATA_SHIFT) * 0.25;           // 4
	AIRTEMP = constrain(AIRTEMP, -99, 333);
	byte H;
	byte L;
	char CharVal[6];

	if (BoxState > 0) {
		if (MAP > OVERBOOST_LIMIT) {
			u8g2.drawBox(x + 1, y, 39, 10);
		}
		if (AIRTEMP < AIR_TEMP_MIN || AIRTEMP > AIR_TEMP_MAX) {
			u8g2.drawBox(x + 1, y + 10, 39, 10);
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, Map_width, Map_height, Map_bits);
	u8g2.drawXBMP(x + 4, y + 11, AirTemp_width, AirTemp_height, AirTemp_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(MAP, 5, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 13 + (22 - L), y + H + 2, CharVal);

	dtostrf(AIRTEMP, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 13 + (22 - L), y + H * 2 + 5, CharVal);
	u8g2.drawXBMP(x + 13 + 22 + 1, y + H + 5, Cels_width, Cels_height, Cels_bits);
}

void draw_trottle(byte x, byte y) {
	float TPS = (float) Data[18] * 0.5;                     // 2
	float IAC = (float) Data[25 + V49_DATA_SHIFT] * 0.5;                     // 2
	char CharVal[6];
	byte H;
	byte L;

	if (TPS > 2) {
		dtostrf(TPS, 5, 1, CharVal);
		u8g2.drawXBMP(x + 2, y + 4, Trottle_width, Trottle_height, Trottle_bits);
	}
	else {
		dtostrf(IAC, 5, 1, CharVal);
		u8g2.drawXBMP(x + 2, y + 4, IAC_width, IAC_height, IAC_bits);
	}
	
	u8g2.setFont(u8g2_font_helvB08_tn);
	H = u8g2.getAscent();
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 14 + (22 - L), y + H + 6, CharVal);
}

void draw_afc(byte x, byte y) {
	float AFC = 0.0;
	float TAFC = 0.0;
	char CharVal[5];
	byte H;
	byte L;

	// Средний расход суточный
	if (Distance + DIST > 1) {
		AFC = min(99.9, FuelBurned / (Distance + DIST) * 100);
	}
	// Средний расход общий
	if (DistanceAll + DIST > 1) {
		TAFC = min(99.9, FuelBurnedAll / (DistanceAll + DIST) * 100);
	}

	u8g2.drawXBMP(x + 3, y + 3, AFC_width, AFC_height, AFC_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(AFC, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 15 + (21 - L), y + H + 2, CharVal);

	dtostrf(TAFC, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 15 + (21 - L), y + H * 2 + 5, CharVal);
}

void draw_O2_sensor(byte x, byte y) {
	// Тип датчика кислорода, 0 - УДК, 1 - ШДК.
	byte LambdaType;
	char CharVal[6];
	byte H;
	byte L;

	float AFR = (float) build_int(60 + V49_DATA_SHIFT) * 0.0078125;      // 128
	AFR = constrain(AFR, 0, 33);
	if (AFR > 0) {
		// Если есть показания АФР, значит показывать надо АФР.
		LambdaType = 1;
		dtostrf(AFR, 4, 1, CharVal);
	}
	else {
		// Иначе показываем напряжение.
		LambdaType = 0;
		AFR = (float) build_int(19) * 0.0025;      // 400
		AFR = constrain(AFR, 0, 5);
		dtostrf(AFR, 3, 2, CharVal);
	}

	float AFR_CORR = (float) build_int(50 + V49_DATA_SHIFT) * 0.1953125; // 5.12
	if (BoxState > 0) {
		if (LambdaType == 1) {
			if (AFR < LAMBDA_AFR_MIN || AFR > LAMBDA_AFR_MAX) {
				u8g2.drawBox(x + 1, y, 41, 10);
			}
		}
		else {
			if (AFR > UDK_VOLT_MAX) {
				u8g2.drawBox(x + 1, y, 41, 10);
			}
		}
		if (AFR_CORR < LAMBDA_CORR_MIN || AFR_CORR > LAMBDA_CORR_MAX) {
			u8g2.drawBox(x + 1, y + 10, 41, 10);
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, O2_width, O2_height, O2_bits);
	u8g2.drawXBMP(x + 2, y + 11, LambdaCorr_width, LambdaCorr_height, LambdaCorr_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 15 + (25 - L), y + H + 2, CharVal);

	dtostrf(AFR_CORR, 5, 1, CharVal);
	if (AFR_CORR > 0) {
		CharVal[0] = '+';
	}

	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 15 + (25 - L), y + H * 2 + 5, CharVal);
}

void draw_angle_batt(byte x, byte y) {
	float BAT = (float) build_int(5) * 0.0025;          // 400
	BAT = constrain(BAT, 0, 33);
	float ANGLE = (float) build_int(9) * 0.03125;           // 32
	ANGLE = constrain(ANGLE, 0, 99);
	float DDANGLE = (float) build_int(13) * 0.03125;           // 32
	DDANGLE = constrain(DDANGLE, 0, 99);
	char CharVal[6];
	byte H;
	byte L;

	if (BoxState > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			u8g2.drawBox(x + 1, y + 1, 41, 9);
		}
		if (DDANGLE > 0.1) {
			u8g2.drawBox(x + 1, y + 10, 41, 10);
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, Batt_width, Batt_height, Batt_bits);
	u8g2.drawXBMP(x + 2, y + 11, Angle_width, Angle_height, Angle_bits);

	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(BAT, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 11 + (26 - L), y + H + 2, CharVal);

	dtostrf(ANGLE, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 11 + (26 - L), y + H * 2 + 5, CharVal);
	u8g2.drawXBMP(x + 11 + 26 + 1, y + H + 5, Cels_width, Cels_height, Cels_bits);
}

void draw_fuel(byte x, byte y) {
	char CharVal[6];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 2, y + 3, Fuel_width, Fuel_height, Fuel_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(FuelBurned, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 19 + (19 - L), y + H + 2, CharVal);

	dtostrf(FuelBurnedAll, 4, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 19 + (19 - L), y + H * 2 + 5, CharVal);
}

void draw_init(byte x, byte y) {
	analogWrite(BRIGHT_PIN, MIN_BRIGHT);
	u8g2.clearBuffer();
	u8g2.drawXBMP(x, y, Trollface_width, Trollface_height, Trollface_bits);
	u8g2.sendBuffer();

	for (byte i = MIN_BRIGHT; i <= min(Bright, MIN_BRIGHT + 45); i++) {
		delay(50);
		analogWrite(BRIGHT_PIN, i);
	}

	lcd_main();
	analogWrite(BRIGHT_PIN, Bright);
}

void draw_map(byte x, byte y) {
	float MAP = (float) build_int(3) * 0.015625;            // 64
	MAP = constrain(MAP, 0, 333);
	char CharVal[6];
	byte H;
	byte L;

	if (BoxState > 0) {
		if (MAP > OVERBOOST_LIMIT) {
			u8g2.drawBox(x + 1, y + 1, 46, 19);
		}
	}

	u8g2.drawXBMP(x + 2, y + 2, MAP2_width, MAP2_height, MAP2_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(MAP, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 41 - L, y + H + 5, CharVal);
}

void draw_speed(byte x, byte y, float SPD) {
	float DDANGLE = (float) build_int(13) * 0.03125;
	DDANGLE = constrain(DDANGLE, 0, 99);
	char CharVal[6];
	byte H;
	byte L;

	if (BoxState > 0) {
		if  (DDANGLE > 0.1) {
			u8g2.drawBox(x + 1, y + 1, 46, 19);
		}
	}

	u8g2.drawXBMP(x + 2, y + 2, SPD_width, SPD_height, SPD_bits);
	u8g2.setFont(u8g2_font_helvB12_tn);
	H = u8g2.getAscent();
	dtostrf(SPD, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(x + 43 - L, y + H + 5, CharVal);
}

void draw_rpm_tm1637() {
	int RPM = build_int(1);
	RPM = constrain(RPM, 0, 9999);
	RPM = trunc(RPM * 0.1) * 10;
	TM1637Display display(TM1637_CLK, TM1637_DIO);
	display.setBrightness(TM1637_BRIGHTNESS);
	display.showNumberDec(RPM, false);
}

void lcd_main() {
	// Очищаем память дисплея
	u8g2.clearBuffer();

	// ===Отрисовываем блоки===
	// Расход топлива
	draw_ff_fc(0, 0);
	// Температура
	draw_water_temp(0, 22);
	// Пробег
	draw_distance(0, 44);

	// УОЗ и давление
	draw_map_airtemp(44, 0);
	// Положение педали газа и РХХ
	draw_trottle(44, 22);
	// Средний расход топлива
	draw_afc(44, 44);

	// УДК
	draw_O2_sensor(86, 0);
	// Напряжение батареи
	draw_angle_batt(86, 22); 
	// Израсходованное топливо
	draw_fuel(86, 44);

	// Линии разметки
	u8g2.drawHLine(0, 21, 128);
	u8g2.drawHLine(0, 43, 128);
	u8g2.drawVLine(43, 0, 64);
	u8g2.drawVLine(85, 0, 64);

	// Рамка при появлении ошибок CE
	if (StatusCE > 0 && BoxState > 0) {
		u8g2.drawFrame(0, 0, 128, 64);
		StatusCE = 0;
	}
	
	// Отсылаем данные на дисплей
	u8g2.sendBuffer();
}

void lcd_acceleration() {
	float SPD = 0.0;
	float PrevSPD = 0.0;
	unsigned int A30 = 0;
	unsigned int A60 = 0;
	unsigned int A100 = 0;
	unsigned long Timer = millis();
	byte L = 0;
	char CharVal[6];

	// Состояния: 0 - не готов к замеру,
	//						1 - готов к замеру,
	//						2 - идет замер,
	//						3 - замер завершен,
	//						4 - замер не удался.
	byte Mode = 0;

	while (LCDMode == 1) {
		loop_2();

		// Сброс длительного нажатия кнопки
		if (ButtonState == 3) {
			ButtonState = 2;
		}

		// Выход из экрана замера разгона
		if (ButtonState == 1) {
			LCDMode = 2;
			ButtonState = 2;
		}

		if (DataOk) {
			SPD = build_speed(27 + V49_DATA_SHIFT);
			//if (Mode == 2) {SPD = min(SPD + 0.001, 120);}
			// Запуск измерения при скрости 0 в течение 2 сек.
			if (Mode == 0) {
				if (SPD > 0) {
					Timer = millis();
				}
				if (millis() - Timer >= 2000) {
					Mode = 1;
				}
			}
			if (Mode == 1) {
				if (SPD > 0) {
					Mode = 2;
					Timer = millis();
				}
			}

			if (Mode == 2) {
				if (millis() - Timer >= 65535) {
					Mode = 4;
				}
				else if (SPD >= 30 && PrevSPD < 30) {
					A30 = millis() - Timer;
				}
				else if (SPD >= 60 && PrevSPD < 60) {
					A60 = millis() - Timer;
				}
				else if (SPD >= 100 && PrevSPD < 100) {
					A100 = millis() - Timer;
					Mode = 3;
				}
			}

			if (millis() - LCDTimer >= LCD_UPDATE_DELAY) {
				LCDTimer = millis();
				ButtonTimer = min(255, ButtonTimer + 1);
				BoxState += 1;
				if (BoxState > 2) {BoxState = -1;}

				// Очищаем память дисплея
				u8g2.clearBuffer();

				u8g2.setFont(u8g2_font_courB18_tr);
				if (Mode == 0) {
					u8g2.setCursor(9, 32 + u8g2.getAscent() / 2);
					u8g2.print(F("STOP"));
				}
				else if (Mode == 1) {
					u8g2.setCursor(3, 32 + u8g2.getAscent() / 2);
					u8g2.print(F("READY"));
				}
				else if (Mode == 4) {
					u8g2.setCursor(4, 32 + u8g2.getAscent() / 2);
					u8g2.print(F("FAIL!"));
				}
				else if (Mode == 2 || Mode == 3) {
					if (A30 > 0) {
						u8g2.setFont(u8g2_font_helvB08_tn);
						u8g2.setCursor(2, 11 + u8g2.getAscent() / 2);
						u8g2.print(F("0-30:"));

						u8g2.setFont(u8g2_font_helvB10_tn);
						dtostrf(A30 / 1000.0, 5, 2, CharVal);
						L = u8g2.getUTF8Width(CharVal);
						u8g2.drawUTF8(75 - L, 11 + u8g2.getAscent() / 2, CharVal);
					}
					if (A60 > 0) {
						u8g2.setFont(u8g2_font_helvB08_tn);
						u8g2.setCursor(2, 33 + u8g2.getAscent() / 2);
						u8g2.print(F("0-60:"));

						u8g2.setFont(u8g2_font_helvB10_tn);
						dtostrf(A60 / 1000.0, 5, 2, CharVal);
						L = u8g2.getUTF8Width(CharVal);
						u8g2.drawUTF8(75 - L, 33 + u8g2.getAscent() / 2, CharVal);
					}
					if (A100 > 0) {
						u8g2.setFont(u8g2_font_helvB08_tn);
						u8g2.setCursor(2, 55 + u8g2.getAscent() / 2);
						u8g2.print(F("0-100:"));

						u8g2.setFont(u8g2_font_helvB10_tn);
						dtostrf(A100 / 1000.0, 5, 2, CharVal);
						L = u8g2.getUTF8Width(CharVal);
						u8g2.drawUTF8(75 - L, 55 + u8g2.getAscent() / 2, CharVal);
					}
				}
				// Скорость
				draw_speed(81, 0, SPD);
				// Давление
				draw_map(81, 22);
				// Температура
				draw_water_temp(82, 44, 7);

				// Линии разметки
				u8g2.drawHLine(80, 21, 48);
				u8g2.drawHLine(80, 43, 48);
				u8g2.drawVLine(80, 0, 64);
				
				// Отсылаем данные на дисплей
				u8g2.sendBuffer();
			}
			PrevSPD = SPD;
		}
	}
}

void check_ce_errors() {
	// Собираем все байты CE в один кусок
	uint32_t CE = 0;
	byte *pValue = (byte*)&CE;
	*pValue = Data[26];  
	*(pValue + 1) = Data[25];
	*(pValue + 2) = Data[24];
	*(pValue + 3) = Data[23];

	//StatusCE = 0;
	for (byte i = 0; i < CE_BITS_COUNT; i++ ) {
		// Проверяем изменение бита CE с 0 на 1
		if (bitRead(CE, i) == 1 && bitRead(PrevCE, i) == 0) {
			CountCE[i] = min(99, CountCE[i] + 1);
		}
		// Считаем количество ошибок CE в данный момент.
		if (i != 15 && bitRead(CE, i) == 1) {
			StatusCE = min(255, StatusCE + 1);
		}
	}
	PrevCE = CE;
}

void lcd_ce_errors() {
	#define ROWS_ON_SCREEN 5
	byte StartRow = 0;
	byte Row = 0;
	byte OverRow = 0;
	uint32_t CE = 0;
	char CharVal[4];
	char CEItemChar[25];
	byte H;
	byte L;

	while (LCDMode == 2) {
		loop_2();

		// Сброс ошибок при длительном нажатия кнопки
		if (ButtonState == 3) {
			for (byte i = 0; i < CE_BITS_COUNT; i++ ) {
				CountCE[i] = 0;
			}
			ButtonState = 2;
		}
		
		if (ButtonState == 1) {
			// Перелистывание ошибок
			if (OverRow > 0) {
				StartRow = OverRow;
				OverRow = 0;
			}
			// Выход из экрана ошибок CE
			else {
				LCDMode = 0;
			}
			ButtonState = 2;
		}

		if (millis() - LCDTimer >= LCD_UPDATE_DELAY) {
			LCDTimer = millis();
			ButtonTimer = min(255, ButtonTimer + 1);
		
			// Очищаем память дисплея
			u8g2.clearBuffer();

			//u8g2.setFont(u8g2_font_lucasfont_alternate_tr);
			u8g2.setFont(u8g2_font_haxrcorp4089_tr);
			//u8g2.setFont(u8g2_font_mozart_nbp_tr);
			H = u8g2.getAscent();

 			Row = 1;
			for (byte i = StartRow; i < CE_BITS_COUNT; i++ ) {
				if (CountCE[i] > 0) {
					dtostrf(CountCE[i], 3, 0, CharVal);
					L = u8g2.getUTF8Width(CharVal);
					// Выковыриваем строку текста из программной памяти
					strcpy_P(CEItemChar, (char*)pgm_read_word(&(CEItemsArray[i])));
					u8g2.drawUTF8(1, (Row - 1) * (H + 4) + H + 1, CEItemChar);
					u8g2.drawUTF8(127 - L, (Row - 1) * (H + 4) + H + 1, CharVal);
					Row += 1;

					// Если ошибок больше чем строк на экране
					if (Row > ROWS_ON_SCREEN) {
						OverRow = i + 1;
						u8g2.drawXBMP(64 - 5, 64 - 8, Next_width, Next_height, Next_bits);
						break;
					}
					OverRow = 0;
				}
			}
			// Если нет ни одной ошибки
			if (Row == 1) {
				u8g2.setCursor(38, 32 + 3);
				u8g2.print(F("NO ERRORS"));
			}

			// Отсылаем данные на дисплей
			u8g2.sendBuffer();
		}
	}
}

void setup() {
	// Включаем прерывание на пине 2
	pinMode(POWER_PIN, INPUT);
	attachInterrupt(0, write_eeprom, FALLING);

	// Настраиваем выводы для энкодера.
	pinMode(ENCODER_PIN_A, INPUT_PULLUP);
	pinMode(ENCODER_PIN_B, INPUT_PULLUP);
	pinMode(ENCODER_PIN_C, INPUT_PULLUP);

	// Считываем данные из EEPROM
	for (byte i = 0; i < 4; i++ ) {
		byte *pValue = (byte*)&Distance;
		*(pValue + i) = EEPROM.read(i); 
	}
	Distance = min(9999.9, Distance);

	for (byte i = 0; i < 4; i++ ) {
		byte *pValue = (byte*)&DistanceAll;
		*(pValue + i) = EEPROM.read(i + 4); 
	}
	DistanceAll = min(999999.0, DistanceAll);
	
	for (byte i = 0; i < 4; i++ ) {
		byte *pValue = (byte*)&FuelBurned;
		*(pValue + i) = EEPROM.read(i + 8); 
	}
	FuelBurned = min(999.0, FuelBurned);
	
	for (byte i = 0; i < 4; i++ ) {
		byte *pValue = (byte*)&FuelBurnedAll;
		*(pValue + i) = EEPROM.read(i + 12); 
	}
	FuelBurnedAll = min(9999.0, FuelBurnedAll);
	
	// Управление подсветкой.
	Bright = EEPROM.read(20);
	if (Bright < MIN_BRIGHT) {
		Bright = STD_BRIGHT;
	}
	pinMode(BRIGHT_PIN, OUTPUT);

	// Пин управления колокольчиком
	pinMode(SPEED_CHIME_PIN, OUTPUT);
	digitalWrite(SPEED_CHIME_PIN, LOW);

	Serial.begin(SERIAL_PORT_SPEED);

	// Старт дисплея
	u8g2.begin();
	// Прозрачный режим шрифтов
	u8g2.setFontMode(1);
	// Прозрачный режим изображений
	u8g2.setBitmapMode(1);
	// Режим XOR при отрисовке текста
	u8g2.setDrawColor(2);

	#ifdef SHOW_RPM_TM1637
		TM1637Display display(TM1637_CLK, TM1637_DIO);
		display.setBrightness(TM1637_BRIGHTNESS);
		display.showNumberDec(0, false);
	#endif
	
	//write_eeprom();
	draw_init(23, 0);
}

void loop_2() {
	button_update();
	encoder_update();
	receive_data();
	build_data();
	check_ce_errors();

	//DataOk = 1;
	//LCDMode = 1;
	if (abs(EncoderState) == 4) {
		Bright = max(MIN_BRIGHT, min(255, Bright + EncoderState / abs(EncoderState) * 2));
		analogWrite(BRIGHT_PIN, Bright);
		EncoderState = 0;
	}
}

void loop() {
	loop_2();

	// Длительное нажатие кнопки - сброс суточного пробега
	if (ButtonState == 3) {
		Distance = -1 * DIST;
		FuelBurned = 0.0;
		write_eeprom();
		ButtonState = 2;
	}
	
	// Короткое нажатие кнопки - вход в режим замера разгона
	if (ButtonState == 1) {
		LCDMode = 1;
		ButtonState = 2;
	}

	// Обновлять дисплей только при наличии данных
	if (DataOk) {
		speed_chime();

		if (millis() - LCDTimer >= LCD_UPDATE_DELAY) {
			LCDTimer = millis();

			ButtonTimer = min(255, ButtonTimer + 1);
			BoxState += 1;
			if (BoxState > 2) {BoxState = -1;}
			// Основной экран
			if (LCDMode == 0) {lcd_main();}
			else if (LCDMode == 1) {lcd_acceleration();}
			else if (LCDMode == 2) {lcd_ce_errors();}

			#ifdef SHOW_RPM_TM1637
				draw_rpm_tm1637();
			#endif
		}
	}
}
