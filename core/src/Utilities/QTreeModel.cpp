#include "Utilities/QTreeModel.h"

#ifdef LOGGER_QT
#include "LoggerTypes/ContextLogger.h"

namespace Log
{
	QTreeModel::QTreeModel(QObject* parent)
		: QAbstractItemModel(parent)
	{
		m_rootItem = new QTreeItem("Root");
	}

	QTreeModel::~QTreeModel()
	{
		delete m_rootItem;
	}

	QVariant QTreeModel::data(const QModelIndex& index, int role) const
	{
		if (!index.isValid())
			return QVariant();

		if (role != Qt::DisplayRole)
			return QVariant();

		QTreeItem* item = static_cast<QTreeItem*>(index.internalPointer());

		return "DATA";
		std::string msg = item->data(index.column()).message;
		return msg.c_str();
	}

	Qt::ItemFlags QTreeModel::flags(const QModelIndex& index) const
	{
		if (!index.isValid())
			return 0;

		return QAbstractItemModel::flags(index);
	}

	QVariant QTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal &&
			role == Qt::DisplayRole)
			return "TITLE"; // m_rootItem->data(section);

		return QVariant();
	}

	QModelIndex QTreeModel::index(int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex(row, column, parent))
			return QModelIndex();

		QTreeItem* parentItem;

		if (!parent.isValid())
			parentItem = m_rootItem;
		else
			parentItem = static_cast<QTreeItem*>(parent.internalPointer());

		QTreeItem* childItem = parentItem->child(row);
		if (childItem)
			return createIndex(row, column, childItem);
		return QModelIndex();
		/*if (!hasIndex(row, column, parent))
			return QModelIndex();

		QTreeItem* parentItem = nullptr;

		if (!parent.isValid())
			parentItem = m_rootItem;
		else
			parentItem = static_cast<QTreeItem*>(parent.internalPointer());
		long messageRows = parentItem->getMessagesCount();
		if ((long)row - messageRows >= 0)
		{
			QTreeItem* childItem = parentItem->child(row - messageRows);
			if (childItem)
				return createIndex(row, column, childItem);
			else
				return QModelIndex();
		}
		else
		{
			return createIndex(row, column, parentItem);
		}
		QTreeItem* childItem = parentItem->child(row);
		if (childItem)
			return createIndex(row, column, childItem);
		else
			return QModelIndex();*/
	}

	QModelIndex QTreeModel::parent(const QModelIndex& index) const
	{
		if (!index.isValid())
			return QModelIndex();

		QTreeItem* childItem = static_cast<QTreeItem*>(index.internalPointer());
		QTreeItem* parentItem = childItem->parentItem();

		if (parentItem == m_rootItem)
			return QModelIndex();

		return createIndex(parentItem->row(), 0, parentItem);
	}

	int QTreeModel::rowCount(const QModelIndex& parent) const
	{
		QTreeItem* parentItem;
		if (parent.column() > 0)
			return	0;

		if (!parent.isValid())
			parentItem = m_rootItem;
		else
			parentItem = static_cast<QTreeItem*>(parent.internalPointer());

		return parentItem->rowCount();
	}

	int QTreeModel::columnCount(const QModelIndex& parent) const
	{
		return 1;
		/*if (parent.isValid())
			return static_cast<QTreeItem*>(parent.internalPointer())->getMessagesCount()+1;
		else
			return 1;*/
	}


	/*void QTreeModel::onPrintAllMessagesRecursive(Logger::ContextLogger& logger)
	{

	}*/

	void QTreeModel::onContextCreate(Logger::ContextLogger& logger)
	{
		attachLogger(logger);
	}
	void QTreeModel::onContextDestroy(Logger::AbstractLogger& logger)
	{
		detachLogger(logger);
	}

	void QTreeModel::onNewSubscribed(Logger::AbstractLogger& logger)
	{
		Logger::ContextLogger* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger);

		const auto &it = m_loggerTreeItems.find(logger.getID());
		std::string name = logger.getName();
		if (it == m_loggerTreeItems.end())
		{
			
			if (contextLogger)
			{
				Logger::AbstractLogger::LoggerID parentID = -1;
				Logger::ContextLogger*  parent = contextLogger->getParent();
				if (parent)
					parentID = parent->getID();

				if (parentID >= 0)
				{
					const auto &parentIt = m_loggerTreeItems.find(parentID);
					if (parentIt != m_loggerTreeItems.end())
					{
						QTreeItem* item = new QTreeItem(name);
						parentIt->second->appendChild(item);
						m_loggerTreeItems[logger.getID()] = item;
					}
					else
					{
						QTreeItem* item = new QTreeItem(name);
						m_loggerTreeItems[logger.getID()] = item;
						m_rootItem->appendChild(item);
					}
				}
				else
				{
					QTreeItem* item = new QTreeItem(name);
					m_loggerTreeItems[logger.getID()] = item;
					m_rootItem->appendChild(item);
				}
				onPrintAllMessagesRecursive(*contextLogger);
			}
			else
			{
				QTreeItem* item = new QTreeItem(name);
				m_loggerTreeItems[logger.getID()] = item;
				m_rootItem->appendChild(item);
				onPrintAllMessages(logger);
			}				
		}
		else
		{
			//it->second->setLogger(contextLogger);
		}

		
	}
	void QTreeModel::onUnsubscribed(Logger::AbstractLogger& logger)
	{
		
	}

	void QTreeModel::onNewMessage(const Message& m)
	{
		const auto& it = m_loggerTreeItems.find(m.getContext()->getID());
		if (it != m_loggerTreeItems.end())
		{
			it->second->addElement(m.createSnapshot());
		}

	}
	void QTreeModel::onClear(Logger::AbstractLogger& logger)
	{
		m_rootItem->clearMessages();
	}
	void QTreeModel::onDelete(Logger::AbstractLogger& logger)
	{
		detachLogger(logger);
	}




}
#endif