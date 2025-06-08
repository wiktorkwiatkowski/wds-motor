/**
 * @file mainwindow.h
 * @brief Deklaracja klasy MainWindow (główne okno aplikacji).
 *
 * Plik nagłówkowy definiuje klasę MainWindow, która odpowiada za obsługę interfejsu użytkownika
 * aplikacji do sterowania silnikiem. Umożliwia wybór portu, sterowanie trybem pracy (manualny/automatyczny),
 * konfigurację PID, wizualizację parametrów w czasie rzeczywistym oraz obsługę komunikacji szeregowej.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serialreader.h"
#include "chartsmanager.h"
#include <QElapsedTimer>
#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QtCharts>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Klasa reprezentująca główne okno aplikacji.
 *
 * Klasa odpowiada za obsługę GUI, odbiór danych z mikrokontrolera oraz komunikację z klasą SerialReader i ChartsManager.
 * Pozwala użytkownikowi m.in. na:
 * - wybór portu i połączenie,
 * - przełączanie trybu pracy silnika,
 * - start/stop silnika,
 * - zmianę parametrów PID,
 * - podgląd parametrów w czasie rzeczywistym na wykresach.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:

    /**
     * @brief Konstruktor klasy MainWindow.
     * @param parent Obiekt nadrzędny (domyślnie nullptr).
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     */
    ~MainWindow();

private slots:

    /**
     * @brief Obsługuje nowe dane odebrane z portu szeregowego.
     * @param data Struktura SerialData z danymi.
     */
    void handleNewSerialData(const SerialData &data);

    /**
     * @brief Obsługuje błędy komunikacji szeregowej.
     * @param error Treść komunikatu błędu.
     */
    void handleSerialError(const QString &error);

    /**
     * @brief Obsługuje rozłączenie portu szeregowego.
     */
    void handlePortDisconnected();

    /**
     * @brief Obsługuje zmianę wartości suwaka PWM w trybie manualnym.
     * @param value Wartość PWM w zakresie 0-100%.
     */
    void on_sliderPWMManual_valueChanged(float value);

    /**
     * @brief Obsługuje przycisk Start/Stop silnika.
     */
    void on_buttonStartStop_clicked();

    /**
     * @brief Obsługuje przełączanie trybu pracy (manualny/automatyczny).
     */
    void on_buttonToggleMode_clicked();

    /**
     * @brief Obsługuje przycisk ustawiania zadanej wartości RPM (tryb automatyczny).
     */
    void on_buttonSetRPM_clicked();

    /**
     * @brief Obsługuje przycisk Połącz/Rozłącz port szeregowy.
     */
    void on_ConnectPortClicked();

    /**
     * @brief Obsługuje przycisk Odśwież porty.
     */
    void refreshSerialPortList();

    /**
     * @brief Obsługuje przycisk Zapisz wartości PID.
     */
    void on_buttonSavePID_clicked();

    /**
     * @brief Przełącza interfejs na język polski.
     */
    void switchToPolish();

    /**
     * @brief Przełącza interfejs na język angielski.
     */
    void switchToEnglish();

private:

    /**
     * @brief Łączy sygnały i sloty aplikacji.
     */
    void connectSignals();

    /**
     * @brief Konfiguruje domyślny tryb pracy aplikacji (tryb ręczny, brak połączenia).
     */
    void configureInitialMode();

    /**
     * @brief Ustawia walidatory pól edycji dla parametrów PID.
     */
    void setupValidators();

    /**
     * @brief Ustawia kolory etykiet odpowiadające kolorom wykresów.
     */
    void setLabelsColors() const;

    /**
     * @brief Aktualizuje dane wykresów.
     */
    void updateCharts() const;

    /**
     * @brief Aktualizuje dane wyświetlane w polach tekstowych GUI.
     */
    void updateGUI() const;

    /**
     * @brief Konfiguruje wykresy dla parametrów pracy silnika.
     */
    void setupCharts();

    /**
     * @brief Konfiguruje i uruchamia timery aktualizujące GUI i wykresy.
     */
    void setupTimers();

    /**
     * @brief Odświeża tytuły i etykiety wykresów po zmianie języka.
     */
    void retranslateCharts();

    Ui::MainWindow *ui;                 ///< Wskaźnik na interfejs użytkownika (GUI).
    SerialReader *serialReader;         ///< Obiekt do komunikacji szeregowej.
    QElapsedTimer elapsed;              ///< Timer odmierzający czas od uruchomienia aplikacji.
    QTimer *updateChartsTimer;          ///< Timer do odświeżania wykresów.
    QTimer *updateGUITimer;             ///< Timer do odświeżania GUI.
    ChartsManager *charts;              ///< Obiekt do zarządzania wykresami.
    SerialData latestData;              ///< Ostatnie dane odebrane z mikrokontrolera.
    QString currentPortName;            ///< Nazwa aktualnie podłączonego portu.
    qint32 currentBaudRate = 115200;    ///< Aktualna prędkość transmisji (domyślnie 115200).
    bool isManualMode = true;           ///< Tryb pracy (true = manualny, false = automatyczny).
    bool isMotorRunning = false;        ///< Stan pracy silnika (true = uruchomiony).
    bool isPortConnected = false;       ///< Status połączenia z portem szeregowym.
    QTranslator translator;             ///< Tłumacz (translator) do zmiany języka interfejsu.
};
#endif // MAINWINDOW_H
