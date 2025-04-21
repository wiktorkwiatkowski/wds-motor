#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QObject>
#include <QSerialPort>

struct SerialData {
    float rpm;
    float pwm;
    float current;
    float voltage;
    float power;
};

// Enum z typami danych
enum DataType : quint8 {
    PWM = 0x01,
    Kd = 0x02,
    RPM = 0x03,
    // Dodaj inne typy, które chcesz wysyłać
};

class SerialReader : public QObject {
    Q_OBJECT

public:
    explicit SerialReader(QObject *parent = nullptr);
    void start(const QString &portName);
    void stop();
    void sendData(DataType type, float value);


signals:
    void newDataReceived(const SerialData &data);
    void errorOccurred(const QString &error);

private slots:
    void handleReadyRead();

private:
    QSerialPort serial;
    QByteArray buffer;
    static constexpr int frameSize = 23;
    bool parseFrame(const QByteArray &frame, SerialData &data);
};

#endif // SERIALREADER_H
