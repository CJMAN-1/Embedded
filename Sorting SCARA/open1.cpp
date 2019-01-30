#include <stdio.h>			//â�� �ҽ�
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <math.h>
//#include <C:\Users\kimcj\Documents\Visual Studio 2015\Projects\DynamixelSDK-3.5.4\DynamixelSDK-3.5.4\c\example\protocol1.0\read_write\read_write.c>
#include "dynamixel.h"
#include "dxl_hal.h"


#define lowY  0
#define lowcb 0
#define lowcr 0
#define highY  255
#define highcb 255
#define highcr 255

#define top 400
#define bottom 400

#define xpix 250
#define ypix 228

#define PI 3.141592
#define L1 24.8
#define L2 24.8

#define ss 0.5
using namespace cv;

int num_label1 = 0;
int num_label2 = 0;
int num_label3 = 0;

int x[4], y[4];

int lcrr, lcbr, lcrg, lcbg, hcrr, hcrg, hcbr, hcbg, hcbb ,hcrb, lcrb, lcbb;

int make_trackbar();//Ʈ���� ����
void callback_f(int, void*);//�Ű������� int, void*�� �־����� �� �𸣰���

CDynamixel g_Dynamixel;
CDxl g_Comm;

void video()
{
	Mat frame_origin, frame_[3],	//[0:r][1:g][2:b]
		frame_label[3],		//�󺧸� ������, �Ⱦ�
		stats[3],		//�󺧸��� �������� �����ִ�����
		center[3];		//�󺧸��� ��ü �߽� ��ǥ

	Mat frame , frame1;		//���� ���� ����

	int admin =0;	//������� �ȵɶ�
	int num[3];		//�󺧸��� ��ü�� ��
//	int area, left, top, width, height;		//�ڽ� ���鶧
		//�߽� x,y��ǥ

	char escape = 0;	//while�� ������

	int buffer[5] = { 0, };
	VideoCapture cap(0);
//	namedWindow("Track bar",0);	//������� �ٽ� ������ admin���� Ȱ��ȭ. 
//	admin = make_trackbar();

	while (1)
	{
		cap >> frame1;
		
		frame = frame1;

		Rect rect(180, 120, 300, 300);//���ɿ��� ����
		frame = frame(rect);

		cvtColor(frame, frame, CV_BGR2YCrCb);
		GaussianBlur(frame, frame, Size(11, 11), 0, 0);

		if (admin != 1) //������ �ߵɶ�
		{
			inRange(frame, Scalar(lowY, 135, lowcb), Scalar(highY, highcr, 111), frame_[0]);
			inRange(frame, Scalar(lowY, lowcr, lowcb), Scalar(highY, 97, 130), frame_[1]);
			inRange(frame, Scalar(lowY, lowcr, 135), Scalar(highY, 106, highcb), frame_[2]);
		}

		else	//������ �ȵɶ� ������� ����
		
		{
			inRange(frame, Scalar(lowY, lcrr, lcbr), Scalar(highY, hcrr, hcbr), frame_[0]);
			inRange(frame, Scalar(lowY, lcrg, lcbg), Scalar(highY, hcrg, hcbg), frame_[1]);
			inRange(frame, Scalar(lowY, lcrb, lcbb), Scalar(highY, hcrb, hcbb), frame_[2]);
		}

		for(int i=0;i<3;i++)	//�󺧸� ������ ������ rgb��
			num[i] = connectedComponentsWithStats(frame_[i], frame_label[i], stats[i], center[i]);

		for (int i=0;i<3;i++)	//rgb �󺧸��� ��ü �߽ɿ� �׸� ġ��
		{
			for (int j = 1; j < num[i]; j++) 
			{
//�󺧸�ũ����area = stats[0].at<int>(j, CC_STAT_AREA);
	//			left = stats[0].at<int>(j, CC_STAT_LEFT);
	//			top = stats[0].at<int>(j, CC_STAT_TOP);
	//			width = stats[0].at<int>(j, CC_STAT_WIDTH);
	//			height = stats[0].at<int>(j, CC_STAT_HEIGHT);

				x[i] = center[i].at<double>(j, 0); //�߽���ǥ
				y[i] = center[i].at<double>(j, 1);

				rectangle(frame,Point(x[i] -5, y[i] -5),
								Point(x[i] +5, y[i] +5),
								Scalar(0, 0, 255), 1);
			}
		}

/*			�߽���ǥ ����Ҷ��� ��
		for (int i = 0; i < 3; i++)
		{
			if (num[i] < 2)		//�󺧸������� �������� 2���̻��϶�
				break;
			x[i] = center[i].at<double>(1, 0); //�߽���ǥ
			y[i] = center[i].at<double>(1, 1);
		}
		for (int i=0;i<3;i++)
		printf("%d,%d	", x[i],y[i]);
		printf("\n");
*/

		if ((num[0] == 2) && (num[1] == 2) && (num[2] == 2))	//��ü �Ѱ����� �󺧸� �Ǿ����� while�� ����
		{
			printf("§");

			for (int i = 0; i < 3; i++)
			{
				x[i] = center[i].at<double>(1, 0);	 //�߽���ǥ
				y[i] = center[i].at<double>(1, 1);
				printf("%d,%d	", x[i], y[i]);
			}
	//			x[3] = center[0].at<double>(2, 0);	 //�߽���ǥ
	//			y[3] = center[0].at<double>(2, 1);1
	//			printf("%d,%d	", x[3], y[3]);

			printf("\n");
	//		break;
		}
		
		imshow("ss", frame);
		imshow("r", frame_[0]);
		imshow("g", frame_[1]);
		imshow("b", frame_[2]);

		escape = waitKey(1);
		if (escape == 'c') break;
	}

	destroyAllWindows();
	return;
}


