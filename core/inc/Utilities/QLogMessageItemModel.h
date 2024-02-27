#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QAbstractItemModel>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <vector>
#include <string>
#include "LogMessage.h"
#include <unordered_map>
#include "LoggerTypes/AbstractLogger.h"

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

        const Message::SnapshotData& getElement(size_t row) const;

        void clear();
    private:
        std::vector<Message::SnapshotData> logs;
        DateTime::Format m_dateTimeFormat;
	};




    class QLogMessageItemProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        QLogMessageItemProxyModel(QObject* parent = nullptr);

        /*QDate filterMinimumDate() const { return minDate; }
        void setFilterMinimumDate(QDate date);

        QDate filterMaximumDate() const { return maxDate; }
        void setFilterMaximumDate(QDate date);*/

        void setLevelVisibility(Level level, bool isVisible);
        bool getLevelVisibility(Level level) const;

        void setContextVisibility(Logger::AbstractLogger::LoggerID loggerID, bool isVisible);
        void setContextVisibility(const Logger::AbstractLogger& logger, bool isVisible);
        bool getContextVisibility(Logger::AbstractLogger::LoggerID loggerID) const;
        bool getContextVisibility(const Logger::AbstractLogger& logger) const;

        void setDateTimeFilter(DateTime min, DateTime max, DateTime::Range rangeType);
        void clearDateTimeFilter();
        const DateTime & getDateTimeFilterMin() const;
        const DateTime & getDateTimeFilterMax() const;
        DateTime::Range getDateTimeFilterRangeType() const;
        bool isDateTimeFilterActive() const;

        void setSourceModel(QAbstractItemModel *sourceModel) override;
    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

    private:
        
        //bool dateInRange(QDate date) const;

        QLogMessageItemModel *m_sourceModel;
        bool m_levelActivated[static_cast<int>(Level::__count)];
        std::unordered_map<Logger::AbstractLogger::LoggerID, bool> m_contextVisibility; // Key unique Logger id
       
        struct DateTimeFilter
        {
            bool active;
            DateTime min;
            DateTime max;
            DateTime::Range rangeType;
        };
        DateTimeFilter m_dateTimeFilter;
        
        //QDate maxDate;
    };
}
#endif