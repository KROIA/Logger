#include "ui/Widgets/QAbstractLogWidget.h"

#ifdef QT_WIDGETS_LIB
#include "ui_QAbstractLogWidget.h"
#include "Utilities/Resources.h"
#include "Utilities/Export.h"
#include <QTreeWidget>
#include <QMetaType>
#include <QSplitter>
#include <QCheckBox>
#include <QFileDialog>
#include "ui/Widgets/DateTimeWidget.h"
#include <fstream>
#include <QApplication>
#include <QThread>


namespace Log
{
	namespace UIWidgets
	{
		QAbstractLogWidget::QAbstractLogWidget(QWidget* parent)
			: QWidget(parent)
			, ui(new Ui::QAbstractLogWidget)
		{
			ui->setupUi(this);
			ui->context_scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
			ui->logLevel_frame->layout()->setAlignment(Qt::AlignTop);
			ui->searchIcon_label->setPixmap(Resources::getIconSearch().pixmap(16, 16));

			m_autoCreateNewCheckBoxForNewContext = true;
			m_filterTextEdits = { ui->contextFilter_lineEdit };
			for (size_t i = 0; i < m_filterTextEdits.size(); ++i)
			{
				QObject::connect(m_filterTextEdits[i], &QLineEdit::textChanged,
					this, &QAbstractLogWidget::onFilterTextChangedSlot);
			}

			for (int i = 0; i < Level::__count; ++i)
			{
				QCheckBox* checkBox = new QCheckBox(parent);
				checkBox->setChecked(true);
				checkBox->setText(Utilities::getLevelStr((Level)i).c_str());
				checkBox->setIcon(Utilities::getIcon((Level)i));
				QObject::connect(checkBox, &QCheckBox::stateChanged,
					this, &QAbstractLogWidget::onLevelCheckBoxStateChangedSlot);
				m_levelCheckBoxes[i] = checkBox;
				if (ui->logLevel_frame->layout())
					ui->logLevel_frame->layout()->addWidget(checkBox);
			}
			QObject::connect(ui->allContext_checkBox, &QCheckBox::stateChanged, this, &QAbstractLogWidget::onAllContextCheckBoxStateChanged);

			ui->dateTimeFilterActivate_checkBox->setChecked(false);
			connect(ui->dateTimeFilterActivate_checkBox, &QCheckBox::stateChanged,
				this, &QAbstractLogWidget::onDateTimeFilterActivate_checkBox_stateChanged);
			connect(ui->dateTimeFilterMin_dateTimeEdit, &DateTimeWidget::dateTimeChanged,
				this, &QAbstractLogWidget::onDateTimeFilterMin_changed);
			connect(ui->dateTimeFilterMax_dateTimeEdit, &DateTimeWidget::dateTimeChanged,
				this, &QAbstractLogWidget::onDateTimeFilterMax_changed);

			connect(ui->dateTimeFilterMinNow_pushButton, &QPushButton::pressed,
				this, &QAbstractLogWidget::onDateTimeFilterMinNow_pushButton_clicked);
			connect(ui->dateTimeFilterMaxNow_pushButton, &QPushButton::pressed,
				this, &QAbstractLogWidget::onDateTimeFilterMaxNow_pushButton_clicked);

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

			for (int i = 0; i < DateTime::Range::__count; ++i)
				ui->dateTimeFilterType_comboBox->addItem(DateTime::getRangeStr((DateTime::Range)i).c_str());
			ui->dateTimeFilterType_comboBox->setCurrentIndex(DateTime::Range::between);
			connect(ui->dateTimeFilterType_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
				this, &QAbstractLogWidget::onDateTimeFilterType_changed);

		}
		QAbstractLogWidget::~QAbstractLogWidget()
		{

		}
		void QAbstractLogWidget::postConstructorInit()
		{
			std::vector< LogObject::Info> loggers = LogManager::getLogObjectsInfo();
			for (const auto& logger : loggers)
			{
				onNewLogger(logger);
			}
		}

		bool QAbstractLogWidget::saveVisibleMessages(const std::string& outputFile) const
		{
			std::unordered_map<LoggerID, std::vector<Message>> list;
			getSaveVisibleMessages(list);
			return Export::saveToFile(list, outputFile);
		}

