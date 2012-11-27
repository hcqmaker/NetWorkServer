#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_socket = new QTcpSocket(this);
    QObject::connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this,
                     SLOT(stateChanged(QAbstractSocket::SocketState)));
    QObject::connect(m_socket, SIGNAL(readyRead()), this, SLOT(sock_ready_read()));
    QObject::connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(sock_error(QAbstractSocket::SocketError)));


    QObject::connect(ui->m_btnConnect, SIGNAL(clicked()), this, SLOT(sock_disconnect_connect()));
    QObject::connect(ui->m_btnSend, SIGNAL(clicked()), this, SLOT(sock_send()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

//---------------------------------------------
//
void MainWindow::sock_disconnect_connect()
{

    if (m_socket->state() == QAbstractSocket::ConnectingState)
    {
        ip_port_enable(false);
        return;
    }


    QString tmpIp = ui->m_txtIP->text();
    int tmpPort = ui->m_txtPort->text().toInt();
    if (m_socket->state() == QAbstractSocket::ConnectedState)
    {
        m_socket->disconnectFromHost();
        ip_port_enable(true);
    }
    else if (m_socket->state() == QAbstractSocket::UnconnectedState ||
             m_socket->state() == QAbstractSocket::ClosingState)
    {
        m_socket->connectToHost(tmpIp, tmpPort);
        ip_port_enable(false);
    }

}

//---------------------------------------------
//
void MainWindow::send_data()
{
    QString tmpData = ui->m_txtInput->text();
    m_socket->write(tmpData.toLocal8Bit());
    m_socket->flush();
}

//---------------------------------------------
//
void MainWindow::sock_ready_read()
{
    sock_rev();
}

//---------------------------------------------
//
void MainWindow::sock_error(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("Fortune Client"),
                                     tr("The host was not found. Please check the "
                                        "host name and port settings."));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("Fortune Client"),
                                     tr("The connection was refused by the peer. "
                                        "Make sure the fortune server is running, "
                                        "and check that the host name and port "
                                        "settings are correct."));
            break;
        default:
            QMessageBox::information(this, tr("Fortune Client"),
                                     tr("The following error occurred: %1.")
                                     .arg(m_socket->errorString()));
        }
}

//---------------------------------------------
//
void MainWindow::sock_send()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState)
    {
        send_data();
    }
}

//---------------------------------------------
//

void MainWindow::sock_rev()
{
    QByteArray tmpBytes = m_socket->readAll();
    ui->m_txtOutput->append(QString(tmpBytes));
}

//---------------------------------------------
//

void MainWindow::ip_port_enable(bool v)
{
    ui->m_txtIP->setEnabled(v);
    ui->m_txtPort->setEnabled(v);
}


//---------------------------------------------
//

void MainWindow::stateChanged(QAbstractSocket::SocketState socketState)
{

}
