#pragma once

#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include <QColor>
#endif

namespace Log
{
	class LOGGER_EXPORT Color
	{
	public:
		constexpr Color()
			: m_r(255)
			, m_g(255)
			, m_b(255)
			, m_consoleValue(15)
		{}
		constexpr Color(uint8_t r, uint8_t g, uint8_t b) 
			: m_r(r)
			, m_g(g)
			, m_b(b)
			, m_consoleValue(15)
		{}
		constexpr Color(uint8_t r, uint8_t g, uint8_t b, int consoleValue) 
			: m_r(r)
			, m_g(g)
			, m_b(b)
			, m_consoleValue(consoleValue)
		{}
		Color(const Color& other);

		Color& operator=(const Color& other);
		Color operator+(const Color& other) const;
		Color& operator+=(const Color& other);
		Color operator-(const Color& other) const;
		Color& operator-=(const Color& other);

		Color& operator*=(float x);
		Color operator*(float x) const;
		Color& operator/=(float x);
		Color operator/(float x) const;


		bool operator==(const Color& other) const;
		bool operator!=(const Color& other) const;

		void setRed(uint8_t r);
		void setGreen(uint8_t g);
		void setBlue(uint8_t b);
		void set(uint8_t r, uint8_t g, uint8_t b);

		void setConsoleValue(int colorValue);
		int getConsoleValue() const;

		uint8_t getRed() const;
		uint8_t getGreen() const;
		uint8_t getBlue() const;

		uint32_t getRGB() const;
		std::string getRGBStr() const;

#ifdef QT_WIDGETS_LIB
		QColor toQColor() const;
#endif

		static void setDarkMode(bool enable);
		static bool isDarkModeEnabled();

		static void setDarkModeFactor(float factor);
		static float getDarkModeFactor();

		static Color lerp(const Color& c1, const Color& c2, float x);


	private:
		uint8_t m_r;
		uint8_t m_g;
		uint8_t m_b;
		int m_consoleValue;

		static bool s_isDarkMode;
		static float s_darkModeFactor;
	};

	namespace Colors
	{
		inline constexpr Color red(255,0, 0, 0x04);
		inline constexpr Color green(0,255, 0, 0x02);
		inline constexpr Color blue(0,0, 255, 0x01);
		inline constexpr Color yellow(255, 255, 0, 0x0E);
		inline constexpr Color magenta(255, 0, 255, 0x05);
		inline constexpr Color cyan(0,255, 255, 0x03);
		inline constexpr Color white(255, 255, 255, 0x0F);
		inline constexpr Color black(0,0, 0, 0x00);
		inline constexpr Color brown(139, 69, 19, 0x06);
		inline constexpr Color lightGray(211, 211, 211, 0x07);
		inline constexpr Color darkGray(169, 169, 169, 0x08);
		inline constexpr Color lightBlue(173, 216, 230, 0x09);
		inline constexpr Color lightGreen(144, 238, 144, 0x0A);
		inline constexpr Color lightCyan(224, 255, 255, 0x0B);
		inline constexpr Color lightRed(255, 0xCC, 0xCB, 0x0C);
		inline constexpr Color lightMagenta(255, 0x77, 255, 0x0D);
		inline constexpr Color orange(255, 165, 0);

		namespace Console
		{
			namespace Foreground
			{
				inline constexpr Color red(255, 0, 0, 0x04);
				inline constexpr Color green(0, 255, 0, 0x02);
				inline constexpr Color blue(0, 0, 255, 0x01);
				inline constexpr Color yellow(255, 255, 0, 0x0E);
				inline constexpr Color magenta(255, 0, 255, 0x05);
				inline constexpr Color cyan(0, 255, 255, 0x03);
				inline constexpr Color white(255, 255, 255, 0x0F);
				inline constexpr Color black(0, 0, 0, 0x00);
				inline constexpr Color brown(139, 69, 19, 0x06);
				inline constexpr Color lightGray(211, 211, 211, 0x07);
				inline constexpr Color darkGray(169, 169, 169, 0x08);
				inline constexpr Color lightBlue(173, 216, 230, 0x09);
				inline constexpr Color lightGreen(144, 238, 144, 0x0A);
				inline constexpr Color lightCyan(224, 255, 255, 0x0B);
				inline constexpr Color lightRed(255, 0xCC, 0xCB, 0x0C);
				inline constexpr Color lightMagenta(255, 0x77, 255, 0x0D);
			}
			namespace Background
			{
				inline constexpr Color red(255, 0, 0, 0x40);
				inline constexpr Color green(0, 255, 0, 0x20);
				inline constexpr Color blue(0, 0, 255, 0x10);
				inline constexpr Color yellow(255, 255, 0, 0xE0);
				inline constexpr Color magenta(255, 0, 255, 0x50);
				inline constexpr Color cyan(0, 255, 255, 0x30);
				inline constexpr Color white(255, 255, 255, 0xF0);
				inline constexpr Color black(0, 0, 0, 0x00);
				inline constexpr Color brown(139, 69, 19, 0x60);
				inline constexpr Color lightGray(211, 211, 211, 0x70);
				inline constexpr Color darkGray(169, 169, 169, 0x80);
				inline constexpr Color lightBlue(173, 216, 230, 0x90);
				inline constexpr Color lightGreen(144, 238, 144, 0xA0);
				inline constexpr Color lightCyan(224, 255, 255, 0xB0);
				inline constexpr Color lightRed(255, 0xCC, 0xCB, 0xC0);
				inline constexpr Color lightMagenta(255, 0x77, 255, 0xD0);
			}
		}
	}
}