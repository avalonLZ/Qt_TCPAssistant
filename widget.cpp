#include "widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->ledo->setVisible(false);
    ui->ledc->setVisible(true);

    ui->PB_Connect->setEnabled(true);
    ui->PB_Break->setDisabled(true);

    sc = new Socket_Client(ui);
    se = new Socket_Server(ui);
    connect(ui->PB_Connect, SIGNAL(clicked()), this, SLOT(PB_Connect()));

}

Widget::~Widget()
{
    delete ui;
}

void Widget::PB_Connect()
{
    QString CB_content = ui->CB_Select->currentText();
    static Connect_Select_t select = OTHER;
    /*锁死选项*/
    ui->CB_Select->setDisabled(true);

    if(CB_content == "Client")
    {
        sc->CreateConnect();

        if(select == SERVER)
        {
            disconnect(ui->PB_Send, SIGNAL(clicked()), se, SLOT(SendContext()));
            disconnect(ui->PB_Break, SIGNAL(clicked()), se, SLOT(BreakConnect()));
        }

        if(select == SERVER || select == OTHER)
        {
            /*关联槽，只能关联一次，关联多次会多次调用,原因是sc没有detele*/
            /*SIGNAL和SLOT中的函数的参数类型需要相同，此处都为无参数，如果带参数在此处也不可传入实参*/
            connect(ui->PB_Send, SIGNAL(clicked()), sc, SLOT(SendContext()));
            connect(ui->PB_Break, SIGNAL(clicked()), sc, SLOT(BreakConnect()));
            select = CLIENT;
        }
    }
    else if(CB_content == "Server")
    {
        if(select == CLIENT)
        {
            disconnect(ui->PB_Send, SIGNAL(clicked()), sc, SLOT(SendContext()));
            disconnect(ui->PB_Break, SIGNAL(clicked()), sc, SLOT(BreakConnect()));
        }

        if(select == CLIENT || select == OTHER)
        {
            /*关联槽*/
            connect(ui->PB_Send, SIGNAL(clicked()), se, SLOT(SendContext()));
            connect(ui->PB_Break, SIGNAL(clicked()), se, SLOT(BreakConnect()));
            select = SERVER;
        }

        if(se->server != NULL)
        {
            delete se->server;
        }
        se->server = new QTcpServer;
        quint16 tempport = ui->LE_Port->text().toUShort();
        se->server->listen(QHostAddress::Any, tempport);
        connect(this->se->server, SIGNAL(newConnection()), this->se, SLOT(CreateConnect()));
        ui->ledo->setVisible(true);
        ui->ledc->setVisible(false);
        ui->PB_Break->setEnabled(true);
        ui->PB_Connect->setDisabled(true);
        /*选项框失效*/
        //ui->LE_IP->setDisabled(true);
    }

}

void Widget::PB_Send()
{

}

void Widget::PB_Break()
{

}

/**************************************UI_Operate类*****************************************/

UI_Operate::UI_Operate(Ui::Widget *tempui)
{
    this->ui = tempui;
}

QString UI_Operate::LE_SendDisplay_ReceiveContent()
{
    return ui->LE_SendDisplay->text();
}

QString UI_Operate::LE_IP_ReceiveContent()
{
    return ui->LE_IP->text();
}

quint16 UI_Operate::LE_Port_ReceiveContent()
{
    quint16 tempport = ui->LE_Port->text().toUShort();
    return tempport;
}

bool UI_Operate::RB_Ascii_GetStatus()
{
    return ui->RB_Ascii->isChecked()? true : false;
}

bool UI_Operate::RB_Hex_GetStatus()
{
    return ui->RB_Hex->isChecked()? true : false;
}

bool UI_Operate::RB_Bin_GetStatus()
{
    return ui->RB_Bin->isChecked()? true : false;
}

void UI_Operate::TB_Display_DisplayContent(QString content)
{
    ui->TB_Display->append(content);
}

void UI_Operate::TB_LedOpen()
{
    ui->ledo->setVisible(true);
    ui->ledc->setVisible(false);
}

