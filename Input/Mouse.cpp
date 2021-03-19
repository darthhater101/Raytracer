#include "Mouse.h"

Mouse Mouse::m_Instance;

Mouse& Mouse::getInstance()
{
	return m_Instance;
}