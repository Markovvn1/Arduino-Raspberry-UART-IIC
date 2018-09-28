#pragma once

#include <mutex>
#include <pthread.h>
#include <memory>

#include "timer.hpp"

using namespace std;

typedef short int sint;
typedef int (*DeviceEventOnRead)(sint command, int data, bool answer);

struct DeviceData;
struct DeviceI2CData;
struct DeviceUARTData;

class Device
{
protected:
	shared_ptr<DeviceData> data;

public:
	Device();
	virtual ~Device();

	bool isActive();

	bool start(int tryTime = TRY_NO);
	bool stop();

	int sendMessage(sint command, int data, bool wait = true);

	void setShowErrorLogs(bool value);
	void setOnRead(DeviceEventOnRead eventOnRead);
};



class DeviceI2C : public Device
{
protected:
	shared_ptr<DeviceI2CData> dataI2C;

public:
	DeviceI2C();
	DeviceI2C(const int addr, const char* fileName = "/dev/i2c-0");
	~DeviceI2C();
};



class DeviceUART : public Device
{
protected:
	shared_ptr<DeviceUARTData> dataUART;

public:
	DeviceUART();
	DeviceUART(const char* fileName);
	~DeviceUART();
};
