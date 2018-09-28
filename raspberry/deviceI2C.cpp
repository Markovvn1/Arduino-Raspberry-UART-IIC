#include "device.hpp"

struct DeviceI2CData
{
	const char* fileName;
	int addr;

	int deviceId = 0;
};



bool startDeviceI2C(DeviceData* data)
{
	DeviceI2CData* dataI2C = (DeviceI2CData*)data->data;

	dataI2C->deviceId = open(dataI2C->fileName, O_RDWR | O_NONBLOCK);

	if(dataI2C->deviceId < 0)
	{
		logError("Не удалось подключится к устройству " << dataI2C->fileName);
		return false;
	}

	if (ioctl(dataI2C->deviceId, I2C_SLAVE, dataI2C->addr) < 0)
	{
		logError("Не удалось подключиться к I2C устройству с адресом " << dataI2C->addr);
		return false;
	}

	return true;
}

void sendDeviceI2C(DeviceData* data, sint id, sint command, int cData)
{
	DeviceI2CData* dataI2C = (DeviceI2CData*)data->data;

	if (dataI2C->deviceId <= 0)
	{
		if (data->showErrorLogs) logError("Ошибка при отправке сообщения на arduino");
		return;
	}

	char buffer[9];
	buffer[0] = 'F';
	memcpy(buffer + 1, &id, 2);
	memcpy(buffer + 3, &command, 2);
	memcpy(buffer + 5, &cData, 4);

	if (write(dataI2C->deviceId, buffer, 9) < 9)
	{
		if (data->showErrorLogs) logError("Ошибка при отправке сообщения на arduino");
		return;
	}

	ioctl(dataI2C->deviceId, TCSBRK, 1);
}

bool stopDeviceI2C(DeviceData* data)
{
	DeviceI2CData* dataI2C = (DeviceI2CData*)data->data;

	close(dataI2C->deviceId);
	dataI2C->deviceId = 0;

	return true;
}

void updateDeviceI2C(DeviceData* data)
{
	DeviceI2CData* dataI2C = (DeviceI2CData*)data->data;

	char cData;

	if (read(dataI2C->deviceId, &cData, 1) != 1) return;

	processChar(data, cData);
}

DeviceI2C::DeviceI2C()
{
	dataI2C = shared_ptr<DeviceI2CData>(new DeviceI2CData);
	data->data = dataI2C.get();
	data->startF = startDeviceI2C;
	data->stopF = stopDeviceI2C;
	data->sendF = sendDeviceI2C;
	data->thrDeviceData.updateF = updateDeviceI2C;
}

DeviceI2C::DeviceI2C(const int addr, const char* fileName) : DeviceI2C()
{
	dataI2C->addr = addr;
	dataI2C->fileName = fileName;
}

DeviceI2C::~DeviceI2C()
{
	if (dataI2C.use_count() != 1) return;

	// Деструктор
}
