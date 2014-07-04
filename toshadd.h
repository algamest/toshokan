#ifndef TOSHADD_H
#define TOSHADD_H

#include "saddwidget.h"
#include "maddwidget.h"

#include <QWidget>
#include <QTabWidget>
#include <QGridLayout>

class ToshAdd : public QWidget
{
    Q_OBJECT

public:
    explicit ToshAdd(QWidget *parent = 0);
    ~ToshAdd();

private:
    SAddWidget *single;
    MAddWidget *mult;

    QLayout *lyt;
    QTabWidget *tbs;

signals:
    void bookSubmitted();
};

#endif // TOSHADD_H
