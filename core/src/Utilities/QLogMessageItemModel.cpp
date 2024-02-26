#include "Utilities/QLogMessageItemModel.h"

#ifdef LOGGER_QT
#include <QBrush>
#include <QFont>
#include "LoggerTypes/AbstractLogger.h"
#include "Utilities/Resources.h"


namespace Log
{
    QLogMessageItemModel::QLogMessageItemModel(QObject* parent) 
        : QAbstractItemModel(parent) 
    {
    
    }

    int QLogMessageItemModel::rowCount(const QModelIndex& parent) const 
    {
        if (parent.isValid())
            return 0;
        return logs.size();
    }

    int QLogMessageItemModel::columnCount(const QModelIndex& parent) const 
    {
        return (int)Column::__count; 
    }

    QVariant QLogMessageItemModel::data(const QModelIndex& index, int role) const 
    {
        if (!index.isValid() || index.row() >= logs.size() || index.column() >= (int)Column::__count)
            return QVariant();

        const Message::SnapshotData& entry = logs[index.row()];

        float colorFactor = 0.5f;

        switch (role) {
        case Qt::DisplayRole:
            switch (index.column())
            {
            case Column::TimeColumn:    return QString::fromStdString(entry.dateTime.toString(DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond));
            case Column::ContextColumn: return QString::fromStdString(entry.contextName);
            case Column::MessageColumn: return QString::fromStdString(entry.message);
            }
            break;
        case Qt::ForegroundRole:
            switch (index.column())
            {
            //case Column::TimeColumn:    return QBrush(entry.getContext()->getColor().toQColor());
            //case Column::ContextColumn: return QBrush(entry.getColor().toQColor());
            case Column::MessageColumn: return QBrush(entry.textColor.toQColor());
            }
            break;

        case Qt::BackgroundRole:
            switch (index.column())
            {
            case Column::TimeColumn:    return QBrush((entry.contextColor * colorFactor).toQColor());
            case Column::ContextColumn: return QBrush((entry.contextColor * colorFactor).toQColor());
            case Column::LevelColumn:   return QBrush((entry.contextColor * colorFactor).toQColor());
            case Column::MessageColumn: return QBrush((entry.contextColor * colorFactor).toQColor());
            }
            break;

        case Qt::DecorationRole:
            switch (index.column())
            {
            case Column::LevelColumn:   return Utilities::getIcon(entry.level);
            }
            break;

        case Qt::FontRole:
            switch (index.column())
            {
			    case Column::MessageColumn:   return QFont("Arial", 10, QFont::Bold);
			}
			break;
            
        default:
            return QVariant();
        }
        return QVariant();
    }

    void QLogMessageItemModel::addLog(const Message& entry)
    {
        beginInsertRows(QModelIndex(), logs.size(), logs.size());
        logs.push_back(entry.createSnapshot());
        endInsertRows();
    }

    QModelIndex QLogMessageItemModel::index(int row, int column, const QModelIndex& parent) const 
    {
        if (parent.isValid() || row < 0 || row >= logs.size() || column < 0 || column >= (int)Column::__count)
            return QModelIndex();
        return createIndex(row, column);
    }

    QModelIndex QLogMessageItemModel::parent(const QModelIndex& child) const 
    {
        Q_UNUSED(child);
        return QModelIndex();
    }

    QVariant QLogMessageItemModel::headerData(int section, Qt::Orientation orientation, int role) const 
    {
        if (orientation == Qt::Horizontal) 
        {
            switch (role)
            {
                case Qt::DisplayRole:
                    switch (section) 
                    {
                        case Column::TimeColumn: return "Timestamp";
                        case Column::LevelColumn: return "";
                        case Column::ContextColumn: return "Context";
                        case Column::MessageColumn: return "Message";
                        default: break;
                    }
                    break;
                /*case Qt::SizeHintRole:
                    switch (section)
                    {
						case Column::TimeColumn: return QSize(500, 20);
						case Column::LevelColumn: return QSize(20, 20);
						case Column::ContextColumn: return QSize(150, 20);
						case Column::MessageColumn: return QSize(300, 20);
						default: break;
					}
					break;*/
            }
        }
        return QVariant();
    }
}
#endif