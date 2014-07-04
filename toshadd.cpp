#include "toshadd.h"

ToshAdd::ToshAdd(QWidget *parent) :
    QWidget(parent)
{
    tbs = new QTabWidget(this);

    single = new SAddWidget(-1 ,this);
    mult = new MAddWidget(this);

    tbs->addTab(single, "Single book");
    tbs->addTab(mult, "Multiple books");

    lyt = new QGridLayout(this);
    lyt->addWidget(tbs);
    this->setLayout(lyt);

    QObject::connect(single, SIGNAL(bookSubmitted()), this, SIGNAL(bookSubmitted()));
    QObject::connect(mult, SIGNAL(bookSubmitted()), this, SIGNAL(bookSubmitted()));
}

ToshAdd::~ToshAdd()
{
    delete single;
    delete mult;
    delete tbs;
    delete lyt;
}
