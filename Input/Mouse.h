#pragma once
#define MAX_BUTTONS 32

class Mouse
{
private:
	Mouse()
	{
		for (int i = 0; i < MAX_BUTTONS; i++)
		{
			m_Buttons[i] = false;
		}
	}
public:
	static Mouse& getInstance();

	inline void setX(double x) { mx = x; }
	inline void setY(double y) { my = y; }

	inline double getX() { return mx; }
	inline double getY() { return my; }

	inline void setButton(unsigned int button, bool isPressed)
	{
		m_Buttons[button] = isPressed;
	}
	inline bool getButton(unsigned int button)
	{
		return m_Buttons[button];
	}

private:
	static Mouse m_Instance; // Singletone
	bool m_Buttons[MAX_BUTTONS];
	double mx, my;
};