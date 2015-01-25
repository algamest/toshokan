#ifndef TOSHVIEW_H
#define TOSHVIEW_H

#include "bookshow.h"

#include <QMainWindow>
#include <QStringListModel>
#include <QtSql/QSqlQueryModel>

namespace Ui {
class ToshView;
}

class ToshView : public QMainWindow
{
    Q_OBJECT

public:
    explicit ToshView(QWidget *parent = 0);
    ~ToshView();

private:
    Ui::ToshView *ui;
    QStringListModel *listmodel;
    QSqlQueryModel *tablemodel;

private slots:
    void addBook();
    void updateList(const QString &str);
    void updateTable(const QModelIndex &index);
    void refreshModels();
    void showBookInfo(const QModelIndex &index);
    void findBook();

    void updateAuthLink(const QString &link);
    void updatePubLink(const QString &link);
    void updateSerLink(const QString &link);
};

#endif // TOSHVIEW_H
