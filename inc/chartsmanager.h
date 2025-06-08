/**
 * @file chartsmanager.h
 * @brief Definicja klasy ChartsManager – menedżera dynamicznych wykresów Qt Charts.
 *
 * Plik zawiera deklarację klasy ChartsManager, która odpowiada za:
 * - tworzenie i konfigurację wykresów typu QChart,
 * - dynamiczne dodawanie danych pomiarowych,
 * - usuwanie starych punktów spoza zadanego zakresu czasu,
 * - zarządzanie serią wykresów dla różnych typów danych (PWM, RPM, prąd, napięcie, moc).
 *
 */

#ifndef CHARTSMANAGER_H
#define CHARTSMANAGER_H

#include <QObject>
#include <QtCharts>
#include <QMap>

/**
 * @brief Typy danych wykresów obsługiwanych przez ChartsManager.
 */
enum class ChartType {
    PWM,     ///< Wypełnienie sygnału PWM [%]
    RPM,     ///< Obroty silnika [obr/min]
    Voltage, ///< Napięcie zasilania [V]
    Current, ///< Prąd pobierany przez silnik [mA]
    Power    ///< Moc chwilowa [W]
};

/**
 * @class ChartsManager
 * @brief Klasa odpowiedzialna za tworzenie i aktualizację wielu wykresów w aplikacji.
 *
 * Zarządza konfiguracją, aktualizacją i renderowaniem danych na wykresach opartych o Qt Charts.
 */
class ChartsManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Tworzy nową instancję ChartsManager.
     */
    explicit ChartsManager(QObject *parent = nullptr);

    /**
     * @brief Inicjalizuje nowy wykres i dodaje go do podanego layoutu.
     */
    void setupChart(ChartType type, QLayout *targetLayout, const QString &title, const QString &yLabel, float yMax, int xRange = 5, bool nice_numbers = true);

    /**
     * @brief Dodaje nowy punkt danych do wykresu.
     */
    void addPoint(ChartType type, qreal time, qreal value);

    void setTitle(ChartType type, const QString &title);

    void setSeriesName(ChartType type, const QString &name);

    void setXAxisTitle(ChartType type, const QString &title);
private:

    /**
     * @brief Struktura przechowująca wszystkie komponenty pojedynczego wykresu.
     */
    struct ChartComponents {
        ChartType type;
        QLineSeries *series;           ///< Seria danych do rysowania
        QChart *chart;                 ///< Wykres Qt
        QChartView *chartView;        ///< Widok wykresu
        QValueAxis *axisX;            ///< Oś X (czas)
        QValueAxis *axisY;            ///< Oś Y (wartości)
        int xRange;                   ///< Zakres czasu na osi X [s]
    };

    /**
     * @brief Mapa przechowująca wszystkie aktywne wykresy.
     */
    QMap<ChartType, ChartComponents> charts;

    /**
     * @brief Usuwa punkty spoza zakresu czasu X.
     */
    void removeOldPoints(QLineSeries *series, qreal currentTime);

};

#endif // CHARTSMANAGER_H
