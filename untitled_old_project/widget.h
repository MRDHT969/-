#ifndef WIDGET_H
#define WIDGET_H
#include <QDialog>
#include <QWidget>
#include<QBluetoothDeviceDiscoveryAgent>
#include<QBluetoothSocket>
#include<QBluetoothAddress>
#include<QListWidget>
#include <QBluetoothDeviceInfo>
#include <QList>
#include <QLowEnergyController>
#include<qbluetoothaddress.h>
#include <QBluetoothUuid>
#include <QLowEnergyService>
#include <QScrollBar>
#include <QListWidgetItem>
#include <QTime>
#include <QThread>
#include <qcustomplot.h>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QLowEnergyController *m_BLEController;
    //https://doc.qt.io/qt-5/qlowenergyservice.html
    QLowEnergyService *m_bleServer;

    QLowEnergyCharacteristic m_writeCharacteristic;
    QLowEnergyCharacteristic m_readCharacteristic;
     QLowEnergyCharacteristic my_write_tezheng;//获取具有写属性特征值，用来发送数据，特征值1有这个功能
     QLowEnergyService::WriteMode my_writeMode;//写属性，用来和特征值1保持一致。
    QLowEnergyDescriptor m_notificationDesc;

    QLowEnergyService::WriteMode m_writeMode;

    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QList<QBluetoothDeviceInfo> devicesList;
    void searchCharacteristic();
    void SendMsg(QString text);



signals:
    void returnAddress(QBluetoothDeviceInfo info);
    void sendserial_signals(QByteArray serialtemp);
private slots:
    //void onUpdateGB(const FLOAT &temp);//////////

    void on_pb_sousuo_clicked();

    void on_pb_lianjie_clicked();

    void on_pb_duankai_clicked();

    void addBlueToothDevicesToList(const QBluetoothDeviceInfo &device);
    //void on_listWidget_lanya_itemClicked(QListWidgetItem *item);
    //void readBluetoothDataEvent();
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void createCtl(QBluetoothDeviceInfo choosenDeviceInfo);
    void serviceScanDone();
    //void serviceDiscovered(const QBluetoothUuid &gatt);
    //特性值由事件改变时发出此信号在外设上
    void BleServiceCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    //当特征读取请求成功返回其值时
    void BleServiceCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    //当特性值成功更改为newValue时
    void BleServiceCharacteristicWrite(const QLowEnergyCharacteristic &c, const QByteArray &value);

    void setupPlot();

    void sendVK_slot(double values);
private:
    Ui::Widget *ui;
    QThread *thread;
    //QTimer dataTimer;
    double lastkey;
    QByteArray buffer;
    uint8_t sec=0;
    QDateTime mycueernttime;//系统当前时间
    QDateTime mystarttime;//系统开始时间
};
#endif // WIDGET_H