		void QAbstractLogWidget::onAllContextCheckBoxStateChanged(int state)
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
				loggerData.second.checkBox->setChecked(isChecked);
			}
		}
		void QAbstractLogWidget::on_clear_pushButton_clicked()
		{

		}
		void QAbstractLogWidget::on_save_pushButton_clicked()
		{
			QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Log Files (*.log);;All Files (*)"));
			if (fileName.isEmpty())
				return;
			saveVisibleMessages(fileName.toStdString());
		}

		void QAbstractLogWidget::onLevelCheckBoxStateChangedSlot(int state)
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
		void QAbstractLogWidget::onFilterTextChangedSlot(const QString& text)
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
		void QAbstractLogWidget::onCheckBoxStateChangedSlot(int state)
		{
			LOGGER_UNUSED(state);
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
			if (checkBox == nullptr)
				return;

			for (const auto& contextData : m_contextData)
			{
				if (contextData.second.checkBox == checkBox)
				{
					onContextCheckBoxChanged(contextData.second, checkBox->isChecked());
					break;
				}
			}
		}

		void QAbstractLogWidget::onDateTimeFilterActivate_checkBox_stateChanged(int state)
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
		void QAbstractLogWidget::onDateTimeFilterMin_changed(const DateTime& dateTime)
		{
			m_dateTimeFilter.min = dateTime;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogWidget::onDateTimeFilterMax_changed(const DateTime& dateTime)
		{
			m_dateTimeFilter.max = dateTime;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogWidget::onDateTimeFilterMinNow_pushButton_clicked()
		{
			ui->dateTimeFilterMin_dateTimeEdit->setNow();
			m_dateTimeFilter.min = ui->dateTimeFilterMin_dateTimeEdit->getDateTime();
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}
		void QAbstractLogWidget::onDateTimeFilterMaxNow_pushButton_clicked()
		{
			ui->dateTimeFilterMax_dateTimeEdit->setNow();
			m_dateTimeFilter.max = ui->dateTimeFilterMax_dateTimeEdit->getDateTime();
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}

		void QAbstractLogWidget::onDateTimeFilterType_changed(int index)
		{
			m_dateTimeFilter.rangeType = (DateTime::Range)index;
			if (m_dateTimeFilter.active)
				onDateTimeFilterChanged(m_dateTimeFilter);
		}





		void QAbstractLogWidget::onNewLogger(LogObject::Info loggerInfo)
		{
			ContextData data;
			data.checkBox = new QCheckBox(this);
			QPalette p = data.checkBox->palette();
			data.checkBox->setAutoFillBackground(true);
			p.setColor(QPalette::Button, loggerInfo.color.toQColor());
			data.checkBox->setPalette(p);
			data.checkBox->setChecked(true);
			data.checkBox->setText(loggerInfo.name.c_str());
			data.id = loggerInfo.id;
			QObject::connect(data.checkBox, &QCheckBox::stateChanged,
				this, &QAbstractLogWidget::onCheckBoxStateChangedSlot);
			m_contextData[loggerInfo.id] = data;
			ui->context_scrollAreaWidgetContents->layout()->addWidget(data.checkBox);
		}
		void QAbstractLogWidget::onLoggerInfoChanged(LogObject::Info info)
		{
			LOGGER_UNUSED(info);
		}
		void QAbstractLogWidget::onLogMessage(Message message)
		{
			//if(m_contextData.find(message.getLoggerID()) == m_contextData.end())
			//	onNewLogger(LogManager::getLogObjectInfo(message.getLoggerID()));
			LOGGER_UNUSED(message);
		}
		void QAbstractLogWidget::onChangeParent(LoggerID childID, LoggerID newParentID)
		{
			LOGGER_UNUSED(childID);
			LOGGER_UNUSED(newParentID);
		}



		void QAbstractLogWidget::setContentWidget(QWidget* widget)
		{
			ui->content_frame->layout()->addWidget(widget);
		}
	

		void QAbstractLogWidget::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			LOGGER_UNUSED(index);
			LOGGER_UNUSED(level);
			LOGGER_UNUSED(isChecked);
		}
		void QAbstractLogWidget::onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text)
		{
			LOGGER_UNUSED(lineEdit);
			if (index > 0)
				return;
			for (auto& loggerData : m_contextData)
			{
				if (loggerData.second.checkBox->text().contains(text.c_str(), Qt::CaseInsensitive))
				{
					loggerData.second.checkBox->setVisible(true);
				}
				else
				{
					loggerData.second.checkBox->setVisible(false);
				}
			}
		}
		void QAbstractLogWidget::onContextCheckBoxChanged(const ContextData& context, bool isChecked)
		{
			LOGGER_UNUSED(context);
			LOGGER_UNUSED(isChecked);
			int checkedCount = 0;
			for (auto& loggerData : m_contextData)
				checkedCount += loggerData.second.checkBox->isChecked();
			m_ignoreAllContextCheckBox_signals = true;
			if (checkedCount == m_contextData.size())
				ui->allContext_checkBox->setCheckState(Qt::Checked);
			else if (checkedCount > 0)
				ui->allContext_checkBox->setCheckState(Qt::PartiallyChecked);
			else
				ui->allContext_checkBox->setCheckState(Qt::Unchecked);
			m_ignoreAllContextCheckBox_signals = false;
		}
	}
}
#endif