#include "ReceiverTypes/ui/QAbstractLogView.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogView.h"
#include "Utilities/Resources.h"
#include "Utilities/Export.h"
#include <QTreeWidget>
#include <QMetaType>
#include <QSplitter>
#include <QCheckBox>
#include <QFileDialog>
#include "ui/DateTimeWidget.h"
#include <fstream>


namespace Log
{
	namespace UI
	{
		QAbstractLogView::QAbstractLogView(QWidget* parent)
			: QWidget(parent)
			, ContextReceiver()
			, ui(new Ui::QAbstractLogView)
		{
			ui->setupUi(this);
			ui->context_scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
			ui->searchIcon_label->setPixmap(Resources::getIconSearch().pixmap(16,16));

			m_autoCreateNewCheckBoxForNewContext = true;
			m_filterTextEdits = { ui->contextFilter_lineEdit };
			for (size_t i = 0; i < m_filterTextEdits.size(); ++i)
			{
				QObject::connect(m_filterTextEdits[i], &QLineEdit::textChanged,
					this, &QAbstractLogView::onFilterTextChangedSlot);
			}

			for (int i = 0; i < Level::__count; ++i)
			{
				QCheckBox* checkBox = new QCheckBox(parent);
				checkBox->setChecked(true);
				checkBox->setText(Utilities::getLevelStr((Level)i).c_str());
				checkBox->setIcon(Utilities::getIcon((Level)i));
				QObject::connect(checkBox, &QCheckBox::stateChanged,
					this, &QAbstractLogView::onLevelCheckBoxStateChangedSlot);
				m_levelCheckBoxes[i] = checkBox;
				if (ui->logLevel_frame->layout())
					ui->logLevel_frame->layout()->addWidget(checkBox);
			}
			QObject::connect(ui->allContext_checkBox, &QCheckBox::stateChanged, this, &QAbstractLogView::onAllContextCheckBoxStateChanged);
		
			ui->dateTimeFilterActivate_checkBox->setChecked(false);
			connect(ui->dateTimeFilterActivate_checkBox, &QCheckBox::stateChanged, 
				    this, &QAbstractLogView::onDateTimeFilterActivate_checkBox_stateChanged);
			connect(ui->dateTimeFilterMin_dateTimeEdit, &DateTimeWidget::dateTimeChanged,
					this, &QAbstractLogView::onDateTimeFilterMin_changed);
			connect(ui->dateTimeFilterMax_dateTimeEdit, &DateTimeWidget::dateTimeChanged,
				    this, &QAbstractLogView::onDateTimeFilterMax_changed);

			connect(ui->dateTimeFilterMinNow_pushButton, &QPushButton::pressed,
					this, &QAbstractLogView::onDateTimeFilterMinNow_pushButton_clicked);
			connect(ui->dateTimeFilterMaxNow_pushButton, &QPushButton::pressed,
					this, &QAbstractLogView::onDateTimeFilterMaxNow_pushButton_clicked);

			{
				QHBoxLayout* layout = new QHBoxLayout();
				layout->setAlignment(Qt::AlignCenter);
				layout->setMargin(0);
				ui->dateTimeFilterMinNow_pushButton->setLayout(layout);
				QLabel* nowLabel = new QLabel();
				nowLabel->setPixmap(Resources::getIconReload().pixmap(20, 20));
				layout->addWidget(nowLabel);
				ui->dateTimeFilterMinNow_pushButton->setFixedSize(20, 20);
				ui->dateTimeFilterMinNow_pushButton->setText("");
			}
			{
				QHBoxLayout* layout = new QHBoxLayout();
				layout->setAlignment(Qt::AlignCenter);
				layout->setMargin(0);
				ui->dateTimeFilterMaxNow_pushButton->setLayout(layout);
				QLabel* nowLabel = new QLabel();
				nowLabel->setPixmap(Resources::getIconReload().pixmap(20, 20));
				layout->addWidget(nowLabel);
				ui->dateTimeFilterMaxNow_pushButton->setFixedSize(20, 20);
				ui->dateTimeFilterMaxNow_pushButton->setText("");
			}

			for(int i=0; i<DateTime::Range::__count; ++i)
				ui->dateTimeFilterType_comboBox->addItem(DateTime::getRangeStr((DateTime::Range)i).c_str());
			ui->dateTimeFilterType_comboBox->setCurrentIndex(DateTime::Range::between);
			connect(ui->dateTimeFilterType_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
					this, &QAbstractLogView::onDateTimeFilterType_changed);

			
		}
		QAbstractLogView::~QAbstractLogView()
		{

		}

