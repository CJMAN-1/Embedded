#include <stdio.h>			//창재 소스
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

int make_trackbar();//트랙바 만듬
void callback_f(int, void*);//매개변수에 int, void*을 왜쓰는지 잘 모르겠음

CDynamixel g_Dynamixel;
CDxl g_Comm;

void video()
{
	Mat frame_origin, frame_[3],	//[0:r][1:g][2:b]
		frame_label[3],		//라벨링 프레임, 안씀
		stats[3],		//라벨링한 프레임이 갖고있는정보
		center[3];		//라벨링한 물체 중심 좌표

	Mat frame , frame1;		//원본 복사 영상

	int admin =0;	//색깔검출 안될때
	int num[3];		//라벨링한 물체의 수
//	int area, left, top, width, height;		//박스 만들때
		//중심 x,y좌표

	char escape = 0;	//while문 나갈때

	int buffer[5] = { 0, };
	VideoCapture cap(0);
//	namedWindow("Track bar",0);	//색깔범위 다시 잡을때 admin까지 활성화. 
//	admin = make_trackbar();

	while (1)
	{
		cap >> frame1;
		
		frame = frame1;

		Rect rect(180, 120, 300, 300);//관심영역 지정
		frame = frame(rect);

		cvtColor(frame, frame, CV_BGR2YCrCb);
		GaussianBlur(frame, frame, Size(11, 11), 0, 0);

		if (admin != 1) //색검출 잘될때
		{
			inRange(frame, Scalar(lowY, 135, lowcb), Scalar(highY, highcr, 111), frame_[0]);
			inRange(frame, Scalar(lowY, lowcr, lowcb), Scalar(highY, 97, 130), frame_[1]);
			inRange(frame, Scalar(lowY, lowcr, 135), Scalar(highY, 106, highcb), frame_[2]);
		}

		else	//색검출 안될때 색깔범위 지정
		
		{
			inRange(frame, Scalar(lowY, lcrr, lcbr), Scalar(highY, hcrr, hcbr), frame_[0]);
			inRange(frame, Scalar(lowY, lcrg, lcbg), Scalar(highY, hcrg, hcbg), frame_[1]);
			inRange(frame, Scalar(lowY, lcrb, lcbb), Scalar(highY, hcrb, hcbb), frame_[2]);
		}

		for(int i=0;i<3;i++)	//라벨링 프레임 순서는 rgb순
			num[i] = connectedComponentsWithStats(frame_[i], frame_label[i], stats[i], center[i]);

		for (int i=0;i<3;i++)	//rgb 라벨링된 물체 중심에 네모 치기
		{
			for (int j = 1; j < num[i]; j++) 
			{
//라벨링크기대로area = stats[0].at<int>(j, CC_STAT_AREA);
	//			left = stats[0].at<int>(j, CC_STAT_LEFT);
	//			top = stats[0].at<int>(j, CC_STAT_TOP);
	//			width = stats[0].at<int>(j, CC_STAT_WIDTH);
	//			height = stats[0].at<int>(j, CC_STAT_HEIGHT);

				x[i] = center[i].at<double>(j, 0); //중심좌표
				y[i] = center[i].at<double>(j, 1);

				rectangle(frame,Point(x[i] -5, y[i] -5),
								Point(x[i] +5, y[i] +5),
								Scalar(0, 0, 255), 1);
			}
		}

/*			중심좌표 출력할때만 씀
		for (int i = 0; i < 3; i++)
		{
			if (num[i] < 2)		//라벨링갯수가 바탕포함 2개이상일때
				break;
			x[i] = center[i].at<double>(1, 0); //중심좌표
			y[i] = center[i].at<double>(1, 1);
		}
		for (int i=0;i<3;i++)
		printf("%d,%d	", x[i],y[i]);
		printf("\n");
*/

		if ((num[0] == 2) && (num[1] == 2) && (num[2] == 2))	//물체 한개씩만 라벨링 되었을때 while문 나감
		{
			printf("짠");

			for (int i = 0; i < 3; i++)
			{
				x[i] = center[i].at<double>(1, 0);	 //중심좌표
				y[i] = center[i].at<double>(1, 1);
				printf("%d,%d	", x[i], y[i]);
			}
	//			x[3] = center[0].at<double>(2, 0);	 //중심좌표
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
	unsigned char data[10] = {'a',};//데이터 패킷
	double s_previous = 0, e_previous = 0, s_present = 0, e_present = 0, x_s = 0, y_s = 0;

							//	e = acos((x*x + y*y - L1*L1 - L2*L2) / (2 * L1*L2)); 역기구학 수식
							//	s = atan(y / x) - acos((x*x + y*y + L1*L1 - L2*L2) / (2 * L1 * sqrt(x*x + y*y)));
	
	if (!g_Dynamixel.dxl_initialize(8, 1)) {
		printf("Can`t open Dynamixel com port!\n");
		return -1;
	}

	if (!g_Comm.dxl_hal_open(4, 9600)) {
		printf("Can`t open AVR com port!\n");
		return -1;
	}//Uart통신 포트 열기

	{//스카라위치 초기화
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
	
	
//------------다이나믹셀 패킷통신----------------------------------------------------------------------------------------------------------------------

	
	
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

		g_Comm.dxl_hal_tx(data, 1);// 128에 물체 위로 이동했음을 알려줌
		
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

	g_Comm.dxl_hal_tx(data, 1);// 128에 물체 위로 이동했음을 알려줌

	while (1)
	{
		
		if ( (g_Comm.dxl_hal_rx(data, 1) > 0) && data[0] == 'b')
		{
			printf("rx완료");
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
	g_Comm.dxl_hal_tx(data, 1);// 물체 놓아라
	Sleep(500);
	++c;
	}
//----Uart-----------------------------------------------------------------------------------------------------------------------------------------
/*
	if ( !g_Comm.dxl_hal_open(4, 9600) ){
		printf("Can`t open AVR com port!\n");
		return -1;
	}//Uart통신 포트 열기

	g_Comm.dxl_hal_tx(data, 1);
*/
	return 0;
}


int make_trackbar()		// ycrcb 트랙바 만듬
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

void callback_f(int, void*)		//콜백 아무것도 안함
{
	return;
}

