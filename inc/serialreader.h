/**
 * @file serialreader.h
 * @brief Deklaracja klasy SerialReader do obsługi komunikacji szeregowej.
 *
 * Plik nagłówkowy definiujący strukturę SerialData, enumerację DataType oraz klasę SerialReader.
 * Klasa umożliwia komunikację z mikrokontrolerem przez port szeregowy (UART-USB),
 * obsługuje wysyłanie i odbiór danych w formacie ramek binarnych oraz
 * emituje odpowiednie sygnały do interfejsu użytkownika.
 */

#ifndef SERIALREADER_H
#define SERIALREADER_H

#include <QObject>
#include <QSerialPort>

/**
 * @struct SerialData
 * @brief Struktura przechowująca dane odebrane z mikrokontrolera.
 */
struct SerialData {
    float rpm = 0.0f;     ///< Obroty silnika [obr/min]
    uint8_t pwm = 0.0f;   ///< Wypełnienie PWM [%]
    float current = 0.0f; ///< Prąd [mA]
    float voltage = 0.0f; ///< Napięcie [V]
    float power = 0.0f;   ///< Moc [W]
    float kp = 0.0f;      ///< Wzmocnienie proporcjonalne regulatora PID
    float ki = 0.0f;      ///< Wzmocnienie całkujące regulatora PID
    float kd = 0.0f;      ///< Wzmocnienie różniczkujące regulatora PID
    uint8_t mode = 0.0f;  ///< Tryb pracy: 0 - ręczny, 1 - automatyczny
};

/**
 * @enum DataType
 * @brief Typ danych wysyłanych do mikrokontrolera.
 */
enum DataType : quint8 {
    PWM = 0x01,       ///< Zmiana wypełnienia PWM.
    RPM = 0x02,       ///< Zmiana zadanej prędkości obrotowej (RPM).
    Kp = 0x03,        ///< Zmiana parametru Kp.
    Ki = 0x04,        ///< Zmiana parametru Ki.
    Kd = 0x05,        ///< Zmiana parametru Kd.
    mode = 0x06,      ///< Zmiana trybu na manualny/automatyczny.
    start_stop = 0x07 ///< Start/Stop silnika.
};

/**
 * @class SerialReader
 * @brief Klasa odpowiedzialna za komunikację z mikrokontrolerem przez port szeregowy.
 */
class SerialReader : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy SerialReader.
     * @param parent Obiekt nadrzędny (domyślnie nullptr).
     */
    explicit SerialReader(QObject *parent = nullptr);

    /**
     * @brief Rozpoczyna komunikację przez port szeregowy.
     * @param portName Nazwa portu (np. COM3 lub /dev/ttyUSB0).
     * @param baudRate Prędkość transmisji (domyślnie 115200 Bd).
     */
    void start(const QString &portName, int baudRate = QSerialPort::Baud115200);

    /**
     * @brief Zatrzymuje komunikację (zamyka port).
     */
    void stop();

    /**
     * @brief Wysyła ramkę danych do mikrokontrolera.
     * @param type Typ danych (enum DataType), określający rodzaj wysyłanej wartości.
     * @param value Wartość typu float do wysłania.
     */
    void sendData(DataType type, float value);

    /**
     * @brief Sprawdza, czy port szeregowy jest otwarty.
     * @return true jeśli port jest otwarty, false w przeciwnym wypadku.
     */
    bool isOpen() const;

    /**
     * @brief Obsługuje błędy portu szeregowego.
     * @param error Kod błędu.
     */
    void handleError(QSerialPort::SerialPortError error);

signals:

    /**
     * @brief Sygnał emitowany po odebraniu i sparsowaniu poprawnej ramki.
     * @param data Struktura zawierająca wszystkie dane z ramki.
     */
    void newDataReceived(const SerialData &data);

    /**
     * @brief Sygnał emitowany w przypadku błędu otwarcia lub pracy z portem.
     * @param error Treść komunikatu błędu.
     */
    void errorOccurred(const QString &error);

    /**
     * @brief Sygnał emitowany w przypadku wykrycia rozłączenia portu.
     */
    void portDisconnected();

private slots:

    /**
     * @brief Slot wywoływany automatycznie przy dostępnych danych na porcie
     * szeregowym.
     */
    void handleReadyRead();

private:
    QSerialPort serial; ///< Obiekt Qt obsługujący port szeregowy
    QByteArray buffer;  ///< Bufor do składania ramek z bajtów
    static constexpr int frameSize = 32; ///< Długość oczekiwanej ramki danych

    /**
   * @brief Próbuje sparsować jedną ramkę danych z bufora.
   * @param frame Bufor zawierający odebraną ramkę.
   * @param data Struktura, do której zapisane zostaną odczytane dane.
   * @return true jeśli parsowanie i suma kontrolna są poprawne, false w przeciwnym wypadku.
   */
    bool parseFrame(const QByteArray &frame, SerialData &data);
};

#endif // SERIALREADER_H
