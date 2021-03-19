#include "Keyboard.h"

Keyboard Keyboard::m_Instance;

Keyboard& Keyboard::getInstance()
{
	return Keyboard::m_Instance;
}