void UI_Operate::TB_LedClose()
{
    ui->ledo->setVisible(false);
    ui->ledc->setVisible(true);
}

void UI_Operate::PB_Break_DisableDisplay()
{
    ui->PB_Break->setDisabled(true);
}

void UI_Operate::PB_Break_EnableDisplay()
{
    ui->PB_Break->setEnabled(true);
}

void UI_Operate::PB_Connect_DisableDisplay()
{
    ui->PB_Connect->setDisabled(true);
}

void UI_Operate::PB_Connect_EnableDisplay()
{
    ui->PB_Connect->setEnabled(true);
}

void UI_Operate::CB_Select_Disable()
{
    ui->CB_Select->setDisabled(true);
}

void UI_Operate::CB_Select_Enable()
{
    ui->CB_Select->setEnabled(true);
}

/**************************************Socked_Client类*****************************************/
/*父类只有带参的构造函数，所以必须要传一个NULL给父类*/
Socket_Client::Socket_Client() : UI_Operate(NULL)
{
    this->client = NULL;
}

Socket_Client::Socket_Client(Ui::Widget *tepui) : UI_Operate(tepui)
{
    this->client = NULL;
}

void Socket_Client::CreateConnect()
{
    if(this->client != NULL)
    {
        delete this->client;
        this->client = new QTcpSocket;
    }
    else
    {
        this->client = new QTcpSocket;
    }

    QString ip = LE_IP_ReceiveContent();
    quint16 port = LE_Port_ReceiveContent();
    this->client->connectToHost(QHostAddress(ip), port);

    /*必须要使用waitForConnected方法才可判断是否连接成功*/
    bool connectok = this->client->waitForConnected();

    if(connectok == true)
    {
         connect(this->client, SIGNAL(readyRead()), this, SLOT(ReceiveContext()));
         TB_LedOpen();
         PB_Break_EnableDisplay();
         PB_Connect_DisableDisplay();
    }
    else
    {
        TB_LedClose();
        PB_Break_DisableDisplay();
        PB_Connect_EnableDisplay();
        CB_Select_Enable();
    }
}

void Socket_Client::SendContext()
{
    if(this->client != NULL)
    {
        if(this->client->state() == this->client->ConnectedState)
        {
            QString send = LE_SendDisplay_ReceiveContent();
            this->client->write(send.toStdString().c_str(), send.toStdString().length());
            send = "Output:" + send;
            TB_Display_DisplayContent(send);
        }
        else
        {
            delete this->client;
            this->client = NULL;
            TB_Display_DisplayContent("Not Connect!");
            TB_LedClose();
            PB_Break_DisableDisplay();
            PB_Connect_EnableDisplay();
            CB_Select_Enable();
        }

    }
    else
    {
        TB_Display_DisplayContent("Not Connect!");
        TB_LedClose();
        PB_Break_DisableDisplay();
        PB_Connect_EnableDisplay();
        CB_Select_Enable();
    }

}

void Socket_Client::ReceiveContext()
{
    QByteArray tempreceive = this->client->readAll();
    QString receive = QVariant(tempreceive).toString();

    if(RB_Hex_GetStatus())
    {
        QByteArray temphex;
        int temp = 0;
        int len = 0;
        int i = 0;

        /*转为16进制显示*/
        temphex = tempreceive.toHex();
        len = temphex.length();

        for(; len > 0; len--)
        {
            if(len % 2 == 0)
            {
                temp = len;
                for(i = temphex.length() + 1; i >= temp; i--)
                {
                    temphex[i] = temphex[i - 1];
                }
                temphex[temp] = ' ';
            }
        }
        receive = QString::fromStdString(temphex.toStdString());
    }
    else if(RB_Bin_GetStatus())
    {
        QByteArray tempbin;
        int i,k = 0;
        bool temp = 0;

        /*千万不能转成hex，直接接收的数据在QByteArray中就是以ASCII码的形式出现*/
        char *tempchar = tempreceive.data();
        int len = tempreceive.length();

        for(i = 0; i < len; i++)
        {

           for(k = 0; k < 8; k++)
           {
                temp = tempchar[i] & 0x80;

                if(temp)
                {
                    tempbin[i*8+k+i] = '1';
                }
                else
                {
                    tempbin[i*8+k+i] = '0';
                }

                tempchar[i] = tempchar[i] << 1;
            }
            tempbin[i*8+k+i+1] = ' ';
        }

       receive = QString::fromStdString(tempbin.toStdString());
    }

    receive = "Input:" + receive;
    this->client->flush();
    TB_Display_DisplayContent(receive);
}