		bool QAbstractLogView::saveVisibleMessages(const std::string& outputFile) const
		{
			std::vector<Logger::AbstractLogger::LoggerSnapshotData> list;
			getSaveVisibleMessages(list);
			return Export::saveToFile(list, outputFile);
		}

		std::vector<Logger::AbstractLogger::LoggerMetaInfo> QAbstractLogView::getContexts() const
		{
			std::vector< Logger::AbstractLogger::LoggerMetaInfo> list;
			for (const auto& context : m_contextData)
			{
				list.push_back(*context.second->loggerInfo);
			}
			return list;
		}

		void QAbstractLogView::onAllContextCheckBoxStateChanged(int state)
		{
			if (m_ignoreAllContextCheckBox_signals)
				return;
			if (state == Qt::PartiallyChecked)
			{
				m_ignoreAllContextCheckBox_signals = true;
				ui->allContext_checkBox->setCheckState(Qt::Checked);
				m_ignoreAllContextCheckBox_signals = false;
			}
			bool isChecked = ui->allContext_checkBox->isChecked();
			for (auto& loggerData : m_contextData)
			{
				loggerData.second->checkBox->setChecked(isChecked);
			}
		}
		void QAbstractLogView::on_clear_pushButton_clicked()
		{
			auto copy = m_contextData;
			for(auto& context : copy)
			{
				if(!context.second->loggerInfo->isAlive)
				{
					removeContext(context.first);
				}
			}
		}
		void QAbstractLogView::on_save_pushButton_clicked()
		{
			QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Log Files (*.log);;All Files (*)"));
			if (fileName.isEmpty())
				return;
			saveVisibleMessages(fileName.toStdString());
		}

		void QAbstractLogView::onLevelCheckBoxStateChangedSlot(int state)
		{
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
			if (checkBox == nullptr)
				return;

			for (size_t i = 0; i < (size_t)Level::__count; ++i)
			{
				if (m_levelCheckBoxes[i] == checkBox)
				{
					onLevelCheckBoxChanged(i, (Level)i, state == Qt::Checked);
					break;
				}
			}
		}
		void QAbstractLogView::onFilterTextChangedSlot(const QString& text)
		{
			QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
			if (lineEdit == nullptr)
				return;

			for (size_t i = 0; i < m_filterTextEdits.size(); ++i)
			{
				if (m_filterTextEdits[i] == lineEdit)
				{
					onFilterTextChanged(i, lineEdit, text.toStdString());
					break;
				}
			}
		}
		void QAbstractLogView::onCheckBoxStateChangedSlot(int state)
		{
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
			if (checkBox == nullptr)
				return;

			for (const auto& contextData : m_contextData)
			{
				if (contextData.second->checkBox == checkBox)
				{
					onContextCheckBoxChanged(contextData.second, state == Qt::Checked);
					break;
				}
			}
		}

