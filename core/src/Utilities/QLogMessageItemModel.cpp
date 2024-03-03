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
    	m_dateTimeFormat = DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond;
    }

    void QLogMessageItemModel::setDateTimeFormat(DateTime::Format format)
    {
        m_dateTimeFormat = format;
    }
    DateTime::Format QLogMessageItemModel::getDateTimeFormat() const
    {
        return m_dateTimeFormat;
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

    const Message::SnapshotData& QLogMessageItemModel::getElement(size_t row) const
    {
        return logs[row];
    }

    void QLogMessageItemModel::clear()
    {
        beginResetModel();
		logs.clear();
		endResetModel();
    }


    QLogMessageItemProxyModel::QLogMessageItemProxyModel(QObject* parent)
        : QSortFilterProxyModel(parent)
        , m_sourceModel(nullptr)
    {
        for(int i=0; i<Level::__count; ++i)
            m_levelActivated[i] = true;
        m_dateTimeFilter.active = false;
    }

    void QLogMessageItemProxyModel::setLevelVisibility(Level level, bool isVisible)
    {
        m_levelActivated[static_cast<int>(level)] = isVisible;
    }
    bool QLogMessageItemProxyModel::getLevelVisibility(Level level) const
    {
        return m_levelActivated[static_cast<int>(level)];
    }

    void QLogMessageItemProxyModel::setContextVisibility(Logger::AbstractLogger::LoggerID loggerID, bool isVisible)
    {
        auto it = m_contextVisibility.find(loggerID);
        if (it != m_contextVisibility.end())
        {
            it->second = isVisible;
        }
        else
        {
            m_contextVisibility[loggerID] = isVisible;
        }
    }

    bool QLogMessageItemProxyModel::getContextVisibility(Logger::AbstractLogger::LoggerID loggerID) const
    {
        auto it = m_contextVisibility.find(loggerID);
        if (it != m_contextVisibility.end())
        {
            return it->second;
        }
        return true;
    }

    void QLogMessageItemProxyModel::setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType)
    {
		m_dateTimeFilter.min = min;
		m_dateTimeFilter.max = max;
		m_dateTimeFilter.rangeType = rangeType;
        m_dateTimeFilter.active = true;

    }
    void QLogMessageItemProxyModel::clearDateTimeFilter()
    {
        m_dateTimeFilter.active = false;
    }
    const DateTime& QLogMessageItemProxyModel::getDateTimeFilterMin() const
    {
        return m_dateTimeFilter.min;
    }
    const DateTime& QLogMessageItemProxyModel::getDateTimeFilterMax() const
    {
        return m_dateTimeFilter.max;
    }
    DateTime::Range QLogMessageItemProxyModel::getDateTimeFilterRangeType() const
    {
        return m_dateTimeFilter.rangeType;
    }
    bool QLogMessageItemProxyModel::isDateTimeFilterActive() const
    {
        return m_dateTimeFilter.active;
    }

    void QLogMessageItemProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
    {
        QLogMessageItemModel * model = dynamic_cast<QLogMessageItemModel*>(sourceModel);
        QSortFilterProxyModel::setSourceModel(sourceModel);
        if (model)
        {
            m_sourceModel = model;
        }
    }
    bool QLogMessageItemProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        if (!m_sourceModel)
			return false;
		const Message::SnapshotData &data = m_sourceModel->getElement(sourceRow);
        if(!m_levelActivated[static_cast<int>(data.level)])
			return false;

        if(!getContextVisibility(data.loggerID))
            return false;

        if (m_dateTimeFilter.active)
        {
            switch (m_dateTimeFilter.rangeType)
            {
                case DateTime::Range::before:
					if(m_dateTimeFilter.min < data.dateTime)
						return false;
					break; 
                case DateTime::Range::after:
                    if(m_dateTimeFilter.min > data.dateTime)
                        return false;
                    break;
                case DateTime::Range::between:
                    if(m_dateTimeFilter.min > data.dateTime ||
                        m_dateTimeFilter.max < data.dateTime)
					    return false;
                    break;
                case DateTime::Range::equal:
                    if(m_dateTimeFilter.min != data.dateTime)
						return false;
					break;
            }
        }
		return true;
    }
    bool QLogMessageItemProxyModel::lessThan(const QModelIndex& left,
        const QModelIndex& right) const
    {
        if(!m_sourceModel)
			return false;
        const Message::SnapshotData &leftData = m_sourceModel->getElement(left.row());
        const Message::SnapshotData &rightData = m_sourceModel->getElement(right.row());

        if(leftData.dateTime < rightData.dateTime)
			return true;
        return false;
    }
}
#endif