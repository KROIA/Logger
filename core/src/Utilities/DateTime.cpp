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
#ifdef LOGGER_QT
	Date& Date::operator=(const QDate& other)
	{
		m_year = other.year();
		m_month = other.month();
		m_day = other.day();
		return *this;
	}
	QDate Date::toQDate() const
	{
		return QDate(m_year, m_month, m_day);
	}
#endif
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

	void Date::setYear(int year)
	{
		m_year = year;
	}
	void Date::setMonth(int month)
	{
		m_month = month;
	}
	void Date::setDay(int day)
	{
		m_day = day;
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
	void Date::normalize()
	{
		// Define arrays to hold the number of days in each month
		int daysInMonth[] = { 31, 28 + isLeapYear(m_year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

		// Adjust day if out of range
		if (m_day < 1) {
			m_month--;
			if (m_month < 1) {
				m_month = 12;
				m_year--;
			}
			m_day += daysInMonth[m_month - 1];
		}
		else if (m_day > daysInMonth[m_month - 1]) {
			m_day -= daysInMonth[m_month - 1];
			m_year++;
			if (m_month > 12) {
				m_month = 1;
				m_year++;
			}
		}

		// Adjust month if out of range
		if (m_month < 1) {
			m_month = 12;
			m_year--;
		}
		else if (m_month > 12) {
			m_month = 1;
			m_year++;
		}
	}
	Date Date::normalized() const
	{
		Date copy = *this;
		copy.normalize();
		return copy;
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
#ifdef LOGGER_QT
	Time& Time::operator=(const QTime& other)
	{
		m_hour = other.hour();
		m_min = other.minute();
		m_sec = other.second();
		m_ms = other.msec();
		return *this;
	}
	QTime Time::toQTime() const
	{
		return QTime(m_hour, m_min, m_sec, m_ms);
	}
#endif
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
	void Time::setHour(int hour)
	{
		m_hour = hour;
	}
	void Time::setMin(int min)
	{
		m_min = min;
	}
	void Time::setSec(int sec)
	{
		m_sec = sec;
	}
	void Time::setMSec(int msec)
	{
		m_ms = msec;
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
	void Time::normalize()
	{
		// Normalize the time by carrying over the excess minutes and seconds
		m_min += m_sec / 60;
		m_sec %= 60;
		if (m_sec < 0) {
			m_sec += 60;
			m_min--;
		}

		m_hour += m_min / 60;
		m_min %= 60;
		if (m_min < 0) {
			m_min += 60;
			m_hour--;
		}

		//m_hour %= 24;
		//if (m_hour < 0) {
		//	m_hour += 24;
		//}
	
		
	}
	Time Time::normalized() const
	{
		Time copy = *this;
		copy.normalize();
		return copy;
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


	const std::string& DateTime::getRangeStr(Range rangeType)
	{
		static const std::string beforeStr = "Before";
		static const std::string afterStr = "After";
		static const std::string betweenStr = "Between";
		static const std::string equalStr = "Equal";
		static const std::string none = "";

		switch (rangeType)
		{
		case Range::before:
			return beforeStr;
		case Range::after:
			return afterStr;
		case Range::between:
			return betweenStr;
		case Range::equal:
			return equalStr;
		}
		return none;
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

#ifdef LOGGER_QT
	DateTime& DateTime::operator=(const QDateTime& other)
	{
		m_date = other.date();
		m_time = other.time();
		return *this;
	}
	QDateTime DateTime::toQDateTime() const
	{
		return QDateTime(m_date.toQDate(), m_time.toQTime());
	}
#endif

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

	void DateTime::setTime(const Time& time)
	{
		m_time = time;
	}
	void DateTime::setDate(const Date& date)
	{
		m_date = date;
	}

	const Time& DateTime::getTime() const
	{
		return m_time;
	}
	const Date& DateTime::getDate() const
	{
		return m_date;
	}

	void DateTime::normalize()
	{
		m_time.normalize();
		if (m_time.m_hour > 24)
		{
			m_date.m_day += m_time.m_hour / 24;
			m_time.m_hour = m_time.m_hour % 24;
		}
		else if (m_time.m_hour < 0)
		{
			m_date.m_day += m_time.m_hour / 24 - 1;
			m_time.m_hour = 24 + m_time.m_hour % 24;
		}
		m_date.normalize();
	}
	DateTime DateTime::normalized() const
	{
		DateTime copy = *this;
		copy.normalize();
		return copy;
	}

	std::string DateTime::toString(Format format) const
	{
		return m_date.toString((Date::Format)format) + " " + m_time.toString((Time::Format)format);
	}
}