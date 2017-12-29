#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include "ui_widget.h"
#include <QMessageBox>
#include <QtNetwork/QtNetwork>
#include <winsock2.h>


namespace Ui {
class Widget;
}

class UI_Operate;
class Socket_Client;
class Socket_Server;

typedef enum
{
    CLIENT,
    SERVER,
    OTHER,
}Connect_Select_t;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    /*槽函数*/
private slots:
    void PB_Connect();
    void PB_Send();
    void PB_Break();

private:
    Ui::Widget *ui;
    Socket_Client *sc;
    Socket_Server *se;
};


/**************************************UI_Operate类*****************************************/

class UI_Operate : public QObject
{
    Q_OBJECT

private:
    Ui::Widget *ui;

public:
    UI_Operate(Ui::Widget *tempui);
    QString LE_SendDisplay_ReceiveContent();
    QString LE_IP_ReceiveContent();
    quint16 LE_Port_ReceiveContent();
    bool RB_Ascii_GetStatus();
    bool RB_Hex_GetStatus();
    bool RB_Bin_GetStatus();
    void TB_Display_DisplayContent(QString content);
    void TB_LedOpen();
    void TB_LedClose();
    void PB_Break_DisableDisplay();
    void PB_Break_EnableDisplay();
    void PB_Connect_DisableDisplay();
    void PB_Connect_EnableDisplay();
    void CB_Select_Disable();
    void CB_Select_Enable();
};

/**************************************Socked_Client类*****************************************/
/**********************************继承了操作UI类就可以对UI进行操作*********************************/
class Socket_Client : public UI_Operate
{
    Q_OBJECT

public:
    Socket_Client();
    /*需要操作UI就需要传递UI地址*/
    Socket_Client(Ui::Widget *tepui);
    void CreateConnect();

private:
    QTcpSocket *client;

    /*槽函数*/
private slots:
    void ReceiveContext();
    void SendContext();
    void BreakConnect();
};


/**************************************Socked_Server类*****************************************/
class Socket_Server : public UI_Operate
{
    Q_OBJECT
public:
    Socket_Server();
    Socket_Server(Ui::Widget *tepui);

    QTcpServer *server;
private:

    QTcpSocket *buildserver;

private slots:
    /*槽函数必须要在cpp中定义*/
    void CreateConnect();
    void ReceiveContext();
    void SendContext();
    void BreakConnect();
};

#endif // WIDGET_H
