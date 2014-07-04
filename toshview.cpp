#include "toshview.h"
#include "ui_toshview.h"

#include "toshadd.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QtSql/QSqlQuery>
#include <QSortFilterProxyModel>

ToshView::ToshView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ToshView)
{
    ui->setupUi(this);

    listmodel = new QStringListModel(this);
    ui->listView->setModel(listmodel);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tablemodel = new QSqlQueryModel(this);
    refreshModels();

    QSortFilterProxyModel *prx = new QSortFilterProxyModel(this);
    prx->setSourceModel(tablemodel);

    ui->tableView->setModel(prx);
    ui->tableView->setSelectionBehavior(QTableView::SelectRows);
    ui->tableView->setSelectionMode(QTableView::SingleSelection);
    ui->tableView->setHorizontalScrollMode(QTableView::ScrollPerPixel);
    //stretching last section in ui-file
    ui->tableView->setSortingEnabled(true);

    ui->bookwidget->hide();

    QObject::connect(ui->actionAdd_new_book, SIGNAL(triggered()), this, SLOT(addBook()));
    QObject::connect(ui->searchLine, SIGNAL(returnPressed()), this, SLOT(findBook()));
    QObject::connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateList(QString)));
    QObject::connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(updateTable(QModelIndex)));
    QObject::connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(showBookInfo(QModelIndex)));

    QObject::connect(ui->bookwidget, SIGNAL(bookDeleted()), this, SLOT(refreshModels()));
    QObject::connect(ui->bookwidget, SIGNAL(bookDeleted()), ui->bookwidget, SLOT(hide()));
    QObject::connect(ui->bookwidget, SIGNAL(bookSubmitted()), this, SLOT(refreshModels()));
    QObject::connect(ui->bookwidget, SIGNAL(bookSubmitted()), ui->bookwidget, SLOT(hide()));

    QObject::connect(ui->bookwidget, SIGNAL(linkAuthActivated(QString)), this, SLOT(updateAuthLink(QString)));
    QObject::connect(ui->bookwidget, SIGNAL(linkPubActivated(QString)), this, SLOT(updatePubLink(QString)));
    QObject::connect(ui->bookwidget, SIGNAL(linkSerActivated(QString)), this, SLOT(updateSerLink(QString)));
}

ToshView::~ToshView()
{
    delete ui;
    delete listmodel;
    delete tablemodel;

    QSqlDatabase::database().close();
}

void ToshView::addBook()
{
    ToshAdd *tadd = new ToshAdd();
    tadd->setAttribute(Qt::WA_DeleteOnClose);
    tadd->setWindowTitle("Toshokan - Add new book(s)");
    tadd->show();

    QObject::connect(tadd, SIGNAL(bookSubmitted()), this, SLOT(refreshModels()));
}

