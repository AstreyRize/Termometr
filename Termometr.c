#include <tiny2313.h>
#include <delay.h>
#include <ds18b20.h>

// Данные
#define DS PORTD.1

// Защелка
#define ST_CP PORTA.0

// Тактовый импульс
#define SH_CP PORTD.2

// Сброс значений регистра
#define MR PORTD.3

// Модуль числа
#define ABS(x) ((x) < 0 ? -(x) : (x))

// Температура была запрошена. 
unsigned char TEMPERATURE_REQUESTED = 0;

// Цифры для e40561 
unsigned char numbers[11] =
{
	//BFAGCpDE
	0b00010100, //0
	0b01110111, //1
	0b01001100, //2
	0b01000101, //3
	0b00100111, //4
	0b10000101, //5
	0b10000100, //6
	0b01010111, //7
	0b00000100, //8
	0b00000101, //9
	0b11101111  //-
};

unsigned char digit[4] =
{
	0b00000001, //разряд 1
	0b00001000, //разряд 2
	0b00000100, //разряд 3
	0b00000010, //разряд 4
};

// Показать цифру на индикаторе e40561
void SetNumber(unsigned char number, unsigned char mask)
{
	unsigned char bits;
	unsigned char i = 0;

	bits = numbers[number];
	bits ^= mask;

	for (i = 0; i < 8; i++)
	{
		DS = (bits >> i) & 1;
		SH_CP = 1;
		SH_CP = 0;
	}

	ST_CP = 1;
	ST_CP = 0;
}

void SetMinus()
{
	unsigned char bits;
	unsigned char i = 0;

	bits = numbers[10];

	for (i = 0; i < 8; i++)
	{
		DS = (bits >> i) & 1;
		SH_CP = 1;
		SH_CP = 0;
	}

	ST_CP = 1;
	ST_CP = 0;
}

// Отобразить число на индикаторе e40561
void ShowNumber(int temp)
{
	unsigned char step = 0;

	// Самый левый заполняем знаком
	if (temp < 0)
	{
		SetMinus();
		PORTB = 0b00000010;
		delay_ms(1);
		PORTB = 0b00000000;
	}

	// Первые 3 разряда справа заполняем числами
	do
	{
		if (step == 1)
		{
			//Младший разряд отображаем с точкой
			SetNumber(ABS(temp) % 10, 0b00000100);
		}
		else
		{
			SetNumber(ABS(temp) % 10, 0b00000000);
		}

		PORTB = digit[step];
		delay_ms(1);
		PORTB = 0b00000000;

		temp /= 10;
		step++;
	} while (temp && step < 3);
}

