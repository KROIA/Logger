#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QTreeWidgetItem>
#include "ContextLogger.h"

namespace Log
{
	class LOGGER_EXPORT QContextLoggerTreeWidgetItem : public QTreeWidgetItem
	{
	public:
		QContextLoggerTreeWidgetItem();
		QContextLoggerTreeWidgetItem(QTreeWidget* treeview/*, ContextLogger* logger*/ );
		QContextLoggerTreeWidgetItem(QTreeWidgetItem* parent/*, ContextLogger* logger*/);

		~QContextLoggerTreeWidgetItem();

		QTreeWidgetItem* clone() const override;

		void updateData(const ContextLogger& logger);

	private:
		//ContextLogger* m_logger;
		std::vector< QTreeWidgetItem*> m_msgItems;
	};
}
#endif