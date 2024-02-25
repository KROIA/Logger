#include "ui/QAbstractContextLoggerView.h"

#ifdef LOGGER_QT
#include <QLayout>

namespace Log
{
	namespace UI
	{

		/*void QAbstractContextLoggerView::connectLogger(Logger::ContextLogger& logger)
		{
			if (m_loggerData.find(&logger) != m_loggerData.end())
				return;
			addContextRecursive(logger);
			logger.connect_onContextCreate_slot(this, &QAbstractContextLoggerView::onContextCreate);
			logger.connect_onContextDestroy_slot(this, &QAbstractContextLoggerView::onContextDestroy);
			logger.connect_onDelete_slot(this, &QAbstractContextLoggerView::onDeletePrivate);
		}
		void QAbstractContextLoggerView::disconnectLogger(Logger::ContextLogger& logger)
		{
			if (m_loggerData.find(&logger) == m_loggerData.end())
				return;
			logger.disconnect_onContextCreate_slot(this, &QAbstractContextLoggerView::onContextCreate);
			logger.disconnect_onContextDestroy_slot(this, &QAbstractContextLoggerView::onContextDestroy);
			logger.disconnect_onDelete_slot(this, &QAbstractContextLoggerView::onDeletePrivate);
			onContextDestroy(logger);
		}*/
		QAbstractContextLoggerView::QAbstractContextLoggerView(QObject* parent)
		{
			m_signalHandler.m_parent = this;
		}
		QAbstractContextLoggerView::~QAbstractContextLoggerView()
		{

		}


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
					m_parent->onFilterTextChanged(i,lineEdit, text.toStdString());
					break;
				}
			}
		}
		void QSignalHandler::onCheckBoxStateChangedSlot(int state)
		{
			QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
			if (checkBox == nullptr)
				return;

			for (const auto &contextData : m_parent->m_contextData)
			{
				if (contextData.second->checkBox == checkBox)
				{
					m_parent->onContextCheckBoxChanged(contextData.second, state == Qt::Checked);
					break;
				}
			}
		}

		void QAbstractContextLoggerView::setupComponents(
			QWidget* parent,
			const std::vector<QLineEdit*>& filterTextEdits,
			QLayout* levelCheckButtonContainer,
			bool autoCreateNewCheckBoxForNewContext)
		{
			m_autoCreateNewCheckBoxForNewContext = autoCreateNewCheckBoxForNewContext;
			m_parent = parent;
			m_filterTextEdits = filterTextEdits;
			for(size_t i=0; i< m_filterTextEdits.size(); ++i)
			{
				QObject::connect(m_filterTextEdits[i], &QLineEdit::textChanged,
						&m_signalHandler, &QSignalHandler::onFilterTextChangedSlot);
			}

			for (int i = 0; i < Level::__count; ++i)
			{
				QCheckBox* checkBox = new QCheckBox(parent);
				checkBox->setChecked(true);
				checkBox->setText(getLevelStr((Level)i).c_str());
				checkBox->setIcon(getIcon((Level)i));
				QObject::connect(checkBox, &QCheckBox::stateChanged,
						&m_signalHandler, &QSignalHandler::onLevelCheckBoxStateChangedSlot);
				m_levelCheckBoxes[i] = checkBox;
				if (levelCheckButtonContainer)
					levelCheckButtonContainer->addWidget(checkBox);
			}
		}
		void QAbstractContextLoggerView::onNewSubscribed(Logger::AbstractLogger& logger)
		{
			if (auto* contextLogger = dynamic_cast<Logger::ContextLogger*>(&logger))
				addContextRecursive(*contextLogger);

		}
		void QAbstractContextLoggerView::onUnsubscribed(Logger::AbstractLogger& logger)
		{

		}
		void QAbstractContextLoggerView::onContextCreate(Logger::ContextLogger& logger)
		{
			if (m_autoCreateNewCheckBoxForNewContext)
			{
				ContextData * data = new ContextData();
				data->logger = &logger;
				data->checkBox = new QCheckBox(m_parent);
				data->checkBox->setChecked(true);
				data->checkBox->setText(logger.getName().c_str());
				QPalette p = data->checkBox->palette();
				//p.setColor(QPalette::Active, QPalette::WindowText, logger.getColor().toQColor());
				//p.setColor(QPalette::WindowText, logger.getColor().toQColor());
				data->checkBox->setAutoFillBackground(true);

				p.setColor(QPalette::Button, logger.getColor().toQColor());

				data->checkBox->setPalette(p);
				QObject::connect(data->checkBox, &QCheckBox::stateChanged,
						&m_signalHandler, &QSignalHandler::onCheckBoxStateChangedSlot);
				m_contextData[&logger] = data;
				onNewContextCheckBoxCreated(data);
			}
		}
		void QAbstractContextLoggerView::onContextDestroy(Logger::ContextLogger& logger)
		{
			const auto& it = m_contextData.find(&logger);
			if (it == m_contextData.end())
				return;
			ContextData * data = it->second;
			onContextCheckBoxDestroyed(data);
			m_contextData.erase(&logger);
			delete data->checkBox;
			delete data;

			for (auto& child : logger.getChilds())
			{
				onContextDestroy(*child);
			}
		}

		

		void QAbstractContextLoggerView::onNewMessage(const Message& m)
		{

		}
		void QAbstractContextLoggerView::onClear(Logger::AbstractLogger& logger)
		{

		}
		void QAbstractContextLoggerView::onDelete(Logger::AbstractLogger& logger)
		{

		}
		void QAbstractContextLoggerView::addContextRecursive(Logger::ContextLogger& logger)
		{
			onContextCreate(logger);
			for (auto& child : logger.getChilds())
			{
				addContextRecursive(*child);
			}
		}
	}
}
#endif