		void QAbstractLogView::onDateTimeFilterActivate_checkBox_stateChanged(int state)
		{
			LOGGER_UNUSED(state);
			if (ui->dateTimeFilterActivate_checkBox->isChecked())
			{
				ui->dateTimeFilterMax_dateTimeEdit->setEnabled(true);
				ui->dateTimeFilterMin_dateTimeEdit->setEnabled(true);
				m_dateTimeFilter.active = true;
				m_dateTimeFilter.min = ui->dateTimeFilterMin_dateTimeEdit->getDateTime();
				m_dateTimeFilter.max = ui->dateTimeFilterMax_dateTimeEdit->getDateTime();
				m_dateTimeFilter.rangeType = (DateTime::Range)ui->dateTimeFilterType_comboBox->currentIndex();
			}
			else
			{
				m_dateTimeFilter.active = false;
				ui->dateTimeFilterMax_dateTimeEdit->setEnabled(false);
				ui->dateTimeFilterMin_dateTimeEdit->setEnabled(false);
			}
			onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogView::onDateTimeFilterMin_changed(const DateTime& dateTime)
		{
			m_dateTimeFilter.min = dateTime;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogView::onDateTimeFilterMax_changed(const DateTime& dateTime)
		{
			m_dateTimeFilter.max = dateTime;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogView::onDateTimeFilterMinNow_pushButton_clicked()
		{
			ui->dateTimeFilterMin_dateTimeEdit->setNow();
			m_dateTimeFilter.min = ui->dateTimeFilterMin_dateTimeEdit->getDateTime();
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogView::onDateTimeFilterMaxNow_pushButton_clicked()
		{
			ui->dateTimeFilterMax_dateTimeEdit->setNow();
			m_dateTimeFilter.max = ui->dateTimeFilterMax_dateTimeEdit->getDateTime();
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}

		void QAbstractLogView::onDateTimeFilterType_changed(int index)
		{
			m_dateTimeFilter.rangeType = (DateTime::Range)index;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}

		void QAbstractLogView::setContentWidget(QWidget* widget)
		{
			ui->content_frame->layout()->addWidget(widget);
		}
		void QAbstractLogView::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			addContext(logger);
			if (auto* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger))
				addChildContextRecursive(*contextLogger);
		}
		void QAbstractLogView::onUnsubscribed(Logger::AbstractLogger& logger)
		{
			LOGGER_UNUSED(logger);
		}

		void QAbstractLogView::onContextCreate(Logger::ContextLogger& logger)
		{
			addContext(logger);		
		}
		void QAbstractLogView::onContextDestroy(Logger::AbstractLogger& logger)
		{
			LOGGER_UNUSED(logger);
		}
		void QAbstractLogView::addContext(Logger::AbstractLogger& logger)
		{
			attachLogger(logger);
			if (m_autoCreateNewCheckBoxForNewContext)
			{
				if (m_contextData.find(logger.getID()) != m_contextData.end())
					return;
				ContextData* data = new ContextData(logger.getMetaInfo(), new QCheckBox(this));
				QPalette p = data->checkBox->palette();
				data->checkBox->setAutoFillBackground(true);
				const QIcon &icon = logger.getIcon();
				if (!icon.isNull())
				{
					data->checkBox->setIcon(logger.getIcon());
					data->checkBox->setIconSize(QSize(30, 30));
				}

				p.setColor(QPalette::Button, logger.getColor().toQColor());

				data->checkBox->setPalette(p);
				QObject::connect(data->checkBox, &QCheckBox::stateChanged,
					this, &QAbstractLogView::onCheckBoxStateChangedSlot);
				m_contextData[logger.getID()] = data;
				onNewContextCheckBoxCreated(data);
			}
			const auto &messages = logger.getMessages();

			for (const auto& message : messages)
			{
				onNewMessage(message);
			}
		}


		void QAbstractLogView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			LOGGER_UNUSED(index);
			LOGGER_UNUSED(level);
			LOGGER_UNUSED(isChecked);
		}
		void QAbstractLogView::onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text)
		{
			LOGGER_UNUSED(lineEdit);
			if (index > 0)
				return;
			for (auto& loggerData : m_contextData)
			{
				if (loggerData.second->checkBox->text().contains(text.c_str(), Qt::CaseInsensitive))
				{
					loggerData.second->checkBox->setVisible(true);
				}
				else
				{
					loggerData.second->checkBox->setVisible(false);
				}
			}
		}
		void QAbstractLogView::onContextCheckBoxChanged(ContextData const* context, bool isChecked)
		{
			LOGGER_UNUSED(context);
			LOGGER_UNUSED(isChecked);
			int checkedCount = 0;
			for (auto& loggerData : m_contextData)
			{
				checkedCount += loggerData.second->checkBox->isChecked();
			}
			m_ignoreAllContextCheckBox_signals = true;
			if (checkedCount == m_contextData.size())
				ui->allContext_checkBox->setCheckState(Qt::Checked);
			else if (checkedCount > 0)
				ui->allContext_checkBox->setCheckState(Qt::PartiallyChecked);
			else
				ui->allContext_checkBox->setCheckState(Qt::Unchecked);
			m_ignoreAllContextCheckBox_signals = false;
		}
		void QAbstractLogView::onNewContextCheckBoxCreated(ContextData const* context)
		{
			ui->context_scrollAreaWidgetContents->layout()->addWidget(context->checkBox);
		}
		void QAbstractLogView::onContextCheckBoxDestroyed(ContextData const* context)
		{
			ui->context_scrollAreaWidgetContents->layout()->removeWidget(context->checkBox);
		}

		void QAbstractLogView::removeContext(Logger::AbstractLogger::LoggerID id)
		{
			const auto& it = m_contextData.find(id);
			if (it == m_contextData.end())
				return;
			ContextData* data = it->second;
			onContextCheckBoxDestroyed(data);
			m_contextData.erase(it);
			delete data->checkBox;
			delete data;
		}

		
		void QAbstractLogView::onClear(Logger::AbstractLogger& logger)
		{
			LOGGER_UNUSED(logger);
		}
		void QAbstractLogView::onDelete(Logger::AbstractLogger& logger)
		{
			LOGGER_UNUSED(logger);
		}
		void QAbstractLogView::addContextRecursive(Logger::ContextLogger& logger)
		{
			addContext(logger);
			for (auto& child : logger.getChilds())
			{
				addContextRecursive(*child);
			}
		}
		void QAbstractLogView::addChildContextRecursive(Logger::ContextLogger& logger)
		{
			for (auto& child : logger.getChilds())
			{
				addContextRecursive(*child);
			}
		}
	}
}
#endif