void Socket_Client::BreakConnect()
{
    if(this->client != NULL)
    {
        if(this->client->state() == this->client->ConnectedState)
        {
           this->client->disconnectFromHost();
        }
        delete this->client;
        this->client = NULL;
    }
    TB_LedClose();
    PB_Break_DisableDisplay();
    PB_Connect_EnableDisplay();
    CB_Select_Enable();
}

/**************************************Socked_Server类*****************************************/
Socket_Server::Socket_Server() : UI_Operate(NULL)
{
    server = NULL;
    buildserver = NULL;
}

Socket_Server::Socket_Server(Ui::Widget *tepui) : UI_Operate(tepui)
{
    server = NULL;
    buildserver = NULL;
}

void Socket_Server::CreateConnect()
{  
    /*此处之后需要改为多线程,目前只可以连一个Client*/
    buildserver = server->nextPendingConnection();
    connect(this->buildserver, SIGNAL(readyRead()), this, SLOT(ReceiveContext()));

}

void Socket_Server::SendContext()
{
    if(this->buildserver != NULL)
    {
        if(this->buildserver->state() == this->buildserver->ConnectedState)
        {
            QString send = LE_SendDisplay_ReceiveContent();
            this->buildserver->write(send.toStdString().c_str(), send.toStdString().length());
            send = "Output:" + send;
            TB_Display_DisplayContent(send);
        }
        else
        {
            TB_Display_DisplayContent("Not Client!");
        }

    }
    /*与Client不同，Server只要不主动Break，被连接一次后buildserver总非NULL*/
    else
    {
        TB_Display_DisplayContent("Not Client!");
    }
}

void Socket_Server::BreakConnect()
{
    if(buildserver != NULL)
    {
        buildserver->close();
        delete buildserver;
        buildserver = NULL;
    }

    if(server != NULL)
    {
        delete server;
        server = NULL;
    }

    TB_LedClose();
    PB_Break_DisableDisplay();
    PB_Connect_EnableDisplay();
    CB_Select_Enable();
}

void Socket_Server::ReceiveContext()
{
    QByteArray tempreceive = this->buildserver->readAll();
    QString receive = QVariant(tempreceive).toString();

    if(RB_Hex_GetStatus())
    {
        QByteArray temphex;
        int temp = 0;
        int len = 0;
        int i = 0;

        /*转为16进制显示*/
        temphex = tempreceive.toHex();
        len = temphex.length();

        for(; len > 0; len--)
        {
            if(len % 2 == 0)
            {
                temp = len;
                for(i = temphex.length() + 1; i >= temp; i--)
                {
                    temphex[i] = temphex[i - 1];
                }
                temphex[temp] = ' ';
            }
        }
        receive = QString::fromStdString(temphex.toStdString());
    }
    else if(RB_Bin_GetStatus())
    {
        QByteArray tempbin;
        int i,k = 0;
        bool temp = 0;

        /*千万不能转成hex，直接接收的数据在QByteArray中就是以ASCII码的形式出现*/
        char *tempchar = tempreceive.data();
        int len = tempreceive.length();

        for(i = 0; i < len; i++)
        {

           for(k = 0; k < 8; k++)
           {
                temp = tempchar[i] & 0x80;

                if(temp)
                {
                    tempbin[i*8+k+i] = '1';
                }
                else
                {
                    tempbin[i*8+k+i] = '0';
                }

                tempchar[i] = tempchar[i] << 1;
            }
            tempbin[i*8+k+i+1] = ' ';
        }

       receive = QString::fromStdString(tempbin.toStdString());
    }

    receive = "Input:" + receive;
    this->buildserver->flush();
    TB_Display_DisplayContent(receive);
}
