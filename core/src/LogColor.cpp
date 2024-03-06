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


	const Color Color::red(255, 0, 0, 0x04);
	const Color Color::green(0, 255, 0, 0x02);
	const Color Color::blue(0, 0, 255, 0x01);
	const Color Color::yellow(255, 255, 0, 0x0E);
	const Color Color::magenta(255, 0, 255, 0x05);
	const Color Color::cyan(0, 255, 255, 0x03);
	const Color Color::white(255, 255, 255, 0x0F);
	const Color Color::black(0, 0, 0, 0x00);
	const Color Color::brown(139, 69, 19, 0x06);
	const Color Color::lightGray(211, 211, 211, 0x07);
	const Color Color::darkGray(169, 169, 169, 0x08);
	const Color Color::lightBlue(173, 216, 230, 0x09);
	const Color Color::lightGreen(144, 238, 144, 0x0A);
	const Color Color::lightCyan(224, 255, 255, 0x0B);
	const Color Color::lightRed(255, 0xCC, 0xCB, 0x0C);
	const Color Color::lightMagenta(255, 0x77, 255, 0x0D);

	const Color Color::orange(255, 165, 0);

	const Color Color::Console::Foreground::red(255, 0, 0, 0x04);
	const Color Color::Console::Foreground::green(0, 255, 0, 0x02);
	const Color Color::Console::Foreground::blue(0, 0, 255, 0x01);
	const Color Color::Console::Foreground::yellow(255, 255, 0, 0x0E);
	const Color Color::Console::Foreground::magenta(255, 0, 255, 0x05);
	const Color Color::Console::Foreground::cyan(0, 255, 255, 0x03);
	const Color Color::Console::Foreground::white(255, 255, 255, 0x0F);
	const Color Color::Console::Foreground::black(0, 0, 0, 0x00);
	const Color Color::Console::Foreground::brown(139, 69, 19, 0x06);
	const Color Color::Console::Foreground::lightGray(211, 211, 211, 0x07);
	const Color Color::Console::Foreground::darkGray(169, 169, 169, 0x08);
	const Color Color::Console::Foreground::lightBlue(173, 216, 230, 0x09);
	const Color Color::Console::Foreground::lightGreen(144, 238, 144, 0x0A);
	const Color Color::Console::Foreground::lightCyan(224, 255, 255, 0x0B);
	const Color Color::Console::Foreground::lightRed(255, 0xCC, 0xCB, 0x0C);
	const Color Color::Console::Foreground::lightMagenta(255, 0x77, 255, 0x0D);

	const Color Color::Console::Background::red(255, 0, 0, 0x40);
	const Color Color::Console::Background::green(0, 255, 0, 0x20);
	const Color Color::Console::Background::blue(0, 0, 255, 0x10);
	const Color Color::Console::Background::yellow(255, 255, 0, 0xE0);
	const Color Color::Console::Background::magenta(255, 0, 255, 0x50);
	const Color Color::Console::Background::cyan(0, 255, 255, 0x30);
	const Color Color::Console::Background::white(255, 255, 255, 0xF0);
	const Color Color::Console::Background::black(0, 0, 0, 0x00);
	const Color Color::Console::Background::brown(139, 69, 19, 0x60);
	const Color Color::Console::Background::lightGray(211, 211, 211, 0x70);
	const Color Color::Console::Background::darkGray(169, 169, 169, 0x80);
	const Color Color::Console::Background::lightBlue(173, 216, 230, 0x90);
	const Color Color::Console::Background::lightGreen(144, 238, 144, 0xA0);
	const Color Color::Console::Background::lightCyan(224, 255, 255, 0xB0);
	const Color Color::Console::Background::lightRed(255, 0xCC, 0xCB, 0xC0);
	const Color Color::Console::Background::lightMagenta(255, 0x77, 255, 0xD0);


	LevelColors Message::s_levelColors = {
		Color::cyan,		// trace,
		Color::magenta,		// debug,
		Color::white,		// info,
		Color::yellow,		// warning,
		Color::red,			// error,
		Color::green		// custom
	};

	bool Color::s_isDarkMode = false;
	float Color::s_darkModeFactor = 0.7f;

	Color::Color()
		: m_r(255)
		, m_g(255)
		, m_b(255)
		, m_consoleValue(15)
	{

	}
	Color::Color(uint8_t r, uint8_t g, uint8_t b)
		: m_r(r)
		, m_g(g)
		, m_b(b)
		, m_consoleValue(15)
	{

	}
	Color::Color(uint8_t r, uint8_t g, uint8_t b, int consoleValue)
		: m_r(r)
		, m_g(g)
		, m_b(b)
		, m_consoleValue(consoleValue)
	{

	}


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

		return Color(r, g, b);
	}
	Color& Color::operator+=(const Color& other)
	{
		unsigned int r = m_r + other.m_r;
		unsigned int g = m_g + other.m_g;
		unsigned int b = m_b + other.m_b;
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;

		m_r = r;
		m_g = g;
		m_b = b;
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

		return Color(r, g, b);
	}
	Color& Color::operator-=(const Color& other)
	{
		int r = m_r - other.m_r;
		int g = m_g - other.m_g;
		int b = m_b - other.m_b;
		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;

		m_r = r;
		m_g = g;
		m_b = b;
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

		return Color(r, g, b);
	
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

		return Color(r, g, b);
	
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
		sprintf(hex, "#%02X%02X%02X", m_r, m_g, m_b);
		return std::string(hex);
	}

#ifdef LOGGER_QT
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

		return Color(r, g, b);
	}
}