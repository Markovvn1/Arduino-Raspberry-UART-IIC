#include "device.hpp"

#include <unistd.h>

void* deviceProcess(void *ptr)
{
	ThrDeviceData &deviceData = *((ThrDeviceData*)ptr);

	while (!deviceData.needStop)
	{
		deviceData.updateF(deviceData.data);
	}

	deviceData.needStop = false;

	return NULL;
}




Device::Device()
{
	data = shared_ptr<DeviceData>(new DeviceData);

	srand(time(NULL));
	data->message = new char[8];

	data->showErrorLogs = true;
	data->thrDeviceData.data = data.get();
	data->deviceThread = 0;
}

Device::~Device()
{
	if (data.use_count() != 1) return;

	stop();
	delete [] data->message;
}

void processChar(DeviceData* data, char cData)
{
	if (data->messageI < 0)
	{
		if (cData == 'F') data->messageI = 0;

		return;
	}

	data->message[(int)data->messageI] = cData;
	data->messageI++;

	if (data->messageI == 8)
	{
		data->messageI = -1;

		data->lock_device.lock();
		data->msg.id      = *((sint*)(data->message + 0));
		data->msg.command = *((sint*)(data->message + 2));
		data->msg.data    = *((int*) (data->message + 4));
		data->lock_device.unlock();

		int result = 0;

		if (data->eventOnRead)
		{
			if (data->msg.command >= 0)
				result = data->eventOnRead(data->msg.command, data->msg.data, false);
			else
				result = data->eventOnRead(-data->msg.command - 1, data->msg.data, true);
		}

		// Если пришло не сообщение-ответ, нужно ответить
		if (data->msg.command >= 0)
		{
			data->lock_device.lock();
			data->sendF(data, data->msg.id, -data->msg.command - 1, result);
			data->lock_device.unlock();
		}
	}
}

bool Device::isActive()
{
	return data->active;
}

bool Device::start(int tryTime)
{
	if (isActive()) return true;

	long long endTime = getCurrentTime() + tryTime;

	do
	{
		data->lock_device.lock();
		if (data->startF)
			data->active = data->startF(data.get());
		data->lock_device.unlock();

		if (!isActive()) continue;

		data->thrDeviceData.needStop = false;
		pthread_create(&data->deviceThread, NULL, deviceProcess, &data->thrDeviceData);

		for (int i = 0; i < 100; i++)
		{
			if (sendMessage(32767, -13239485) == -81726)
			{
				data->firstMessage = false;
				return true;
			}
			usleep(100000);
		}

		stop();

		usleep(100000);
	}
	while ((tryTime == TRY_INFINITY || getCurrentTime() < endTime));

	return false;
}

bool Device::stop()
{
	if (!isActive()) return true;

	data->thrDeviceData.needStop = true;

	while (data->thrDeviceData.needStop)
	{
		usleep(1000);
	}

	data->lock_device.lock();
	data->active = !data->stopF(data.get());
	data->lock_device.unlock();

	return !isActive();
}

int Device::sendMessage(sint command, int cData, bool wait)
{
	if ((command < 0 || command == 32767) && !data->firstMessage)
	{
		logError("Невозможно отправить команду с номером " << command << " (" << command << " < 0 || " << command << " == 32767)");
		return 0;
	}

	sint id = rand() * (rand() % 2 ? 1 : -1);

	data->lock_device.lock();
	data->sendF(data.get(), id, command, cData);
	data->lock_device.unlock();

	if (!wait) return 0;

	long long endTime = getCurrentTime() + MAX_TIME_WAIT;

	while (getCurrentTime() < endTime)
	{
		data->lock_device.lock();
		if (data->msg.id == id || data->msg.command == -command - 1)
		{
			data->lock_device.unlock();
			return data->msg.data;
		}
		data->lock_device.unlock();
		usleep(100);
	}

	return 0;
}

void Device::setShowErrorLogs(bool value)
{
	data->showErrorLogs = value;
}

void Device::setOnRead(DeviceEventOnRead eventOnRead)
{
	data->lock_device.lock();

	data->eventOnRead = eventOnRead;

	data->lock_device.unlock();
}
