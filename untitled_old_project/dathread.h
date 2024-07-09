#ifndef DATHREAD_H
#define DATHREAD_H

#include <QObject>
#include <QSharedDataPointer>
#include <qcustomplot.h>
#include <QDebug>
#include <QVector>

class daThreadData;

class daThread : public QObject
{
    Q_OBJECT
public:
    explicit daThread(QObject *parent = nullptr);
    daThread(const daThread &);
    daThread &operator=(const daThread &);
    ~daThread();
signals:
    void sendVK_signal(double);

public slots:
    void sendserial_slots(QByteArray);

    void Run();

    void HSa();

private:
    QSharedDataPointer<daThreadData> data;
    QString mytemp;
    QString pairs;
    double values;
};

#endif // DATHREAD_H
