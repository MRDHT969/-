#include "widget.h"
#include "ui_widget.h"
#include "dathread.h"


//static const QLatin1String serviceUuid("00001101-0000-1000-8000-00805F9B34FB");
static const QLatin1String serviceUuid("0000fff0-0000-1000-8000-00805f9b34fb");
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //创建搜索服务：https://doc.qt.io/qt-5/qbluetoothdevicediscoveryagent.html
    discoveryAgent =new QBluetoothDeviceDiscoveryAgent(this);
    //设置BLE的搜索时间
    discoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    connect(discoveryAgent,SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),this,
            SLOT(addBlueToothDevicesToList(QBluetoothDeviceInfo)));//找到设备之后添加到列表显示出来
    connect(discoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));
    connect(discoveryAgent, SIGNAL(canceled()), this, SLOT(scanCanceled()));
    connect(this, SIGNAL(returnAddress(QBluetoothDeviceInfo)), this, SLOT(createCtl(QBluetoothDeviceInfo)));

    //开始进行设备搜索
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    this->showFullScreen();
    thread = new QThread;
    daThread* da=new daThread;
    da->moveToThread(thread);
    connect(thread,&QThread::finished,da,&daThread::deleteLater);
    connect(thread,&QThread::started,da,&daThread::Run);
    connect(this,SIGNAL(sendserial_signals(QByteArray)),da,SLOT(sendserial_slots(QByteArray)));
    connect(da,SIGNAL(sendVK_signal(double)),this,SLOT(sendVK_slot(double)));
    thread->start();
    setupPlot();
}

Widget::~Widget()
{
    delete ui;


}

void Widget::addBlueToothDevicesToList(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) //获取设备信息，并判断该设备是否为BLE设备
    {
        //格式化设备地址和设备名称
        QString label = QString("%1 %2").arg(info.address().toString()).arg(info.name());
        //检查设备是否已存在，避免重复添加
        QList<QListWidgetItem *> items = ui->listWidget_lanya->findItems(label, Qt::MatchExactly);

        //不存在则添加至设备列表
        if (items.empty())
        {
            QListWidgetItem *item = new QListWidgetItem(label);
            ui->listWidget_lanya->addItem(item);
            devicesList.append(info);
        }
    }
}

