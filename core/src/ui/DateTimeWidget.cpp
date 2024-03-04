#include "ui/DateTimeWidget.h"


#ifdef LOGGER_QT
namespace Log
{
	namespace UI
	{
		DateTimeWidget::DateTimeWidget(QWidget* parent)
			: QDateTimeEdit(parent)
		{
			connect(this, &QDateTimeEdit::dateTimeChanged, 
				    this, &DateTimeWidget::onDateTimeChanged);
		}

		DateTimeWidget::~DateTimeWidget()
		{
		}

		void DateTimeWidget::setDateTime(const DateTime& dateTime)
		{
			m_ignoreSignals = true;
			m_dateTime = dateTime;
			m_dateTime.normalize();
			QDateTimeEdit::setDateTime(m_dateTime.toQDateTime());
			m_ignoreSignals = false;
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
