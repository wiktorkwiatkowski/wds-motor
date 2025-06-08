
/**
 * @file mainwindow.h
 * @brief Deklaracja klasy MainWindow — głównego okna aplikacji GUI.
 *
 * Plik zawiera definicję klasy MainWindow, która odpowiada za:
 * - interfejs użytkownika,
 * - przetwarzanie danych odebranych z ESP32,
 * - interakcję z użytkownikiem.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serialreader.h"
#include <QByteArray>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QVBoxLayout>
#include <QtCharts>
#include "chartsmanager.h"
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Główne okno aplikacji GUI do sterowania silnikiem i komunikacji z
 * ESP32.
 *
 * Odpowiada za wyświetlanie danych, obsługę interfejsu użytkownika
 * oraz pośredniczy w komunikacji z klasą SerialReader.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
   * @brief Konstruktor głównego okna.
   */
    MainWindow(QWidget *parent = nullptr);
    /**
   * @brief Destruktor — zwalnia zasoby.
   */
    ~MainWindow();

private slots:
    /**
   * @brief Slot wywoływany po odebraniu danych z ESP32 (aktualizacja danych).
   */
    void handleNewSerialData(const SerialData &data);

    /**
   * @brief Slot obsługujący komunikaty o błędach z portu szeregowego.
   */
    void handleSerialError(const QString &error);

    /**
   * @brief Slot wywoływany przy zmianie wartości suwaka sterowania ręcznego
   * PWM.
   */
    void on_sliderPWMManual_valueChanged(float value);

    /**
     * @brief Slot obsługujący przycisk start/stop silnika.
     */
    void on_buttonStartStop_clicked();

    /**
     * @brief Slot przełączający tryb pracy (ręczny/automatyczny).
     */
    void on_buttonToggleMode_clicked();

    /**
     * @brief Slot obsługujący zadanie nowej wartości RPM.
     */
    void on_buttonSetRPM_clicked();

    /**
     * @brief Slot obsługujący nawiązywanie i zrywanie połączenia z portem szeregowym.
     */
    void on_ConnectPortClicked();

    /**
     * @brief Slot obsługujący przycisk zapisu wartości PID.
     */
    void on_buttonSavePID_clicked();

    /**
     * @brief Odświeża listę dostępnych portów szeregowych.
     */
    void refreshSerialPortList();

    /**
    * @brief Ustawia kolory etykiet w GUI.
    */
    void setLabelsColors() const;

    /**
     * @brief Aktualizuje wykresy i etykiety GUI.
    */
    void updateCharts() const;

    /**
     * @brief Aktualizuje wartości i etykiety w interfejsie użytkownika.
     */
    void updateGUI() const;

    /**
     * @brief Obsługa zdarzenia rozłączenia portu szeregowego.
     */
    void handlePortDisconnected();

    void switchToPolish();

    void switchToEnglish();
private:

    /**
     * @brief Łączy sygnały i sloty aplikacji.
     */
    void connectSignals();

    /**
     * @brief Konfiguruje początkowy tryb widoku GUI.
     */
    void configureInitialMode();

    /**
     * @brief Ustawia walidatory dla pól wejściowych.
     */
    void setupValidators();

    /**
     * @brief Inicjalizuje wykresy.
     */
    void setupCharts();

    /**
     * @brief Inicjalizuje i uruc
     * hamia timery GUI.
     */
    void setupTimers();

    void retranslateCharts();

    Ui::MainWindow *ui;                 ///< Interfejs użytkownika
    SerialReader *serialReader;         ///< Obiekt komunikacji szeregowej z ESP32
    QElapsedTimer elapsed;              ///< Timer odmierzający czas od uruchomienia aplikacji
    QTimer *updateChartsTimer;          ///< Timer do odświeżania wykresów
    QTimer *updateGUITimer;             ///< Timer do odświeżania GUI
    ChartsManager *charts;              ///< Menedżer obsługujący wykresy
    SerialData latestData;              ///< Ostatnio odebrane dane z ESP32
    bool isManualMode = true;           ///< Flaga trybu ręcznego
    bool isMotorRunning = false;        ///< Flaga stanu silnika (czy działa)
    QString currentPortName;            ///< Nazwa aktualnie używanego portu szeregowego
    qint32 currentBaudRate = 115200;    ///< Aktualna prędkość transmisji (domyślnie 115200)
    bool isPortConnected = false;       ///< Flaga informująca o stanie połączenia szeregowego
    QTranslator translator;
    QString currentLanguage;
};
#endif // MAINWINDOW_H