void ToshView::updateList(const QString &str)
{
    QStringList list;
    QSqlQuery q;

    if(str == "Publishers")
    {
        q.prepare("SELECT Pub_ID, Publisher FROM Publishers ORDER BY Publisher ASC");
        q.exec();
        while(q.next())
        {
            QSqlQuery y;
            y.prepare("SELECT NULL FROM Books WHERE Pub_ID = :pub_id");
            y.bindValue(":pub_id", q.value(0).toInt());
            y.exec();
            int i = 0;
            while(y.next())
                ++i;
            list.append(QString("%1\t(%2)").arg(q.value(1).toString()).arg(i));
        }
    }
    if(str == "Year")
    {
        q.prepare("SELECT DISTINCT Year FROM Books ORDER BY Year ASC");
        q.exec();
        while(q.next())
        {
            QSqlQuery y;
            y.prepare("SELECT NULL FROM Books WHERE Year = :y");
            y.bindValue(":y", q.value(0).toInt());
            y.exec();
            int i = 0;
            while(y.next())
                ++i;
            list.append(QString("%1\t(%2)").arg(q.value(0).toInt()).arg(i));
        }
    }
    if(str == "Authors")
    {
        q.prepare("SELECT Auth_ID, Name, Middle, Surname FROM Authors ORDER BY Surname ASC");
        q.exec();
        while(q.next())
            list.append(QString("%1, %2 %3").arg(q.value(3).toString()).arg(q.value(1).toString()).arg(q.value(2).toString()));
    }
    if(str == "Titles") // TODO add ukr letters
    {
        for(int code = 0x0030; code <= 0x0039; code++) // Digits
        {
            QChar qc(code);
            q.exec(QString("SELECT NULL FROM Books WHERE Title LIKE '%1%'").arg(qc));
            int i = 0;
            while(q.next())
                ++i;
            if(!i)
                continue;
            list.append(QString("%1\t(%2)").arg(qc).arg(i));
        }

        for(int code = 0x0041; code <= 0x005A; code++) // Latin
        {
            QChar qc(code);
            q.exec(QString("SELECT NULL FROM Books WHERE Title LIKE '%1%'").arg(qc));
            int i = 0;
            while(q.next())
                ++i;
            if(!i)
                continue;
            list.append(QString("%1\t(%2)").arg(qc).arg(i));
        }

        for(int code = 0x0400; code <= 0x04FF; code++) // Cyrillic
        {
            QChar qc(code);
            q.exec(QString("SELECT NULL FROM Books WHERE Title LIKE '%1%'").arg(qc));
            int i = 0;
            while(q.next())
                ++i;
            if(!i)
                continue;
            list.append(QString("%1\t(%2)").arg(qc).arg(i));
        }
    }
    if(str == "Series")
    {
        q.prepare("SELECT DISTINCT Series FROM Books ORDER BY Series ASC");
        q.exec();
        while(q.next())
        {
            QSqlQuery y;
            y.prepare("SELECT NULL FROM Books WHERE Series = :s");
            y.bindValue(":s", q.value(0).toString());
            y.exec();
            int i = 0;
            while(y.next())
                ++i;
            list.append(QString("%1\t(%2)").arg(q.value(0).toString()).arg(i));
        }
    }

    listmodel->setStringList(list);
}

void ToshView::updateTable(const QModelIndex &index)
{
    QString sender = index.data().toString();
    QString q;

    if(ui->comboBox->currentText() == "Titles")
    {
        QString l = sender.split("\t").front();
        q = QString("SELECT Title,\
                    (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                     WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                    (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS Publisher,\
                     Series, Year, Language, ISBN FROM Books WHERE Title LIKE '%1%'").arg(l);
    }
    if(ui->comboBox->currentText() == "Publishers")
    {
        QString p = sender.split("\t").front();
        q = QString("SELECT Title,\
                    (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                     WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                     '%1' AS 'Publisher',\
                     Series, Year, Language, ISBN FROM Books\
                     WHERE Pub_ID = (SELECT Pub_ID FROM Publishers WHERE Publisher = '%1')").arg(p);
    }
    if(ui->comboBox->currentText() == "Year")
    {
        QString y = sender.split("\t").front();
        q = QString("SELECT Title,\
                    (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                     WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                    (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS Publisher,\
                     Series, Year, Language, ISBN FROM Books WHERE Year = '%1'").arg(y);
    }
    if(ui->comboBox->currentText() == "Authors")
    {
        QStringList SNM = sender.split(QRegExp("\\,?\\s"));
        QString q_aid =
                QString("SELECT Auth_ID FROM Authors WHERE Surname = '%1' AND Name = '%2'").arg(SNM[0]).arg(SNM[1]);//.arg(SNM[2]);
        QString q_bid = QString("SELECT Book_ID FROM Author_Book WHERE Auth_ID IN (%1)").arg(q_aid);

        q = QString("SELECT Title,\
                    (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                     WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                    (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS Publisher,\
                     Series, Year, Language, ISBN FROM Books WHERE Book_ID IN (%1)").arg(q_bid);
    }
    if(ui->comboBox->currentText() == "Series")
    {
        QString s = sender.split("\t").front();
        q = QString("SELECT Title,\
                    (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                     WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                    (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS Publisher,\
                     Series, Year, Language, ISBN FROM Books WHERE Series = '%1'").arg(s);
    }

    q.append(" ORDER BY Title ASC");
    tablemodel->setQuery(q);
}

