#define F_CPU 16000000UL

#include<avr/io.h>
#include<avr/interrupt.h>

//void Exi_init();
void Timer_init();
void Adc_init();
void Uart_init();

void Uart_trans(unsigned char data);
void Uart_U16Bit_PutNum(int NumData);

int Adc_Channel(unsigned char Adc_input);
void Adc_Max(int j);
void Adc_Min(int j);
int Adc_data = 0;
float weight[8] = { 8.5,3.2,1.2,1,-1,-1.2,-3.2,-8.5 };
int sum[8] = { 0, };
int ssum = 0;
int jun = 0;

int i = 0;
int k = 0;
int mode = 0;
int a = 0, b = 0;

float Adc_max[8] = { 1023,1023,1023,1023,1023,1023,1023,1023 };
float Adc_min[8] = { 1023,1023,1023,1023,1023,1023,1023,1023 };
float Adc_nor[8] = { 0, };

int main(void)
{

	DDRA = 0xff;
	PORTA = 0xff;
	DDRE = 0x00;
	Timer_init();
	Adc_init();
	//	Uart_init();
	PORTB = 0xff;
	PORTC = 0xff;
	ICR1 = 800;
	while (1)
	{
		if ((PINE & 0b0100000) == 0)
			mode = 1;
		if (mode == 1)
		{
			while (1)
			{
				for (int i = 0; i<8; i++)
				{
					Adc_data = Adc_Channel(i);

					Adc_Max(i);

				}
				//	PORTA = 0b11111110;		
				if ((PINE & 0b10000000) == 0)
					mode = 2;
				break;
			}
		}

		if (mode == 2)
		{
			while (1)
			{
				for (int i = 0; i<8; i++)
				{
					Adc_data = Adc_Channel(i);

					Adc_Min(i);

				}
				//			PORTA = 0b11111101;

				if ((PINE & 0b01000000) == 0)
				{
					mode = 3;
					break;
				}
			}

		}
		if (mode == 3)
		{
			PORTC = 0x00;
			break;
		}
	}
	while (mode == 3)
	{
		for (i = 0; i<8; i++)
		{
			Adc_data = Adc_Channel(i);

			Adc_nor[i] = ((100 * (Adc_max[i] - Adc_data)) / (Adc_max[i] - Adc_min[i]));

			if (Adc_nor[i]>100)
				Adc_nor[i] = 100;

			else if (Adc_nor[i]<0)
				Adc_nor[i] = 0;

			if (Adc_nor[i] >= 70)
			{
				PORTA &= ~(1 << i);
			}

			else if (Adc_nor[i] <= 69)
			{
				PORTA |= ~(0 << i);
			}
			sum[i] = Adc_nor[i] * weight[i];
		}

		if (Adc_nor[7]>60)
			k = 1;

		else if (Adc_nor[0]>60)
			k = 2;

		if (k == 1)
		{
			OCR1A = 30;
			OCR1B = 790;

			if ((Adc_nor[6]>60) | (Adc_nor[5]>60))
				k = 0;

			goto here;
		}

		if (k == 2)
		{
			OCR1A = 790;
			OCR1B = 30;

			if ((Adc_nor[1]>70) | (Adc_nor[2]>70))
				k = 0;

			goto here;
		}
		for (i = 0; i<8; i++)
		{
			ssum += sum[i];
		}
		jun = ssum;

		a = 230 + (ssum) * 2 / 5;
		b = 230 - (ssum) * 2 / 5;


		if (a>800)
			a = 800;
		else if (a<0)
			a = 0;

		if (b>800)
			b = 800;
		else if (b<0)
			b = 0;

		if ((ssum<100)&(ssum>-100))
		{
			a = 330 + (ssum) * 2 / 5;
			b = 330 - (ssum) * 2 / 5;
		}

		OCR1A = a;
		OCR1B = b;

	here:
		ssum = 0;
	}

	return 0;
}

void Timer_init()
{
	DDRB = 0xff;
	DDRC = 0xff;
	TCCR1A = 0b10100010;//OC1A,OC1B핀 사용;
	TCCR1B = 0b00011001;//14번 모드,분주비1;
}

/*void Exi_init()
{
DDRE = 0b00000000;
SREG = 0x80;
EIMSK = 0b00110000;//외부 45번사용
EICRB = 0b00001010;//falling
}*/
/*void Uart_init()
{
DDRD = 0b00001000;
UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(0<<UCSZ12);//RX TX 사용
UCSR1C = (0<<UMSEL1)|(1<<USBS1)|(1<<UCSZ11)|(1<<UCSZ10);//8비트 통신,비동기,스탑비트2개;

UBRR1H = 0;
UBRR1L = 103;//통신속도 9600;
}

void Uart_trans(unsigned char data)
{
while(!(UCSR1A&(1<<UDRE1)));
UDR1 = data;
}*/

void Adc_init()
{
	DDRF = 0x00;

	ADMUX = (0 << REFS1) | (1 << REFS0) | (0 << ADLAR);
	ADCSRA = (1 << ADEN) | (0 << ADFR) | (0 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	//프리러닝모드쓰면면안됌  128분주 인터럽트 쓴다고하면 안됌
}

int Adc_Channel(unsigned char Adc_input)
{
	int data = 0;
	ADMUX = (Adc_input | 0x40);
	ADCSRA |= 0x40;
	while ((ADCSRA & 0x10) == 0x00);
	data = ADC;
	return data;
}

/*void Uart_U16Bit_PutNum(int NumData )
{
int TempData;

if(NumData<0)
{
NumData = -NumData;
Uart_trans('-');
}


TempData = NumData/10000;
Uart_trans(TempData+48);                // 1000 자리 출력
TempData = (NumData%10000)/1000;
Uart_trans(TempData+48);                 // 1000 자리 출력

TempData = (NumData%1000)/100;
Uart_trans(TempData+48);                 // 100 자리 출력

TempData = (NumData%100)/10;
Uart_trans(TempData+48);                  // 10 자리 출력
TempData = NumData%10;
Uart_trans(TempData+48);          // 1자리는 값이 0이라도 그냥 찍는다.
}*/

void Adc_Max(int j)
{
	if (Adc_data<Adc_max[j])
	{
		Adc_max[j] = Adc_data;
	}
	return;
}

void Adc_Min(int j)
{
	if (Adc_data<Adc_min[j])
	{
		Adc_min[j] = Adc_data;
	}
	return;
}