void main(void)
{
	// Crystal Oscillator division factor: 1
#pragma optsize-
	CLKPR = (1 << CLKPCE);
	CLKPR = (0 << CLKPCE) | (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

	// Input/Output Ports initialization
	// Port A initialization
	DDRA = (0 << DDA2) | (0 << DDA1) | (1 << DDA0);
	PORTA = (0 << PORTA2) | (0 << PORTA1) | (0 << PORTA0);

	// Port B initialization 
	DDRB = (0 << DDB7) | (0 << DDB6) | (0 << DDB5) | (0 << DDB4) | (1 << DDB3) | (1 << DDB2) | (1 << DDB1) | (1 << DDB0);
	PORTB = (0 << PORTB7) | (0 << PORTB6) | (0 << PORTB5) | (0 << PORTB4) | (0 << PORTB3) | (0 << PORTB2) | (0 << PORTB1) | (0 << PORTB0);

	// Port D initialization
	DDRD = (0 << DDD6) | (0 << DDD5) | (0 << DDD4) | (1 << DDD3) | (1 << DDD2) | (1 << DDD1) | (0 << DDD0);
	PORTD = (0 << PORTD6) | (0 << PORTD5) | (0 << PORTD4) | (0 << PORTD3) | (0 << PORTD2) | (0 << PORTD1) | (0 << PORTD0);

	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: Timer 0 Stopped
	// Mode: Normal top=0xFF
	// OC0A output: Disconnected
	// OC0B output: Disconnected
	TCCR0A = (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (0 << WGM01) | (0 << WGM00);
	TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00);
	TCNT0 = 0x00;
	OCR0A = 0x00;
	OCR0B = 0x00;

	// Timer/Counter 1 initialization
	// Clock source: System Clock
	// Clock value: Timer1 Stopped
	// Mode: Normal top=0xFFFF
	// OC1A output: Disconnected
	// OC1B output: Disconnected
	// Noise Canceler: Off
	// Input Capture on Falling Edge
	// Timer1 Overflow Interrupt: Off
	// Input Capture Interrupt: Off
	// Compare A Match Interrupt: Off
	// Compare B Match Interrupt: Off
	TCCR1A = (0 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (0 << WGM10);
	TCCR1B = (0 << ICNC1) | (0 << ICES1) | (0 << WGM13) | (0 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10);
	TCNT1H = 0x00;
	TCNT1L = 0x00;
	ICR1H = 0x00;
	ICR1L = 0x00;
	OCR1AH = 0x00;
	OCR1AL = 0x00;
	OCR1BH = 0x00;
	OCR1BL = 0x00;

	// Timer(s)/Counter(s) Interrupt(s) initialization
	TIMSK = (0 << TOIE1) | (0 << OCIE1A) | (0 << OCIE1B) | (0 << ICIE1) | (0 << OCIE0B) | (0 << TOIE0) | (0 << OCIE0A);

	// External Interrupt(s) initialization
	// INT0: Off
	// INT1: Off
	// Interrupt on any change on pins PCINT0-7: Off
	GIMSK = (0 << INT1) | (0 << INT0) | (0 << PCIE);
	MCUCR = (0 << ISC11) | (0 << ISC10) | (0 << ISC01) | (0 << ISC00);

	// USI initialization
	// Mode: Disabled
	// Clock source: Register & Counter=no clk.
	// USI Counter Overflow Interrupt: Off
	USICR = (0 << USISIE) | (0 << USIOIE) | (0 << USIWM1) | (0 << USIWM0) | (0 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);

	// USART initialization
	// USART disabled
	UCSRB = (0 << RXCIE) | (0 << TXCIE) | (0 << UDRIE) | (0 << RXEN) | (0 << TXEN) | (0 << UCSZ2) | (0 << RXB8) | (0 << TXB8);

	// Analog Comparator initialization
	// Analog Comparator: Off
	// The Analog Comparator's positive input is
	// connected to the AIN0 pin
	// The Analog Comparator's negative input is
	// connected to the AIN1 pin
	ACSR = (1 << ACD) | (0 << ACBG) | (0 << ACO) | (0 << ACI) | (0 << ACIE) | (0 << ACIC) | (0 << ACIS1) | (0 << ACIS0);
	// Digital input buffer on AIN0: On
	// Digital input buffer on AIN1: On
	DIDR = (0 << AIN0D) | (0 << AIN1D);

	// Global enable interrupts
	#asm("sei")

		// Инициализация датчика
		ds18b20_init(0, -55, +99, DS18B20_12BIT_RES);

	while (1)
	{
		float temperature;

		MR = 1;

		// Если температура не запрошена - запрашиваем.
		// Если температура была запрошена - читаем значение.
		if (TEMPERATURE_REQUESTED == 0)
		{
			ds18b20_request_temperature(0);
			TEMPERATURE_REQUESTED = 1;
		}
		else
		{
			temperature = ds18b20_get_temperature(0);
			TEMPERATURE_REQUESTED = 0;
		}

		// Если датчик выдаёт больше 1000
		if (temperature > 1000)
		{
			// Отнимаем от данных 4096
			temperature = 4096 - temperature;
			// И ставим знак "минус"
			temperature = -temperature;
		}

		// Избавляемся от дробной части,
		// Десятки теперь дробная часть.
		ShowNumber(temperature * 10);
	}
}