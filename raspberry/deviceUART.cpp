#include "device.hpp"

struct DeviceUARTData
{
	const char* fileName;
	int deviceId = 0;
};



bool startDeviceUART(DeviceData* data)
{
	DeviceUARTData* dataUART = (DeviceUARTData*)data->data;

	dataUART->deviceId = open(dataUART->fileName, O_RDWR | O_NONBLOCK); // Открывает последовательный порт

	if(dataUART->deviceId < 0)
	{
		if (data->showErrorLogs) logError("Не удалось подключиться к COM-порту");
		return false;
	}

	struct termios options;

	options.c_iflag = 0;
	options.c_oflag = 0;
	options.c_cflag = CS8 | CREAD | HUPCL | CLOCAL;
	options.c_lflag = NOFLSH;

	// Установить скорость соединения (2000000 бодов в секунду)
	cfsetispeed(&options, B2000000);
	cfsetospeed(&options, B2000000);

	tcsetattr(dataUART->deviceId, TCSANOW, &options); // Применить новые настройки порта

	return true;
}

void sendDeviceUART(DeviceData* data, sint id, sint command, int cData)
{
	DeviceUARTData* dataUART = (DeviceUARTData*)data->data;

	if (dataUART->deviceId <= 0)
	{
		if (data->showErrorLogs) logError("Ошибка при отправке сообщения на arduino");
		return;
	}

	char buffer[9];
	buffer[0] = 'F';
	memcpy(buffer + 1, &id, 2);
	memcpy(buffer + 3, &command, 2);
	memcpy(buffer + 5, &cData, 4);

	if (write(dataUART->deviceId, buffer, 9) < 9)
	{
		if (data->showErrorLogs) logError("Ошибка при отправке сообщения на arduino");
		return;
	}

	ioctl(dataUART->deviceId, TCSBRK, 1);
}

bool stopDeviceUART(DeviceData* data)
{
	DeviceUARTData* dataUART = (DeviceUARTData*)data->data;

	close(dataUART->deviceId);
	dataUART->deviceId = 0;

	return true;
}

void updateDeviceUART(DeviceData* data)
{
	DeviceUARTData* dataUART = (DeviceUARTData*)data->data;

	char cData;

	if (read(dataUART->deviceId, &cData, 1) != 1) return;

	processChar(data, cData);
}

DeviceUART::DeviceUART()
{
	dataUART = shared_ptr<DeviceUARTData>(new DeviceUARTData);
	data->data = dataUART.get();
	data->startF = startDeviceUART;
	data->stopF = stopDeviceUART;
	data->sendF = sendDeviceUART;
	data->thrDeviceData.updateF = updateDeviceUART;
}

DeviceUART::DeviceUART(const char* fileName) : DeviceUART()
{
	dataUART->fileName = fileName;
}

DeviceUART::~DeviceUART()
{
	if (dataUART.use_count() != 1) return;

	// Деструктор
}
