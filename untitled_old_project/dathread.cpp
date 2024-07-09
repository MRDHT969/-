#include "dathread.h"
#include "widget.h"

class daThreadData : public QSharedData
{
public:

};

daThread::daThread(QObject *parent) : QObject(parent), data(new daThreadData)
{

}

daThread::daThread(const daThread &rhs) : data(rhs.data)
{

}

daThread &daThread::operator=(const daThread &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

daThread::~daThread()
{

}

void daThread::sendserial_slots(QByteArray myData)
{
    mytemp=myData;
    mytemp.chop(1);//删除最后一位空格
    values=mytemp.toDouble()/1000.0;
        //qDebug()<<"a3"<<a3;
        //qDebug()<<"timestampMs"<<timestampMs;
//    for (int i = 0; i < values.size(); ++i) {
//        qDebug() << "Timestamp:" << key[i] << "Value:" << values[i];
//    }
    emit this->sendVK_signal(values);
}
void daThread::Run()
{
    qDebug() << "run:" << QThread::currentThread();
    HSa();
}

void daThread::HSa()
{
//        qDebug()<<2222;
//        QString decimalValue = mytemp; // 你的十六进制字符串
//        int length = decimalValue.length();
//        for (int i = 0; i < length; i += 2) {
//            // 截取两个字符，并添加到结果字符串中
//            pairs = decimalValue.mid(i, 2);
//            j11++;
//            if(j11%2==1){
//                pairs1=pairs;
//                qint64 intValue=pairs1.toInt(nullptr, 16);
//                a2 = intValue / 10.0f;
//            }
//            if(j11%2==0){
//                qint64 intValue=pairs.toInt(nullptr, 16);
//                a1 = intValue / 1000.0f;
//                double a3 = static_cast<double>(a1+a2);
//                double timestampMs = startTimeMs + sec * 0.2;
//                qDebug()<<"a3"<<a3;
//                qDebug()<<"timestampMs"<<timestampMs;
//                emit this->sendVK_signal(a3,timestampMs);
//                sec++;
//            }
//        }
        //mytemp.clear();
}
