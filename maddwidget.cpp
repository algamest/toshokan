#include "maddwidget.h"
#include "ui_maddwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtSql/QSqlQuery>

MAddWidget::MAddWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MAddWidget)
{
    ui->setupUi(this);

    dirmodel = new QFileSystemModel();
    dirmodel->setRootPath(QDir::currentPath());
    dirmodel->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    ui->treeView->setModel(dirmodel);
    ui->treeView->setColumnHidden(1,true); // size
    ui->treeView->setColumnHidden(2,true); // mime
    ui->treeView->setColumnHidden(3,true); // date modified
    ui->treeView->setSelectionMode(QTreeView::ExtendedSelection);
    ui->treeView->setSelectionBehavior(QTreeView::SelectItems);

    listmodel = new QStringListModel(filelist);
    ui->listView->setModel(listmodel);
}

MAddWidget::~MAddWidget()
{
    delete ui;
    delete dirmodel;
    delete listmodel;
}

void MAddWidget::scanDir(const QString &dirpath, const QStringList &ftr)
{
    QDir dir(dirpath);
    foreach (QString entry, dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files))
    {
        QDir r(dir.absoluteFilePath(entry));
        if(r.exists())
            scanDir(dir.absoluteFilePath(entry), ftr);
        else
        {
            if(ftr.contains(dir.absoluteFilePath(entry).split(".").back().toLower()) || ftr.isEmpty())
                filelist << dir.absoluteFilePath(entry);
        }
    }
}

void MAddWidget::getFiles(const QStringList &ftr)
{
    foreach(QModelIndex index, ui->treeView->selectionModel()->selectedIndexes())
    {
        if(dirmodel->isDir(index))
            scanDir(dirmodel->filePath(index), ftr);
        else
        {
            if(ftr.contains(dirmodel->filePath(index).split(".").back().toLower()) || ftr.isEmpty())
                filelist << dirmodel->filePath(index);
        }
    }
}

void MAddWidget::on_clearButton_clicked()
{
    filelist.clear();
    listmodel->setStringList(filelist);
}

void MAddWidget::on_listButton_clicked()
{
    filelist.clear();

    QStringList filters;
    if(ui->checkCHMButton->isChecked())
        filters << "chm";
    if(ui->checkDJVUButton->isChecked())
        filters << "djvu" << "djv";
    if(ui->checkPDFButton->isChecked())
        filters << "pdf";
    if(ui->checkAllBox->isChecked())
        filters.clear();

    getFiles(filters);

    listmodel->setStringList(filelist);
}

void MAddWidget::on_processButton_clicked()
{
    QMessageBox::information(this, "Info", "This option will add all matching books to your library. You should edit each book later manually.");

    QSqlQuery q;
    int win = 0;
    int dup = 0;

    q.exec("SELECT NULL FROM Publishers WHERE Publisher = 'Unknown Publisher'");
    if(!q.next())
        q.exec("INSERT INTO Publishers(Publisher) VALUES('Unknown Publisher')");

    int pub_id = -1;
    q.exec("SELECT Pub_ID FROM Publishers WHERE Publisher = 'Unknown Publisher'");
    if(q.next())
        pub_id = q.value(0).toInt();

    q.exec("SELECT NULL FROM Authors WHERE Surname = 'Unknown'");
    if(!q.next())
        q.exec("INSERT INTO Authors(Name, Middle, Surname) VALUES('Unknown', 'Unknown', 'Unknown')");

    int auth_id = -1;
    q.exec("SELECT Auth_ID FROM Authors WHERE Surname = 'Unknown'");
    if(q.next())
        auth_id = q.value(0).toInt();

    QProgressDialog prog;
    prog.setWindowModality(Qt::WindowModal);
    prog.setWindowTitle("Toshokan - Adding books");
    prog.setMaximumWidth(500);
    int i = 0;
    prog.setRange(i, filelist.size());

    foreach(QString entry, filelist)
    {
        QString name(entry.split("/").back());
        prog.setValue(++i);
        prog.setLabelText(name);
        if(prog.wasCanceled())
            break;

        q.prepare("SELECT NULL FROM Books WHERE Filepath = :path");
        q.bindValue(":path", entry);
        q.exec();
        if(q.next())
        {
            dup++;
            continue;
        }

        q.prepare("INSERT INTO Books(Title, Pub_ID, Series, Year, Filepath) VALUES(:tit, :pid, 'None', '0', :path)");
        q.bindValue(":tit", name);
        q.bindValue(":pid", pub_id);
        q.bindValue(":path", entry);
        if(!q.exec())
            continue;

        int book_id = -1;
        q.exec("SELECT MAX(Book_ID) FROM Books");
        if(q.next())
            book_id = q.value(0).toInt();

        q.prepare("INSERT INTO Author_Book VALUES(:aid, :bid)");
        q.bindValue(":aid", auth_id);
        q.bindValue(":bid", book_id);
        if(q.exec())
            win++;
    }

    QMessageBox::information(this, "Success",
                             QString("%1/%2 (%3 were already present) books were added successfully.").arg(win).arg(filelist.size()).arg(dup));

    emit bookSubmitted();
}
