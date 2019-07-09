#ifndef BOOKSHOW_H
#define BOOKSHOW_H

#include <QWidget>

namespace Ui {
class BookShow;
}

class BookShow : public QWidget
{
    Q_OBJECT

public:
    explicit BookShow(QWidget *parent = 0);
    ~BookShow();

    void setInfo(int id);

private:
    Ui::BookShow *ui;

    int book_id;

private slots:
    void editBook();
    void deleteBook();
    void openBook();

signals:
    void bookDeleted();
    void bookSubmitted();
    void linkAuthActivated(QString);
    void linkPubActivated(QString);
    void linkSerActivated(QString);
};

#endif // BOOKSHOW_H
