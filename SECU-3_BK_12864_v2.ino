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
	2021-10-05 - Исправил еще один баг с переполнением буфера теперь уже для параметров, 
					которые хранятся в EEPROM. Во-первых, неверно указал длину строки для пробега
					и израсходованного топлива, во-вторых забыл, что на новой Ардуино в EEPROM
					могут быть не обязательно нули.
	==================================================================================================
		Переход на версию 2
	==================================================================================================
	2021-10-06 - Разбил все показания на экране на отдельные блоки. Сделал конфигуратор экрана в Excel.
	2021-10-07 - Отсортировал и поделил на группы переменные и функции.
			   	- Добавил второй экран.
			   	- Добавил блоки для отображения: 
					* обороты (F),
					* уровень топлива (F), 
					* EGT (F),
					* Давление масла (F).
	2021-10-08 - Вынес в отдельные файлы:
					* Название ошибок CE в CE_Errors.h,
					* Настроки в Configuration.h,
					* Глобальные переменные в Global_Variables.h,
					* Пиктограммы в Pictograms.h.
				- Наделал кучу новых багов и героически с ними сражался,
			   	- Сделал автоопределение размера пакета от SECU,
			   	- Добавил второй экран с данными.
			   	- Добавил блоки для отображения:
			   		* ДАД2, давление газа (F),
			   		* Дифф.давление, ДАД2-ДАД (F),
			   		* Темп. 2, температура газа (F),
			   		* Загрузка форсунок (F).
	2021-10-09 - Добавил блок номера расхода воздуха.
	2021-10-11 - Добавил в настройки шаг регулировки яркости (BRIGHT_STEP),
				- Добавил блоки для отображения:
	 				* Напряжение сети (F),
					* Напряжение УДК / AFR (F),
					* Температура воздуха на впуске (F),
					* Средний расход топлива суточный (F),
					* Средний расход топлива общий (F).
	2021-10-15 - Добавил анимацию включения для дисплея TM1637.
				- Добавил возможность выбора данных для отображения для TM1637:
					* Обороты,
					* EGT,
					* AFR,
					* Загрузка форсунок,
					* ДАД.
	2021-10-19 - Немного оптимизировал код.
				- Посчитал максимальный объем ОЗУ, используемый 
					локальными переменными - 77 байт.
	2021-10-21 - Добавил счетчик моточасов и блок для отображения.
				- Добавил линейный тахометр на экран замера разгона.
				- Немного поправил отрисовку некоторых блоков.
				- Исправил баг с зависанием дисплея TM1637 при переключении экранов.
	2021-11-04 - Добавил управление кнопками, энкодер остается как опция.
	2021-11-05 - Добавил два режима яркости в зависимости от состояния габаритов.
				- Добавил плавное изменение яркости.
				- Добавил блок отображение яркости при изменении значения.
				- Питание БК теперь постоянное с мониторингом состояния замка зажигания.
	2021-11-06 - Добавил управление подсветкой приборной панели.
					Яркость приборной панели регулируется на дополнительном экране.
	2021-11-08 - Добавил экран завершения.


	ПЛАН:
	 - Управление кнопками, 2021-11-04
	 - Шим подсветка приборки, 2021-11-06
	 - Постоянное питание БК, 2021-11-05
	 - Экран завершения, 2021-11-08
	 - Экран при выключенном зажигании,
	 - Дисплей на MAX7219.
*/

// Подключение бибилиотек
#include <U8g2lib.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <TM1637Display.h>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!! ВСЕ НАСТРОЙКИ В ФАЙЛЕ Configuration.h !!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//=============================================================================
//================================== Отладка ==================================
//=============================================================================
// Режим отладки. Проверка работы экранов без подключения к SECU.
// Раскомментировать для включения.
//#define DEBUG_MODE
// Номер экрана при запуске в режиме отладки
#define LCD_MODE_ON_START 0

//=============================================================================
//=============================== СБРОС EEPROM  ===============================
//=============================================================================
/* 
Перед первой прошивкой необходимо стереть EEPROM,
так как там может храниться всякая ересь.
Для этого можно предварительно залить пример для работы с EEPROM
"eeprom_clear" из Arduino IDE, после чего залить уже код БК.

    Или раскомментировать строку ниже, прошиться, 
закомментировать обратно и прошить заново.
*/

//#define EEPROM_CLEAR_ON_START

// Подключение дополнительных файлов.
#include "CE_Errors.h"
#include "Configuration.h"
#include "Global_Variables.h"
#include "Pictograms.h"

// Настройка LCD дисплея

// Обычный на чипе ST7920
//U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, SPI_CS_RS_PIN);
// OLED 1.3" на чипе SH1106 (stasoks)
//U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, SPI_CS_RS_PIN, SPI_DC_PIN, SPI_RS_PIN);
// OLED 2.42" на чипе SSD1309
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, SPI_CS_RS_PIN, SPI_DC_PIN);

//=============================================================================
//=================== Функциия отрисовки блоков данных ========================
//=============================================================================

