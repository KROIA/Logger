#pragma once

#include "Logger_base.h"
#include <string>
#include <windows.h>

#ifdef LOGGER_QT
#include <QDate>
#include <QTime>
#endif

namespace Log
{
	class LOGGER_EXPORT DateTime;
	class LOGGER_EXPORT Date
	{
		friend DateTime;
	public:
		enum class Format : uint16_t
		{
			dayMonthYear = 16,
			yearMonthDay = 32,
		};
		Date();
		Date(const Date& other);

		Date& operator=(const Date& other);
#ifdef LOGGER_QT
		Date& operator=(const QDate& other);
#endif

		bool operator<(const Date& other) const;
		bool operator>(const Date& other) const;
		bool operator<=(const Date& other) const;
		bool operator>=(const Date& other) const;
		bool operator==(const Date& other) const;
		bool operator!=(const Date& other) const;

		void update();

		int getYear() const;
		int getMonth() const;
		int getDay() const;

		void normalize();
		Date normalized() const;

		std::string toString(Format format) const;

		// Check if a year is a leap year
		static bool isLeapYear(int year) {
			return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
		}
	private:
		int m_year;
		int m_month;
		int m_day;
	};
	class LOGGER_EXPORT Time
	{
		friend DateTime;
	public:
		enum class Format : uint16_t
		{
			millisecond = 1,
			second = 2,
			minute = 4,
			hour = 8,

			hourMinuteSecond = hour | minute | second,
			hourMinuteSecondMillisecond = hour | minute | second | millisecond,
		};
		Time();
		Time(const Time& other);

		Time& operator=(const Time& other);
#ifdef LOGGER_QT
		Time& operator=(const QTime& other);
#endif


		bool operator<(const Time& other) const;
		bool operator>(const Time& other) const;
		bool operator<=(const Time& other) const;
		bool operator>=(const Time& other) const;
		bool operator==(const Time& other) const;
		bool operator!=(const Time& other) const;

		void update();

		int getHour() const;
		int getMin() const;
		int getSec() const;
		int getMSec() const;

		void normalize();
		Time normalized() const;

		std::string toString(Format format) const;

	private:
		int m_hour;
		int m_min;
		int m_sec;
		int m_ms;
	};

	class LOGGER_EXPORT DateTime
	{
	public:
		enum Format: uint16_t
		{
			millisecond = 1,
			second = 2,
			minute = 4,
			hour = 8,

			hourMinuteSecond = hour | minute | second,
			hourMinuteSecondMillisecond = hour | minute | second | millisecond,

			dayMonthYear = 16,
			yearMonthDay = 32,


		};
		enum Range
		{
			before,
			after,
			between,
			equal
		};
		
		
		DateTime();
		DateTime(const DateTime& other);

		DateTime& operator=(const DateTime& other);

		bool operator<(const DateTime& other) const;
		bool operator>(const DateTime& other) const;
		bool operator<=(const DateTime& other) const;
		bool operator>=(const DateTime& other) const;
		bool operator==(const DateTime& other) const;
		bool operator!=(const DateTime& other) const;


		void update();

		const Time& getTime() const;
		const Date& getDate() const;

		void normalize();
		DateTime normalized() const;

		std::string toString(Format format) const;

	private:
		Date m_date;
		Time m_time;
	};

	typedef struct
	{
		bool active;
		DateTime min;
		DateTime max;
		DateTime::Range rangeType;
	} DateTimeFilter;

	DEFINE_ENUM_FLAG_OPERATORS(Time::Format);
	DEFINE_ENUM_FLAG_OPERATORS(Date::Format);
	DEFINE_ENUM_FLAG_OPERATORS(DateTime::Format);
}