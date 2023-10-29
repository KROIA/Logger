#include "QT/QContextLoggerTreeWidgetItem.h"

#ifdef LOGGER_QT

namespace Log
{
	QContextLoggerTreeWidgetItem::QContextLoggerTreeWidgetItem()
		: QTreeWidgetItem()
	{

	}
	QContextLoggerTreeWidgetItem::QContextLoggerTreeWidgetItem(QTreeWidget* treeview/*, ContextLogger* logger*/ )
		: QTreeWidgetItem(treeview)
		//, m_logger(logger)
	{

	}
	QContextLoggerTreeWidgetItem::QContextLoggerTreeWidgetItem(QTreeWidgetItem* parent/*, ContextLogger* logger*/)
		: QTreeWidgetItem(parent)
		//, m_logger(logger)
	{

	}

	QContextLoggerTreeWidgetItem::~QContextLoggerTreeWidgetItem()
	{

	}


	QTreeWidgetItem* QContextLoggerTreeWidgetItem::clone() const
	{
		QContextLoggerTreeWidgetItem *c = new QContextLoggerTreeWidgetItem(this->parent());

		return c;
	}


	void QContextLoggerTreeWidgetItem::updateData(const ContextLogger& logger)
	{
		for (size_t i = 0; i < m_msgItems.size(); ++i)
			delete m_msgItems[i];
		m_msgItems.clear();
		QTreeWidgetItem::setData(0, Qt::DisplayRole, logger.getDateTime().toString().c_str());
		QTreeWidgetItem::setData(1, Qt::DisplayRole, logger.getName().c_str());

		std::vector<Message> messages = logger.getMessages();
		std::string times;
		std::string msgs;
		for (size_t i = 0; i < messages.size(); ++i)
		{
			QTreeWidgetItem* line = new QTreeWidgetItem(this);
			line->setData(0, Qt::DisplayRole, messages[i].getDateTime().toString().c_str());
			line->setData(1, Qt::DisplayRole, messages[i].getText().c_str());

			addChild(line);
			m_msgItems.push_back(line);
		}
	}
}
#endif