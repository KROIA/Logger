#include "ReceiverTypes/ui/QAbstractLogView.h"

#ifdef LOGGER_QT
#include "ui_QAbstractLogView.h"
#include "Utilities/Resources.h"
#include <QTreeWidget>
#include <QMetaType>
#include <QSplitter>


namespace Log
{
	namespace UI
	{

		void QSignalHandler::onLevelCheckBoxStateChangedSlot(int state)
		{
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
			if (checkBox == nullptr)
				return;

			for (size_t i = 0; i < (size_t)Level::__count; ++i)
			{
				if (m_parent->m_levelCheckBoxes[i] == checkBox)
				{
					m_parent->onLevelCheckBoxChanged(i, (Level)i, state == Qt::Checked);
					break;
				}
			}
		}
		void QSignalHandler::onFilterTextChangedSlot(const QString& text)
		{
			QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
			if (lineEdit == nullptr)
				return;

			for (size_t i = 0; i < m_parent->m_filterTextEdits.size(); ++i)
			{
				if (m_parent->m_filterTextEdits[i] == lineEdit)
				{
					m_parent->onFilterTextChanged(i, lineEdit, text.toStdString());
					break;
				}
			}
		}
		void QSignalHandler::onCheckBoxStateChangedSlot(int state)
		{
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
			if (checkBox == nullptr)
				return;

			for (const auto& contextData : m_parent->m_contextData)
			{
				if (contextData.second->checkBox == checkBox)
				{
					m_parent->onContextCheckBoxChanged(contextData.second, state == Qt::Checked);
					break;
				}
			}
		}


		QAbstractLogView::QAbstractLogView(QWidget* parent)
			: QWidget(parent)
			, ContextReceiver()
			, ui(new Ui::QAbstractLogView)
		{
			m_signalHandler.m_parent = this;
			ui->setupUi(this);
			//m_treeWidget = new QTreeWidget();

			//ui->tree_widget->layout()->addWidget(m_treeWidget);
			//ui->context_listView->setLayout(new QVBoxLayout());
			//ui->context_scrollAreaWidgetContents->setItemAlignment(Qt::AlignTop);
			ui->context_scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
			ui->searchIcon_label->setPixmap(Resources::getIconSearch().pixmap(16,16));

			/*QSplitter* splitter = new QSplitter(this);
			splitter->addWidget(ui->settings_frame);
			splitter->addWidget(ui->content_frame);
			splitter->resize(100, 1000);
			layout()->addWidget(splitter);*/


			//m_treeItem = new Receiver::QContextLoggerTree(m_treeWidget);



			m_autoCreateNewCheckBoxForNewContext = true;
			m_filterTextEdits = { ui->filter_lineEdit };
			for (size_t i = 0; i < m_filterTextEdits.size(); ++i)
			{
				QObject::connect(m_filterTextEdits[i], &QLineEdit::textChanged,
					&m_signalHandler, &QSignalHandler::onFilterTextChangedSlot);
			}

			for (int i = 0; i < Level::__count; ++i)
			{
				QCheckBox* checkBox = new QCheckBox(parent);
				checkBox->setChecked(true);
				checkBox->setText(Utilities::getLevelStr((Level)i).c_str());
				checkBox->setIcon(Utilities::getIcon((Level)i));
				QObject::connect(checkBox, &QCheckBox::stateChanged,
					&m_signalHandler, &QSignalHandler::onLevelCheckBoxStateChangedSlot);
				m_levelCheckBoxes[i] = checkBox;
				if (ui->logLevel_frame->layout())
					ui->logLevel_frame->layout()->addWidget(checkBox);
			}

			/*QAbstractContextLoggerView::setupComponents(
				this, 
				{ ui->filter_lineEdit }, 
				ui->logLevel_frame->layout(), 
				true);*/


			//m_updateTimer.setInterval(100);
			//connect(&m_updateTimer, &QTimer::timeout, this, &QAbstractLogView::onUpdateTimer);
			//connect(ui->filter_lineEdit, &QLineEdit::textChanged, this, &QAbstractLogView::onFilterTextChanged);
			//m_updateTimer.start();

			/*for (int i = 0; i<Level::__count; ++i)
			{
				QCheckBox* checkBox = new QCheckBox(this);
				checkBox->setChecked(true);
				checkBox->setText(getLevelStr((Level)i).c_str());
				checkBox->setIcon(getIcon((Level)i));
				//checkBox->setIcon(QIcon(":/icons/trace.png"));
				QObject::connect(checkBox, &QCheckBox::stateChanged, this, &QAbstractLogView::onLevelCheckBoxStateChanged);
				ui->logLevel_frame->layout()->addWidget(checkBox);
				m_levelCheckBoxes[i] = checkBox;
			}*/
			QObject::connect(ui->allContext_checkBox, &QCheckBox::stateChanged, this, &QAbstractLogView::onAllContextCheckBoxStateChanged);
		}
		QAbstractLogView::~QAbstractLogView()
		{

		}

