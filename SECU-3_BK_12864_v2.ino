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
			   		* Давление масла (F),
			   		* ДАД2, давление газа (F),
			   		* Дифф.давление, ДАД2-ДАД (F),
			   		* Темп. 2, температура газа (F),
			   		* Загрузка форсунок (F).


	
==================================================================================================
	Назначения выводов Arduino

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

// Подключение бибилиотек
#include <U8g2lib.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <TM1637Display.h>

// Подключение дополнительных файлов.
#include "CE_Errors.h"
#include "Configuration.h"
#include "Global_Variables.h"
#include "Pictograms.h"


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!! ВСЕ НАСТРОЙКИ В ФАЙЛЕ Configuration.h !!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//=============================================================================
//================================== Отладка ==================================
//=============================================================================
// Режим отладки. Проверка работы экранов без подключения к SECU.
// Раскомментировать для включения.
//#define DEBUG_MODE


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

// Настройка LCD дисплея
U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, SPI_RS_PIN);

//=============================================================================
//=================== Функциия отрисовки блоков данных ========================
//=============================================================================

void draw_ff_fc_f(byte x, byte y) {
	float FF_FRQ = (float) build_int(32 + DataShift) * 0.00390625;  // 256
	FF_FRQ = constrain(FF_FRQ, 0, 256);
	float TPS = (float) Data[18] * 0.5;                     // 2
	float SPD = build_speed(27 + DataShift);
	float FF = 0.0;
	float FC = 0.0;

	// Режим отображения
	byte Mode = 1;

	// Мгновенный расход л/ч
	FF = (float) FF_FRQ * 0.225; // (3600 / 16000)
	FF = constrain(FF, 0, 99.9);
	// Мгновенный расход л/100км
	if (SPD > 3 && TPS > 3) {
		FC = (FF * 100.0) / SPD;
		FC = constrain(FC, 0, 99.9);
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
		dtostrf(FF, 4, 1, CharVal);
		u8g2.drawUTF8(x + 2, y + H * 2 + 4, "F");
	}
	else if (Mode == 2) {
		dtostrf(FC, 4, 1, CharVal);
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

void draw_water_temp_f(byte x, byte y) {
	float TEMP = (float) build_int(7) * 0.25;
	//TEMP = DataSize;
	TEMP = constrain(TEMP, -99, 333);
	char CharVal[4];
	byte H;
	byte L;

	// Рисуем прямоугольник, при выходе параметра за допустимые пределы.
	if (BoxState > 0) {
		if (TEMP < WATER_TEMP_MIN || TEMP > WATER_TEMP_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 19);
		}
	}

	// Рисуем пиктограмму параметра.
	u8g2.drawXBMP(x + 2, y + 3, Temp_width, Temp_height, Temp_bits);
	
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(TEMP, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 10 + H/2, CharVal);
	u8g2.drawXBMP(41 - 4 + x, y + 5, Cels_width, Cels_height, Cels_bits);
}

void draw_distance_f(byte x, byte y) {
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

void draw_map_h(byte x, byte y) {
	float MAP = (float) build_int(3) * 0.015625;            // 64
	MAP = constrain(MAP, 0, 333);

	byte H;
	byte L;
	char CharVal[6];

	if (BoxState > 0) {
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

void draw_map_f(byte x, byte y) {
	float MAP = (float) build_int(3) * 0.015625;            // 64
	MAP = constrain(MAP, 0, 333);
	char CharVal[4];
	byte H;
	byte L;

	if (BoxState > 0) {
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

void draw_map2_f(byte x, byte y) {
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

void draw_map_diff_f(byte x, byte y) {
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

void draw_temp2_f(byte x, byte y) {
	float Temp2 = (float) build_int(58 + DataShift) * 0.25;            // 4
	Temp2 = constrain(Temp2, -99, 333);
	char CharVal[4];
	byte H;
	byte L;

	u8g2.drawXBMP(x + 2, y + 2, Temp2_width, Temp2_height, Temp2_bits);
	u8g2.setFont(u8g2_font_pxplusibmvga8_tn);
	H = u8g2.getAscent();

	dtostrf(Temp2, 3, 0, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 2 - L + x, y + 10 + H/2, CharVal);
}

void draw_airtemp_h(byte x, byte y) {
	float AIRTEMP = (float) build_int(34 + DataShift) * 0.25;           // 4
	AIRTEMP = constrain(AIRTEMP, -99, 333);
	byte H;
	byte L;
	char CharVal[6];

	if (BoxState > 0) {
		if (AIRTEMP < AIR_TEMP_MIN || AIRTEMP > AIR_TEMP_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 9);
		}
	}

	u8g2.drawXBMP(x + 4, y + 1, AirTemp_width, AirTemp_height, AirTemp_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(AIRTEMP, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
	u8g2.drawXBMP(41 - 4 + x, y + 2, Cels_width, Cels_height, Cels_bits);
}

void draw_trottle_f(byte x, byte y) {
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
	u8g2.drawUTF8(41 - 5 - L + x, y + 10 + H/2, CharVal);
}

void draw_afc_f(byte x, byte y) {
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

void draw_O2_sensor_h(byte x, byte y) {
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

	if (BoxState > 0) {
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

	u8g2.drawXBMP(x + 2, y + 1, O2_width, O2_height, O2_bits);
	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
}

void draw_lambda_corr_h(byte x, byte y) {
	char CharVal[6];
	byte H;
	byte L;

	float AFR_CORR = (float) build_int(50 + DataShift) * 0.1953125; // 5.12
	AFR_CORR = constrain(AFR_CORR, -99.9, 99.9);
	if (BoxState > 0) {
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

void draw_battery_h(byte x, byte y) {
	float BAT = (float) build_int(5) * 0.0025;          // 400
	BAT = constrain(BAT, 0, 33);

	char CharVal[5];
	byte H;
	byte L;

	if (BoxState > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			u8g2.drawBox(x + 1, y + 1, 40, 9);
		}
	}

	u8g2.drawXBMP(x + 2, y + 1, Batt_width, Batt_height, Batt_bits);

	u8g2.setFont(u8g2_font_haxrcorp4089_tn);
	H = u8g2.getAscent();

	dtostrf(BAT, 4, 1, CharVal);
	L = u8g2.getUTF8Width(CharVal);
	u8g2.drawUTF8(41 - 5 - L + x, y + 9, CharVal);
}

void draw_angle_h(byte x, byte y) {
	float ANGLE = (float) build_int(9) * 0.03125;           // 32
	ANGLE = constrain(ANGLE, 0, 99);
	float DDANGLE = (float) build_int(13) * 0.03125;           // 32
	DDANGLE = constrain(DDANGLE, 0, 99);
	char CharVal[6];
	byte H;
	byte L;

	if (BoxState > 0) {
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

void draw_fuel_burned_f(byte x, byte y) {
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

void draw_speed_f(byte x, byte y) {
	float SPD =  build_speed(27 + DataShift);
	float DDANGLE = (float) build_int(13) * 0.03125;
	DDANGLE = constrain(DDANGLE, 0, 99);
	char CharVal[4];
	byte H;
	byte L;

	if (BoxState > 0) {
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

void draw_rpm_f(byte x, byte y) {
	int RPM = build_int(1);
	RPM = constrain(RPM, 0, 9999);
	RPM = trunc(RPM * 0.1) * 10;
	char CharVal[5];
	byte H;
	byte L;

	if (BoxState > 0) {
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

void draw_fuel_tank_f(byte x, byte y) {
	float FTLS = (float) build_int(75) * 0.015625;            // 64
	FTLS = constrain(FTLS, 0, 333);
	byte H;
	byte L;
	char CharVal[4];
	if (BoxState > 0) {
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

void draw_egt_f(byte x, byte y) {
	float EGT = (float) build_int(77) * 0.25;            // 4
	EGT = constrain(EGT, 0, 1999);
	EGT = trunc(EGT * 0.1) * 10;

	char CharVal[5];
	byte H;
	byte L;

	if (BoxState > 0) {
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

void draw_oil_pressure_f(byte x, byte y) {
	float OPS = (float) build_int(79) * 0.015625;            // 64
	OPS = constrain(OPS, 0, 9.9);
	byte H;
	byte L;
	char CharVal[4];
	if (BoxState > 0) {
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

void draw_inj_duty_f(byte x, byte y) {
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

// Обороты на дополнительном сегментном дисплее
void draw_rpm_tm1637() {
	int RPM = build_int(1);
	RPM = constrain(RPM, 0, 9999);
	RPM = trunc(RPM * 0.1) * 10;
	TM1637Display display(TM1637_CLK_PIN, TM1637_DIO_PIN);
	display.setBrightness(TM1637_BRIGHTNESS);
	display.showNumberDec(RPM, false);
}

//=============================================================================
//===================== Функциия отрисовки экранов ============================
//=============================================================================
// Экран приветствия
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

// Основной экран
void lcd_main() {
	// Очищаем память дисплея
	u8g2.clearBuffer();

	// ========================== Блоки данных ==========================
    draw_ff_fc_f(0, 0);    // Мгновенный расход топлива (F)	
	
    draw_water_temp_f(0, 22);    // Температура ОЖ (F)	
	
    draw_distance_f(0, 44);    // Суточный и общий пробег (F)	
	
    draw_map_h(43, 0);    // ДАД (H)	
    draw_airtemp_h(43, 10);    // Температура воздуха на впуске (H)	
    draw_trottle_f(43, 22);    // ДПДЗ / РХХ (F)	
	
    draw_afc_f(43, 44);    // Средний расход топлива (F)	
	
    draw_O2_sensor_h(86, 0);    // Напряжение УДК / AFR (H)	
    draw_lambda_corr_h(86, 10);    // Лямбда коррекция (H)	
    draw_battery_h(86, 22);    // Напряжение сети (H)	
    draw_angle_h(86, 32);    // УОЗ (H)	
    draw_fuel_burned_f(86, 44);    // Израсходованное топливо (F)	

	// ========================== Блоки данных ==========================


	// Линии разметки
	u8g2.drawHLine(0, 21, 128);
	u8g2.drawHLine(0, 43, 128);
	u8g2.drawVLine(42, 0, 64);
	u8g2.drawVLine(85, 0, 64);

	// Рамка при появлении ошибок CE
	if (StatusCE > 0 && BoxState > 0) {
		u8g2.drawFrame(0, 0, 128, 64);
		StatusCE = 0;
	}
	
	// Отсылаем данные на дисплей
	u8g2.sendBuffer();
}

// Второй экран
void lcd_second() {
	// Очищаем память дисплея
	u8g2.clearBuffer();

	// ========================== Блоки данных ==========================
    draw_oil_pressure_f(0, 0);    // Давление масла (F)	
	
    draw_map_f(0, 22);    // ДАД (F)	
	
    draw_map2_f(0, 44);    // ДАД2, давление газа (F)	
	
    draw_map_diff_f(43, 0);    // Дифф.давление, ДАД2-ДАД (F)	
	
    draw_rpm_f(43, 22);    // Обороты (F)	
	
    draw_egt_f(43, 44);    // Температура выхлопных газов (F)	
	
    draw_fuel_tank_f(86, 0);    // Уровень топлива (F)	
	
    draw_inj_duty_f(86, 22);    // Загрузка форсунок (F)	
	
    draw_temp2_f(86, 44);    // Темп. 2, температура газа (F)	
	

	// ========================== Блоки данных ==========================


	// Линии разметки
	u8g2.drawHLine(0, 21, 128);
	u8g2.drawHLine(0, 43, 128);
	u8g2.drawVLine(42, 0, 64);
	u8g2.drawVLine(85, 0, 64);

	// Рамка при появлении ошибок CE
	if (StatusCE > 0 && BoxState > 0) {
		u8g2.drawFrame(0, 0, 128, 64);
		StatusCE = 0;
	}
	
	// Отсылаем данные на дисплей
	u8g2.sendBuffer();
}

// Экран замера разгона
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

	while (LCDMode == 2) {
		loop_2();

		// Сброс длительного нажатия кнопки
		if (ButtonState == 3) {
			ButtonState = 2;
		}

		// Переход на следующий экран
		if (ButtonState == 1) {
			LCDMode = 3;
			ButtonState = 2;
		}

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

				// ========================== Блоки данных ==========================
			    draw_speed_f(86, 0);    // Скорость (F)	
				
			    draw_map_f(86, 22);    // ДАД (F)	
				
			    draw_water_temp_f(86, 44);    // Температура ОЖ (F)	
				// ========================== Блоки данных ==========================

				// Линии разметки
				u8g2.drawHLine(86, 21, 42);
				u8g2.drawHLine(86, 43, 42);
				u8g2.drawVLine(85, 0, 64);

				// Отсылаем данные на дисплей
				u8g2.sendBuffer();
			}
			PrevSPD = SPD;
		}
	}
}

// Экран ошибок CE
void lcd_ce_errors() {
	byte BitsCount = 16;
	if (DataSize >= 74) {BitsCount = 21;}

	#define ROWS_ON_SCREEN 5
	byte StartRow = 0;
	byte Row = 0;
	byte OverRow = 0;
	uint32_t CE = 0;
	char CharVal[4];
	char CEItemChar[25];
	byte H;
	byte L;

	while (LCDMode == 3) {
		loop_2();

		// Сброс ошибок при длительном нажатия кнопки
		if (ButtonState == 3) {
			for (byte i = 0; i < BitsCount; i++ ) {
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
			for (byte i = StartRow; i < BitsCount; i++ ) {
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

//=============================================================================
//============================== Основные функции  ============================
//=============================================================================

// Запись EEPROM
void write_eeprom() {
	analogWrite(BRIGHT_PIN, 0);
	float Dst = Distance + DIST;
	float DstAll = DistanceAll + DIST;

	// Dst = 210;
	//DstAll = 5998;
	// FuelBurned = 15.6;
	//FuelBurnedAll = 472;

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

// Чтение буфера данных от SECU
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

// Определение размера пакета данных
void get_data_size() {
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
void receive_data() {
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
		else {
			button_update();
			encoder_update();
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
			else {
				button_update();
				encoder_update();
			}
		}
	}
}

// Сборка int из двух байт
int build_int(byte i) {
	int Value = 0;
	byte *pValue = (byte*)&Value;
	*pValue = Data[i + 1];  
	*(pValue + 1) = Data[i];
	return Value;
}

// Расчет скорости ТС
float build_speed(byte i) {
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
float build_distance(byte i) {
	unsigned long Value = 0;
	byte* pValue = (byte*)&Value;
	*pValue = Data[i + 2]; 
	*(pValue + 1) = Data[i + 1];
	*(pValue + 2) = Data[i];

	float Dist = (float) (M_PERIOD_DISTANCE * Value) * 0.001;
	return Dist;
}

// Сборка некоторых данных из байтов от SECU
void build_data() {
	if (DataOk) {
		if (millis() - FuelTimer >= 500) {
			DIST = build_distance(29 + DataShift);

			// Расчет израсходованного топлива за интервал
			float FF_FRQ = (float) build_int(32 + DataShift) * 0.00390625;  // 256
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

// Разбор блока ошибок CE
void check_ce_errors() {
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

// Проверка состояние энкодера
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

// Колокольчик AE86
void speed_chime() {
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

//=============================================================================
//=========================== Системные функции ===============================
//=============================================================================
// Инициальзация при включении 
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
	
	#ifdef EEPROM_CLEAR_ON_START
		for (int i = 0; i < EEPROM.length(); i++) {
			EEPROM.write(i, 0);
		}
	#endif

	// Отображение заставки
	draw_init(23, 0);

	// Определение длины пакета
	#ifndef DEBUG_MODE
		get_data_size();
	#endif
}

// Вспомогательный цикл
void loop_2() {
	button_update();
	encoder_update();

	#ifndef DEBUG_MODE
		receive_data();
	#endif

	build_data();
	check_ce_errors();

	#ifdef DEBUG_MODE
		DataOk = 1;
	#endif

	if (abs(EncoderState) == 4) {
		Bright = max(MIN_BRIGHT, min(255, Bright + EncoderState / abs(EncoderState) * 2));
		analogWrite(BRIGHT_PIN, Bright);
		EncoderState = 0;
	}
}

// Основной цикл
void loop() {
	loop_2();

	// Обновлять дисплей только при наличии данных
	if (DataOk) {
		// Длительное нажатие кнопки - сброс суточного пробега
		if (ButtonState == 3) {
			Distance = -1 * DIST;
			FuelBurned = 0.0;
			write_eeprom();
			ButtonState = 2;
		}
		
		// Короткое нажатие кнопки переход на следующий экран
		if (ButtonState == 1) {
			LCDMode += 1;
			ButtonState = 2;
		}

		speed_chime();

		if (millis() - LCDTimer >= LCD_UPDATE_DELAY) {
			LCDTimer = millis();

			ButtonTimer = min(255, ButtonTimer + 1);
			BoxState += 1;
			if (BoxState > 2) {BoxState = -1;}
			// Основной экран
			if (LCDMode == 0) {lcd_main();}
			else if (LCDMode == 1) {lcd_second();}
			else if (LCDMode == 2) {lcd_acceleration();}
			else if (LCDMode == 3) {lcd_ce_errors();}

			#ifdef SHOW_RPM_TM1637
				draw_rpm_tm1637();
			#endif
		}
	}
}
