#pragma once
#include "Logger_base.h"

#ifdef LOGGER_QT
#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class QContextLoggerTreeView; }
QT_END_NAMESPACE

namespace Log 
{

    class LOGGER_EXPORT QContextLoggerTreeView : public QWidget
    {
        Q_OBJECT
    public:
        QContextLoggerTreeView(QWidget* parent = nullptr);
        ~QContextLoggerTreeView();

        void updateView();

    private:

        Ui::QContextLoggerTreeView* ui;


    };
}
#endif