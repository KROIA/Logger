#include "Utilities/DateTime.h"
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

	Date& Date::operator=(const Date& other)
	{
		m_year = other.m_year;
		m_month = other.m_month;
		m_day = other.m_day;
		return *this;
	}
	bool Date::operator<(const Date& other) const
	{
		if (m_year < other.m_year) return true;
		if (m_year > other.m_year) return false;

		if (m_month < other.m_month) return true;
		if (m_month > other.m_month) return false;

		if (m_day < other.m_day) return true;
		if (m_day > other.m_day) return false;

		return false;
	}
	bool Date::operator>(const Date& other) const
	{
		return !(*this < other) && *this != other;
	}
	bool Date::operator<=(const Date& other) const
	{
		return *this < other || *this == other;
	}
	bool Date::operator>=(const Date& other) const
	{
		return !(*this < other);
	}
	bool Date::operator==(const Date& other) const
	{
		return m_year == other.m_year && m_month == other.m_month && m_day == other.m_day;
	}
	bool Date::operator!=(const Date& other) const
	{
		return !(*this == other);
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

	std::string Date::toString(Format format) const
	{
		std::string day = std::to_string(m_day);
		std::string month = std::to_string(m_month);
		std::string year = std::to_string(m_year);

		if (m_day < 10) day = "0" + day;
		if (m_month < 10) month = "0" + month;

		if (((int)format & (int)Format::dayMonthYear) == (int)Format::dayMonthYear)
			return day + "." + month + "." + year;
		if (((int)format & (int)Format::yearMonthDay) == (int)Format::yearMonthDay)
			return year + "." + month + "." + day;
		return "";
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
	Time& Time::operator=(const Time& other)
	{
		m_hour = other.m_hour;
		m_min = other.m_min;
		m_sec = other.m_sec;
		m_ms = other.m_ms;
		return *this;
	
	}
	bool Time::operator<(const Time& other) const
	{
		if (m_hour < other.m_hour) 
			return true;
		if (m_hour > other.m_hour) 
			return false;

		if (m_min < other.m_min) 
			return true;
		if (m_min > other.m_min) 
			return false;

		if (m_sec < other.m_sec) 
			return true;
		if (m_sec > other.m_sec) 
			return false;

		if (m_ms < other.m_ms) 
			return true;
		if (m_ms > other.m_ms) 
			return false;

		return false;
	}
	bool Time::operator>(const Time& other) const
	{
		return !(*this < other) && *this != other;
	}
	bool Time::operator<=(const Time& other) const
	{
		return *this < other || *this == other;
	}
	bool Time::operator>=(const Time& other) const
	{
		return !(*this < other);
	}
	bool Time::operator==(const Time& other) const
	{
		return m_hour == other.m_hour && m_min == other.m_min && m_sec == other.m_sec && m_ms == other.m_ms;
	}
	bool Time::operator!=(const Time& other) const
	{
		return !(*this == other);
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

	std::string Time::toString(Format format) const
	{
		std::string hour = std::to_string(m_hour);
		std::string min = std::to_string(m_min);
		std::string sec = std::to_string(m_sec);
		std::string ms = std::to_string(m_ms);

		if (m_hour < 10) hour = "0" + hour;
		if (m_min < 10) min = "0" + min;
		if (m_sec < 10) sec = "0" + sec;
		if (m_ms < 10) ms = "00" + ms;
		else if (m_ms < 100) ms = "0" + ms;
		//else if (m_ms < 1000) ms = "0" + ms;
		if(((int)format & (int)Format::hourMinuteSecondMillisecond) == (int)Format::hourMinuteSecondMillisecond)
			return hour + ":" + min + ":" + sec + ":" + ms;
		if (((int)format & (int)Format::hourMinuteSecond) == (int)Format::hourMinuteSecond)
			return hour + ":" + min + ":" + sec;

		std::string time;
		if ((int)format & (int)Format::hour)
			time = hour;
		if ((int)format & (int)Format::minute)
			time += (time.size()?":":"") + min;
		if ((int)format & (int)Format::second)
			time += (time.size() ? ":" : "") + sec;
		if ((int)format & (int)Format::millisecond)
			time += (time.size() ? ":" : "") + ms;
		return time;
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

	DateTime& DateTime::operator=(const DateTime& other)
	{
		m_date = other.m_date;
		m_time = other.m_time;
		return *this;
	
	}

	bool DateTime::operator<(const DateTime& other) const
	{
		if (m_date < other.m_date) 
			return true;
		if (m_date > other.m_date) 
			return false;

		if (m_time < other.m_time) 
			return true;
		if (m_time > other.m_time) 
			return false;

		return false;
	}
	bool DateTime::operator>(const DateTime& other) const
	{
		return !(*this < other) && *this != other;
	}
	bool DateTime::operator<=(const DateTime& other) const
	{
		return *this < other || *this == other;
	}
	bool DateTime::operator>=(const DateTime& other) const
	{
		return !(*this < other);
	}
	bool DateTime::operator==(const DateTime& other) const
	{
		return m_date == other.m_date && m_time == other.m_time;
	}
	bool DateTime::operator!=(const DateTime& other) const
	{
		return !(*this == other);
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

	std::string DateTime::toString(Format format) const
	{
		return m_date.toString((Date::Format)format) + " " + m_time.toString((Time::Format)format);
	}
}