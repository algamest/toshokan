#include "saddwidget.h"
#include "ui_saddwidget.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

SAddWidget::SAddWidget(int id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SAddWidget),
    book_id(id)
{
    ui->setupUi(this);

    authmodel = new QStringListModel(this);
    ui->authView->setModel(authmodel);

    authcomp = new QCompleter(this);
    authcomp->setCaseSensitivity(Qt::CaseInsensitive);
    authcomp->setCompletionMode(QCompleter::PopupCompletion);
    authcomp->setFilterMode(Qt::MatchContains);
    ui->authLine->setCompleter(authcomp);

    pubcomp = new QCompleter(this);
    pubcomp->setCaseSensitivity(Qt::CaseInsensitive);
    pubcomp->setCompletionMode(QCompleter::PopupCompletion);
    pubcomp->setFilterMode(Qt::MatchContains);
    ui->pubLine->setCompleter(pubcomp);

    sercomp = new QCompleter(this);
    sercomp->setCaseSensitivity(Qt::CaseInsensitive);
    sercomp->setCompletionMode(QCompleter::PopupCompletion);
    sercomp->setFilterMode(Qt::MatchContains);
    ui->serLine->setCompleter(sercomp);

    refreshBoxes();

    book = new Book();

    if(book_id != -1)
        fillFields();

    QObject::connect(ui->authAdd, SIGNAL(clicked()), this, SLOT(addAuthor()));
    QObject::connect(ui->authLine, SIGNAL(returnPressed()), this, SLOT(addAuthor()));
    QObject::connect(ui->authView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(deleteAuthor(QModelIndex)));
}

SAddWidget::~SAddWidget()
{
    delete ui;
    delete book;
    delete authmodel;
    delete authcomp;
    delete pubcomp;
    delete sercomp;
}

void SAddWidget::on_getfileButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Select a book", QString(), "Books (*.chm *.pdf *.djvu *.dvj *.ps);;All (*.*)");
    if(file.isEmpty() || file.isNull())
    {
        QMessageBox::critical(this, "Error", "Select a book");
        return;
    }

    QSqlQuery q;
    q.prepare("SELECT NULL FROM Books WHERE Filepath = :file");
    q.bindValue(":file", file);
    if(q.exec() && q.next())
    {
        QMessageBox::information(this, "Info", "This book is already present in your library");
        return;
    }

    ui->pathLabel->setText(file);
}

void SAddWidget::on_submitButton_clicked()
{
    if(ui->pathLabel->text().isEmpty() || ui->pathLabel->text().isNull())
    {
        QMessageBox::critical(this, "Error", "Select a book");
        return;
    }

    if(ui->titleLine->text().isEmpty())
        book->title = "Unknown Title";
    else
        book->title = ui->titleLine->text();

    if(authmodel->stringList().isEmpty())
        book->authors << "Unknown Unknown";
    else
        book->authors = authmodel->stringList();

    if(ui->pubLine->text().isEmpty())
        book->publisher = "Unknown Publisher";
    else
        book->publisher = ui->pubLine->text();

    if(ui->serLine->text().isEmpty())
        book->series = "None";
    else
        book->series = ui->serLine->text();

    book->volume = ui->volBox->value();
    book->edition = ui->edBox->value();
    book->year = ui->yearLine->text().toInt();
    book->language = ui->langBox->currentText();
    book->isbn = ui->isbnLine->text().remove(QRegExp("\\s|\\-"));
    book->file_path = ui->pathLabel->text();

    submitBook();
    clearFields();
    refreshBoxes();
}

