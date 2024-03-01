#include "Logger_base.h"


#ifdef LOGGER_QT
#include <vector>
#include <QAbstractItemModel>
#include <unordered_map>
#include "LogMessage.h"
#include "QTreeItem.h"
#include "ReceiverTypes/ContextReceiver.h"

namespace Log
{
    class QTreeModel : public QAbstractItemModel, public Receiver::ContextReceiver
    {
        Q_OBJECT

    public:
        explicit QTreeModel(QObject* parent = nullptr);
        ~QTreeModel();

        QVariant data(const QModelIndex& index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column,
            const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    private:
      // void onPrintAllMessagesRecursive(Logger::ContextLogger& logger) override;

       void onContextCreate(Logger::ContextLogger& logger) override;
       void onContextDestroy(Logger::ContextLogger& logger) override;

       void onNewSubscribed(Logger::AbstractLogger& logger) override;
       void onUnsubscribed(Logger::AbstractLogger& logger) override;

       void onNewMessage(const Message& m) override;
       void onClear(Logger::AbstractLogger& logger) override;
       void onDelete(Logger::AbstractLogger& logger) override;

   
        QTreeItem* m_rootItem;
        std::unordered_map<Logger::AbstractLogger::LoggerID, QTreeItem*> m_loggerTreeItems;

    };
}
#endif