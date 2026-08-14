#pragma once

#include "Logger_base.h"


#ifdef QT_WIDGETS_LIB
#include <QCheckBox>
#include <QObject>


namespace Log
{
	namespace Utilities
	{
		// Qt 6.9 deprecated QCheckBox::stateChanged(int) in favour of
		// checkStateChanged(Qt::CheckState). Both carry the same value, so the
		// library keeps its int-based slots (they are part of the public API and
		// are overridden by user subclasses) and only the connection differs.
		template<typename Receiver, typename Slot>
		QMetaObject::Connection connectCheckBoxStateChanged(QCheckBox* checkBox, Receiver* receiver, Slot slot)
		{
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
			return QObject::connect(checkBox, &QCheckBox::checkStateChanged, receiver,
				[receiver, slot](Qt::CheckState state)
				{
					(receiver->*slot)(static_cast<int>(state));
				});
#else
			return QObject::connect(checkBox, &QCheckBox::stateChanged, receiver, slot);
#endif
		}
	}
}
#endif
