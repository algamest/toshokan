#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>
#include <QDropEvent>


class ListView : public QListView
{
    Q_OBJECT
public:
    explicit ListView(QWidget *parent = 0);
protected:
    void dropEvent(QDropEvent *e);

};

#endif // LISTVIEW_H