void ToshView::refreshModels()
{
    updateList(ui->comboBox->currentText());
    tablemodel->setQuery("SELECT Title,\
                         (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                          WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                         (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS 'Publisher',\
                          Series, Year, Language, ISBN FROM Books ORDER BY Title ASC");

    ui->tableView->resizeColumnsToContents();
}

void ToshView::showBookInfo(const QModelIndex &index)
{
    QString tit = ui->tableView->model()->index(index.row(), 0).data().toString(); // points to title column
    QString pub = ui->tableView->model()->index(index.row(), 2).data().toString(); // points to pub column
    QString year = ui->tableView->model()->index(index.row(), 4).data().toString(); // points to year column
//    QString isbn = ui->tableView->model()->index(index.row(), 6).data().toString(); // points to isbn column
    int book_id = -1;

    QSqlQuery q;
    q.prepare("SELECT Book_ID FROM Books WHERE Title = :tit\
               AND Pub_ID = (SELECT Pub_ID FROM Publishers WHERE Publisher = :pub)\
               AND Year = :y");
    q.bindValue(":tit", tit);
    q.bindValue(":pub", pub);
    q.bindValue(":y", year);
    if(q.exec() && q.next())
        book_id = q.value(0).toInt();

    ui->bookwidget->setInfo(book_id);
    ui->bookwidget->show();
}

void ToshView::findBook()
{
    QString text = ui->searchLine->text();
    if(text.isEmpty())
        return;

    QString q;
    q = QString("SELECT Title,\
                (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                 WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS 'Publisher',\
                 Series, Year, Language, ISBN FROM Books\
                 WHERE Title LIKE '%%1%' ORDER BY Title ASC").arg(text);

    tablemodel->setQuery(q);
}

void ToshView::updateAuthLink(const QString &link)
{
    QString q;

    QStringList NMS = link.split(" ");
    QString q_aid =
            QString("SELECT Auth_ID FROM Authors WHERE Name = '%1' AND Middle = '%2' AND Surname = '%3'").arg(NMS[0]).arg(NMS[1]).arg(NMS[2]);
    QString q_bid = QString("SELECT Book_ID FROM Author_Book WHERE Auth_ID = (%1)").arg(q_aid);

    q = QString("SELECT Title, (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS Publisher,\
                 Series, Year, Language FROM Books WHERE Book_ID IN (%1)").arg(q_bid);

    q.append(" ORDER BY Title ASC");
    tablemodel->setQuery(q);
}

void ToshView::updatePubLink(const QString &link)
{
    QString q;

    q = QString("SELECT Title,\
                (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                 WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                 '%1' AS 'Publisher',\
                 Series, Year, Language FROM Books\
                 WHERE Pub_ID = (SELECT Pub_ID FROM Publishers WHERE Publisher = '%1')").arg(link);

    q.append(" ORDER BY Title ASC");
    tablemodel->setQuery(q);
}

void ToshView::updateSerLink(const QString &link)
{
    QString q;

    q = QString("SELECT Title,\
                (SELECT group_concat((Name || ' ' || Middle || ' ' || Surname), ', ') FROM Authors\
                 WHERE Auth_ID IN (SELECT Auth_ID FROM Author_Book WHERE Book_ID = Books.Book_ID)) AS 'Author(s)',\
                (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS Publisher,\
                 Year, Language FROM Books WHERE Series = '%1'").arg(link);
    q.append(" ORDER BY Title ASC");
    tablemodel->setQuery(q);
}
