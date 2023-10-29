#pragma once

#include "Logger_base.h"
#include <QTreeWidgetItem>
#include "ContextLogger.h"

namespace Log
{
	class LOGGER_EXPORT QContextLoggerTreeWidgetItem : public QTreeWidgetItem
	{
	public:
		QContextLoggerTreeWidgetItem(QTreeWidget* treeview);
		QContextLoggerTreeWidgetItem(QTreeWidgetItem* parent);

		~QContextLoggerTreeWidgetItem();

		QTreeWidgetItem* clone() const override;
	};
}