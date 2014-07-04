#ifndef MADDWIDGET_H
#define MADDWIDGET_H

#include <QWidget>
#include <QFileSystemModel>
#include <QStringListModel>

namespace Ui {
class MAddWidget;
}

class MAddWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MAddWidget(QWidget *parent = 0);
    ~MAddWidget();

private:
    Ui::MAddWidget *ui;

    QFileSystemModel *dirmodel;
    QStringListModel *listmodel;
    QStringList filelist;

private slots:
    void getFiles(const QStringList &ftr);
    void scanDir(const QString &dirpath, const QStringList &ftr);
    void on_clearButton_clicked();
    void on_listButton_clicked();
    void on_processButton_clicked();

signals:
    void bookSubmitted();
};

#endif // MADDWIDGET_H
