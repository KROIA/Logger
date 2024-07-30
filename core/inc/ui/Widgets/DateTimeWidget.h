#pragma once
#include "Logger_base.h"

#ifdef QT_WIDGETS_LIB
#include <QDateTimeEdit>
#include "Utilities/DateTime.h"

namespace Log
{
	namespace UIWidgets
	{
		class DateTimeWidget : public QDateTimeEdit
		{
			Q_OBJECT

		public:
			DateTimeWidget(QWidget* parent = nullptr);
			~DateTimeWidget();

			void setDateTime(const DateTime& dateTime);
			void setNow();
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