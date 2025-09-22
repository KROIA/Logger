#include "ui/StaticObjs.h"

namespace Log
{
	namespace UI
	{
		void createConsoleView(ConsoleViewType type)
		{
			switch (type)
			{
				case ConsoleViewType::nativeConsoleView:
					NativeConsoleView::createStaticInstance();
					break;
				case ConsoleViewType::qConsoleView:
					QConsoleView::createStaticInstance();
					break;
				case ConsoleViewType::qTreeConsoleView:
					QTreeConsoleView::createStaticInstance();
					break;
				default:
					break;
			}
		}
		void destroyConsoleView(ConsoleViewType type)
		{
			switch (type)
			{
				case ConsoleViewType::nativeConsoleView:
					NativeConsoleView::destroyStaticInstance();
					break;
				case ConsoleViewType::qConsoleView:
					QConsoleView::destroyStaticInstance();
					break;
				case ConsoleViewType::qTreeConsoleView:
					QTreeConsoleView::destroyStaticInstance();
					break;
				default:
					break;
			}
		}
		void destroyAllConsoleViews()
		{
			NativeConsoleView::destroyStaticInstance();
			QConsoleView::destroyStaticInstance();
			QTreeConsoleView::destroyStaticInstance();
		}

		template <>
		NativeConsoleView* getConsoleView<NativeConsoleView>()
		{
			return NativeConsoleView::getStaticInstance();
		}
		template <>
		QConsoleView* getConsoleView<QConsoleView>()
		{
			return QConsoleView::getStaticInstance();
		}
		template <>
		QTreeConsoleView* getConsoleView<QTreeConsoleView>()
		{
			return QTreeConsoleView::getStaticInstance();
		}
	}
}