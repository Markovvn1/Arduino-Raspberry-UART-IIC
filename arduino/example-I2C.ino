#include <AM_v2.h>

// Пример обработки сообщения на arduino через I2C

long onRead(int command, long data, bool answer)
{
	if (answer) return 0; // Ответы игнорируем
	
	// Пример обработки команды
	if (command == 130 && data == -13239485)
	{
		return -81726;
	}
	
	return 12;
}

void setup()
{
	dWire.init();
	dWire.setOnRead(onRead);
}

void loop()
{
	dWire.update();
}
