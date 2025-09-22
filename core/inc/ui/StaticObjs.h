#pragma once
#include "Logger_base.h"
#include "QtreeConsoleView.h"
#include "QConsoleView.h"
#include "NativeConsoleView.h"

#ifdef QT_WIDGETS_LIB


namespace Log
{
    namespace UI
    {
		void createConsoleView(ConsoleViewType type);
		void destroyConsoleView(ConsoleViewType type);
		void destroyAllConsoleViews();

		// Template function to get the console view
		template <typename T>
		T* getConsoleView()
		{
			return nullptr;
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
#endif