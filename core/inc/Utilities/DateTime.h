#pragma once

#include "Logger_base.h"
#include <string>
#include <windows.h>


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

		std::string toString(Format format) const;

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

		std::string toString(Format format) const;

	private:
		Date m_date;
		Time m_time;
	};

	DEFINE_ENUM_FLAG_OPERATORS(Time::Format);
	DEFINE_ENUM_FLAG_OPERATORS(Date::Format);
	DEFINE_ENUM_FLAG_OPERATORS(DateTime::Format);
}