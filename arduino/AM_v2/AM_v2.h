#pragma once

#include <Arduino.h>
#include <Wire.h>

#define MAX_TIME_WAIT 400

/*
	1. Добавить возможность Wire быть мастером
*/

struct MessageData
{
	int id = 0;
	int command = 0;
	long data = 0;
};

typedef long (*EventOnRead)(int command, long data, bool answer);

class DeviceUART
{
protected:
	EventOnRead eventOnRead;
	HardwareSerial* serial = NULL;

	char* message = NULL;
	char messageI = -1;

	MessageData msg;

	void send(int id, int command, long data);

public:
	DeviceUART() {};
	DeviceUART(HardwareSerial &serial);
	~DeviceUART();

	void init();

	long sendMessage(int command, long data, bool wait = true);

	bool update();

	void setOnRead(EventOnRead eventOnRead);
};

class DeviceWire
{
protected:
	EventOnRead eventOnRead;
	HardwareSerial* serial = NULL;

	char* message = NULL;
	char messageI = -1;

	MessageData msg;

	void send(int id, int command, long data);

public:
	DeviceWire();
	~DeviceWire();

	void __onReceive(int count);
	void __onRequest();

	void init(int addr);

	void update();

	void setOnRead(EventOnRead eventOnRead);
};


extern DeviceUART dSerial;
// static DeviceUART dSerial1 = DeviceUART(Serial1);
// static DeviceUART dSerial2 = DeviceUART(Serial2);
// static DeviceUART dSerial3 = DeviceUART(Serial3);

extern DeviceWire dWire;
