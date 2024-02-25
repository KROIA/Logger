#include "ui/QContextLoggerTreeView.h"

#ifdef LOGGER_QT
#include "ui_QContextLoggerTreeView.h"
#include <QTreeWidget>
#include <QMetaType>


namespace Log
{
	/*class STATIC_RESOURCE_LOADER
	{
			STATIC_RESOURCE_LOADER()
			{
				// name of the resource file = "icons.qrc"
				Q_INIT_RESOURCE(icons);
			}
		static STATIC_RESOURCE_LOADER s_loader;
	};
	STATIC_RESOURCE_LOADER STATIC_RESOURCE_LOADER::s_loader;*/

	namespace UI
	{

		QContextLoggerTreeView::QContextLoggerTreeView(QWidget* parent)
			: QWidget(parent)
			, QAbstractContextLoggerView()
			, ui(new Ui::QContextLoggerTreeView)
		{
			ui->setupUi(this);
			m_treeWidget = new QTreeWidget();

			ui->tree_widget->layout()->addWidget(m_treeWidget);
			ui->context_listView->setLayout(new QVBoxLayout());
			ui->context_listView->setItemAlignment(Qt::AlignTop);
			ui->context_listView->layout()->setAlignment(Qt::AlignTop);

			m_treeItem = new Receiver::QContextLoggerTree(m_treeWidget);

			QAbstractContextLoggerView::setupComponents(
				this, 
				{ ui->filter_lineEdit }, 
				ui->logLevel_frame->layout(), 
				true);


			//m_updateTimer.setInterval(100);
			//connect(&m_updateTimer, &QTimer::timeout, this, &QContextLoggerTreeView::onUpdateTimer);
			//connect(ui->filter_lineEdit, &QLineEdit::textChanged, this, &QContextLoggerTreeView::onFilterTextChanged);
			//m_updateTimer.start();

			/*for (int i = 0; i<Level::__count; ++i)
			{
				QCheckBox* checkBox = new QCheckBox(this);
				checkBox->setChecked(true);
				checkBox->setText(getLevelStr((Level)i).c_str());
				checkBox->setIcon(getIcon((Level)i));
				//checkBox->setIcon(QIcon(":/icons/trace.png"));
				QObject::connect(checkBox, &QCheckBox::stateChanged, this, &QContextLoggerTreeView::onLevelCheckBoxStateChanged);
				ui->logLevel_frame->layout()->addWidget(checkBox);
				m_levelCheckBoxes[i] = checkBox;
			}*/
			QObject::connect(ui->allContext_checkBox, &QCheckBox::stateChanged, this, &QContextLoggerTreeView::onAllContextCheckBoxStateChanged);
		}
		QContextLoggerTreeView::~QContextLoggerTreeView()
		{

		}

		void QContextLoggerTreeView::setDateTimeFormat(DateTime::Format format)
		{
			m_treeItem->setDateTimeFormat(format);
		}
		DateTime::Format QContextLoggerTreeView::getDateTimeFormat() const
		{
			return m_treeItem->getDateTimeFormat();
		}
		/*void QContextLoggerTreeView::connectLogger(Logger::ContextLogger& logger)
		{
			if(m_loggerData.find(&logger) != m_loggerData.end())
				return;
			addContextRecursive(logger);
			m_treeItem->attachLogger(logger);
			logger.connect_onContextCreate_slot(this, &QContextLoggerTreeView::onContextCreate);
			logger.connect_onContextDestroy_slot(this, &QContextLoggerTreeView::onContextDestroy);
			logger.connect_onDelete_slot(this, &QContextLoggerTreeView::onDeletePrivate);
		}
		void QContextLoggerTreeView::disconnectLogger(Logger::ContextLogger& logger)
		{
			if(m_loggerData.find(&logger) == m_loggerData.end())
				return;
			m_treeItem->detachLogger(logger);
			logger.disconnect_onContextCreate_slot(this, &QContextLoggerTreeView::onContextCreate);
			logger.disconnect_onContextDestroy_slot(this, &QContextLoggerTreeView::onContextDestroy);
			logger.disconnect_onDelete_slot(this, &QContextLoggerTreeView::onDeletePrivate);
			onContextDestroy(logger);
		}*/

