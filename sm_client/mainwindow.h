#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QAbstractSocket>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void send_data();

public slots:
    void sock_disconnect_connect();
    void sock_send();

    void sock_rev();
    void sock_ready_read();
    void sock_error(QAbstractSocket::SocketError error);

    void ip_port_enable(bool v);

    // data
    void stateChanged(QAbstractSocket::SocketState socketState);
private:
    Ui::MainWindow *ui;
    QTcpSocket *m_socket;
};

#endif // MAINWINDOW_H
