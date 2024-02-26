#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QAbstractItemModel>
#include <QVariant>
#include <vector>
#include <string>
#include "LogMessage.h"

namespace Log
{
	class LOGGER_EXPORT QLogMessageItemModel : public QAbstractItemModel
	{
	public:
        enum Column
        {
			TimeColumn,
			LevelColumn,
			ContextColumn,
			MessageColumn,
            __count
		};
        explicit QLogMessageItemModel(QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        void addLog(const Message& entry);
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& child) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;


    private:
        std::vector<Message::SnapshotData> logs;
	};
}
#endif