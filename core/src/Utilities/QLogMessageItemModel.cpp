#include "Utilities/QLogMessageItemModel.h"

#ifdef QT_WIDGETS_LIB
#include <QBrush>
#include <QFont>
#include "Utilities/Resources.h"
#include "LogManager.h"


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
        LOGGER_UNUSED(parent);
        return (int)Column::__count; 
    }

    QVariant QLogMessageItemModel::data(const QModelIndex& index, int role) const 
    {
        if (!index.isValid() || index.row() >= logs.size() || index.column() >= (int)Column::__count)
            return QVariant();

        const Message& entry = logs[index.row()];
        LogObject::Info info = LogManager::getLogObjectInfo(entry.getLoggerID());

        float colorFactor = 0.5f;

        switch (role) {
        case Qt::DisplayRole:
            switch (index.column())
            {
            case Column::TimeColumn:    return QString::fromStdString(entry.getDateTime().toString(m_dateTimeFormat));
            case Column::ContextColumn: return QString::fromStdString(info.name);
            case Column::MessageColumn: return QString::fromStdString(entry.getText());
            }
            break;
        case Qt::ForegroundRole:
            switch (index.column())
            {
            //case Column::TimeColumn:    return QBrush(entry.getContext()->getColor().toQColor());
            //case Column::ContextColumn: return QBrush(entry.getColor().toQColor());
            case Column::MessageColumn: return QBrush(entry.getColor().toQColor());
            }
            break;

        case Qt::BackgroundRole:
            switch (index.column())
            {
            case Column::TimeColumn:    return QBrush((info.color * colorFactor).toQColor());
            case Column::ContextColumn: return QBrush((info.color * colorFactor).toQColor());
            case Column::LevelColumn:   return QBrush((info.color * colorFactor).toQColor());
            case Column::MessageColumn: return QBrush((info.color * colorFactor).toQColor());
            }
            break;

        case Qt::DecorationRole:
            switch (index.column())
            {
            case Column::LevelColumn:   return Utilities::getIcon(entry.getLevel());
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
        logs.push_back(entry);
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
            }
        }
        return QVariant();
    }

    const Message& QLogMessageItemModel::getElement(size_t row) const
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

    void QLogMessageItemProxyModel::setContextVisibility(LoggerID loggerID, bool isVisible)
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

    bool QLogMessageItemProxyModel::getContextVisibility(LoggerID loggerID) const
    {
        auto it = m_contextVisibility.find(loggerID);
        if (it != m_contextVisibility.end())
        {
            return it->second;
        }
        return true;
    }


    void QLogMessageItemProxyModel::setDateTimeFilter(const DateTimeFilter& filter)
    {
        m_dateTimeFilter = filter;
        invalidate(); // force update the new filter
    }
    const DateTimeFilter& QLogMessageItemProxyModel::getDateTimeFilter() const
    {
		return m_dateTimeFilter;
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

    bool QLogMessageItemProxyModel::filterAcceptsRow(int sourceRow) const
    {
        return filterAcceptsRow(sourceRow, QModelIndex());
    }

    void QLogMessageItemProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
    {
        LOGGER_UNUSED(sourceModel);
        QLogMessageItemModel * model = dynamic_cast<QLogMessageItemModel*>(sourceModel);
        QSortFilterProxyModel::setSourceModel(sourceModel);
        if (model)
        {
            m_sourceModel = model;
        }
    }
    bool QLogMessageItemProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        LOGGER_UNUSED(sourceParent);
        if (!m_sourceModel)
			return false;
		const Message &data = m_sourceModel->getElement(sourceRow);
        if(!m_levelActivated[static_cast<int>(data.getLevel())])
			return false;

        if(!getContextVisibility(data.getLoggerID()))
            return false;

        return m_dateTimeFilter.matches(data.getDateTime());
    }
    bool QLogMessageItemProxyModel::lessThan(const QModelIndex& left,
        const QModelIndex& right) const
    {
        if(!m_sourceModel)
			return false;
        const Message &leftData = m_sourceModel->getElement(left.row());
        const Message &rightData = m_sourceModel->getElement(right.row());

        if(leftData.getDateTime() < rightData.getDateTime())
			return true;
        return false;
    }
}
#endif