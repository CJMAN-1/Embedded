#ifndef _DYNAMIXEL_HAL_HEADER
#define _DYNAMIXEL_HAL_HEADER

#include <Windows.h>


class CDxl {

public:
	CDxl() :
		ghSerial_Handle(INVALID_HANDLE_VALUE),
		gfByteTransTime(0.f),
		gfRcvWaitTime(0.f)
	{

	}

	HANDLE ghSerial_Handle; // Serial port handle
	float gfByteTransTime;
	float gfRcvWaitTime;
	LARGE_INTEGER gStartTime;

	int dxl_hal_open(int devIndex, float baudrate);
	void dxl_hal_close();
	void dxl_hal_clear();
	int dxl_hal_tx(unsigned char *pPacket, int numPacket);
	int dxl_hal_rx(unsigned char *pPacket, int numPacket);
	void dxl_hal_set_timeout(int NumRcvByte);
	int dxl_hal_timeout();

};

#endif
