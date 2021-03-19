#pragma once

#define MAX_KEYS 1024 

class Keyboard
{
private:
	Keyboard()
	{
		for (int i = 0; i < MAX_KEYS; i++)
		{
			m_Keys[i] = false;
		}
	}

public:
	static Keyboard& getInstance();
	inline void setKey(unsigned int key, bool isPressed)
	{
		m_Keys[key] = isPressed;
	}

	inline bool getKey(unsigned int key)
	{
		return m_Keys[key];
	}

private:
	static Keyboard m_Instance; // Singletone
	bool m_Keys[MAX_KEYS];
};

