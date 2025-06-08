/**
 * @file serialreader.cpp
 * @brief Implementacja klasy SerialReader.
 *
 * Plik implementuje klasę SerialReader odpowiedzialną za komunikację z mikrokontrolerem
 * przez port szeregowy (UART). Obsługuje odbiór i wysyłanie danych w formie ramek binarnych,
 * parsowanie ramek, wykrywanie błędów oraz emitowanie odpowiednich sygnałów do GUI.
 */

#include "../inc/serialreader.h"
#include <QDebug>
#include <QtEndian>

/**
 * Inicjalizuje obiekt QSerialPort, ustawia tryb komunikacji i podłącza obsługę błędów.
 */
SerialReader::SerialReader(QObject *parent) : QObject(parent) {
    // Po otrzymaniu nowych danych wywołuje funkcję handleReadyRead()
    connect(&serial, &QSerialPort::readyRead, this, &SerialReader::handleReadyRead);

    // Jeśli pojawi się jakikolwiek bląd z połączenie wywołaj handleError
    connect(&serial, &QSerialPort::errorOccurred, this, &SerialReader::handleError);

}

/**
 * Otwiera wskazany port szeregowy i ustawia zadaną prędkość transmisji.
 * W przypadku błędu emisja sygnału errorOccurred().
 */
void SerialReader::start(const QString &portName, int baudRate) {
    serial.setPortName(portName);
    serial.setBaudRate(baudRate);
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

/**
 * Jeśli port jest otwarty, zostaje zamknięty i jest czyszczony bufor odbiorczy.
 */
void SerialReader::stop() {
    if (serial.isOpen())
        serial.close();
}

/**
 * Funkcja odczytuje dostępne dane z portu i składa je w buforze.
 * Jeśli w buforze znajduje się poprawna ramka (pełne frameSize),
 * próbuje ją sparsować i emituje sygnał newDataReceived().
 */
void SerialReader::handleReadyRead() {
    // Dodanie do buffera dane z seriala
    buffer.append(serial.readAll());

    while (buffer.size() >= frameSize) {
        int startIndex = buffer.indexOf(static_cast<char>(0xA5));
        if (startIndex == -1) {
            qDebug() << "Nie znaleziono bajtu startu (0xA5), czyszczenie buforu";
            buffer.clear();
            return;
        }

        if (startIndex > 0) {
            qDebug() << "Usunięcie bajtów przed startem:" << buffer.left(startIndex).toHex(' ');
            buffer.remove(0, startIndex);
        }

        if (buffer.size() < frameSize) {
            qDebug() << "Czekanie na resztę danych, jest" << buffer.size() << "z" << frameSize;
            return;
        }

        QByteArray frame = buffer.left(frameSize);
        buffer.remove(0, frameSize);

        // qDebug() << "Znalaziono pełną ramkę:" << frame.toHex(' ').toUpper();

        SerialData data;
        if (parseFrame(frame, data)) {
            emit newDataReceived(data);
        } else {
            qDebug() << "Błąd parsowania lub checksum!";
        }
    }
}

/**
 * Funkcja weryfikuje poprawność sumy kontrolnej (XOR) ramki oraz odczytuje z niej poszczególne pola:
 * RPM, PWM, prąd, napięcie, moc, parametry PID oraz tryb pracy.
 */
bool SerialReader::parseFrame(const QByteArray &frame, SerialData &data) {
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

    // Parsowanie pól (zgodnie z kolejnością w buforze)
    memcpy(&data.rpm, frame.constBegin() + 1, 4);
    memcpy(&data.pwm, frame.constBegin() + 5, 4);
    memcpy(&data.current, frame.constBegin() + 6, 4);
    memcpy(&data.voltage, frame.constBegin() + 10, 4);
    memcpy(&data.power, frame.constBegin() + 14, 4);
    memcpy(&data.kp, frame.constBegin() + 18, 4);
    memcpy(&data.ki, frame.constBegin() + 22, 4);
    memcpy(&data.kd, frame.constBegin() + 26, 4);
    memcpy(&data.mode, frame.constBegin() + 30, 1);

    return true;
}

/**
 * Funkcja automatycznie składa ramkę i wysyła ją przez port szeregowy.
 * Ramka ma następujący format:
 * - Start Byte (0xB5)
 * - Typ danych (DataType)
 * - Wartość typu float (4 bajty)
 * - Suma kontrolna (XOR)
 */
void SerialReader::sendData(DataType type, float value) {
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
    // Jeśli wartość to PWM trzeba ją rzutować
    if (type == PWM) {
        value = static_cast<uint8_t>(value);
    }

    if(type == RPM){
        value = static_cast<uint8_t>(value);
    }

    memcpy(frame.begin() + 2, &value, sizeof(quint32));

    // Liczymy checksum: XOR z bajtów od 0 do 5 (bez samej sumy)
    quint8 checksum = 0;
    for (int i = 0; i < 6; ++i) {
        checksum ^= static_cast<quint8>(frame[i]);
    }
    memcpy(frame.begin() + 6, &checksum, sizeof(quint8));

    serial.write(frame);
    // Wymuś opróżnienie bufora
    serial.flush();

    // qDebug() << "Wysłano ramkę:" << frame.toHex(' ').toUpper();
}

bool SerialReader::isOpen() const {
    return serial.isOpen();
}

/**
 * Jeśli wykryty zostanie błąd rozłączenia (ResourceError lub DeviceNotFoundError),
 * emituje sygnał portDisconnected(). Pozostałe błędy są zgłaszane przez errorOccurred().
 */
void SerialReader::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        qDebug() << "Urządzenie zostało odłączone!";
        emit portDisconnected();
        stop();
    }
}