void Widget::on_pb_lianjie_clicked()
{
    //确认选取了某一个蓝牙设备
    if(!ui->listWidget_lanya->currentItem()->text().isEmpty())
    {
        //获取选择的地址
        QString bltAddress = ui->listWidget_lanya->currentItem()->text().left(17);

        for (int i = 0; i<devicesList.count(); i++)
        {
            //地址对比
            if(devicesList.at(i).address().toString().left(17) == bltAddress)
            {
                QBluetoothDeviceInfo choosenDevice = devicesList.at(i);
                //发送自定义signals==>执行slots:createCtl
                emit returnAddress(choosenDevice);
                //停止搜索服务
                discoveryAgent->stop();
                break;
            }
        }
    }
}
void Widget::createCtl(QBluetoothDeviceInfo choosenDeviceInfo)
{
    //创建中央控制器：https://doc.qt.io/qt-5/qlowenergycontroller.html
    m_BLEController = QLowEnergyController::createCentral(choosenDeviceInfo, this);

    //This signal is emitted each time a new service is discovered. The newService parameter contains the UUID of the found service.
    //每次发现新服务时都会发出serviceDiscovered信号，新服务的参数为其UUID值。
    //This signal can only be emitted if the controller is in the CentralRole.
    //这个信号只有这个控制器是中央控制器(CentralRole)的时候才能发射serviceDiscovered信号
    //connect(m_BLEController, &QLowEnergyController::serviceDiscovered,this, &Widget::serviceDiscovered);

    connect(m_BLEController, &QLowEnergyController::discoveryFinished,this, &Widget::serviceScanDone);

    //Lambda 函数写法
    connect(m_BLEController, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
           this, [this](QLowEnergyController::Error error)
    {
        Q_UNUSED(error)

        ui->label_zhuangtai->setText("错误");	//错误连接
    });

    connect(m_BLEController, &QLowEnergyController::connected, this, [this]()
    {
        ui->label_zhuangtai->setText("搜索服务");	//成功连接触发的槽函数
        m_BLEController->discoverServices();
    });

    connect(m_BLEController, &QLowEnergyController::disconnected, this, [this]()
    {
       ui->label_zhuangtai->setText("未连接");	//错误连接
    });

    ui->label_zhuangtai->setText("开始连接");

    m_BLEController->connectToDevice();//建立连接
}
//服务扫描完毕，调用如下函数
void Widget::serviceScanDone()
{

    //创建服务QLowEnergyService，此服务即是与TB-02模块通讯的服务
    m_bleServer = m_BLEController->createServiceObject(QBluetoothUuid(serviceUuid),this);
    if(m_bleServer)
    {
        ui->label_zhuangtai->setText("创建成功");
        m_bleServer->discoverDetails();
    }
    else
    {
        ui->label_zhuangtai->setText("未找到服务");
        return;
    }

    connect(m_bleServer, &QLowEnergyService::stateChanged, this,
            &Widget::serviceStateChanged);
    connect(m_bleServer, &QLowEnergyService::characteristicChanged, this,
            &Widget::BleServiceCharacteristicChanged);
    /*connect(m_bleServer, &QLowEnergyService::characteristicChanged, this,
                &Widget::XianShi);*/
    connect(m_bleServer, &QLowEnergyService::characteristicRead, this,
            &Widget::BleServiceCharacteristicRead);
    connect(m_bleServer, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
            this, SLOT(BleServiceCharacteristicWrite(QLowEnergyCharacteristic,QByteArray)));

    if(m_bleServer->state()==QLowEnergyService::DiscoveryRequired)
    {
        //调用discoverDetails（）时会触发对其包含的服务，特征和描述符的发现。
        m_bleServer->discoverDetails();
    }
    else
    {
        searchCharacteristic();
    }
}
void Widget::serviceStateChanged(QLowEnergyService::ServiceState s)
{

    if(s == QLowEnergyService::ServiceDiscovered)
    {
        ui->label_zhuangtai->setText("连接成功");//服务同步
        mystarttime = QDateTime::currentDateTime();//图像横坐标初始值参考点，读取初始时间
        searchCharacteristic();

//        ui->groupBox_wendu->clearGraphs();
//        ui->groupBox_qiangdu->clearGraphs();
//        ui->groupBox_wendu->replot();
//        ui->groupBox_qiangdu->replot();
    }
}

void Widget::searchCharacteristic()
{
    if(m_bleServer)
    {
        QList<QLowEnergyCharacteristic> list=m_bleServer->characteristics();
        //遍历characteristics

        for(int i=0;i<list.count();i++)
        {
           my_write_tezheng=list.at(1);//获取第一个特征值，具有读写属性。
           my_writeMode=QLowEnergyService::WriteWithResponse;
            QLowEnergyCharacteristic c=list.at(i);
            /*如果QLowEnergyCharacteristic对象有效，则返回true，否则返回false*/
            if(c.isValid())
            {
                //返回特征的属性。
                //这些属性定义了特征的访问权限。
                if(c.properties() & QLowEnergyCharacteristic::WriteNoResponse || c.properties() & QLowEnergyCharacteristic::Write)
                {
                    //ui->label_zhuangtai->insertPlainText("\n具有写权限!");
                    m_writeCharacteristic = c;  //保存写权限特性
                    if(c.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                    {

                        m_writeMode = QLowEnergyService::WriteWithoutResponse;
                    }
                    else
                    {

                        m_writeMode = QLowEnergyService::WriteWithResponse;
                    }
                }

                if(c.properties() & QLowEnergyCharacteristic::Read)
                {
                    m_readCharacteristic = c; //保存读权限特性
                }

                //描述符定义特征如何由特定客户端配置。
                m_notificationDesc = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                //值为真
                if(m_notificationDesc.isValid())
                {
                    //写描述符
                    m_bleServer->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
                    //ui->ctrSystemLogInfo->insertPlainText("\n写描述符!");
                }
            }
        }
    }
}

void Widget::SendMsg(QString text)
{
    QByteArray array=text.toLocal8Bit();

    m_bleServer->writeCharacteristic(m_writeCharacteristic,array, m_writeMode);
}

void Widget::BleServiceCharacteristicChanged(const QLowEnergyCharacteristic &c,const QByteArray &value)
{   
    emit this->sendserial_signals(value);
    Q_UNUSED(c)

}


void Widget::BleServiceCharacteristicRead(const QLowEnergyCharacteristic &c,const QByteArray &value)
{
    Q_UNUSED(c)

}

//如果写尝试成功,characteristicWritten()发出信号，下面槽响应。
void Widget::BleServiceCharacteristicWrite(const QLowEnergyCharacteristic &c,const QByteArray &value)
{
    Q_UNUSED(c)

    //移动滚动条到底部
    //QScrollBar *scrollbar = ui->ctrSystemLogInfo->verticalScrollBar();

    /*if(scrollbar)
    {
        scrollbar->setSliderPosition(scrollbar->maximum());
    }*/
}

void Widget::setupPlot()
{
    ui->read_plot->addGraph();//添加温度曲线
    QPen pen;
    pen.setWidth(1);//设置画笔线条宽度
    pen.setColor(Qt::blue);
    ui->read_plot->graph(0)->setPen(pen);//设置画笔颜色
    ui->read_plot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); //设置曲线画刷背景
    ui->read_plot->graph(0)->setName("voltage");
    ui->read_plot->graph(0)->setAntialiasedFill(false);
    ui->read_plot->graph(0)->setLineStyle((QCPGraph::LineStyle)1);//曲线画笔
    ui->read_plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone,5));//曲线形状


