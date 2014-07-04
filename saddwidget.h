#ifndef SADDWIDGET_H
#define SADDWIDGET_H

//=books=//

#include <QString>
#include <QStringList>
#include <QStringListModel>

class Book
{
public:
    Book() {}
    ~Book() {}

    QString file_path;
    QString file_ext;

    QString title;
    QStringList authors;
    QString publisher;
    QString series;
    QString language;
    int year;
    int edition;
    int volume;
    QString isbn;
};

class Issue
{
    Issue() {}
    ~Issue() {}

    QString magazine;
    QString title;
    QStringList authors;
    QString publisher;
    QString language;
    int year;
    int number;
    QString issn;
};

//=books=//

#include <QWidget>
#include <QCompleter>

namespace Ui {
class SAddWidget;
}

class SAddWidget : public QWidget
{
    Q_OBJECT

public:
    //explicit SAddWidget(QWidget *parent = 0);
    explicit SAddWidget(int book_id = -1, QWidget *parent = 0);
    ~SAddWidget();

private slots:
    void on_getfileButton_clicked();
    void on_submitButton_clicked();
    void on_fetchButton_clicked();

private:
    Ui::SAddWidget *ui;

    int book_id;
    Book *book;
    QStringListModel *authmodel;
    QCompleter *authcomp;
    QCompleter *pubcomp;
    QCompleter *sercomp;

    void submitBook();
    void refreshBoxes();
    void clearFields();
    void fillFields();

private slots:
    void addAuthor();
    void deleteAuthor(const QModelIndex &index);

signals:
    void bookSubmitted();
};

#endif // SADDWIDGET_H