int main()
{	
	int c = 0;
	unsigned char data[10] = {'a',};//������ ��Ŷ
	double s_previous = 0, e_previous = 0, s_present = 0, e_present = 0, x_s = 0, y_s = 0;

							//	e = acos((x*x + y*y - L1*L1 - L2*L2) / (2 * L1*L2)); ���ⱸ�� ����
							//	s = atan(y / x) - acos((x*x + y*y + L1*L1 - L2*L2) / (2 * L1 * sqrt(x*x + y*y)));
	
	if (!g_Dynamixel.dxl_initialize(8, 1)) {
		printf("Can`t open Dynamixel com port!\n");
		return -1;
	}

	if (!g_Comm.dxl_hal_open(4, 9600)) {
		printf("Can`t open AVR com port!\n");
		return -1;
	}//Uart��� ��Ʈ ����

	{//��ī����ġ �ʱ�ȭ
		x_s = 30;
		y_s = 20;

		e_present = acos(((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s) - L1*L1 - L2*L2) / (2 * L1*L2));
		s_present = atan((10 - x_s) / (45 - y_s)) - acos(((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s) + L1*L1 - L2*L2) / (2 * L1 * sqrt((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s))));
		
		g_Dynamixel.dxl_write_word(1, 0x1E, 2048 + s_present * 180 / PI * 4095 / 250);
		Sleep(100);
		g_Dynamixel.dxl_write_word(2, 0x1E, 2048 - e_present * 180 / PI * 4095 / 250);
		Sleep(2000);
	
	}
	video();
	
	
//------------���̳��ͼ� ��Ŷ���----------------------------------------------------------------------------------------------------------------------

	
	
	/*for (int i = 0; i<3; i++)

	{
		x_s = x[i] * 20 / xpix - 25 * 20 / xpix;
		y_s = y[i] * 20 / ypix - 23 * 20 / ypix;

		e_present = acos(((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s) - L1*L1 - L2*L2) / (2 * L1*L2));
		s_present = atan((10 - x_s) / (45 - y_s)) - acos(((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s) + L1*L1 - L2*L2) / (2 * L1 * sqrt((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s))));

		g_Dynamixel.dxl_write_word(2, 0x1E, 2048 - e_present * 180 / PI * 4092 / 250);
		Sleep(2000);
		g_Dynamixel.dxl_write_word(1, 0x1E, 2048 + s_present * 180 / PI * 4095 / 250);
		Sleep(2000);

		g_Comm.dxl_hal_tx(data, 1);// 128�� ��ü ���� �̵������� �˷���
		
	}*/
	
	for(int i=0; i<3;i++)
	{
	data[0] = 'a';
	
	x_s = x[i] * 20 / xpix - 25 * 20 / xpix;	
	y_s = y[i] * 20 / ypix - 23 * 20 / ypix;

	if (x_s > 5 && x_s < 12)
	{
		x_s += 2;

		if (y_s > 7 && y_s < 14)
			y_s += 0.5;

		else if (y_s > 14)
			y_s += 1.2;
	}

	else if (x_s > 12)
	{
		x_s += 3;

		if (y_s > 7 && y_s < 14)
			y_s += 1;

		else if (y_s > 14)
			y_s += 2;
	}

	e_present = acos(((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s) - L1*L1 - L2*L2) / (2 * L1*L2));
	s_present = atan((10 - x_s) / (45 - y_s)) - acos(((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s) + L1*L1 - L2*L2) / (2 * L1 * sqrt((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s))));
	
	g_Dynamixel.dxl_write_word(2, 0x1E, (int)(2048 - e_present * 180 / PI * 4092 / 250));
	Sleep(100);
	g_Dynamixel.dxl_write_word(1, 0x1E, (int)(2048 + s_present * 180 / PI * 4095 / 250));
	printf("%d	%d\n", (int)(2048 - e_present * 180 / PI * 4092 / 250), (int)(2048 + s_present * 180 / PI * 4095 / 250));
	Sleep(2000);

	g_Comm.dxl_hal_tx(data, 1);// 128�� ��ü ���� �̵������� �˷���

	while (1)
	{
		
		if ( (g_Comm.dxl_hal_rx(data, 1) > 0) && data[0] == 'b')
		{
			printf("rx�Ϸ�");
			break;
		}
	}

	Sleep(2000);



	g_Dynamixel.dxl_terminate();
	if (!g_Dynamixel.dxl_initialize(8, 1)) {
		printf("Can`t open Dynamixel com port!\n");
		return -1;
	}
	
	x_s = 35; 
	y_s = 25-10*c;


	e_present = acos(((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s) - L1*L1 - L2*L2) / (2 * L1*L2));
	s_present = atan((10 - x_s) / (45 - y_s)) - acos(((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s) + L1*L1 - L2*L2) / (2 * L1 * sqrt((45 - y_s)*(45 - y_s) + (10 - x_s)*(10 - x_s))));

	g_Dynamixel.dxl_write_word(1, 0x1E, 2048 + s_present * 180 / PI * 4095 / 250);
	Sleep(100);
	g_Dynamixel.dxl_write_word(2, 0x1E, 2048 - e_present * 180 / PI * 4095 / 250);
	
	Sleep(2000);

	data[0] = 'c';
	g_Comm.dxl_hal_tx(data, 1);// ��ü ���ƶ�
	Sleep(500);
	++c;
	}
//----Uart-----------------------------------------------------------------------------------------------------------------------------------------
/*
	if ( !g_Comm.dxl_hal_open(4, 9600) ){
		printf("Can`t open AVR com port!\n");
		return -1;
	}//Uart��� ��Ʈ ����

	g_Comm.dxl_hal_tx(data, 1);
*/
	return 0;
}


int make_trackbar()		// ycrcb Ʈ���� ����
{
	createTrackbar("lcrr value", "Track bar", &lcrr, 255, callback_f);
	createTrackbar("lcbr value", "Track bar", &lcbr, 255, callback_f);
	createTrackbar("hcrr value", "Track bar", &hcrr, 255, callback_f);
	createTrackbar("hcbr value", "Track bar", &hcbr, 255, callback_f);
	createTrackbar("lcrg value", "Track bar", &lcrg, 255, callback_f);
	createTrackbar("lcbg value", "Track bar", &lcbg, 255, callback_f);
	createTrackbar("hcrg value", "Track bar", &hcrg, 255, callback_f);
	createTrackbar("hcbg value", "Track bar", &hcbg, 255, callback_f);
	createTrackbar("lcrb value", "Track bar", &lcrb, 255, callback_f);
	createTrackbar("lcbb value", "Track bar", &lcbb, 255, callback_f);
	createTrackbar("hcrb value", "Track bar", &hcrb, 255, callback_f);
	createTrackbar("hcbb value", "Track bar", &hcbb, 255, callback_f);


	return 1;
}

void callback_f(int, void*)		//�ݹ� �ƹ��͵� ����
{
	return;
}

