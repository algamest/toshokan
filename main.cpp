#include <QApplication>
#include <QSplashScreen>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include "toshview.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QSplashScreen splash(QPixmap(":/img/tosho.png"));
    splash.show();
    a.processEvents();

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./Library.db");
    db.open();

    QSqlQuery q;
    q.exec("PRAGMA foreign_keys = ON");
    if(db.tables().empty())
    {
        q.exec("CREATE TABLE Publishers(Pub_ID INTEGER PRIMARY KEY, Publisher VARCHAR)");
        q.exec("CREATE TABLE Books(Book_ID INTEGER PRIMARY KEY, Title VARCHAR,\
                Pub_ID INTEGER, Volume INTEGER, Edition INTEGER, Series VARCHAR, Year INTEGER, ISBN VARCHAR, Language VARCHAR, Filepath VARCHAR,\
                FOREIGN KEY(Pub_ID) REFERENCES Publishers(Pub_ID))");

        q.exec("CREATE TABLE Authors(Auth_ID INTEGER PRIMARY KEY, Name VARCHAR, Middle VARCHAR, Surname VARCHAR)");
        q.exec("CREATE TABLE Author_Book(Auth_ID INTEGER, Book_ID INTEGER,\
                FOREIGN KEY(Auth_ID) REFERENCES Authors(Auth_ID) ON DELETE CASCADE,\
                FOREIGN KEY(Book_ID) REFERENCES Books(Book_ID) ON DELETE CASCADE)");
    }

    ToshView w;
    w.showMaximized();
    splash.finish(&w);

    return a.exec();
}
