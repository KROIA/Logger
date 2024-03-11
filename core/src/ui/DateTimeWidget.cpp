#include "ui/DateTimeWidget.h"


#ifdef QT_WIDGETS_LIB
namespace Log
{
	namespace UI
	{
		DateTimeWidget::DateTimeWidget(QWidget* parent)
			: QDateTimeEdit(parent)
		{
			connect(this, &QDateTimeEdit::dateTimeChanged, 
				    this, &DateTimeWidget::onDateTimeChanged);
			setDateTime(DateTime());
			setDisplayFormat("yyyy.MM.dd HH:mm:ss");
		}

		DateTimeWidget::~DateTimeWidget()
		{
		}

		void DateTimeWidget::setDateTime(const DateTime& dateTime)
		{
			m_ignoreSignals = true;
			m_dateTime = dateTime;
			m_dateTime.normalize();
			Time time = m_dateTime.getTime();
			time.setMSec(0);
			m_dateTime.setTime(time);
			QDateTimeEdit::setDateTime(m_dateTime.toQDateTime());
			m_ignoreSignals = false;
		}
		void DateTimeWidget::setNow()
		{
			setDateTime(DateTime());
		}
		DateTime DateTimeWidget::getDateTime() const
		{
			return m_dateTime;
		}

		void DateTimeWidget::onDateTimeChanged(const QDateTime& datetime)
		{
			if (m_ignoreSignals)
				return;
			m_dateTime = datetime;
			emit dateTimeChanged(m_dateTime);
		}

	}
}
#endif
