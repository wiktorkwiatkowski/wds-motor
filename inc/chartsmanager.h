/**
 * @file chartsmanager.h
 * @brief Deklaracja klasy ChartsManager do zarządzania wykresami QtCharts.
 *
 * Plik nagłówkowy definiuje klasę ChartsManager, która umożliwia tworzenie,
 * konfigurowanie i aktualizowanie dynamicznych wykresów parametrów silnika.
 * Obsługuje typowe parametry: PWM, RPM, napięcie, prąd, moc.
 */

#ifndef CHARTSMANAGER_H
#define CHARTSMANAGER_H

#include <QObject>
#include <QtCharts>
#include <QMap>

/**
 * @enum ChartType
 * @brief Typ wykresu.
 *
 * Określa, które dane są wyświetlane na danym wykresie.
 */
enum class ChartType {
    PWM,     ///< Wypełnienie sygnału PWM [%].
    RPM,     ///< Obroty silnika [obr/min].
    Voltage, ///< Napięcie zasilania [V].
    Current, ///< Prąd [mA].
    Power    ///< Moc [W].
};

/**
 * @class ChartsManager
 * @brief Klasa do zarządzania dynamicznymi wykresami QtCharts.
 *
 * Klasa umożliwia:
 * - tworzenie i konfigurowanie wykresów dla różnych parametrów,
 * - dodawanie nowych punktów do wykresów,
 * - usuwanie starych punktów spoza aktualnego zakresu X,
 * - dynamiczną zmianę tytułów i opisów wykresów.
 */
class ChartsManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy ChartsManager.
     * @param parent Obiekt nadrzędny (domyślnie nullptr).
     */
    explicit ChartsManager(QObject *parent = nullptr);

    /**
     * @brief Konfiguruje i dodaje nowy wykres do podanego layoutu.
     * @param type Typ wykresu (ChartType).
     * @param targetLayout Layout, do którego ma zostać dodany wykres.
     * @param title Tytuł wykresu.
     * @param yLabel Opis osi Y.
     * @param yMax Maksymalna wartość osi Y.
     * @param xRange Zakres osi X (czas w sekundach).
     * @param nice_numbers Czy użyć zaokrąglonych wartości osi Y (applyNiceNumbers).
     */
    void setupChart(ChartType type, QLayout *targetLayout, const QString &title, const QString &yLabel, float yMax, int xRange = 5, bool nice_numbers = true);

    /**
     * @brief Dodaje nowy punkt danych do wykresu.
     * @param type Typ wykresu.
     * @param time Czas w sekundach.
     * @param value Wartość parametru w danym momencie.
     */
    void addPoint(ChartType type, qreal time, qreal value);

    /**
     * @brief Ustawia tytuł wykresu.
     * @param type Typ wykresu.
     * @param title Nowy tytuł wykresu.
     */
    void setTitle(ChartType type, const QString &title);

    /**
     * @brief Ustawia nazwę serii danych na wykresie.
     * @param type Typ wykresu.
     * @param name Nowa nazwa serii.
     */
    void setSeriesName(ChartType type, const QString &name);

    /**
     * @brief Ustawia tytuł osi X na wykresie.
     * @param type Typ wykresu.
     * @param title Nowy tytuł osi X.
     */
    void setXAxisTitle(ChartType type, const QString &title);

private:
    /**
     * @struct ChartComponents
     * @brief Struktura przechowująca komponenty pojedynczego wykresu.
     */
    struct ChartComponents {
        ChartType type;               ///< Typ wykresu
        QLineSeries *series;          ///< Seria danych do rysowania
        QChart *chart;                ///< Wskaźnik na obiekt QChart.
        QChartView *chartView;        ///< Wskaźnik na obiekt QChartView (widok wykresu)
        QValueAxis *axisX;            ///< Oś X (czas)
        QValueAxis *axisY;            ///< Oś Y (wartość parametru).
        int xRange;                   ///< Zakres osi X (czas w sekundach).
    };

    /**
     * @brief Usuwa stare punkty danych spoza aktualnego zakresu osi X.
     * @param series Seria danych.
     * @param currentTime Aktualny czas (prawy koniec osi X).
     */
    void removeOldPoints(QLineSeries *series, qreal currentTime);

    QMap<ChartType, ChartComponents> charts; ///< Mapa wykresów powiązana z ich typami.
};

#endif // CHARTSMANAGER_H