//   ui->read_plot->addGraph();//添加湿度曲线
//   pen.setColor(Qt::red);
//   ui->read_plot->graph(1)->setPen(pen);//设置画笔颜色
//   ui->read_plot->graph(1)->setBrush(QBrush(QColor(0, 0, 255, 20))); //设置曲线画刷背景
//   ui->read_plot->graph(1)->setName("humi");
//   ui->read_plot->graph(1)->setAntialiasedFill(false);
//   ui->read_plot->graph(1)->setLineStyle((QCPGraph::LineStyle)1);//曲线画笔
//   ui->read_plot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone,5));//曲线形状

   //设置图表
   ui->read_plot->xAxis->setLabel(QStringLiteral("时间/s"));//设置x坐标轴名称
   ui->read_plot->xAxis->setLabelColor(QColor(20,20,20));//设置x坐标轴名称颜色
   ui->read_plot->xAxis->setAutoTickStep(false);//设置是否自动分配刻度间距
   ui->read_plot->xAxis->setTickStep(2);//设置刻度间距
   ui->read_plot->xAxis->setRange(0,30);//设定x轴的范围

   ui->read_plot->yAxis->setLabel(QStringLiteral("电压/V"));//设置y坐标轴名称
   ui->read_plot->yAxis->setLabelColor(QColor(20,20,20));//设置y坐标轴名称颜色
   ui->read_plot->yAxis->setAutoTickStep(false);//设置是否自动分配刻度间距
   ui->read_plot->yAxis->setTickStep(10);//设置刻度间距1
   ui->read_plot->yAxis->setRange(0,2);//设定y轴范围

   ui->read_plot->axisRect()->setupFullAxesBox(true);//设置缩放，拖拽，设置图表的分类图标显示位置
   ui->read_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom| QCP::iSelectAxes);
   ui->read_plot->axisRect()->insetLayout()->setInsetAlignment(0,Qt::AlignTop | Qt::AlignRight);//图例显示位置右上
   ui->read_plot->legend->setVisible(true);//显示图例

   ui->read_plot->replot();
}

void Widget::sendVK_slot(double values)
{
    mycueernttime = QDateTime::currentDateTime();//获取系统时间
    double xzb = mystarttime.msecsTo(mycueernttime)/1000.0;//获取横坐标，相对时间就是从0开始
    ui->read_plot->graph(0)->addData(xzb,values);
    ui->temp_num->setText(QString::number(values, 'f',3));
    if(xzb>30)
           {
               ui->read_plot->xAxis->setRange((double)qRound(xzb-30),xzb);//设定x轴的范围
           }
           else ui->read_plot->xAxis->setRange(0,30);//设定x轴的范围
    ui->read_plot->replot();
    //qDebug()<<2222;
}

void Widget::on_pb_sousuo_clicked()
{
    //m_BLEController->disconnectFromDevice();//搜索时先断开连接使设备不会因重复不能添加至listwidget
    //ui->listWidget_lanya->clear();
    discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

}

void Widget::on_pb_duankai_clicked()
{
   m_BLEController->disconnectFromDevice();
   //ui->label_zhuangtai->setText("已断开");

}