void SAddWidget::submitBook()
{
    QSqlQuery q;

    q.prepare("SELECT NULL FROM Publishers WHERE Publisher = :pub");
    q.bindValue(":pub", book->publisher);
    q.exec();
    if(!q.next())
    {
        q.prepare("INSERT INTO Publishers(Publisher) VALUES(:pub)");
        q.bindValue(":pub", book->publisher);
        if(!q.exec())
        {
            QMessageBox::critical(this, "Failure", QString("Errors were occured(publisher):\n%1").arg(q.lastError().text()));
            clearFields();
            return;
        }
    }

    if(book_id == -1)
    {
        q.exec("SELECT MAX(Book_ID) FROM Books");
        if(q.next())
            book_id = q.value(0).toInt();

        q.prepare("INSERT INTO Books(Title, Pub_ID, Volume, Edition, Series, Year, ISBN, Language, Filepath)\
                   VALUES(:title, (SELECT Pub_ID FROM Publishers WHERE Publisher = :pub), :vol, :ed, :ser, :year, :isbn, :lang, :path)");
    }
    else
    {
        q.prepare("UPDATE Books SET Title = :title,\
                   Pub_ID = (SELECT Pub_ID FROM Publishers WHERE Publisher = :pub),\
                   Volume = :vol, Edition = :ed, Series = :ser, Year = :year,\
                   ISBN = :isbn, Language = :lang, Filepath = :path\
                   WHERE Book_ID = :id");

        q.bindValue(":id", book_id);
    }
    q.bindValue(":title", book->title);
    q.bindValue(":pub", book->publisher);
    q.bindValue(":vol", book->volume);
    q.bindValue(":ed", book->edition);
    q.bindValue(":ser", book->series);
    q.bindValue(":year", book->year);
    q.bindValue(":isbn", book->isbn);
    q.bindValue(":lang", book->language);
    q.bindValue(":path", book->file_path);
    if(!q.exec())
    {
        QMessageBox::critical(this, "Failure", QString("Errors were occured(book):\n%1").arg(q.lastError().text()));
        clearFields();
        return;
    }

    q.prepare("DELETE FROM Author_Book WHERE Book_ID = :id");
    q.bindValue(":id", book_id);
    q.exec();

    foreach(QString entry, book->authors)
    {
        q.prepare("SELECT NULL FROM Authors WHERE Name = :auth");
        q.bindValue(":auth", entry);
        q.exec();
        if(!q.next())
        {
            q.prepare("INSERT INTO Authors(Author) Values(:auth)");
            q.bindValue(":auth", entry);
            if(!q.exec())
            {
                QMessageBox::critical(this, "Failure", QString("Errors were occured(author):\n%1").arg(q.lastError().text()));
                clearFields();
                return;
            }
        }

        int auth_id = -1;
        q.prepare("SELECT Auth_ID FROM Authors WHERE Author = :auth");
        q.bindValue(":auth", entry);
        if(q.exec() && q.next())
            auth_id = q.value(0).toInt();

        q.prepare("INSERT INTO Author_Book(Auth_ID, Book_ID) VALUES(:aid, :bid)");
        q.bindValue(":aid", auth_id);
        q.bindValue(":bid", book_id);
        if(!q.exec())
        {
            QMessageBox::critical(this, "Failure", QString("Errors were occured(link):\n%1").arg(q.lastError().text()));
            clearFields();
            return;
        }
    }

    emit bookSubmitted();
}

void SAddWidget::refreshBoxes()
{
    QSqlQuery q;

    QStringList pcomplist;
    q.exec("SELECT Publisher FROM Publishers");
    while(q.next())
        pcomplist << q.value(0).toString();
    pubcomp->setModel(new QStringListModel(pcomplist, this));

    q.exec("SELECT DISTINCT Language FROM Books ORDER BY Language ASC");
    while(q.next())
        ui->langBox->addItem(q.value(0).toString());

    QStringList scomplist;
    q.exec("SELECT DISTINCT Series FROM Books ORDER BY Series ASC");
    while(q.next())
        scomplist << q.value(0).toString();
    sercomp->setModel(new QStringListModel(scomplist, this));

    QStringList acomplist;
    q.exec("SELECT Author FROM Authors");
    while(q.next())
        acomplist << q.value(0).toString();
    authcomp->setModel(new QStringListModel(acomplist, this));
}

void SAddWidget::clearFields()
{
    authmodel->setStringList(QStringList());

    ui->volBox->clear();
    ui->isbnLine->clear();
    ui->langBox->clear();
    ui->pathLabel->clear();
    ui->pubLine->clear();
    ui->serLine->clear();
    ui->titleLine->clear();
    ui->volBox->clear();
    ui->yearLine->clear();
}

void SAddWidget::fillFields()
{
    QSqlQuery q;

    q.prepare("SELECT Title,\
              (SELECT Publisher FROM Publishers WHERE Pub_ID = Books.Pub_ID) AS 'Publisher',\
               Volume, Edition, Series, Year, ISBN, Language, Filepath FROM Books WHERE Book_ID = :id");
    q.bindValue(":id", book_id);
    q.exec();
    if(q.next())
    {
        ui->pathLabel->setText(q.value("Filepath").toString());
        ui->titleLine->setText(q.value("Title").toString());

        QSqlQuery y;
        y.prepare("SELECT Author FROM Authors WHERE Auth_ID IN "
                  "(SELECT Auth_ID FROM Author_Book WHERE Book_ID = :id)");
        y.bindValue(":id", book_id);
        y.exec();
        QStringList auth;
        while(y.next())
            auth << y.value(0).toString();
        authmodel->setStringList(auth);

        ui->volBox->setValue(q.value("Volume").toInt());
        ui->edBox->setValue(q.value("Edition").toInt());
        ui->yearLine->setText(QString::number(q.value("Year").toInt()));
        ui->langBox->setCurrentText(q.value("Language").toString());
        ui->pubLine->setText(q.value("Publisher").toString());
        ui->serLine->setText(q.value("Series").toString());
        ui->isbnLine->setText(q.value("ISBN").toString());
    }
}

void SAddWidget::addAuthor()
{
    if(ui->authLine->text().isEmpty())
    {
        QMessageBox::critical(this, "Error", "You cannot add empty author");
        return;
    }

    QStringList NMS = ui->authLine->text().trimmed().split(" ");

    auto sur = NMS.back();
    NMS.pop_back();
    auto name = NMS.join(" ");

    QStringList t = authmodel->stringList();
    t << QString("%1 %2").arg(name).arg(sur);
    authmodel->setStringList(t);

    ui->authLine->clear();
}

void SAddWidget::deleteAuthor(const QModelIndex &index)
{
    authmodel->removeRow(index.row());
}
