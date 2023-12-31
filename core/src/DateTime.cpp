#include "DateTime.h"
#include <chrono>
#include <ctime>

namespace Log
{
	Date::Date()
	{
		update();
	}
	Date::Date(const Date& other)
		: m_year(other.m_year)
		, m_month(other.m_month)
		, m_day(other.m_day)
	{

	}

	void Date::update()
	{
		std::time_t now = std::time(nullptr);
		std::tm localTime = *std::localtime(&now);

		m_year = localTime.tm_year + 1900;
		m_month = localTime.tm_mon + 1;
		m_day = localTime.tm_mday;
	}

	int Date::getYear() const
	{
		return m_year;
	}
	int Date::getMonth() const
	{
		return m_month;
	}
	int Date::getDay() const
	{
		return m_day;
	}

	std::string Date::toString() const
	{
		std::string day = std::to_string(m_day);
		std::string month = std::to_string(m_month);
		std::string year = std::to_string(m_year);

		if (m_day < 10) day = "0" + day;
		if (m_month < 10) month = "0" + month;

		return day + "." + month + "." + year;
	}

	Time::Time()
	{
		update();
	}
	Time::Time(const Time& other)
		: m_hour(other.m_hour)
		, m_min(other.m_min)
		, m_sec(other.m_sec)
		, m_ms(other.m_ms)
	{

	}

	void Time::update()
	{
		std::time_t now = std::time(nullptr);
		std::tm localTime = *std::localtime(&now);

		// Get the current time with milliseconds
#ifdef _MSC_VER 
		auto duration = std::chrono::system_clock::now().time_since_epoch();
		int ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
#else
		int ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 1000;
#endif

		m_hour = localTime.tm_hour;
		m_min = localTime.tm_min;
		m_sec = localTime.tm_sec;
		m_ms = ms;
	}

	int Time::getHour() const
	{
		return m_hour;
	}
	int Time::getMin() const
	{
		return m_min;
	}
	int Time::getSec() const
	{
		return m_sec;
	}
	int Time::getMSec() const
	{
		return m_ms;
	}

	std::string Time::toString() const
	{
		std::string hour = std::to_string(m_hour);
		std::string min = std::to_string(m_min);
		std::string sec = std::to_string(m_sec);
		std::string ms = std::to_string(m_ms);

		if (m_hour < 10) hour = "0" + hour;
		if (m_min < 10) min = "0" + min;
		if (m_sec < 10) sec = "0" + sec;
		if (m_ms < 10) ms = "000" + ms;
		else if (m_ms < 100) ms = "00" + ms;
		else if (m_ms < 1000) ms = "0" + ms;

		return hour + ":" + min + ":" + sec + ":" + ms;
	}


	DateTime::DateTime()
	{
		update();
	}
	DateTime::DateTime(const DateTime& other)
		: m_date(other.m_date)
		, m_time(other.m_time)
	{

	}

	void DateTime::update()
	{
		std::time_t now = std::time(nullptr);
		std::tm localTime = *std::localtime(&now);

		// Get the current time with milliseconds
#ifdef _MSC_VER 
		auto duration = std::chrono::system_clock::now().time_since_epoch();
		int ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;
#else
		int ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 1000;
#endif

		m_date.m_year = localTime.tm_year + 1900;
		m_date.m_month = localTime.tm_mon + 1;
		m_date.m_day = localTime.tm_mday;

		m_time.m_hour = localTime.tm_hour;
		m_time.m_min = localTime.tm_min;
		m_time.m_sec = localTime.tm_sec;
		m_time.m_ms = ms;
	}

	const Time& DateTime::getTime() const
	{
		return m_time;
	}
	const Date& DateTime::getDate() const
	{
		return m_date;
	}

	std::string DateTime::toString() const
	{
		return m_date.toString() + " " + m_time.toString();
	}
}