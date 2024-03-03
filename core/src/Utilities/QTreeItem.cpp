#include "Utilities/QTreeItem.h"


#ifdef LOGGER_QT
namespace Log
{
	QTreeItem::QTreeItem(const std::string& name)
		: m_name(name)
		, m_itemData()
		, m_parentItem(nullptr)
	{}

	QTreeItem::~QTreeItem()
	{
		for (size_t i = 0; i < m_childItems.size(); ++i)
		{
			delete m_childItems[i];
		}
	}

	void QTreeItem::appendChild(QTreeItem* item)
	{
		item->m_parentItem = this;
		m_childItems.push_back(item);
	}

	QTreeItem* QTreeItem::child(size_t row)
	{
		if (row < 0 || row >= m_childItems.size())
			return nullptr;
		return m_childItems.at(row);
	}

	size_t QTreeItem::childCount() const
	{
		return m_childItems.size();
	}

	size_t QTreeItem::row() const
	{
		if (m_parentItem)
		{
			const auto &it = std::find(m_parentItem->m_childItems.begin(), m_parentItem->m_childItems.end(), const_cast<QTreeItem*>(this));
			if (it != m_parentItem->m_childItems.end())
				return std::distance(m_parentItem->m_childItems.begin(), it);
		}
		return 0;
	}

	/*size_t QTreeItem::columnCount() const
	{
		return m_itemData.size();
	}*/
	size_t QTreeItem::rowCount() const
	{
		return m_itemData.size() + childCount() + 1;
	}
	size_t QTreeItem::getMessagesCount() const
	{
		return m_itemData.size();
	}

	const Message::SnapshotData &QTreeItem::data(size_t column) const
	{
		if (column < 0 || column >= m_itemData.size())
		{
			static Message::SnapshotData empty;
			return empty;
		}
		return m_itemData[column];
	}

	QTreeItem* QTreeItem::parentItem()
	{
		return m_parentItem;
	}
	void QTreeItem::addElement(const Message::SnapshotData& data)
	{
		m_itemData.push_back(data);
	}
	void QTreeItem::clearMessages()
	{
		m_itemData.clear();
		for (size_t i = 0; i < m_childItems.size(); ++i)
		{
			m_childItems[i]->clearMessages();
		}
	}
	void QTreeItem::clear()
	{
		m_itemData.clear();
		for (size_t i = 0; i < m_childItems.size(); ++i)
		{
			delete m_childItems[i];
		}
	}
}
#endif