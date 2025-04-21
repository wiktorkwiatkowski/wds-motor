#include "../inc/serialreader.h"

#include <QtEndian>
#include <QDebug>

SerialReader::SerialReader(QObject *parent)
    : QObject(parent)
{
    // Kiedy są nowe dane na uart to wywoła się funkcja hadnleReadyRead readyRead to sygnał
    connect(&serial, &QSerialPort::readyRead, this, &SerialReader::handleReadyRead);
}

// Ustawia wszystkie parametru portu
void SerialReader::start(const QString &portName)
{
    serial.setPortName(portName);
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadWrite)) {
        emit errorOccurred("Nie udało się otworzyć portu: " + serial.errorString());
        return;
    }

    buffer.clear();
}

// Zamyka port jeśli jest otwarty
void SerialReader::stop()
{
    if (serial.isOpen())
        serial.close();
}

void SerialReader::handleReadyRead()
{
    // Dodajemy do buffera dane z seriala
    buffer.append(serial.readAll());

    qDebug() << "Odebrano bajty:" << buffer.toHex(' ').toUpper();

    while (buffer.size() >= frameSize) {
        int startIndex = buffer.indexOf(static_cast<char>(0xA5));
        if (startIndex == -1) {
            qDebug() << "Nie znaleziono bajtu startu (0xA5), czyszczę bufor";
            buffer.clear();
            return;
        }

        if (startIndex > 0) {
            qDebug() << "Usuwam bajty przed startem:" << buffer.left(startIndex).toHex(' ');
            buffer.remove(0, startIndex);
        }

        if (buffer.size() < frameSize) {
            qDebug() << "Czekam na więcej danych, mam" << buffer.size() << "z" << frameSize;
            return;
        }

        QByteArray frame = buffer.left(frameSize);
        buffer.remove(0, frameSize);

        qDebug() << "Znalazłem pełną ramkę:" << frame.toHex(' ').toUpper();

        SerialData data;
        if (parseFrame(frame, data)) {
            emit newDataReceived(data);
        } else {
            qDebug() << "Błąd parsowania lub checksum!";
            // Szukamy kolejnej ramki — nie returnujemy
            continue;
        }
    }
}

bool SerialReader::parseFrame(const QByteArray &frame, SerialData &data)
{
    if (frame.size() != frameSize)
        return false;

    quint8 checksum = 0;
    // Chechsum dla wszystkich oprócz ostatniego
    for (int i = 0; i < frameSize - 1; ++i) {
        checksum ^= static_cast<quint8>(frame[i]);
    }
    // Sprawdzenie czy policzona suma zgadza się z otrzymaną sumą
    if (checksum != static_cast<quint8>(frame[frameSize - 1]))
        return false;
    // const char *raw = frame.constData();
    memcpy(&data.rpm, frame.constBegin() + 2, 4);
    memcpy(&data.pwm, frame.constBegin() + 6, 4);
    memcpy(&data.current, frame.constBegin() + 10, 4);
    memcpy(&data.voltage, frame.constBegin() + 14, 4);
    memcpy(&data.power, frame.constBegin() + 18, 4);

    return true;
}

void SerialReader::sendData(DataType type, float value)
{
    if (!serial.isOpen()) {
        qDebug() << "Port nie jest otwarty!";
        return;
    }

    QByteArray frame;
    frame.resize(7); // 1 start + 1 typ + 4 bajty float + 1 checksum

    // Start byte
    quint8 startByte = 0xB5;
    memcpy(frame.begin(), &startByte, sizeof(quint8));
    // Typ danych (enum)
    memcpy(frame.begin() + 1, &type, sizeof(quint8));

    memcpy(frame.begin() + 2, &value, sizeof(quint32));

    // Liczymy checksum: XOR z bajtów od 0 do 5 (bez samej sumy)
    quint8 checksum = 0;
    for (int i = 0; i < 6; ++i) {
        checksum ^= static_cast<quint8>(frame[i]);
    }
    memcpy(frame.begin() + 6, &checksum,sizeof(quint8));

    serial.write(frame);
    serial.flush();

    qDebug() << "Wysłano ramkę:" << frame.toHex(' ').toUpper();
}
