#include "LogColor.h"
#include "LogLevel.h"
#include "LogMessage.h"

namespace Log
{


	/*
The different color codes are

0		BLACK
1		BLUE
2		GREEN
3		CYAN
4		RED
5		MAGENTA
6   BROWN
7   LIGHTGRAY
8   DARKGRAY
9   LIGHTBLUE
10  LIGHTGREEN
11  LIGHTCYAN
12  LIGHTRED
13  LIGHTMAGENTA
14		YELLOW
15		WHITE
*/


	

	bool Color::s_isDarkMode = false;
	float Color::s_darkModeFactor = 0.7f;


	Color::Color(const Color& other)
		: m_r(other.m_r)
		, m_g(other.m_g)
		, m_b(other.m_b)
		, m_consoleValue(other.m_consoleValue)
	{

	}

	Color& Color::operator=(const Color& other)
	{
		m_r = other.m_r;
		m_g = other.m_g;
		m_b = other.m_b;
		m_consoleValue = other.m_consoleValue;
		return *this;
	}
	Color Color::operator+(const Color& other) const
	{
		unsigned int r = m_r + other.m_r;
		unsigned int g = m_g + other.m_g;
		unsigned int b = m_b + other.m_b;
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;

		return Color((uint8_t)r, (uint8_t)g, (uint8_t)b);
	}
	Color& Color::operator+=(const Color& other)
	{
		unsigned int r = m_r + other.m_r;
		unsigned int g = m_g + other.m_g;
		unsigned int b = m_b + other.m_b;
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;

		m_r = (uint8_t)r;
		m_g = (uint8_t)g;
		m_b = (uint8_t)b;
		return *this;
	}
	Color Color::operator-(const Color& other) const
	{
		int r = m_r - other.m_r;
		int g = m_g - other.m_g;
		int b = m_b - other.m_b;
		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;

		return Color((uint8_t)r, (uint8_t)g, (uint8_t)b);
	}
	Color& Color::operator-=(const Color& other)
	{
		int r = m_r - other.m_r;
		int g = m_g - other.m_g;
		int b = m_b - other.m_b;
		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;

		m_r = (uint8_t)r;
		m_g = (uint8_t)g;
		m_b = (uint8_t)b;
		return *this;
	}
	Color& Color::operator*=(float x)
	{
		m_r *= x;
		m_g *= x;
		m_b *= x;
		if (m_r < 0) m_r = 0; else if (m_r > 255) m_r = 255;
		if (m_g < 0) m_g = 0; else if (m_g > 255) m_g = 255;
		if (m_b < 0) m_b = 0; else if (m_b > 255) m_b = 255;
		return *this;
	}
	Color Color::operator*(float x) const
	{
		float r = m_r * x;
		float g = m_g * x;
		float b = m_b * x;
		if (r < 0) r = 0; else if (r > 255) r = 255;
		if (g < 0) g = 0; else if (g > 255) g = 255;
		if (b < 0) b = 0; else if (b > 255) b = 255;

		return Color((uint8_t)r, (uint8_t)g, (uint8_t)b);
	
	}
	Color& Color::operator/=(float x)
	{
		m_r /= x;
		m_g /= x;
		m_b /= x;
		if (m_r < 0) m_r = 0; else if (m_r > 255) m_r = 255;
		if (m_g < 0) m_g = 0; else if (m_g > 255) m_g = 255;
		if (m_b < 0) m_b = 0; else if (m_b > 255) m_b = 255;
		return *this;
	
	}
	Color Color::operator/(float x) const
	{
		float r = m_r / x;
		float g = m_g / x;
		float b = m_b / x;
		if (r < 0) r = 0; else if (r > 255) r = 255;
		if (g < 0) g = 0; else if (g > 255) g = 255;
		if (b < 0) b = 0; else if (b > 255) b = 255;

		return Color((uint8_t)r, (uint8_t)g, (uint8_t)b);
	
	}

	bool Color::operator==(const Color& other) const
	{
		if (m_consoleValue != m_consoleValue)
			return false;
		int x = (m_r & other.m_r) + (m_g & other.m_g) + (m_b & other.m_g);
		return x == 3;
	}
	bool Color::operator!=(const Color& other) const
	{
		if (m_consoleValue != other.m_consoleValue)
			return true;
		int x = (m_r & other.m_r) + (m_g & other.m_g) + (m_b & other.m_g);
		return x != 3;
	}

	void Color::setRed(uint8_t r)
	{
		m_r = r;
	}
	void Color::setGreen(uint8_t g)
	{
		m_g = g;
	}
	void Color::setBlue(uint8_t b)
	{
		m_b = b;
	}
	void Color::set(uint8_t r, uint8_t g, uint8_t b)
	{
		m_r = r;
		m_g = g;
		m_b = b;
	}
	void Color::setConsoleValue(int colorValue)
	{
		m_consoleValue = colorValue;
	}
	int Color::getConsoleValue() const
	{
		if (m_consoleValue == 0)
			return Colors::white.getConsoleValue();
		return m_consoleValue;
	}

	uint8_t Color::getRed() const
	{
		return m_r;
	}
	uint8_t Color::getGreen() const
	{
		return m_g;
	}
	uint8_t Color::getBlue() const
	{
		return m_b;
	}
	uint32_t Color::getRGB() const
	{
		return ((uint32_t)m_r << 16) | ((uint32_t)m_g << 8) | (uint32_t)m_b;
	}
	std::string Color::getRGBStr() const
	{
		char hex[8];
		sprintf_s(hex, "#%02X%02X%02X", m_r, m_g, m_b);
		return std::string(hex);
	}

#ifdef QT_WIDGETS_LIB
	QColor Color::toQColor() const
	{
		if(!s_isDarkMode)
			return QColor(m_r, m_g, m_b);

		//int sum = m_r + m_g + m_b;
		//if(sum > 240*3)
		//	return QColor(50, 50, 50);
		// Darken the color by reducing its brightness
		qreal h, s, v;
		QColor(m_r, m_g, m_b).getHsvF(&h, &s, &v);
		v *= s_darkModeFactor; // Reduce brightness to 70%
		//v = 0.5*v;
		return QColor::fromHsvF(h, s, v);
	}
#endif

	void Color::setDarkMode(bool enable)
	{
		s_isDarkMode = enable;
	}
	bool Color::isDarkModeEnabled()
	{
		return s_isDarkMode;
	}

	void Color::setDarkModeFactor(float factor)
	{
		s_darkModeFactor = factor;
	}
	float Color::getDarkModeFactor()
	{
		return s_darkModeFactor;
	}


	Color Color::lerp(const Color& c1, const Color& c2, float x)
	{
		float c1r = c1.m_r;
		float c1g = c1.m_g;
		float c1b = c1.m_b;

		float c2r = c2.m_r;
		float c2g = c2.m_g;
		float c2b = c2.m_b;


		float r = c1r + x * (c2r - c1r);
		float g = c1g + x * (c2g - c1g);
		float b = c1b + x * (c2b - c1b);

		if (r < 0) r = 0; else if (r > 255) r = 255;
		if (g < 0) g = 0; else if (g > 255) g = 255;
		if (b < 0) b = 0; else if (b > 255) b = 255;

		return Color((uint8_t)r, (uint8_t)g, (uint8_t)b);
	}
}