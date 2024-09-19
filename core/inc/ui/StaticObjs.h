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

		template<typename T>
		T* getConsoleView()
		{
			// Return the console view of the specified type
			if (std::is_same<T, NativeConsoleView>::value)
				return NativeConsoleView::getStaticInstance();
			else if (std::is_same<T, QConsoleView>::value)
				return QConsoleView::getStaticInstance();
			else if (std::is_same<T, QTreeConsoleView>::value)
				return QTreeConsoleView::getStaticInstance();
			return nullptr;
		}
    }
}
#endif