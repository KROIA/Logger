#include "Logger_base.h"


#ifdef LOGGER_QT
#include <QVariant>
#include <vector>
#include "LogMessage.h"


namespace Log
{
    class QTreeItem
    {
    public:
        explicit QTreeItem();
        ~QTreeItem();

        void appendChild(QTreeItem* child);

        QTreeItem* child(size_t row);
        size_t childCount() const;
        size_t columnCount() const;
        const Message::SnapshotData& data(size_t column) const;
        size_t row() const;
        QTreeItem* parentItem();
        void addElement(const Message::SnapshotData& data);
        void clearMessages();
        void clear();

    private:
        std::vector<QTreeItem*> m_childItems;
        std::vector<Message::SnapshotData> m_itemData;
        QTreeItem* m_parentItem;
    };
}
#endif