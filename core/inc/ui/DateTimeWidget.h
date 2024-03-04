#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QDateTimeEdit>
//#include "ui_DateTimeWidget.h"
#include "Utilities/DateTime.h"

namespace Log
{
	namespace UI
	{
		class DateTimeWidget : public QDateTimeEdit
		{
			Q_OBJECT

		public:
			DateTimeWidget(QWidget* parent = nullptr);
			~DateTimeWidget();

			void setDateTime(const DateTime& dateTime);
			DateTime getDateTime() const;

		signals:
			void dateTimeChanged(const DateTime& dateTime);

		private slots:
			void onDateTimeChanged(const QDateTime& datetime);
		private:
			bool m_ignoreSignals = false;
			DateTime m_dateTime;
		};
	}
}
#endif