void draw_ff_fc_f(byte x, byte y) { // 22
	float TPS = (float) Data[18] * 0.5;                     // 2
	float SPD = build_speed(27 + DataShift); // 11

	float FF_FRQ = (float) build_int(32 + DataShift) * 0.00390625;  // 256
	FF_FRQ = constrain(FF_FRQ, 0, 256);

	// Мгновенный расход л/ч
	FF_FRQ = (float) FF_FRQ * 0.225; // (3600 / 16000)
	FF_FRQ = constrain(FF_FRQ, 0, 99.9);

	// Режим отображения
	byte Mode = 1;

	// Мгновенный расход л/100км
	if (SPD > 3 && TPS > 3) {
		// Режим отображения №2 - движение 
		Mode = 2; // Drive
	}
  
	char CharVal[5];
	byte H;
	byte L;

	u8g2.setFont(u8g2_font_haxrcorp4089_tr);
	H = u8g2.getAscent();
	u8g2.drawUTF8(x + 2, y + H + 2, "F");

	if (Mode == 1) {
		dtostrf(FF_FRQ, 4, 1, CharVal);
		u8g2.drawUTF8(x + 2, y + H * 2 + 4, "F");
	}
	else if (Mode == 2) {
		FF_FRQ = (FF_FRQ * 100.0) / SPD;
		FF_FRQ = constrain(FF_FRQ, 0, 99.9);
		dtostrf(FF_FRQ, 4, 1, CharVal);
		u8g2.drawUTF8(x + 2, y + H * 2 + 4, "C");
	}

	u8g2.setFont(u8g2_font_helvB12_tn);
	H = u8g2.getAscent();
	L = u8g2.getUTF8Width(CharVal);
	if (CharVal[3] == '1') {L += 2;}

	// Позиция по X для строки на всю ячейку
	// X = 41 - 2 - L + x, где
	// 41 - крайняя правая координата в ячейке,
	// 2 - отступ справа,
	// L - длина строки,
	// x - крайняя левая координата в ячейке.

	// Позиция по Y для строки на всю ячейку
	// Y = y + 10 + H/2, где
	// y - крайняя верхняя координата в ячейке,
	// 10 - центральная координата в ячейке,
	// H/2 - половина высоты символа,

	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

void draw_water_temp_f(byte x, byte y) { // 12
	float TEMP = (float) build_int(7) * 0.25;
	//TEMP = DataSize;
	TEMP = constrain(TEMP, -99, 333);
	char CharVal[4];
	byte H;
	byte L;

	// Рисуем прямоугольник, при выходе параметра за допустимые пределы.
	if (AlarmBoxState > 0) {
		if (TEMP < WATER_TEMP_MIN || TEMP > WATER_TEMP_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}

	// Рисуем пиктограмму параметра.
	u8g2.drawXBMP(x + 2, y + 3, WaterTempL_width, WaterTempL_height, WaterTempL_bits);
	
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(TEMP, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 10 + H/2, CharVal);
	u8g2.drawXBMP(41 - 4 + x, y + 5, Cels_width, Cels_height, Cels_bits);
}

void draw_distance_f(byte x, byte y) { // 11
	char CharVal[7];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 1, y + 2, Road_width, Road_height, Road_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(Distance + DIST, 6, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 1 - L + x, y + H + 2, CharVal);

	dtostrf(DistanceAll + DIST, 6, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 1 - L + x, y + H * 2 + 5, CharVal);
}

void draw_map_h(byte x, byte y) { // 14
	float MAP = (float) build_int(3) * 0.015625;            // 64
	MAP = constrain(MAP, 0, 333);

	byte H;
	byte L;
	char CharVal[6];

	if (AlarmBoxState > 0) {
		if (MAP > OVERBOOST_LIMIT) {
			u8g2.drawBox(x + 1, y + 1, 40, 9);
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, MapS_width, MapS_height, MapS_bits);

	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(MAP, 5, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
}

void draw_map_f(byte x, byte y) { // 12
	float MAP = (float) build_int(3) * 0.015625;            // 64
	MAP = constrain(MAP, 0, 333);
	char CharVal[4];
	byte H;
	byte L;

	if (AlarmBoxState > 0) {
		if (MAP > OVERBOOST_LIMIT) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}

	u8g2.drawXBMP(x + 2, y + 2, MapL_width, MapL_height, MapL_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(MAP, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

void draw_map2_f(byte x, byte y) { // 12
	float MAP = (float) build_int(56 + DataShift) * 0.015625;            // 64
	MAP = constrain(MAP, 0, 333);
	char CharVal[4];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 2, y + 2, Map2L_width, Map2L_height, Map2L_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(MAP, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

void draw_map_diff_f(byte x, byte y) { // 13
	float MAP = (float) build_int(56 + DataShift) * 0.015625 - (float) build_int(3) * 0.015625;    
	MAP = constrain(MAP, -333, 333);
	char CharVal[5];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 2, y + 2, MapLDiff_width, MapLDiff_height, MapLDiff_bits);
	u8g2.setFont(u8g2_font_helvB10_tn);
	H = u8g2.getAscent();

	dtostrf(MAP, 4, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 1 - L + x, y + 10 + H/2, CharVal);
}

void draw_temp2_f(byte x, byte y) { // 12
	float Temp2 = (float) build_int(58 + DataShift) * 0.25;            // 4
	Temp2 = constrain(Temp2, -99, 333);
	char CharVal[4];
	byte H;
	byte L;
	u8g2.drawXBMP(x + 2, y + 3, Temp2_width, Temp2_height, Temp2_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(Temp2, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 10 + H/2, CharVal);
	u8g2.drawXBMP(41 - 4 + x, y + 5, Cels_width, Cels_height, Cels_bits);
}

void draw_airtemp_h(byte x, byte y) { // 14
	float AIRTEMP = (float) build_int(34 + DataShift) * 0.25;           // 4
	AIRTEMP = constrain(AIRTEMP, -99, 333);
	byte H;
	byte L;
	char CharVal[6];

	if (AlarmBoxState > 0) {
		if (AIRTEMP < AIR_TEMP_MIN || AIRTEMP > AIR_TEMP_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 9);
		}
	}

	u8g2.drawXBMP(x + 4, y + 1, AirTempS_width, AirTempS_height, AirTempS_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(AIRTEMP, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
	u8g2.drawXBMP(41 - 4 + x, y + 2, Cels_width, Cels_height, Cels_bits);
}

void draw_airtemp_f(byte x, byte y) { // 12
	float AIRTEMP = (float) build_int(34 + DataShift) * 0.25;           // 4
	AIRTEMP = constrain(AIRTEMP, -99, 333);
	byte H;
	byte L;
	char CharVal[4];
	if (AlarmBoxState > 0) {
		if (AIRTEMP < AIR_TEMP_MIN || AIRTEMP > AIR_TEMP_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}

	u8g2.drawXBMP(x + 2, y + 2, AirTempL_width, AirTempL_height, AirTempL_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(AIRTEMP, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 10 + H/2, CharVal);
	u8g2.drawXBMP(41 - 4 + x, y + 5, Cels_width, Cels_height, Cels_bits);
}

void draw_trottle_f(byte x, byte y) { // 18
	float TPS = (float) Data[18] * 0.5;                     // 2
	float IAC = (float) Data[25 + DataShift] * 0.5;                     // 2
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
	u8g2.drawUTF8(41 - 3 - L + x, y + 10 + H/2, CharVal);
}

void draw_afc_f(byte x, byte y) { // 17
	float AFC = 0.0;
	float TAFC = 0.0;
	char CharVal[5];
	byte H;
	byte L;

	// Средний расход суточный
	if (Distance + DIST > 1) {
		AFC = FuelBurned / (Distance + DIST) * 100;
		AFC = constrain(AFC, 0, 99.9);
	}
	// Средний расход общий
	if (DistanceAll + DIST > 1) {
		TAFC = FuelBurnedAll / (DistanceAll + DIST) * 100;
		TAFC = constrain(TAFC, 0, 99.9);
	}

	u8g2.drawXBMP(x + 3, y + 3, AFC_width, AFC_height, AFC_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(AFC, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 1 - L + x, y + H + 2, CharVal);

	dtostrf(TAFC, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 1 - L + x, y + H * 2 + 5, CharVal);
}

void draw_afc_daily_f(byte x, byte y) { // 13
	float AFC = 0.0;
	char CharVal[5];
	byte H;
	byte L;

	// Средний расход суточный
	if (Distance + DIST > 1) {
		AFC = FuelBurned / (Distance + DIST) * 100;
		AFC = constrain(AFC, 0, 99.9);
	}

	u8g2.drawXBMP(x + 1, y + 3, AFCS_width, AFCS_height, AFCS_bits);
	u8g2.setFont(u8g2_font_helvB10_tn);
	H = u8g2.getAscent();

	dtostrf(AFC, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 11 + H/2, CharVal);
}

void draw_afc_total_f(byte x, byte y) { // 13
	float TAFC = 0.0;
	char CharVal[5];
	byte H;
	byte L;

	// Средний расход общий
	if (DistanceAll + DIST > 1) {
		TAFC = FuelBurnedAll / (DistanceAll + DIST) * 100;
		TAFC = constrain(TAFC, 0, 99.9);
	}

	u8g2.drawXBMP(x + 1, y + 3, AFCS_width, AFCS_height, AFCS_bits);
	u8g2.setFont(u8g2_font_helvB10_tn);
	H = u8g2.getAscent();

	dtostrf(TAFC, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 11 + H/2, CharVal);
}

void draw_O2_sensor_h(byte x, byte y) { // 14
	// Тип датчика кислорода, 0 - УДК, 1 - ШДК.
	byte LambdaType;
	char CharVal[5];
	byte H;
	byte L;

	float AFR = (float) build_int(60 + DataShift) * 0.0078125;      // 128
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
		dtostrf(AFR, 4, 2, CharVal);
	}

	if (AlarmBoxState > 0) {
		if (LambdaType == 1) {
			if (AFR < LAMBDA_AFR_MIN || AFR > LAMBDA_AFR_MAX) {
				u8g2.drawBox(x + 1, y + 1, 40, 9);
			}
		}
		else {
			if (AFR < UDK_VOLT_MIN || AFR > UDK_VOLT_MAX) {
				u8g2.drawBox(x + 1, y + 1, 40, 9);
			}
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, O2S_width, O2S_height, O2S_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
}

void draw_O2_sensor_f(byte x, byte y) { // 14
	// Тип датчика кислорода, 0 - УДК, 1 - ШДК.
	byte LambdaType;
	char CharVal[5];
	byte H;
	byte L;

	float AFR = (float) build_int(60 + DataShift) * 0.0078125;      // 128
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
		dtostrf(AFR, 4, 2, CharVal);
	}

	if (AlarmBoxState > 0) {
		if (LambdaType == 1) {
			if (AFR < LAMBDA_AFR_MIN || AFR > LAMBDA_AFR_MAX) {
				u8g2.drawBox(x + 1, y + 1, 40, 19);
			}
		}
		else {
			if (AFR < UDK_VOLT_MIN || AFR > UDK_VOLT_MAX) {
				u8g2.drawBox(x + 1, y + 1, 40, 19);
			}
		}
	}

	u8g2.drawXBMP(x + 2, y + 2, O2L_width, O2L_height, O2L_bits);
	u8g2.setFont(u8g2_font_helvB10_tn);
	H = u8g2.getAscent();

	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 11 + H/2, CharVal);
}

void draw_lambda_corr_h(byte x, byte y) { // 14
	char CharVal[6];
	byte H;
	byte L;

	float AFR_CORR = (float) build_int(50 + DataShift) * 0.1953125; // 5.12
	AFR_CORR = constrain(AFR_CORR, -99.9, 99.9);
	if (AlarmBoxState > 0) {
		if (AFR_CORR < LAMBDA_CORR_MIN || AFR_CORR > LAMBDA_CORR_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 9);
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, LambdaCorr_width, LambdaCorr_height, LambdaCorr_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(AFR_CORR, 5, 1, CharVal);
	if (AFR_CORR > 0) {
		CharVal[0] = '+';
	}

	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
}

void draw_battery_h(byte x, byte y) { // 13
	float BAT = (float) build_int(5) * 0.0025;          // 400
	BAT = constrain(BAT, 0, 33);

	char CharVal[5];
	byte H;
	byte L;

	if (AlarmBoxState > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 9);
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, BatteryS_width, BatteryS_height, BatteryS_bits);

	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(BAT, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
}

void draw_battery_f(byte x, byte y) { // 13
	float BAT = (float) build_int(5) * 0.0025;          // 400
	BAT = constrain(BAT, 0, 33);

	char CharVal[5];
	byte H;
	byte L;
	if (AlarmBoxState > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}

	u8g2.drawXBMP(x + 2, y + 2, BatteryL_width, BatteryL_height, BatteryL_bits);

	u8g2.setFont(u8g2_font_helvB10_tn);
	H = u8g2.getAscent();

	dtostrf(BAT, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 11 + H/2, CharVal);
}

void draw_angle_h(byte x, byte y) { // 18
	float ANGLE = (float) build_int(9) * 0.03125;           // 32
	ANGLE = constrain(ANGLE, 0, 99);
	float DDANGLE = (float) build_int(13) * 0.03125;           // 32
	DDANGLE = constrain(DDANGLE, 0, 99);
	char CharVal[6];
	byte H;
	byte L;

	if (AlarmBoxState > 0) {
		if (DDANGLE > 0.1) {
			u8g2.drawBox(x + 1, y + 1, 40, 9);
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, Angle_width, Angle_height, Angle_bits);

	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(ANGLE, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
	u8g2.drawXBMP(41 - 4 + x, y + 2, Cels_width, Cels_height, Cels_bits);
}

void draw_fuel_burned_f(byte x, byte y) { // 10
	char CharVal[6];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 2, y + 3, FuelCanister_width, FuelCanister_height, FuelCanister_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(FuelBurned, 5, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 1 - L + x, y + H + 2, CharVal);

	dtostrf(FuelBurnedAll, 5, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 1 - L + x, y + H * 2 + 5, CharVal);
}

void draw_speed_f(byte x, byte y) { // 16
	float SPD =  build_speed(27 + DataShift);
	float DDANGLE = (float) build_int(13) * 0.03125;
	DDANGLE = constrain(DDANGLE, 0, 99);
	char CharVal[4];
	byte H;
	byte L;

	if (AlarmBoxState > 0) {
		if  (DDANGLE > 0.1) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}

	u8g2.drawXBMP(x + 2, y + 2, SPD_width, SPD_height, SPD_bits);
	u8g2.setFont(u8g2_font_helvB12_tn);
	H = u8g2.getAscent();
	dtostrf(SPD, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

void draw_rpm_f(byte x, byte y) { // 11
	int RPM = build_int(1);
	RPM = constrain(RPM, 0, 9999);
	RPM = trunc(RPM * 0.1) * 10;
	char CharVal[5];
	byte H;
	byte L;

	if (AlarmBoxState > 0) {
		if  (RPM > RPM_LIMIT) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}
	u8g2.setFont(u8g2_font_helvB12_tn);
	H = u8g2.getAscent();
	dtostrf(RPM, 4, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

void draw_fuel_tank_f(byte x, byte y) { // 12
	float FTLS = (float) build_int(75) * 0.015625;            // 64
	FTLS = constrain(FTLS, 0, 333);
	byte H;
	byte L;
	char CharVal[4];
	if (AlarmBoxState > 0) {
		if (FTLS < FUEL_TANK_MIN) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}

	u8g2.drawXBMP(x + 1, y + 3, FuelTank_width, FuelTank_height, FuelTank_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(FTLS, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 3 - L + x, y + 10 + H/2, CharVal);
}

void draw_egt_f(byte x, byte y) { // 13
	float EGT = (float) build_int(77) * 0.25;            // 4
	EGT = constrain(EGT, 0, 1999);
	EGT = trunc(EGT * 0.1) * 10;

	char CharVal[5];
	byte H;
	byte L;

	if (AlarmBoxState > 0) {
		if  (EGT > EGT_LIMIT) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(EGT, 4, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 6 - L + x, y + 10 + H/2, CharVal);
	u8g2.drawXBMP(41 - 5 + x, y + 5, Cels2_width, Cels2_height, Cels2_bits);
}

void draw_oil_pressure_f(byte x, byte y) { // 12
	float OPS = (float) build_int(79) * 0.015625;            // 64
	OPS = constrain(OPS, 0, 9.9);
	byte H;
	byte L;
	char CharVal[4];
	if (AlarmBoxState > 0) {
		if (OPS < OIL_PRESSURE_MIN || OPS > OIL_PRESSURE_MAX ) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}

	u8g2.drawXBMP(x + 2, y + 2, OilPressure_width, OilPressure_height, OilPressure_bits);
	u8g2.setFont(u8g2_font_helvB12_tn);
	H = u8g2.getAscent();

	dtostrf(OPS, 3, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

void draw_inj_duty_f(byte x, byte y) { // 12
	float InjDuty = Data[81] * 0.5; // 2
	InjDuty = constrain(InjDuty, 0, 100);
	char CharVal[4];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 2, y + 3, InjDuty_width, InjDuty_height, InjDuty_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(InjDuty, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

void draw_air_map_index_f(byte x, byte y) { // 8
	byte MapIndex = Data[15];
	MapIndex = constrain(MapIndex, 0, 16);
	char CharVal[3];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 2, y + 4, AirMapIndex_width, AirMapIndex_height, AirMapIndex_bits);
	u8g2.setFont(u8g2_font_helvB12_tn);
	H = u8g2.getAscent();

	dtostrf(MapIndex, 2, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 3 - L + x, y + 10 + H/2, CharVal);
}

void draw_engine_hours_f(byte x, byte y) { // 13
	float Hours = (float) EngineHours / 3600;
	char CharVal[7];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 2, y + 2, PistonL_width, PistonL_height, PistonL_bits);
	u8g2.setFont(u8g2_font_helvB08_tn);
	H = u8g2.getAscent();

	dtostrf(Hours, 6, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

// Обороты на дополнительном сегментном дисплее
#ifdef TM1637_ENABLE
void draw_tm1637() { // 3 
	byte DataType = TM1637_DATA_TYPE;
	if (DataType == 0) {
		int Value = build_int(1); // 0 - Обороты
		Value = constrain(Value, 0, 9999);
		Value = trunc(Value * 0.1) * 10;
		Display7S.showNumberDec(Value, false);
	}
	else if (DataType == 1) {
		int Value = build_int(77) * 0.25; // 1 - EGT
		Value = constrain(Value, 0, 1999);
		Value = trunc(Value * 0.1) * 10;
		Display7S.showNumberDec(Value, false);
	}
	else if (DataType == 2) {
		int Value = build_int(60 + DataShift) * 0.078125; // 2 - AFR
		Value = constrain(Value, 0, 33);
		Display7S.showNumberDecEx(Value, 0x80 >> 2, false);
	}
	else if (DataType == 3) {
		int Value = Data[81] * 0.5; // 3 - Загрузка форсунок
		Value = constrain(Value, 0, 100);
		Display7S.showNumberDec(Value, false);
	}
	else if (DataType == 4) {
		int Value = (float) build_int(3) * 0.015625; // 4 - ДАД
		Value = constrain(Value, 0, 333);
		Display7S.showNumberDec(Value, false);
	}
	else if (DataType == 5) {
		int Value =  build_speed(27 + DataShift); // 5 - Скорость
		Display7S.showNumberDec(Value, false);
	}
}
#endif

//=============================================================================
//===================== Функциия отрисовки экранов ============================
//=============================================================================
// Экран приветствия
void draw_init() { // 20
	#define STEP_DELAY_TM1637 100
	#define STEP_DELAY_LCD 35
	
	//ШИМ подсветка приборной панели на минимум
	#ifdef PWM_BRIGHT_PIN
		analogWrite(PWM_BRIGHT_PIN, PWM_BRIGHT_MIN);
	#endif
	
	// Очищаем LCD дисплей
	u8g2.clear();
	#ifdef LCD_BRIGHT_PIN
		analogWrite(LCD_BRIGHT_PIN, LCD_BRIGHT_MIN);
	#else
		u8g2.setContrast(0);
	#endif


	#ifdef TM1637_ENABLE
		Display7S.setBrightness(0, false);
		Display7S.clear();
		// Таймер для TM1637 дисплея
		unsigned long InitSegmentTimer = millis();
		byte TM1637BrightBuffer = 0;
		byte Data[4];
		byte Segment[] = {0b00000001,
							0b00000011,
							0b00000111,
							0b00001111,
							0b00011110,
							0b00111100,
							0b00111001,
							0b00110001,
							0b00100001,
							0b00000001,
							0b00000000};
	#endif

	// Таймер для LCD дисплея
	unsigned long InitLCDTimer = millis();
	// Шаг изменения подсветки экрана
	byte BrightStepLCD = (BrightLCD[BrightMode] - LCD_BRIGHT_MIN) / 31;
	// Шаг изменения подсветки приборной панели
	byte BrightStepPWM = (BrightPWM[BrightMode] - PWM_BRIGHT_MIN) / 31;

	byte State = 0;
	byte i = 0;
	byte j = 0;

	while (1) {
		if (millis() - InitLCDTimer >= STEP_DELAY_LCD) {
			if (j < 32) {

				#ifdef PWM_BRIGHT_PIN
					analogWrite(PWM_BRIGHT_PIN, PWM_BRIGHT_MIN + j * BrightStepPWM);
				#endif
				
				#ifdef LCD_BRIGHT_PIN
					analogWrite(LCD_BRIGHT_PIN, LCD_BRIGHT_MIN + j * BrightStepLCD);
				#else
					u8g2.setContrast(j * BrightStepLCD);
				#endif

				u8g2.clearBuffer();
				u8g2.setClipWindow(0, 31 - j, 127, 32 + j);
				// Заставка
				u8g2.drawXBMP(23, 0, Trollface_width, Trollface_height, Trollface_bits);
				u8g2.sendBuffer();
				j++;
			}
			#ifndef TM1637_ENABLE
				else {
					break;
				}
			#endif
			InitLCDTimer = millis();
		}
		#ifdef TM1637_ENABLE
			if (millis() - InitSegmentTimer >= STEP_DELAY_TM1637) {
				TM1637BrightBuffer = min(TM1637Bright, TM1637BrightBuffer += 4);
				byte Buffer = TM1637BrightBuffer / TM1637_BRIGHT_STEP;
				Display7S.setBrightness(Buffer);
				if (State == 0) {
					for (byte k = 0; k < 4; k++) {
						Data[k] = Segment[i];
					}
					Display7S.setSegments(Data);
					i++;
					if (i >= 10) {
						State = 1;
						i = 0;
					}
				}
				else if (State == 1) {
					Data[i] = 0b00000000;
					Display7S.setSegments(Data);
					i++;
					if (i >= 3) {
						State = 2;
						i = 0;
					}
				}
				else if (State == 2) {
					Data[3] = Data[3] << 1;
					Data[3] += 1;
					Display7S.setSegments(Data);
					i++;
					if (i >= 5) {
						byte Buffer = TM1637Bright / TM1637_BRIGHT_STEP;
						Display7S.setBrightness(Buffer);
						break;
					}
				}
				InitSegmentTimer = millis();
			}
		#endif
	}
	#ifdef LCD_BRIGHT_PIN
		analogWrite(LCD_BRIGHT_PIN, BrightLCD[BrightMode]);
	#else
		u8g2.setContrast(BrightLCD[BrightMode]);
	#endif
	#ifdef PWM_BRIGHT_PIN
		analogWrite(PWM_BRIGHT_PIN, BrightPWM[BrightMode]);
	#endif
				
	lcd_main();
}

void draw_finish() {
	#define ROW_SPACE 7
	#define ROW_SHIFT 5
	//RideTimer = 600001;
	//DIST = 12.15;
	//FuelBurnedRide = 2.2;

	char CharVal[6];
	byte Row;
	byte H;
	byte L;
	u8g2.setFont(u8g2_font_helvB08_tr);
	H = u8g2.getAscent();

	u8g2.clearBuffer();
	// Строка 1 - Время поездки в минутах
	Row = 1;
	float TimeM = (float) RideTimer / 60000;
	dtostrf(TimeM, 4, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.setCursor(1, (Row - 1) * (H + ROW_SPACE) + H + ROW_SHIFT);
	u8g2.print(F("Travel Time"));
	u8g2.drawUTF8(120 - L, (Row - 1) * (H + ROW_SPACE) + H + ROW_SHIFT, CharVal);

	// Строка 2 - Пройденное расстояние
	Row = 2;
	dtostrf(DIST, 5, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.setCursor(1, (Row - 1) * (H + ROW_SPACE) + H + ROW_SHIFT);
	u8g2.print(F("Distance Traveled"));
	u8g2.drawUTF8(120 - L, (Row - 1) * (H + ROW_SPACE) + H + ROW_SHIFT, CharVal);

	// Строка 3 - Израсходовано топлива
	Row = 3;
	dtostrf(FuelBurnedRide, 5, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.setCursor(1, (Row - 1) * (H + ROW_SPACE) + H + ROW_SHIFT);
	u8g2.print(F("Fuel Consumed"));
	u8g2.drawUTF8(120 - L, (Row - 1) * (H + ROW_SPACE) + H + ROW_SHIFT, CharVal);

	// Строка 4 - Средняя скорость
	Row = 4;
	float AVGSpeed = (float) DIST * (60 / TimeM);
	dtostrf(AVGSpeed, 5, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.setCursor(1, (Row - 1) * (H + ROW_SPACE) + H + ROW_SHIFT);
	u8g2.print(F("Average Speed"));
	u8g2.drawUTF8(120 - L, (Row - 1) * (H + ROW_SPACE) + H + ROW_SHIFT, CharVal);

	u8g2.sendBuffer();
}

// Основной экран
void lcd_main() { // 0
	// Очищаем память дисплея
	u8g2.clearBuffer();

	// ========================== Блоки данных ==========================
    draw_ff_fc_f(0, 0);    // Мгновенный расход топлива (F)	
	
    draw_water_temp_f(0, 22);    // Температура ОЖ (F)	
	
    draw_distance_f(0, 44);    // Суточный и общий пробег (F)	
	
    draw_map_f(43, 0);    // ДАД (F)	
	
    draw_trottle_f(43, 22);    // ДПДЗ / РХХ (F)	
	
    draw_afc_f(43, 44);    // Средний расход топлива суточный и общий (F)	
	
    draw_O2_sensor_h(86, 0);    // Напряжение УДК / AFR (H)	
    draw_lambda_corr_h(86, 10);    // Лямбда коррекция (H)	
    draw_airtemp_h(86, 22);    // Температура воздуха на впуске (H)	
    draw_battery_h(86, 32);    // Напряжение сети (H)	
    draw_fuel_burned_f(86, 44);    // Израсходованное топливо (F)	
	// ========================== Блоки данных ==========================


	// Линии разметки
	u8g2.drawHLine(0, 21, 128);
	u8g2.drawHLine(0, 43, 128);
	u8g2.drawVLine(42, 0, 64);
	u8g2.drawVLine(85, 0, 64);

	// Рамка при появлении ошибок CE
	if (StatusCE > 0 && AlarmBoxState > 0) {
		u8g2.drawFrame(0, 0, 128, 64);
		StatusCE = 0;
	}
	
	// Блок параметра яркости
	draw_bright();

	// Отсылаем данные на дисплей
	u8g2.sendBuffer();
}

// Второй экран
void lcd_second() { // 0 
	// Очищаем память дисплея
	u8g2.clearBuffer();

	// ========================== Блоки данных ==========================
    draw_rpm_f(0, 0);    // Обороты (F)	
	
    draw_speed_f(0, 22);    // Скорость (F)	
	
    draw_engine_hours_f(0, 44);    // Счетчик моточасов (F)	
	
    draw_map_f(43, 0);    // ДАД (F)	
	
    draw_trottle_f(43, 22);    // ДПДЗ / РХХ (F)	
	
    draw_water_temp_f(43, 44);    // Температура ОЖ (F)	
	
    draw_O2_sensor_f(86, 0);    // Напряжение УДК / AFR (F)	
	
    draw_inj_duty_f(86, 22);    // Загрузка форсунок (F)	
	
    draw_air_map_index_f(86, 44);    // Номер расхода воздуха (F)	

	// ========================== Блоки данных ==========================

	// Линии разметки
	u8g2.drawHLine(0, 21, 128);
	u8g2.drawHLine(0, 43, 128);
	u8g2.drawVLine(42, 0, 64);
	u8g2.drawVLine(85, 0, 64);

	// Рамка при появлении ошибок CE
	if (StatusCE > 0 && AlarmBoxState > 0) {
		u8g2.drawFrame(0, 0, 128, 64);
		StatusCE = 0;
	}

	// Блок параметра яркости
	draw_bright();

	// Отсылаем данные на дисплей
	u8g2.sendBuffer();
}

// Экран замера разгона
void lcd_acceleration() { // 29
	int RPM = 0;
	byte BoxHeight = 0;
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

	while (LCDMode == 2) {
		loop_2();
		if (DataOk) {
			SPD = build_speed(27 + DataShift);
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

			if (millis() - LCDTimer >= LCD_ACC_UPDATE_DELAY) {
				LCDTimer = millis();

				AlarmBoxState += 1;
				if (AlarmBoxState > ALARM_BOX_TIMER) {AlarmBoxState = 1 - ALARM_BOX_TIMER;}

				// Очищаем память дисплея
				u8g2.clearBuffer();

				u8g2.setFont(u8g2_font_courB14_tr);
				if (Mode == 0) {
					u8g2.setCursor(9, 32 + u8g2.getAscent() / 2);
					u8g2.print(F("STOP"));
				}
				else if (Mode == 1) {
					u8g2.setCursor(5, 32 + u8g2.getAscent() / 2);
					u8g2.print(F("READY"));
				}
				else if (Mode == 4) {
					u8g2.setCursor(4, 32 + u8g2.getAscent() / 2);
					u8g2.print(F("FAIL!"));
				}
				else if (Mode == 2 || Mode == 3) {
					if (A30 > 0) {
						u8g2.setFont(u8g2_font_helvB08_tn);
						u8g2.setCursor(1, 11 + u8g2.getAscent() / 2);
						u8g2.print(F("30"));

						u8g2.setFont(u8g2_font_helvB10_tn);
						dtostrf(A30 / 1000.0, 5, 2, CharVal);
						L = u8g2.getUTF8Width(CharVal);
						u8g2.drawUTF8(61 - L, 11 + u8g2.getAscent() / 2, CharVal);
					}
					if (A60 > 0) {
						u8g2.setFont(u8g2_font_helvB08_tn);
						u8g2.setCursor(1, 33 + u8g2.getAscent() / 2);
						u8g2.print(F("60"));

						u8g2.setFont(u8g2_font_helvB10_tn);
						dtostrf(A60 / 1000.0, 5, 2, CharVal);
						L = u8g2.getUTF8Width(CharVal);
						u8g2.drawUTF8(61 - L, 33 + u8g2.getAscent() / 2, CharVal);
					}
					if (A100 > 0) {
						u8g2.setFont(u8g2_font_helvB08_tn);
						u8g2.setCursor(1, 55 + u8g2.getAscent() / 2);
						u8g2.print(F("100"));

						u8g2.setFont(u8g2_font_helvB10_tn);
						dtostrf(A100 / 1000.0, 5, 2, CharVal);
						L = u8g2.getUTF8Width(CharVal);
						u8g2.drawUTF8(61 - L, 55 + u8g2.getAscent() / 2, CharVal);
					}
				}

				// ========================== Блоки данных ==========================
			    draw_speed_f(86, 0);    // Скорость (F)	
				
			    draw_map_f(86, 22);    // ДАД (F)	
				
			    draw_water_temp_f(86, 44);    // Температура ОЖ (F)	
				// ========================== Блоки данных ==========================

				// Линии разметки
				u8g2.drawHLine(86, 21, 42);
				u8g2.drawHLine(86, 43, 42);
				u8g2.drawVLine(85, 0, 64);

				// Тахометр
				u8g2.setFont(u8g2_font_haxrcorp4089_tn);
				u8g2.drawHLine(73, 2, 3); // 7000

				u8g2.setCursor(77, 11);
				u8g2.print(F("6"));
				u8g2.drawHLine(73, 12, 9); // 6000
				u8g2.drawHLine(73, 22, 3); // 5000

				u8g2.setCursor(77, 31);
				u8g2.print(F("4"));
				u8g2.drawHLine(73, 32, 9); // 4000
				u8g2.drawHLine(73, 42, 3); // 3000

				u8g2.setCursor(77, 51);
				u8g2.print(F("2"));
				u8g2.drawHLine(73, 52, 9); // 2000
				u8g2.drawHLine(73, 61, 3); // 1000

				u8g2.drawFrame(66, 0, 7, 64);

				RPM = build_int(1);
				//RPM = 1003;
				if (RPM > 900) {
					RPM = constrain(RPM, 900, 7100);
					BoxHeight = map(RPM, 900, 7100, 62, 1);
					u8g2.drawBox(67, BoxHeight, 5, 63 - BoxHeight);
				}

				// Блок параметра яркости
				draw_bright();

				// Отсылаем данные на дисплей
				u8g2.sendBuffer();

				#ifdef TM1637_ENABLE
					draw_tm1637();
				#endif
			}
			PrevSPD = SPD;
		}
	}
}

// Экран ошибок CE
void lcd_ce_errors() { // 36
	byte BitsCount = 16;
	if (DataSize >= 74) {BitsCount = 21;}

	#define ROWS_ON_SCREEN 5
	byte StartRow = 0;
	byte Row = 0;
	byte OverRow = 0;
	//uint32_t CE = 0;
	char CharVal[4];
	char CEItemChar[25];
	byte H;
	byte L;

	while (LCDMode == 3) {
		loop_2();

		// Перелистывание ошибок
		if (ButtonState[0] == 32) {
			StartRow = max(0, StartRow - 1);
			ButtonState[0] = 128;
		}
		if (ButtonState[1] == 32) {
			if (OverRow > 0) {
				StartRow += 1;
			}
			ButtonState[1] = 128;
		}

		if (millis() - LCDTimer >= LCD_UPDATE_DELAY) {
			LCDTimer = millis();
		
			// Очищаем память дисплея
			u8g2.clearBuffer();

			//u8g2.setFont(u8g2_font_lucasfont_alternate_tr);
			u8g2.setFont(u8g2_font_haxrcorp4089_tr);
			//u8g2.setFont(u8g2_font_mozart_nbp_tr);
			H = u8g2.getAscent();

 			Row = 1;
			for (byte i = StartRow; i < BitsCount; i++ ) {
				if (CountCE[i] > 0) {
					dtostrf(CountCE[i], 3, 0, CharVal);
					L = u8g2.getUTF8Width(CharVal);
					// Выковыриваем строку текста из программной памяти
					strcpy_P(CEItemChar, (char*)pgm_read_word(&(CEItemsArray[i])));
					u8g2.drawUTF8(1, (Row - 1) * (H + 4) + H + 1, CEItemChar);
					u8g2.drawUTF8(125 - L, (Row - 1) * (H + 4) + H + 1, CharVal);
					Row += 1;

					// Если ошибок больше чем строк на экране
					if (Row > ROWS_ON_SCREEN && i + 1 < BitsCount) {
						OverRow = i + 1;
						u8g2.drawXBMP(56 - 5, 64 - 8, Next_width, Next_height, Next_bits);
						break;
					}
					OverRow = 0;
				}
			}
			if (Row > 1 && StartRow > 0) {
				u8g2.drawXBMP(72 - 5, 64 - 8, Prev_width, Prev_height, Prev_bits);
			}

			// Если нет ни одной ошибки
			if (Row == 1) {
				u8g2.setCursor(38, 32 + 3);
				u8g2.print(F("NO ERRORS"));
			}

			// Отсылаем данные на дисплей
			u8g2.sendBuffer();

			#ifdef TM1637_ENABLE
				draw_tm1637();
			#endif
		}
	}
}

//=============================================================================
//============================== Основные функции  ============================
//=============================================================================

// Чтение EEPROM
void read_eeprom() { // 1
	#ifdef DEBUG_MODE
		Serial.println(F("Read EEPROM"));
	#endif	

	// Пробег суточный
	EEPROM.get(0, Distance); // 0-3
	Distance = min(9999.9, Distance);
	//	Пробег общий
	EEPROM.get(4, DistanceAll); // 4-7
	DistanceAll = min(999999.0, DistanceAll);
	// Топливо суточный	
	EEPROM.get(8, FuelBurned); // 8-11
	FuelBurned = min(999.0, FuelBurned);
	// Топливо всего
	EEPROM.get(12, FuelBurnedAll); // 12-15
	FuelBurnedAll = min(9999.0, FuelBurnedAll);
	// Моточасы
	EEPROM.get(16, EngineHours); // 16-19
	EngineHours = min(35996400, EngineHours); // 9999 * 3600

	// Подсветка OLED
	BrightLCD[0] = EEPROM.read(20);
	BrightLCD[0] = max(LCD_BRIGHT_MIN, BrightLCD[0]);
	BrightLCD[1] = EEPROM.read(21);
	BrightLCD[1] = max(LCD_BRIGHT_MIN, BrightLCD[1]);

	// Подсветка ШИМ
	BrightPWM[0] = EEPROM.read(22);
	BrightPWM[0] = max(PWM_BRIGHT_MIN, BrightPWM[0]);
	BrightPWM[1] = EEPROM.read(23);
	BrightPWM[1] = max(PWM_BRIGHT_MIN, BrightPWM[1]);
}

// Запись EEPROM
void write_eeprom() { // 9
	#ifdef DEBUG_MODE
		Serial.println(F("Write EEPROM"));
	#endif

	float Dst = Distance + DIST;
	float DstAll = DistanceAll + DIST;

	//Dst = 60;
	//DstAll = 6333;
	//FuelBurned = 6.2;
	//FuelBurnedAll = 500;

	// Пробег суточный
	EEPROM.put(0, Dst); // 0-3
	//	Пробег общий
	EEPROM.put(4, DstAll); // 4-7
	// Топливо суточный
	EEPROM.put(8, FuelBurned); // 8-11
	// Топливо всего
	EEPROM.put(12, FuelBurnedAll); // 12-15
	// Моточасы
	EEPROM.put(16, EngineHours); // 16-19

	// Подсветка OLED
	EEPROM.write(20, BrightLCD[0]);
	EEPROM.write(21, BrightLCD[1]);
	// Подсветкой ШИМ
	EEPROM.write(22, BrightPWM[0]);
	EEPROM.write(23, BrightPWM[1]);
}

// Чтение буфера данных от SECU
byte read_buff(byte b1) { // 2
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

// Определение размера пакета данных
void get_data_size() { // 4
	byte IncomingByte = 0;
	byte Receive = 0;
	byte N = 0;
	byte Count = 0;

	// Должно быть 5 пакетов одинаковой длины подряд
	while (Count < 5) {
		// Ждем символ начала пакета.
		if (Receive == 0) {
			if (Serial.available() > 1) {
				if (Serial.read() == FOBEGIN) {
					if (Serial.read() == 0x71) {
						Receive = 1;
						N = 0;
					}
				}
			}
		}
		// Получаем и считаем байты
		else  {
			if (Serial.available() > 1) {
				IncomingByte = Serial.read();
				// Если получили завершающий байт.
				if (IncomingByte == FIOEND) {
					// Проверяем получившуюся длину пакета
					if (DataSize == N) {
						// Длина совпадает с предыдущим пакетом, счетчик +1
						Count += 1;
					}
					else {
						// Длина совпадает, сброс счетчика и по новой.
						Count = 0;
						DataSize = N;
					}
					Receive = 0;
				}
				else {
					// Считаем кол-во байт с учетом символа подмены байта
					if (IncomingByte == FESC) {Serial.read();}
					N += 1;
					// Если размер пакета превысил предел, сброс счетчика.
					if (N > MAX_DATA_SIZE) {Count = 0;}
				}
			}
		}
	}
	if (DataSize >= 74) {DataShift = 2;}
}

// Прием данных от SECU в буфер 
void receive_data() { // 7
	byte IncomingByte = 0;
	unsigned long Timer = millis();
	byte Receive = 0;
	byte N = 0;
	DataOk = 0;

	// Ждем символ начала пакета.
	while (millis() - Timer < 250) {
		if (Serial.available() > 1) {
			if (Serial.read() == FOBEGIN) {
				// Проверяем, что это нужный нам пакет с данными 0x71
				if (Serial.read() == 0x71) {
					Receive = 1;
					break;
				}
			}
		}
	}
	if (Receive) {
		Timer = millis();
		while (millis() - Timer < 250) {
			if (Serial.available() > 1) {
				IncomingByte = Serial.read();
				if (IncomingByte == FIOEND) {
					if (N == DataSize) {
						DataOk = 1;
					}
					break;
				}
				else {
					// Записываем байт в буфер, начиная с 1, а не нуля.
					N += 1;
					if (N > DataSize) {break;}
					Data[N] = read_buff(IncomingByte);
				}
			}
		}
	}
}

// Сборка int из двух байт
int build_int(byte i) { // 4
	int Value = 0;
	byte *pValue = (byte*)&Value;
	*pValue = Data[i + 1];  
	*(pValue + 1) = Data[i];
	return Value;
}

// Расчет скорости ТС
float build_speed(byte i) { // 11
	// Коэффициент для вычисления скорости и дистанции
	#define M_PERIOD_DISTANCE 1000.0 / SPEED_SENSOR_COUNT

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
float build_distance(byte i) { // 9
	unsigned long Value = 0;
	byte* pValue = (byte*)&Value;
	*pValue = Data[i + 2]; 
	*(pValue + 1) = Data[i + 1];
	*(pValue + 2) = Data[i];

	float Dist = (float) (M_PERIOD_DISTANCE * Value) * 0.001;
	return Dist;
}

// Сборка некоторых данных из байтов от SECU
void build_data() { // 12
	if (DataOk) {
		if (FuelTimer == 0) {FuelTimer = millis();}
		else if (millis() - FuelTimer >= 500) {
			DIST = build_distance(29 + DataShift);
			// Расчет израсходованного топлива за интервал
			float FF_FRQ = (float) build_int(32 + DataShift) * 0.00390625;  // 256
			FF_FRQ = constrain(FF_FRQ, 0, 256);

			float FFAVG = (float) (PrevFF_FRQ + FF_FRQ) * 0.1125; // Расход л/ч (3600 / 16000) / 2
			float Fuel = (float) FFAVG * ((millis() - FuelTimer) / 3600000);

			FuelBurnedRide += Fuel;
			FuelBurned += Fuel;
			FuelBurnedAll += Fuel;

			FuelTimer = millis();
			PrevFF_FRQ = FF_FRQ;
		}
		// Подсчет моточасов
		if (EngineTimer == 0) {EngineTimer = millis();}
		else if (millis() - EngineTimer >= 999) {
			int RPM = build_int(1);
			RPM = 501;
			if (RPM > 500) {
				EngineHours += 1;
				RideTimer += 1;
			}
			EngineTimer = millis();
		}
	}
}

// Разбор блока ошибок CE
void check_ce_errors() { // 5
	byte BitsCount = 16;
	if (DataSize >= 74) {BitsCount = MAX_CE_BITS_COUNT;}

	// Собираем все байты CE в один кусок
	uint32_t CE = 0;
	byte *pValue = (byte*)&CE;
	*pValue = Data[26];  
	*(pValue + 1) = Data[25];
	*(pValue + 2) = Data[24];
	*(pValue + 3) = Data[23];

	//StatusCE = 0;
	for (byte i = 0; i < BitsCount; i++ ) {
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

// Проверка состояние кнопки
void button_update(byte N, byte Pin) { // 0
	// Каждые 250 мс при нажатой кнопке добавляется 1 к статусу
	if (!digitalRead(Pin)) {
		if (ButtonState[N] == 0) {
			ButtonState[N] = 1;
			ButtonTimer = millis();
		}
		if (millis() - ButtonTimer >= 250) {
			ButtonState[N] = min(16, ButtonState[N] + 1);
			ButtonTimer = millis();
		}
	}
	else {
		if (ButtonState[N] >= 128) {
			if (millis() - ButtonTimer >= 250) {
				ButtonState[N] = 0;
				ButtonTimer = millis();
			}
		}	
		// 8 циклов = 2 секунды = долгое нажатие
		else if (ButtonState[N] >= 8) {
			ButtonState[N] = 64;
		}
		else if (ButtonState[N] >= 1) {
			ButtonState[N] = 32;
		}
		else {
			ButtonState[N] = 0;
		}
	}


	//#ifdef DEBUG_MODE
	//	Serial.print(F("Button "));
	//	Serial.print(N);
	//	Serial.print(F(" - "));
	//	Serial.println(ButtonState[N]);
	//#endif

	// ButtonState 0 - выкл,
	//				32 - короткое включение,
	//				64 - длинное включение
	//				128 - действие выполнено
}

// Проверка состояние энкодера
void encoder_update() { // 2
	boolean PinState[2];
	PinState[0] = digitalRead(BUTTON_UP_PIN);
	PinState[1] = digitalRead(BUTTON_DOWN_PIN);

	// Начальная позиция 11.
	if (PinState[0] && PinState[1]) {
		// Если переход на начальную позицию после шага 3,
		// то выполняем команду.
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

// Колокольчик AE86
void speed_chime() { // 4
	float SPD = 0.0;
	SPD = build_speed(27 + DataShift);

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

// Прерывание на включение замка зажигания
void power_on() {
	#ifdef DEBUG_MODE
		Serial.println(F("Power On"));
	#endif

	PowerState = 1;
	RideTimer = 0;
	FuelBurnedRide = 0.0;

	// Установка яркости
	BrightLCD[2] = BrightLCD[BrightMode];
	BrightPWM[2] = BrightPWM[BrightMode];

	if (BrightMode == 1) {TM1637Bright = TM1637_BRIGHT_NIGHT * TM1637_BRIGHT_STEP;}
	else {TM1637Bright = TM1637_BRIGHT_DAY * TM1637_BRIGHT_STEP;}

	// Отображение экрана приветствия
	draw_init();
}

// Прерывание на выключение замка зажигания
void power_off() {
	#ifdef DEBUG_MODE
		Serial.println(F("Power Off"));
	#endif

	PowerState = 0;
	write_eeprom();
	LCDMode = 0;

	#ifdef TM1637_ENABLE
		Display7S.setBrightness(0, false);
		Display7S.clear();
	#endif

	// Экран завершения
	draw_finish();
	delay(FINISH_SCREEN_TIME * 1000);

	// Шаг изменения подсветки экрана
	byte BrightStepLCD = (BrightLCD[BrightMode] - LCD_BRIGHT_MIN) / 31;
	// Шаг изменения подсветки приборной панели
	byte BrightStepPWM = (BrightPWM[BrightMode] - PWM_BRIGHT_MIN) / 31;

	byte Bright = 0;
	for (byte i = 1; i < 32; i++) {
		#ifdef PWM_BRIGHT_PIN
			if (BrightPWM[2]) {BrightPWM[2] -= BrightStepPWM;}
			analogWrite(PWM_BRIGHT_PIN, BrightPWM[2]);
		#endif

		if (BrightLCD[2]) {BrightLCD[2] -= BrightStepLCD;}
		#ifdef LCD_BRIGHT_PIN
			analogWrite(LCD_BRIGHT_PIN, BrightLCD[2]);
		#else
			u8g2.setContrast(BrightLCD[2]);
		#endif

		u8g2.clearBuffer();
		u8g2.setClipWindow(0, i, 127, 63 - i);
		// Заставка
		draw_finish();
		u8g2.sendBuffer();
		delay(15);
	}
	#ifdef PWM_BRIGHT_PIN
		analogWrite(PWM_BRIGHT_PIN, 0);
	#endif
	#ifdef LCD_BRIGHT_PIN
		analogWrite(LCD_BRIGHT_PIN, 0);
	#else
		u8g2.setContrast(0);
	#endif

	// Очищаем LCD дисплей
	u8g2.clear();

	while (!digitalRead(INT_POWER_PIN)) {
		delay(100);
	}
}

void power_n_light_status() {
	// Проверки состояния габаритов
	if (digitalRead(INT_LIGHT_PIN)) {
		if (BrightMode == 0) {BrightMode = 1;}
	}
	else {
		if (BrightMode == 1) {BrightMode = 0;}
	}
	// Проверки состояния замка зажигания
	if (digitalRead(INT_POWER_PIN)) {
		if (PowerState == 0) {power_on();}
	}
	else {
		if (PowerState == 1) {power_off();}
	}
}

void lcd_bright_change() {
	// Подсветка дисплея
	if (BrightLCD[BrightMode] != BrightLCD[2]) {
		if (BrightLCD[BrightMode] < BrightLCD[2]) {
			BrightLCD[2] -= 1;
		}
		else if (BrightLCD[BrightMode] > BrightLCD[2]) {
			BrightLCD[2] += 1;
		}
		#ifdef LCD_BRIGHT_PIN
			analogWrite(LCD_BRIGHT_PIN, BrightLCD[2]);
		#else
			u8g2.setContrast(BrightLCD[2]);
		#endif
	}
	// Подсветка приборной панели
	#ifdef PWM_BRIGHT_PIN
		if (BrightPWM[BrightMode] != BrightPWM[2]) {
			if (BrightPWM[BrightMode] < BrightPWM[2]) {
				BrightPWM[2] -= 1;
			}
			else if (BrightPWM[BrightMode] > BrightPWM[2]) {
				BrightPWM[2] += 1;
			}
			analogWrite(PWM_BRIGHT_PIN, BrightPWM[2]);
		}
	#endif
	// Яркость семисегментного дисплея
	#ifdef TM1637_ENABLE
		byte Buffer;
		if (BrightMode == 1) {Buffer = TM1637_BRIGHT_NIGHT * TM1637_BRIGHT_STEP;}
		else {Buffer = TM1637_BRIGHT_DAY * TM1637_BRIGHT_STEP;}
		if (Buffer != TM1637Bright) {
			if (Buffer > TM1637Bright) {
				TM1637Bright++;
				byte Buffer = TM1637Bright / TM1637_BRIGHT_STEP;
				Display7S.setBrightness(Buffer);
			}
			else if (Buffer < TM1637Bright) {
				TM1637Bright--;
				byte Buffer = TM1637Bright / TM1637_BRIGHT_STEP;
				Display7S.setBrightness(Buffer);
			}
		}
	#endif
}

// Отрисовка яркости экрана
void draw_bright() {
	if (BrightBoxState > 0) {
		byte L;
		char CharVal[4];
		u8g2.setFont(u8g2_font_helvB12_tn);

		// На дополнительном экране отображаеться яркость приборной панели
		if (LCDMode == 1) {dtostrf(BrightPWM[BrightMode], 3, 0, CharVal);}
		else {dtostrf(BrightLCD[BrightMode], 3, 0, CharVal);}

		// Рисуем блок поверх экрана
		u8g2.setDrawColor(1);
		u8g2.drawBox(43, 22, 42, 21);
		L = u8g2.getUTF8Width(CharVal);
		u8g2.setDrawColor(2);
		u8g2.drawUTF8(63 - L / 2, 39, CharVal);
		BrightBoxState -= 1;
	}
}

//=============================================================================
//=========================== Системные функции ===============================
//=============================================================================
// Инициализация при включении 
void setup() { // 0
	// Вход для проверки состояния замка зажигания
	pinMode(INT_POWER_PIN, INPUT_PULLUP);
	// Вход для проверки состояния габаритов
	pinMode(INT_LIGHT_PIN, INPUT_PULLUP);

	// Настраиваем выводы для энкодера.
	pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
	pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
	pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
	pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);

	// ШИМ управление подсветкой экрана
	#ifdef LCD_BRIGHT_PIN
		pinMode(LCD_BRIGHT_PIN, OUTPUT);
	#endif

	// ШИМ управление подсветкой приборной панели
	#ifdef PWM_BRIGHT_PIN
		pinMode(PWM_BRIGHT_PIN, OUTPUT);
	#endif

	// Очиста EEPROM
	#ifdef EEPROM_CLEAR_ON_START
		for (int i = 0; i < EEPROM.length(); i++) {
			EEPROM.write(i, 0);
		}
	#endif

	// Считываем данные из EEPROM
	read_eeprom();
	
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


	// Ожидаем включение замка зажигания
	while (!digitalRead(INT_POWER_PIN)) {
		delay(100);
	}

	// Проверка состояния габаритов и замка зажигания
	power_n_light_status();

	#ifdef DEBUG_MODE
		LCDMode = LCD_MODE_ON_START;
		for (byte i = 0; i < CE_COUNT_MAX; i++ ) {
			CountCE[i] = i + 1;
		}
	#else
		// Определение длины пакета
		get_data_size();
	#endif
}

// Вспомогательный цикл
void loop_2() { // 0
	// Проверка состояния габаритов и замка зажигания
	power_n_light_status();
	// Плавное изменение яркости
	if (millis() - BrightTimer >= 10) {
		lcd_bright_change();
		BrightTimer = millis();
	}
	// Обновление состояние кнопок
	#ifndef ENCODER_CONTROL
		button_update(0, BUTTON_UP_PIN);
		button_update(1, BUTTON_DOWN_PIN);
		button_update(2, BUTTON_LEFT_PIN);
	#endif
	button_update(3, BUTTON_RIGHT_PIN);

	#ifdef ENCODER_CONTROL
		encoder_update(); // 2
		// Энкодер является заменителем кнопок ввкрх/вниз. 
		if (EncoderState == 4) {ButtonState[0] = 32;}
		else if (EncoderState == -4) {ButtonState[1] = 32;}
	#endif

	// Выполнение комманд от кнопок
	// Вверх/вниз короткое:
	//  - На экране ошибок СЕ - перелистывание ошибок
	//	- На дополнительном экране - регулировка яркости приборной панели 
	//	- На остальных - регулировка яркости экрана
	if (LCDMode != LCD_MODE_CE) {
		if (ButtonState[0] == 32) {
			if (LCDMode == 1) {
				BrightPWM[BrightMode] = min(BrightPWM[BrightMode] + PWM_BRIGHT_STEP, 255);
			}
			else {
				BrightLCD[BrightMode] = min(BrightLCD[BrightMode] + LCD_BRIGHT_STEP, 255);
			}
			ButtonState[1] = 128;
			BrightBoxState = BRIGHT_BOX_TIMER;
		}
		if (ButtonState[1] == 32) {
			if (LCDMode == 1) {
				BrightPWM[BrightMode] = max(BrightPWM[BrightMode] - PWM_BRIGHT_STEP, PWM_BRIGHT_MIN);
			}
			else {
				BrightLCD[BrightMode] = max(BrightLCD[BrightMode] - LCD_BRIGHT_STEP, LCD_BRIGHT_MIN);
			}
			ButtonState[1] = 128;
			BrightBoxState = BRIGHT_BOX_TIMER;
		}
	}

	// Вправо/влево короткое смена экрана
	if (ButtonState[2] == 32) {
		LCDMode = max(LCDMode - 1, 0);
		ButtonState[2] = 128;
	}
	if (ButtonState[3] == 32) {
		if (LCDMode >= LCD_MODE_CE) {LCDMode = 0;}
		else {LCDMode += 1;}
		ButtonState[3] = 128;
	}

	// Вверх/вниз не задействовано.
	if (ButtonState[0] == 64) {
		ButtonState[0] = 128;
	}
	if (ButtonState[1] == 64) {
		ButtonState[1] = 128;
	}

	// Долгое нажатие влево - сброс счетчика работы двигателя
	if (ButtonState[2] == 64) {
		#ifdef DEBUG_MODE
			Serial.println(F("Engine Hours Clear"));
		#endif	
		EngineHours = 0;
		write_eeprom();
		ButtonState[2] = 128;
	}
	// Долгое нажатие вправо - сброс суточного пробега или ошибок CE
	if (ButtonState[3] == 64) {
		if (LCDMode < LCD_MODE_CE) {
			#ifdef DEBUG_MODE
				Serial.println(F("Distance Clear"));
			#endif	
			Distance = -1 * DIST;
			FuelBurned = 0.0;
			write_eeprom();
		}
		else {
			#ifdef DEBUG_MODE
				Serial.println(F("CE Clear"));
			#endif	
			for (byte i = 0; i < CE_COUNT_MAX; i++ ) {
				CountCE[i] = 0;
			}
		}
			ButtonState[3] = 128;
	}

	#ifndef DEBUG_MODE
		receive_data(); // 7
	#endif

	build_data(); // 8
	check_ce_errors(); // 5

	#ifdef DEBUG_MODE
		DataOk = 1;
		// Пишем в консоль время работы одного цикла.
		unsigned int CycleTime = millis() - ProgramTimer;
		if (CycleTime > 2) {
			//Serial.print(F("Cycle time: "));
			//Serial.println(CycleTime);
		}
		ProgramTimer = millis();
		//Serial.println(Bright);
	#endif
}

// Основной цикл
void loop() { // 0
	loop_2(); // 8

	// Обновлять дисплей только при наличии данных
	if (DataOk && PowerState) {

		speed_chime(); // 4

		if (millis() - LCDTimer >= LCD_UPDATE_DELAY) {
			LCDTimer = millis();
			AlarmBoxState += 1;
			if (AlarmBoxState > ALARM_BOX_TIMER) {AlarmBoxState = 1 - ALARM_BOX_TIMER;}

			// Основной экран
			if (LCDMode == 0) {lcd_main();}
			else if (LCDMode == 1) {lcd_second();}
			else if (LCDMode == 2) {lcd_acceleration();}
			else if (LCDMode == 3) {lcd_ce_errors();}

			#ifdef TM1637_ENABLE
				draw_tm1637();
			#endif
		}
	}
}
