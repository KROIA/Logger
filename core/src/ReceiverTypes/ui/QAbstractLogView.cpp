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
		QAbstractLogView::QAbstractLogView(QWidget* parent)
			: QWidget(parent)
			, ContextReceiver()
			, ui(new Ui::QAbstractLogView)
		{
			ui->setupUi(this);
			ui->context_scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop);
			ui->searchIcon_label->setPixmap(Resources::getIconSearch().pixmap(16,16));

			m_autoCreateNewCheckBoxForNewContext = true;
			m_filterTextEdits = { ui->filter_lineEdit };
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
		}
		QAbstractLogView::~QAbstractLogView()
		{

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

		}

		void QAbstractLogView::onContextCreate(Logger::ContextLogger& logger)
		{
			addContext(logger);		
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
				data->checkBox->setAutoFillBackground(true);

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
		void QAbstractLogView::onContextCheckBoxChanged(ContextData const* context, bool isChecked)
		{
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