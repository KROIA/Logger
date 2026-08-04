#pragma once
#include "Logger_base.h"
#include "Utilities/DateTime.h"

#include "LogMessage.h"
#include "AbstractReceiver.h"

#ifdef QT_WIDGETS_LIB
#include "ui/Widgets/QAbstractLogWidget.h"
#include "ui/Widgets/QContextLoggerTreeWidget.h"
#include <QWidget>
#include <QTreeWidget>
#include <QTimer>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <unordered_map>
#include <QMutex>

QT_BEGIN_NAMESPACE
namespace Ui { class QAbstractLogWidget; }
class QSplitter;
class QTextBrowser;
QT_END_NAMESPACE

namespace Log 
{
    namespace UIWidgets
    {
        class LOGGER_API QAbstractLogWidget : public QWidget, public AbstractReceiver
        {
            Q_OBJECT
            friend class QSignalHandler;
        public:
            enum SubWidget
            {
                settingsFrame,
                logLevelFilter,
				contextFilter,
                dateTimeFilter,
				editFrame,
                contentFrame,
				__count
            };

            // Toggleable feature set. Applications can strip a view down (e.g.
            // hide the regex checkbox on a simplified read-only console) or
            // enable optional features (e.g. details pane) at construction
            // time. Defaults are set by each view's ctor.
            enum Feature
            {
                SearchBar,           // whole search row visible
                SearchRegexCheckBox, // ".*" checkbox in the search row
                FindNextPrev,        // ▲/▼ jump-to-match buttons
                MatchCount,          // "N matches" label
                DetailsPane,         // multi-line details view for selected row
                RowContextMenu,      // right-click menu on message rows
                __featureCount
            };
            virtual void setFeatureEnabled(Feature f, bool enabled);
            bool isFeatureEnabled(Feature f) const;

            QAbstractLogWidget(QWidget* parent = nullptr);
            ~QAbstractLogWidget();

            bool saveVisibleMessages(const std::string& outputFile) const;
			bool loadMessagesFromFile(const std::string& inputFile);
            virtual void getSaveVisibleMessages(std::unordered_map<LoggerID, std::vector<Message>>& list) const = 0;

            virtual void setDateTimeFormat(DateTime::Format format) = 0;
            virtual DateTime::Format getDateTimeFormat() const = 0;

			void setLevelEnabled(Level level, bool enable);

			void disableSubWidget(SubWidget widget);
			void enableSubWidget(SubWidget widget);

            virtual void clear();
        protected:
            void postConstructorInit();

            

        protected slots:
            virtual void onAllContextCheckBoxStateChanged(int state);
            void on_clear_pushButton_clicked();
            virtual void on_save_pushButton_clicked();

        

        private slots:
            void onLevelCheckBoxStateChangedSlot(int state);
            void onFilterTextChangedSlot(const QString& text);
            void onCheckBoxStateChangedSlot(int state);
            
            void onDateTimeFilterActivate_checkBox_stateChanged(int state);
            void onDateTimeFilterMin_changed(const DateTime& dateTime);
            void onDateTimeFilterMax_changed(const DateTime& dateTime);
            void onDateTimeFilterMinNow_pushButton_clicked();
            void onDateTimeFilterMaxNow_pushButton_clicked();
            void onDateTimeFilterType_changed(int index);

        protected:

            struct ContextData
            {
                LoggerID id = 0;
                std::vector<Message> messages;
                QCheckBox* checkBox = nullptr;
                LogObject::Info info;
            };

            
            void onNewLogger(LogObject::Info loggerInfo) override;
            void onLoggerInfoChanged(LogObject::Info info) override;
            void onLogMessage(Message message) override;
            void onChangeParent(LoggerID childID, LoggerID newParentID) override;


            void setContentWidget(QWidget* widget);

            virtual void onLevelCheckBoxChanged(size_t index, Level level, bool isChecked);
            virtual void onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text);
            virtual void onContextCheckBoxChanged(const ContextData& context, bool isChecked);
            virtual void onDateTimeFilterChanged(const DateTimeFilter& filter) = 0;
            // Called when the search text or regex-mode toggle changes.
            // Subclasses override to forward to their content widget's text filter.
            virtual void onSearchTextChanged(const QString& text, bool regex);
            // Optional overrides for match-count / find-next-prev on the search bar.
            // Default implementations return 0 / no-op so views can opt in.
            virtual int matchCount() const { return 0; }
            virtual void findNext(bool forward) { (void)forward; }
            // Views call this after their internal search state changes.
            void refreshMatchCount();

        public:
            // Programmatic filter helpers. Public so composed views (e.g. the
            // combined view forwarding filter changes to embedded sub-views)
            // can drive filter state on unrelated QAbstractLogWidget instances.
            void soloContext(LoggerID id);
            void hideContext(LoggerID id);
            void setContextEnabled(LoggerID id, bool enabled);
            void setSearchTextProgrammatic(const QString& text, bool regex = false);
        protected:

            Ui::QAbstractLogWidget* ui;
        private slots:
            void onSearchLineEditChanged(const QString& text);
            void onSearchRegexToggled(int state);
        private:
            void emitSearchFilter();

            QCheckBox* m_levelCheckBoxes[Level::__count];
            std::vector<QLineEdit*> m_filterTextEdits;
            QLineEdit* m_searchLineEdit = nullptr;
            QCheckBox* m_searchRegexCheckBox = nullptr;
            QLabel* m_matchCountLabel = nullptr;
            QPushButton* m_findPrevButton = nullptr;
            QPushButton* m_findNextButton = nullptr;
            QWidget* m_searchBarWidget = nullptr;
            QWidget* m_detailsPane = nullptr;
            QSplitter* m_contentSplitter = nullptr;
            bool m_features[__featureCount];
        protected:
            // Views call this after their selection changes so the details pane
            // can update. Text is HTML-formatted (safe subset).
            void setDetailsHtml(const QString& html);
            // Format a message as HTML for the details pane.
            static QString formatMessageAsHtml(const Message& msg, const std::string& contextName);
            std::string getContextNameFor(LoggerID id) const;
            void updateDetailsFor(const Message& msg, bool hasSelection);

            std::unordered_map<LoggerID, ContextData> m_contextData;
            DateTimeFilter m_dateTimeFilter;

            bool m_autoCreateNewCheckBoxForNewContext = false;
            bool m_ignoreAllContextCheckBox_signals = false;
        };
    }
}
#endif