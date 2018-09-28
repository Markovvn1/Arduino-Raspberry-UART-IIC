#include "AM_v2.h"

DeviceUART dSerial = DeviceUART(Serial);
DeviceWire dWire = DeviceWire();

DeviceUART::DeviceUART(HardwareSerial &serial)
{
	randomSeed(micros());
	this->serial = &serial;
}

DeviceUART::~DeviceUART()
{
	if (message != NULL) delete [] message;
}

void DeviceUART::send(int id, int command, long data)
{
	serial->write('F');
	serial->write((char*)&id, 2);
	serial->write((char*)&command, 2);
	serial->write((char*)&data, 4);
}

void DeviceUART::init()
{
	if (serial == NULL) return;

	if (message == NULL)
	{
		message = new char[8];
	}

	serial->end();
	serial->begin(2000000);
}

long DeviceUART::sendMessage(int command, long data, bool wait)
{
	if (serial == NULL) return 0;
	if (command < 0) return 0;

	int id = random(-32768, 32768);

	send(id, command, data);

	if (!wait) return 0;

	long endTime = millis() + MAX_TIME_WAIT;

	while ((!update() || this->msg.id != id || this->msg.command != -command - 1) && (millis() < endTime));

	return msg.data;
}

bool DeviceUART::update()
{
	if (serial == NULL) return false;
	if (serial->available() <= 0) return false;

	char data = serial->read();

	if (messageI < 0)
	{
		if (data == 'F') messageI = 0;

		return false;
	}

	message[messageI] = data;
	messageI++;

	if (messageI == 8)
	{
		messageI = -1;

		msg.id      = *((int*) (message + 0));
		msg.command = *((int*) (message + 2));
		msg.data    = *((long*)(message + 4));

		long result = 0;

		if (eventOnRead)
		{
			if (msg.command >= 0)
				result = eventOnRead(msg.command, msg.data, false);
			else
				result = eventOnRead(-msg.command - 1, msg.data, true);
		}

		// Если пришло сообщение-ответ
		if (msg.command < 0) return true;

		send(msg.id, -msg.command - 1, result);
	}

	return false;
}

void DeviceUART::setOnRead(EventOnRead eventOnRead)
{
	this->eventOnRead = eventOnRead;
}




void __DWEventReceive(int count)
{
	dWire.__onReceive(count);
}

void __DWEventRequest()
{
	dWire.__onRequest();
}




DeviceWire::DeviceWire()
{
	randomSeed(micros());
}

DeviceWire::~DeviceWire()
{
	if (message != NULL) delete [] message;
}

void DeviceWire::__onReceive(int count)
{
	for (int i = 0; i < count; i++)
	{
		char data = Wire.read();

		if (messageI < 0)
		{
			if (data == 'F') messageI = 0;

			continue;
		}

		message[messageI] = data;
		messageI++;

		if (messageI == 8)
		{
			messageI = -1;

			msg.id      = *((int*) (message + 0));
			msg.command = *((int*) (message + 2));
			msg.data    = *((long*)(message + 4));
		}

	}
}

void DeviceWire::__onRequest()
{
	long result = 0;
	if (eventOnRead)
	{
		if (msg.command >= 0)
			result = eventOnRead(msg.command, msg.data, false);
		else
			result = eventOnRead(-msg.command - 1, msg.data, true);
	}

	send(msg.id, -msg.command - 1, result);
}

void DeviceWire::send(int id, int command, long data)
{
	Wire.write('F');
	Wire.write((char*)&id, 2);
	Wire.write((char*)&command, 2);
	Wire.write((char*)&data, 4);
}

void DeviceWire::init(int addr)
{
	if (message == NULL)
	{
		message = new char[8];
	}

	Wire.begin(addr);
	Wire.onReceive(__DWEventReceive);
	Wire.onRequest(__DWEventRequest);
}

void DeviceWire::update()
{

}

void DeviceWire::setOnRead(EventOnRead eventOnRead)
{
	this->eventOnRead = eventOnRead;
}
