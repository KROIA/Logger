#pragma once

#include "Logger_base.h"
#include <string>

namespace Log
{
	class LOGGER_EXPORT DateTime;
	class LOGGER_EXPORT Date
	{
		friend DateTime;
	public:
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

		std::string toString() const;

	private:
		int m_year;
		int m_month;
		int m_day;
	};
	class LOGGER_EXPORT Time
	{
		friend DateTime;
	public:
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

		std::string toString() const;

	private:
		int m_hour;
		int m_min;
		int m_sec;
		int m_ms;
	};

	class LOGGER_EXPORT DateTime
	{
	public:
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

		std::string toString() const;

	private:
		Date m_date;
		Time m_time;
	};
}