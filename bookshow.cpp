#include "bookshow.h"
#include "ui_bookshow.h"

#include "saddwidget.h"

#include <QtSql/QSqlQuery>
#include <QPixmap>
#include <QDesktopServices>

BookShow::BookShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BookShow)
{
    ui->setupUi(this);

    QObject::connect(ui->editButton, SIGNAL(clicked()), this, SLOT(editBook()));
    QObject::connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteBook()));

    QObject::connect(ui->coverButton, SIGNAL(clicked()), this, SLOT(openBook()));
    QObject::connect(ui->authLine, SIGNAL(linkActivated(QString)), this, SIGNAL(linkAuthActivated(QString)));
    QObject::connect(ui->pubLine, SIGNAL(linkActivated(QString)), this, SIGNAL(linkPubActivated(QString)));
    QObject::connect(ui->serLine, SIGNAL(linkActivated(QString)), this, SIGNAL(linkSerActivated(QString)));
}

BookShow::~BookShow()
{
    delete ui;
}

void BookShow::setInfo(const int &id)
{
    book_id = id;

    QSqlQuery q;
    q.prepare("SELECT Title, (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS Publisher,\
               Volume, Edition, Series, Year, ISBN, Language, Filepath FROM Books WHERE Book_ID = :id");
    q.bindValue(":id", book_id);
    q.exec();
    if(q.next())
    {
        QStringList auth;
        QSqlQuery y;
        y.prepare("SELECT Name, Middle, Surname FROM Authors WHERE Auth_ID IN\
                   (SELECT Auth_ID FROM Author_Book WHERE Book_ID = :id)");
        y.bindValue(":id", book_id);
        y.exec();
        while(y.next())
            auth << QString("<a href=\"%1 %2 %3\">%1 %2 %3</a>").arg(y.value(0).toString()).arg(y.value(1).toString()).arg(y.value(2).toString());

        ui->authLine->setText(auth.join(", "));
        ui->titleLine->setText(q.value("Title").toString());
        ui->pubLine->setText(QString("<a href=\"%1\">%1</a>").arg(q.value("Publisher").toString()));
        ui->yearLine->setText(QString::number(q.value("Year").toInt()));
        ui->langLine->setText(q.value("Language").toString());
        ui->volLine->setText(QString::number(q.value("Volume").toInt()));
        ui->edLine->setText(QString::number(q.value("Edition").toInt()));
        ui->serLine->setText(QString("<a href=\"%1\">%1</a>").arg(q.value("Series").toString()));
        ui->isbnLine->setText(q.value("ISBN").toString());

        ui->coverButton->setToolTip(q.value("Filepath").toString());
    }

    QPixmap pix(142, 200);
    pix.fill();
    ui->coverButton->setIcon(QIcon(pix));
    ui->coverButton->setIconSize(pix.size());
}

void BookShow::editBook()
{
    SAddWidget *sedit = new SAddWidget(book_id);
    sedit->setAttribute(Qt::WA_DeleteOnClose);
    sedit->setWindowTitle("Toshokan - Edit book");
    sedit->show();

    QObject::connect(sedit, SIGNAL(bookSubmitted()), this, SIGNAL(bookSubmitted()));
    QObject::connect(sedit, SIGNAL(bookSubmitted()), sedit, SLOT(close()));
}

void BookShow::deleteBook()
{
    QSqlQuery q;
    q.prepare("DELETE FROM Books WHERE Book_ID = :id");
    q.bindValue(":id", book_id);
    q.exec();

    ui->authLine->clear();
    ui->titleLine->clear();
    ui->pubLine->clear();
    ui->volLine->clear();
    ui->edLine->clear();
    ui->isbnLine->clear();
    ui->yearLine->clear();
    ui->serLine->clear();

    emit bookDeleted();
}

void BookShow::openBook()
{
    QSqlQuery q;
    q.prepare("SELECT Filepath FROM Books WHERE Book_ID = :id");
    q.bindValue(":id", book_id);
    q.exec();
    if(q.next())
        QDesktopServices::openUrl(QUrl::fromLocalFile(q.value(0).toString()));
}