		/*void QContextLoggerTreeView::onUpdateTimer()
		{

		}*/
		/*void QContextLoggerTreeView::onCheckBoxStateChanged(int state)
		{
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(QObject::sender());
			if (checkBox == nullptr)
				return;
			int checkedCount = 0;
			bool hadMatch = false;
			for (auto& loggerData : m_contextData)
			{
				checkedCount += loggerData.second->checkBox->isChecked();
				if (hadMatch)
					continue;
				if (loggerData.second->checkBox == checkBox)
				{
					hadMatch = true;
					//loggerData.second->logger->setActive(state == Qt::Checked);
					m_treeItem->setContextVisibility(*loggerData.second->logger, checkBox->isChecked());
				}
			}
			m_ignoreAllContextCheckBox_signals = true;
			if (checkedCount == m_contextData.size())
			{
				ui->allContext_checkBox->setCheckState(Qt::Checked);
			}
			else if(checkedCount > 0)
			{
				ui->allContext_checkBox->setCheckState(Qt::PartiallyChecked);
			}
			else
			{
				ui->allContext_checkBox->setCheckState(Qt::Unchecked);
			}
			m_ignoreAllContextCheckBox_signals = false;
		}*/
		void QContextLoggerTreeView::onAllContextCheckBoxStateChanged(int state)
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
		/*void QContextLoggerTreeView::onLevelCheckBoxStateChanged(int state)
		{
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(QObject::sender());
			if (checkBox == nullptr)
				return;
			
			for (int i = 0; i < Level::__count; ++i)
			{
				if (m_levelCheckBoxes[i] == checkBox)
				{
					m_treeItem->setLevelVisibility((Level)i, checkBox->isChecked());
					break;
				}
			}
		}*/
		/*void QContextLoggerTreeView::onFilterTextChanged(const QString& text)
		{
			for (auto& loggerData : m_loggerData)
			{
				if (loggerData.second->checkBox->text().contains(text, Qt::CaseInsensitive))
				{
					loggerData.second->checkBox->setVisible(true);
				}
				else
				{
					loggerData.second->checkBox->setVisible(false);
				}
			}
		}*/

		//void QContextLoggerTreeView::onDeletePrivate(Logger::AbstractLogger& logger)
		//{
			//detachLogger(logger);
		//}
		/*void QContextLoggerTreeView::addContextRecursive(Logger::ContextLogger& logger)
		{
			onContextCreate(logger);
			for(auto& child : logger.getChilds())
			{
				addContextRecursive(*child);
			}
		}*/
		/*void QContextLoggerTreeView::onContextCreate(Logger::ContextLogger& logger)
		{
			
		}
		void QContextLoggerTreeView::onContextDestroy(Logger::ContextLogger& logger)
		{
			
		}
		*/




		void QContextLoggerTreeView::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractContextLoggerView::onNewSubscribed(logger);
			m_treeItem->attachLogger(logger);
		}
		void QContextLoggerTreeView::onUnsubscribed(Logger::AbstractLogger& logger)
		{
			QAbstractContextLoggerView::onUnsubscribed(logger);
			m_treeItem->detachLogger(logger);
		}

		void QContextLoggerTreeView::onContextCreate(Logger::ContextLogger& logger)
		{
			QAbstractContextLoggerView::onContextCreate(logger);
			/*m_loggerData[&logger] = new LoggerData();
			m_loggerData[&logger]->logger = &logger;
			QCheckBox* checkBox = new QCheckBox(this);
			checkBox->setChecked(true);
			checkBox->setText(logger.getName().c_str());
			QPalette p = checkBox->palette();
			//p.setColor(QPalette::Active, QPalette::WindowText, logger.getColor().toQColor());
			//p.setColor(QPalette::WindowText, logger.getColor().toQColor());
			checkBox->setAutoFillBackground(true);

			p.setColor(QPalette::Button, logger.getColor().toQColor());

			checkBox->setPalette(p);
			QObject::connect(checkBox, &QCheckBox::stateChanged, this, &QContextLoggerTreeView::onCheckBoxStateChanged);
			m_loggerData[&logger]->checkBox = checkBox;*/

			
		}
		void QContextLoggerTreeView::onContextDestroy(Logger::ContextLogger& logger)
		{
			QAbstractContextLoggerView::onContextDestroy(logger);
			/*if (m_loggerData.find(&logger) == m_loggerData.end())
				return;
			delete m_loggerData[&logger];
			m_loggerData.erase(&logger);
			for (auto& child : logger.getChilds())
			{
				onContextDestroy(*child);
			}*/
		}


		void QContextLoggerTreeView::onLevelCheckBoxChanged(size_t index, Level level, bool isChecked)
		{
			m_treeItem->setLevelVisibility(level, isChecked);
		}
		void QContextLoggerTreeView::onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text)
		{
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
		void QContextLoggerTreeView::onContextCheckBoxChanged(ContextData* context, bool isChecked)
		{
			int checkedCount = 0;
			m_treeItem->setContextVisibility(*context->logger, isChecked);
			for (auto& loggerData : m_contextData)
			{
				checkedCount += loggerData.second->checkBox->isChecked();
			}
			m_ignoreAllContextCheckBox_signals = true;
			if (checkedCount == m_contextData.size())
			{
				ui->allContext_checkBox->setCheckState(Qt::Checked);
			}
			else if (checkedCount > 0)
			{
				ui->allContext_checkBox->setCheckState(Qt::PartiallyChecked);
			}
			else
			{
				ui->allContext_checkBox->setCheckState(Qt::Unchecked);
			}
			m_ignoreAllContextCheckBox_signals = false;
		}
		void QContextLoggerTreeView::onNewContextCheckBoxCreated(ContextData* context)
		{
			ui->context_listView->layout()->addWidget(context->checkBox);
		}
		void QContextLoggerTreeView::onContextCheckBoxDestroyed(ContextData* context) 
		{
			ui->context_listView->layout()->removeWidget(context->checkBox);
		}
	}
}
#endif