		/*void QAbstractLogView::connectLogger(Logger::ContextLogger& logger)
		{
			if(m_loggerData.find(&logger) != m_loggerData.end())
				return;
			addContextRecursive(logger);
			m_treeItem->attachLogger(logger);
			logger.connect_onContextCreate_slot(this, &QAbstractLogView::onContextCreate);
			logger.connect_onContextDestroy_slot(this, &QAbstractLogView::onContextDestroy);
			logger.connect_onDelete_slot(this, &QAbstractLogView::onDeletePrivate);
		}
		void QAbstractLogView::disconnectLogger(Logger::ContextLogger& logger)
		{
			if(m_loggerData.find(&logger) == m_loggerData.end())
				return;
			m_treeItem->detachLogger(logger);
			logger.disconnect_onContextCreate_slot(this, &QAbstractLogView::onContextCreate);
			logger.disconnect_onContextDestroy_slot(this, &QAbstractLogView::onContextDestroy);
			logger.disconnect_onDelete_slot(this, &QAbstractLogView::onDeletePrivate);
			onContextDestroy(logger);
		}*/

		/*void QAbstractLogView::onUpdateTimer()
		{

		}*/
		/*void QAbstractLogView::onCheckBoxStateChanged(int state)
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
		
		/*void QAbstractLogView::onLevelCheckBoxStateChanged(int state)
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
		/*void QAbstractLogView::onFilterTextChanged(const QString& text)
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

		//void QAbstractLogView::onDeletePrivate(Logger::AbstractLogger& logger)
		//{
			//detachLogger(logger);
		//}
		/*void QAbstractLogView::addContextRecursive(Logger::ContextLogger& logger)
		{
			onContextCreate(logger);
			for(auto& child : logger.getChilds())
			{
				addContextRecursive(*child);
			}
		}*/
		/*void QAbstractLogView::onContextCreate(Logger::ContextLogger& logger)
		{
			
		}
		void QAbstractLogView::onContextDestroy(Logger::ContextLogger& logger)
		{
			
		}
		*/



		void QAbstractLogView::setContentWidget(QWidget* widget)
		{
			ui->content_frame->layout()->addWidget(widget);
		}
		void QAbstractLogView::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			addContext(logger);
			if (auto* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger))
				addChildContextRecursive(*contextLogger);
			//m_treeItem->attachLogger(logger);
		}
		void QAbstractLogView::onUnsubscribed(Logger::AbstractLogger& logger)
		{
			//QAbstractContextLoggerView::onUnsubscribed(logger);
			//m_treeItem->detachLogger(logger);
		}

		void QAbstractLogView::onContextCreate(Logger::ContextLogger& logger)
		{
			addContext(logger);
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
			QObject::connect(checkBox, &QCheckBox::stateChanged, this, &QAbstractLogView::onCheckBoxStateChanged);
			m_loggerData[&logger]->checkBox = checkBox;*/

			
		}
		void QAbstractLogView::onContextDestroy(Logger::AbstractLogger& logger)
		{
			
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
				//p.setColor(QPalette::Active, QPalette::WindowText, logger.getColor().toQColor());
				//p.setColor(QPalette::WindowText, logger.getColor().toQColor());
				data->checkBox->setAutoFillBackground(true);

				p.setColor(QPalette::Button, logger.getColor().toQColor());

				data->checkBox->setPalette(p);
				QObject::connect(data->checkBox, &QCheckBox::stateChanged,
					&m_signalHandler, &QSignalHandler::onCheckBoxStateChangedSlot);
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
			//m_treeItem->setLevelVisibility(level, isChecked);
		}
		void QAbstractLogView::onFilterTextChanged(size_t index, QLineEdit* lineEdit, const std::string& text)
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
		void QAbstractLogView::onContextCheckBoxChanged(ContextData* context, bool isChecked)
		{
			int checkedCount = 0;
			//m_treeItem->setContextVisibility(*context->logger, isChecked);
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
		void QAbstractLogView::onNewContextCheckBoxCreated(ContextData* context)
		{
			ui->context_scrollAreaWidgetContents->layout()->addWidget(context->checkBox);
		}
		void QAbstractLogView::onContextCheckBoxDestroyed(ContextData* context) 
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

		}
		void QAbstractLogView::onDelete(Logger::AbstractLogger& logger)
		{

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