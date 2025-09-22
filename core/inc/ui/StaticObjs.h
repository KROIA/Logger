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
		NativeConsoleView* getConsoleView<NativeConsoleView>();
		template <>
		QConsoleView* getConsoleView<QConsoleView>();
		template <>
		QTreeConsoleView* getConsoleView<QTreeConsoleView>();
    }
}
#endif