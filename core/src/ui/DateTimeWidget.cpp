#include "ui/DateTimeWidget.h"


#ifdef LOGGER_QT
namespace Log
{
	namespace UI
	{
		DateTimeWidget::DateTimeWidget(QWidget* parent)
			: QDateTimeEdit(parent)
		{
			//ui.setupUi(this);
		}

		DateTimeWidget::~DateTimeWidget()
		{
		}

		void DateTimeWidget::setDateTime(const DateTime& dateTime)
		{
			m_dateTime = dateTime;
			m_dateTime.normalize();

		}
		DateTime DateTimeWidget::getDateTime() const
		{
			return m_dateTime;
		}

		void DateTimeWidget::onDateTimeChanged(const QDateTime& datetime)
		{

			m_dateTime = datetime;
			emit dateTimeChanged(m_dateTime);
		}

		/*bool DateTimeWidget::event(QEvent* event)
		{
			QDateTimeEdit::event(event);

		}*/
	}
}
#endif
