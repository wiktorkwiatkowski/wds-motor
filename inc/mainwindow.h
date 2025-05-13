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
   * @param portName Nazwa portu szeregowego do połączenia z ESP32.
   * @param parent Obiekt nadrzędny (domyślnie nullptr).
   */
    MainWindow(QString portName, QWidget *parent = nullptr);
    /**
   * @brief Destruktor — zwalnia zasoby.
   */
    ~MainWindow();

private slots:
    /**
   * @brief Slot wywoływany po odebraniu danych z ESP32 (aktualizacja danych).
   * @param data Struktura danych zawierająca wartości z ramki (RPM, PWM,
   * napięcie, itd.).
   */
    void handleNewSerialData(const SerialData &data);

    /**
   * @brief Slot obsługujący komunikaty o błędach z portu szeregowego.
   * @param error Tekst błędu do wyświetlenia.
   */
    void handleSerialError(const QString &error);

    /**
   * @brief Slot wywoływany przy zmianie wartości suwaka sterowania ręcznego
   * PWM.
   * @param value Wartość procentowa ustawiona przez użytkownika (0–100%).
   */
    void on_sliderPWMManual_valueChanged(float value);

    void setLabelsColors() const;
private:
    /**
   * @brief Konfiguruje i inicjalizuje wykres czasu trwania sygnału PWM.
   */
    void setupPWMChart();

    Ui::MainWindow *ui;
    SerialReader *serialReader; ///< Obiekt obsługujący komunikację z ESP32

    QElapsedTimer elapsed; ///< Timer odmierzający czas od uruchomienia aplikacji.

    ChartsManager *charts;

};
#endif // MAINWINDOW_H
