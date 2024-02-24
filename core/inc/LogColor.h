#pragma once

#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QColor>
#endif

namespace Log
{
	class LOGGER_EXPORT Color
	{
	public:
		Color(uint8_t r, uint8_t g, uint8_t b);
		Color(uint8_t r, uint8_t g, uint8_t b, int consoleValue);
		Color(const Color& other);

		Color& operator=(const Color& other);
		Color operator+(const Color& other) const;
		Color& operator+=(const Color& other);
		Color operator-(const Color& other) const;
		Color& operator-=(const Color& other);

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

#ifdef LOGGER_QT
		QColor toQColor() const;
#endif

		static Color lerp(const Color& c1, const Color& c2, float x);

		static const Color red;
		static const Color green;
		static const Color blue;
		static const Color yellow;
		static const Color magenta;
		static const Color cyan;
		static const Color white;
		static const Color black;
		static const Color brown;
		static const Color lightGray;
		static const Color darkGray;
		static const Color lightBlue;
		static const Color lightGreen;
		static const Color lightCyan;
		static const Color lightRed;
		static const Color lightMagenta;

		static const Color orange;

		class Console
		{
		public:
			class Foreground
			{
			public:
				static const Color red;
				static const Color green;
				static const Color blue;
				static const Color yellow;
				static const Color magenta;
				static const Color cyan;
				static const Color white;
				static const Color black;
				static const Color brown;
				static const Color lightGray;
				static const Color darkGray;
				static const Color lightBlue;
				static const Color lightGreen;
				static const Color lightCyan;
				static const Color lightRed;
				static const Color lightMagenta;
			};
			class Background
			{
			public:
				static const Color red;
				static const Color green;
				static const Color blue;
				static const Color yellow;
				static const Color magenta;
				static const Color cyan;
				static const Color white;
				static const Color black;
				static const Color brown;
				static const Color lightGray;
				static const Color darkGray;
				static const Color lightBlue;
				static const Color lightGreen;
				static const Color lightCyan;
				static const Color lightRed;
				static const Color lightMagenta;
			};
		};


	private:
		uint8_t m_r;
		uint8_t m_g;
		uint8_t m_b;
		int m_consoleValue;
	};
}