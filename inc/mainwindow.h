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

    void on_buttonStartStop_clicked();
    void on_buttonToggleMode_clicked();
    void on_buttonSetRPM_clicked();
    void on_ConnectPortClicked();
    void on_buttonSavePID_clicked();

    void refreshSerialPortList();

    /**
    * @brief Ustawia kolory etykiet w GUI.
    */
    void setLabelsColors() const;

    /**
     * @brief Aktualizuje wykresy i etykiety GUI.
    */
    void updateCharts() const;

    void updateGUI() const;

    void handlePortDisconnected();

    // void on_buttonSavePID_clicked();
private:

    void connectSignals();
    void configureInitialMode();
    void setupValidators();
    void setupCharts();
    void setupTimers();

    Ui::MainWindow *ui;                 ///< Interfejs użytkownika
    SerialReader *serialReader;        ///< Komunikacja z ESP32
    QElapsedTimer elapsed;             ///< Timer od startu aplikacji
    QTimer *updateChartsTimer;               ///< Timer GUI i wykresów
    QTimer *updateGUITimer;
    ChartsManager *charts;             ///< Menedżer wykresów
    SerialData latestData;             ///< Ostatnio odebrane dane
    bool isManualMode = true;
    bool isMotorRunning = false;QString currentPortName;
    qint32 currentBaudRate = 115200; // domyślna wartość
    bool isPortConnected = false;



};
#endif // MAINWINDOW_H
