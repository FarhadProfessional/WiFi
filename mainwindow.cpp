#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    findTimer = new QTimer();
    findTimer->setInterval(1000);
    connect(findTimer,&QTimer::timeout,this,&MainWindow::findActiveWirelesses);
    findTimer->start();
    connected = false;
    foundCount = 0;
    ui->treeWidgetWiFis->setColumnWidth(0,50);
    ui->treeWidgetWiFis->setColumnWidth(1,200);
    findActiveWirelesses();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::findActiveWirelesses()
{
    QNetworkConfigurationManager *ncm = new QNetworkConfigurationManager();
    connect(ncm, &QNetworkConfigurationManager::onlineStateChanged, this, &MainWindow::configurationStateChanged);

    netcfgList = ncm->allConfigurations();
    WiFisList.clear();
    for (auto &x : netcfgList)
    {
        if (x.bearerType() == QNetworkConfiguration::BearerWLAN)
        {
            if(x.name() == "")
                WiFisList << "Unknown(Other Network)";
            else
                WiFisList << x.name();

            //qDebug() << x.type();
        }
    }
    for(int i=0; i<WiFisList.size(); i++)
    {
        bool exist = false;
        QTreeWidgetItem * item = new QTreeWidgetItem();
        for(int j=0; j<ui->treeWidgetWiFis->topLevelItemCount(); j++)
        {
            QTreeWidgetItem *index = ui->treeWidgetWiFis->topLevelItem(j);
            QString str = index->text(1);
            if(str == WiFisList[i])
            {
                exist = true;
                break;
            }
        }
        if(!exist)
        {
            item->setTextAlignment(0,Qt::AlignVCenter);
            item->setTextAlignment(1,Qt::AlignHCenter);
            item->setText(0,QString::number(++foundCount));
            item->setText(1,WiFisList[i]);
            ui->treeWidgetWiFis->addTopLevelItem(item);
        }
    }
}

void MainWindow::connectionStatusOpened()
{
    connected = true;
    ui->btnConnect->setText("Disconnect");
    statusBarMessage("Successfully Connected.","green");
}

void MainWindow::connectionStatusClosed()
{
    connected = false;
    ui->btnConnect->setText("Connect");
    statusBarMessage("Successfully Disonnected.","red");
}

void MainWindow::on_btnConnect_clicked()
{
    if(!connected)
    {
        QString pass = QInputDialog::getText(this, "Password", "Enter Password:",QLineEdit::Password);
        if(pass.isEmpty()) return;

        QModelIndex index = ui->treeWidgetWiFis->currentIndex();
        QString str = index.data(Qt::DisplayRole).toString();
        for (auto &x : netcfgList)
        {
            if (x.bearerType() == QNetworkConfiguration::BearerWLAN)
            {
                if (x.name() == str)
                {
                    netcfg = x;
                    session = new QNetworkSession(netcfg, this);
                    connect(session,&QNetworkSession::opened,this,&MainWindow::connectionStatusOpened);
                    connect(session,&QNetworkSession::closed,this,&MainWindow::connectionStatusClosed);
                    session->open();
                    if(session->isOpen())
                    {
                        connected = true;
                        ui->btnConnect->setText("Disconnect");
                    }
                }
            }
        }
    }
    else
    {
        session->close();
        if(!session->isOpen())
        {
            connected = false;
            ui->btnConnect->setText("Connect");
        }
    }
}

void MainWindow::statusBarMessage(QString msg, QString color)
{
    ui->statusBar->clearMessage();
    ui->statusBar->setStyleSheet("color:"+color+";");
    ui->statusBar->showMessage(msg, 5000);
}

void MainWindow::configurationStateChanged()
{
    qDebug()<< "emited";
}
