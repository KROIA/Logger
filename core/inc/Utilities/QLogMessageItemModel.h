#pragma once
#include "Logger_base.h"
#include "LogObject.h"

#ifdef QT_WIDGETS_LIB
#include <QAbstractItemModel>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <vector>
#include <string>
#include "LogMessage.h"
#include <unordered_map>

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

        void setDateTimeFormat(DateTime::Format format);
        DateTime::Format getDateTimeFormat() const;

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        void addLog(const Message& entry);
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& child) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        const Message& getElement(size_t row) const;

        void clear();
    private:
        std::vector<Message> logs;
        DateTime::Format m_dateTimeFormat;
	};




    class QLogMessageItemProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        QLogMessageItemProxyModel(QObject* parent = nullptr);

        void setLevelVisibility(Level level, bool isVisible);
        bool getLevelVisibility(Level level) const;

        void setContextVisibility(LoggerID loggerID, bool isVisible);
        bool getContextVisibility(LoggerID loggerID) const;

        void setDateTimeFilter(const DateTimeFilter &filter);
        const DateTimeFilter & getDateTimeFilter() const;
        void setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType);
        void clearDateTimeFilter();
        const DateTime & getDateTimeFilterMin() const;
        const DateTime & getDateTimeFilterMax() const;
        DateTime::Range getDateTimeFilterRangeType() const;
        bool isDateTimeFilterActive() const;

        void setSourceModel(QAbstractItemModel *sourceModel) override;
        bool filterAcceptsRow(int sourceRow) const;
    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    private:

        QLogMessageItemModel *m_sourceModel;
        bool m_levelActivated[static_cast<int>(Level::__count)];
        std::unordered_map<LoggerID, bool> m_contextVisibility; // Key unique Logger id
       
        
        DateTimeFilter m_dateTimeFilter;
    };
}
#endif