#include "Utilities/QLogMessageItemModel.h"

#ifdef QT_WIDGETS_LIB
#include <QBrush>
#include <QFont>
#include <algorithm>
#include <iterator>
#include "Utilities/Resources.h"
#include "LogManager.h"


namespace Log
{
    QLogMessageItemModel::QLogMessageItemModel(QObject* parent) 
        : QAbstractItemModel(parent) 
    {
    	m_dateTimeFormat = DateTime::Format::yearMonthDay | DateTime::Format::hourMinuteSecondMillisecond;
        logs.reserve(4096);
        
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
        const CachedLoggerData& loggerData = getCachedLoggerData(entry.getLoggerID());
        static const QFont boldFont("Arial", 10, QFont::Bold);

        switch (role) {
        case Qt::DisplayRole:
            switch (index.column())
            {
            case Column::TimeColumn:    return QString::fromStdString(entry.getDateTime().toString(m_dateTimeFormat));
            case Column::ContextColumn: return loggerData.name;
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
            case Column::TimeColumn:    
            case Column::ContextColumn:
            case Column::LevelColumn:   
            case Column::MessageColumn: return QBrush(loggerData.backgroundColor);
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
			    case Column::MessageColumn:   return boldFont;
			}
			break;
        case Qt::ToolTipRole:
        {
            switch (index.column())
            {
				case Column::TimeColumn: return QString::fromStdString(entry.getDateTime().toString(m_dateTimeFormat));
				case Column::ContextColumn: return loggerData.name;
                case Column::LevelColumn: return QString::fromStdString(Utilities::getLevelStr(entry.getLevel()));
				case Column::MessageColumn: return QString::fromStdString(entry.getText());
            }
            break;
        }
            
        default:
            return QVariant();
        }
        return QVariant();
    }

    Qt::ItemFlags QLogMessageItemModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return Qt::NoItemFlags;
        // ItemIsEditable is required so that the read-only line-edit editor
        // (installed by the view for mouse text selection) can be opened.
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    void QLogMessageItemModel::addLog(const Message& entry)
    {
        beginInsertRows(QModelIndex(), logs.size(), logs.size());
        logs.push_back(entry);
        endInsertRows();
    }
    void QLogMessageItemModel::addLogs(const std::vector<Message>& entries)
    {
        if (entries.empty())
            return;

        const int firstRow = static_cast<int>(logs.size());
        const int lastRow = firstRow + static_cast<int>(entries.size()) - 1;
        beginInsertRows(QModelIndex(), firstRow, lastRow);
        logs.insert(logs.end(), entries.begin(), entries.end());
        endInsertRows();
    }
    void QLogMessageItemModel::addLogs(std::vector<Message>&& entries)
    {
        if (entries.empty())
            return;

        const int firstRow = static_cast<int>(logs.size());
        const int lastRow = firstRow + static_cast<int>(entries.size()) - 1;
        beginInsertRows(QModelIndex(), firstRow, lastRow);
        logs.reserve(logs.size() + entries.size());
        std::move(entries.begin(), entries.end(), std::back_inserter(logs));
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
    void QLogMessageItemModel::clearLoggerCache()
    {
        m_cachedLoggerData.clear();
    }
    void QLogMessageItemModel::setLoggerInfo(const LogObject::Info& info)
    {
        static const float colorFactor = 0.5f;
        CachedLoggerData data;
        data.name = QString::fromStdString(info.name);
        data.backgroundColor = (info.color * colorFactor).toQColor();
        m_cachedLoggerData[info.id] = std::move(data);
    }
    const QLogMessageItemModel::CachedLoggerData& QLogMessageItemModel::getCachedLoggerData(LoggerID loggerID) const
    {
        const auto cachedIt = m_cachedLoggerData.find(loggerID);
        if (cachedIt != m_cachedLoggerData.end())
            return cachedIt->second;

        static const float colorFactor = 0.5f;
        const LogObject::Info info = LogManager::getLogObjectInfo(loggerID);

        CachedLoggerData data;
        data.name = QString::fromStdString(info.name);
        data.backgroundColor = (info.color * colorFactor).toQColor();

        const auto inserted = m_cachedLoggerData.emplace(loggerID, std::move(data));
        return inserted.first->second;
    }


    QLogMessageItemProxyModel::QLogMessageItemProxyModel(QObject* parent)
        : QSortFilterProxyModel(parent)
        , m_sourceModel(nullptr)
    {
        for(int i=0; i<Level::__count; ++i)
            m_levelActivated[i] = true;
        m_dateTimeFilter.active = false;
    }

    void QLogMessageItemProxyModel::beginFilterUpdate()
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        beginFilterChange();
#endif
    }
    void QLogMessageItemProxyModel::endFilterUpdate()
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        // Default is Direction::Both, matching what invalidateFilter() did.
        endFilterChange();
#else
        invalidateFilter();
#endif
    }

    void QLogMessageItemProxyModel::setLevelVisibility(Level level, bool isVisible)
    {
        beginFilterUpdate();
        m_levelActivated[static_cast<int>(level)] = isVisible;
        endFilterUpdate();
    }
    bool QLogMessageItemProxyModel::getLevelVisibility(Level level) const
    {
        return m_levelActivated[static_cast<int>(level)];
    }

    void QLogMessageItemProxyModel::setContextVisibility(LoggerID loggerID, bool isVisible)
    {
        beginFilterUpdate();
        auto it = m_contextVisibility.find(loggerID);
        if (it != m_contextVisibility.end())
        {
            it->second = isVisible;
        }
        else
        {
            m_contextVisibility[loggerID] = isVisible;
        }
        endFilterUpdate();
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


    void QLogMessageItemProxyModel::setTextFilter(const QString& text, bool useRegex)
    {
        beginFilterUpdate();
        // A leading '!' inverts the match (exclude rows containing the term).
        QString effective = text;
        m_searchNegate = effective.startsWith('!');
        if (m_searchNegate)
            effective = effective.mid(1);
        m_searchText = effective;
        m_searchUseRegex = useRegex;
        if (useRegex && !effective.isEmpty())
            m_searchRegex = QRegularExpression(effective, QRegularExpression::CaseInsensitiveOption);
        else
            m_searchRegex = QRegularExpression();
        endFilterUpdate();
    }

    void QLogMessageItemProxyModel::setDateTimeFilter(const DateTimeFilter& filter)
    {
        beginFilterUpdate();
        m_dateTimeFilter = filter;
        endFilterUpdate();
    }
    const DateTimeFilter& QLogMessageItemProxyModel::getDateTimeFilter() const
    {
		return m_dateTimeFilter;
    }
    void QLogMessageItemProxyModel::setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType)
    {
        beginFilterUpdate();
		m_dateTimeFilter.min = min;
		m_dateTimeFilter.max = max;
		m_dateTimeFilter.rangeType = rangeType;
        m_dateTimeFilter.active = true;
        endFilterUpdate();
    }
    void QLogMessageItemProxyModel::clearDateTimeFilter()
    {
        beginFilterUpdate();
        m_dateTimeFilter.active = false;
        endFilterUpdate();
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

        if (!m_searchText.isEmpty())
        {
            const QString msg = QString::fromStdString(data.getText());
            bool hit;
            if (m_searchUseRegex)
                hit = m_searchRegex.isValid() && m_searchRegex.match(msg).hasMatch();
            else
                hit = msg.contains(m_searchText, Qt::CaseInsensitive);
            if (hit == m_searchNegate)
                return false;
        }

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
