#include <avr/io.h>  //P����
#include <avr/interrupt.h>
#include <util/delay.h>

#define kP 65
#define kD 0.05
#define kI 1
#define sum_max 90

void Uart_init();
void Uart_trans(unsigned char data);
void Uart_U16Bit_PutNum(int NumData);
void Timer1_init();
void Timer2_init();
void Timer3_init();
void Interrupt_init();
void Trigger1();
void Trigger2();

int a = 0;
int b = 0;
float dis1 = 0;
float dis2 = 0;
float error = 0;
float error_sum = 0;
float error_pre = 0;

int motor = 0;

SIGNAL(SIG_INTERRUPT4)
{
	if ((EICRB & 0b00000011) == 0b00000010)//����¡�϶�
	{
		a = TCNT3 - 5536;
		dis1 = a / 116;
		EICRB = 0b00001111;//����¡���� �ٲ�
	}

	else//�����϶�
	{
		TCNT3 = 5536;
		EICRB = 0b00001010;//�������� �ٲ�
	}
}

SIGNAL(SIG_INTERRUPT5)//10�� ���� 11�� ����¡
{
	if ((EICRB & 0b00001100) == 0b00001000)//����¡�϶�
	{
		b = TCNT3 - 5536;
		dis2 = b / 116;
		EICRB = 0b00001111;
		Trigger1();//����¡���� �ٲ�
	}

	else//�����϶�
	{
		TCNT3 = 5536;
		EICRB = 0b00001011;//�������� �ٲ�
	}
}

SIGNAL(SIG_OVERFLOW2)
{
	TCNT2 = 100;
	Trigger2();
	error = (dis1 - dis2);

	motor = kP*error + kD*(error - error_pre) + kI*error_sum;

	if (motor>790)
		motor = 790;
	else if (motor<-790)
		motor = -790;


	if (motor<0)
	{
		motor = -motor;
		PORTC = 0x03;
		OCR1A = motor;
		OCR1B = motor;
	}
	else if (motor>0)
	{
		PORTC = 0x00;
		OCR1A = motor*1.4;
		OCR1B = motor*1.4;
	}

	/* �Ѿ�� �� ���� ���� */
	if (error > 9)
	{
		OCR1A = 0;
		OCR1B = 0;
	}
	else if (error < -9)
	{
		OCR1A = 0;
		OCR1B = 0;
	}

	error_pre = error;

	/* ������ ���� */
	error_sum += error;

	/* ���а� ���� */
	if (error_sum > sum_max)
		error_sum = sum_max;
	else if (error_sum < -sum_max)
		error_sum = -sum_max;
	/* ���� ��ȯ �� ���а� �ʱ�ȭ */
	// if((error*error_pre)<0)
	//   error_sum = 0;


}


int main()
{
	DDRF = 0xff;// ������ Ʈ����
	DDRE = 0x00;// ������ ����

	Interrupt_init();
	Timer1_init();
	Timer2_init();
	Timer3_init();
	Uart_init();

	ICR1 = 800;
	SREG = 0x80;

	while (1)// �ٴ� ���� �Ÿ� 146
	{
	}
}

void Uart_init()
{
	DDRD = 0b00001000;
	UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (0 << UCSZ12);//RX TX ���
	UCSR1C = (0 << UMSEL1) | (1 << USBS1) | (1 << UCSZ11) | (1 << UCSZ10);//8��Ʈ ���,�񵿱�,��ž��Ʈ2��; 
	UBRR1H = 0;
	UBRR1L = 103;//��żӵ� 9600;
}

void Uart_trans(unsigned char data)
{
	while (!(UCSR1A&(1 << UDRE1)));
	UDR1 = data;
}

void Uart_U16Bit_PutNum(int NumData)
{
	int TempData;

	if (NumData<0)
	{
		NumData = -NumData;
		Uart_trans('-');
	}

	TempData = NumData / 10000;
	Uart_trans(TempData + 48);                // 1000 �ڸ� ��� 
	TempData = (NumData % 10000) / 1000;
	Uart_trans(TempData + 48);                 // 1000 �ڸ� ��� 
	TempData = (NumData % 1000) / 100;
	Uart_trans(TempData + 48);                 // 100 �ڸ� ��� 
	TempData = (NumData % 100) / 10;
	Uart_trans(TempData + 48);                  // 10 �ڸ� ��� 
	TempData = NumData % 10;
	Uart_trans(TempData + 48);          // 1�ڸ��� ���� 0�̶� �׳� ��´�. 
}

void Timer1_init()
{
	DDRB = 0xff;
	DDRC = 0xff;
	PORTC = 0b00000000;
	TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0) | (1 << WGM11) | (0 << WGM10);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);
}

void Timer2_init()
{
	TCCR2 = (0 << WGM21) | (0 << WGM20) | (1 << CS20) | (0 << CS21) | (1 << CS22);//�븻��� 1024����;
	TIMSK = (1 << TOIE2);
}

void Timer3_init()
{
	TCCR3A = 0b10000000;
	TCCR3B = (1 << CS31);//8���� 0�� ���;
}

void Interrupt_init()
{
	EIMSK = 0b00110000;
	EICRB = 0b00001111;
}

void Trigger2()
{
	PORTF = 0x02;
	_delay_us(8);
	PORTF = 0x00;
}

void Trigger1()
{
	PORTF = 0x01;
	_delay_us(8);
	PORTF = 0x00;
}