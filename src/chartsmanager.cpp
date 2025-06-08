/**
 * @file chartsmanager.cpp
 * @brief Implementacja klasy ChartsManager.
 *
 * Klasa ChartsManager odpowiada za tworzenie i zarządzanie dynamicznymi wykresami QtCharts
 * dla parametrów pracy silnika: PWM, RPM, napięcie, prąd, moc.
 * Umożliwia dynamiczne dodawanie punktów, automatyczne przewijanie osi X,
 * usuwanie starych punktów oraz zmianę tytułów i opisów wykresów.
 */

#include "../inc/chartsmanager.h"

/**
 * Inicjalizuje obiekt ChartsManager.
 * Początkowo mapa wykresów (charts) jest pusta, wykresy są dodawane dynamicznie przez setupChart().
 */
ChartsManager::ChartsManager(QObject *parent) : QObject{parent}{}

/**
 * Funkcja tworzy nowy wykres o zadanym typie, konfiguruje osie, tytuły, kolory oraz dodaje go do podanego layoutu.
 * Wykres wyświetla parametr w funkcji czasu.
 * Użytkownik może ustawić czy oś Y ma korzystać z "ładnych" wartości (nice numbers).
 */

void ChartsManager::setupChart(ChartType type, QLayout *targetLayout, const QString &title, const QString &yLabel, float yMax, int xRange, bool nice_numbers) {
    ChartComponents components;

    components.type = type;
    components.series = new QLineSeries;
    components.chart = new QChart;
    components.chartView = new QChartView(components.chart);
    components.axisX = new QValueAxis;
    components.axisY = new QValueAxis;
    components.xRange = xRange;
    components.series->setName(title);
    components.chart->addSeries(components.series);
    components.chart->setTitle(title);

    components.axisX->setRange(0, xRange);
    components.axisX->setLabelFormat("%.1f");
    components.axisX->setTitleText(tr("Czas [s]"));
    components.chart->addAxis(components.axisX, Qt::AlignBottom);
    components.series->attachAxis(components.axisX);
    components.axisY->setRange(0, yMax);
    if(nice_numbers){
        components.axisY->applyNiceNumbers();
    }
    components.axisY->setTitleText(yLabel);
    components.chart->addAxis(components.axisY, Qt::AlignLeft);
    components.series->attachAxis(components.axisY);
    if (type == ChartType::PWM)     components.series->setColor(QColor("purple"));
    if (type == ChartType::RPM)     components.series->setColor(QColor("orange"));
    if (type == ChartType::Voltage) components.series->setColor(QColor("blue"));
    if (type == ChartType::Power)   components.series->setColor(QColor("green"));
    if (type == ChartType::Current) components.series->setColor(QColor("red"));

    components.chartView->setRenderHint(QPainter::Antialiasing);

    if (targetLayout) {
        targetLayout->addWidget(components.chartView);
    }


    charts[type] = components;
}

/**
 * Punkt reprezentuje wartość parametru w danym czasie.
 * Jeśli aktualny czas przekracza zakres osi X, oś X jest przesuwana w prawo.
 * Stare punkty spoza aktualnego okna czasu są usuwane automatycznie.
 */
void ChartsManager::addPoint(ChartType type, qreal time, qreal value) {
    if (!charts.contains(type)) return;

    auto &c = charts[type];
    c.series->append(time, value);

    // Jeśli czas przekracza 5s, przesuwaj oś X
    if (time > c.xRange) {
        c.axisX->setRange(time - c.xRange, time);
    }

    removeOldPoints(c.series, time);
}

/**
 * Funkcja iteracyjnie usuwa najstarsze punkty z początku serii,
 * tak aby na wykresie pozostawały tylko punkty w aktualnym oknie czasu
 * (np. ostatnie 5 sekund).
 */

void ChartsManager::removeOldPoints(QLineSeries *series, qreal currentTime) {
    // Usuwanie starych punktów spoza zakresu ostatnich 5 sekund
    // Jeśli są jakiekolwiek punkty oraz wartość pierwszego punktu na osi X jest
    // starsza niż ostatnie 5 sekund to go usuń. I tak w pętli do momentu pozbycia
    // się punktów, które przekroczyły zadany czas, w tym przypadku 5 sekund.
    while (!series->points().isEmpty() && series->points().first().x() < (currentTime - 5.0)) {
        series->remove(0);
    }
}

/**
 * Funkcja pozwala na dynamiczną zmianę tytułu wykresu (np. po zmianie języka GUI).
 */
void ChartsManager::setTitle(ChartType type, const QString &title) {
    charts[type].chart->setTitle(title);
}

/**
 * Funkcja pozwala na dynamiczną zmianę nazwy serii (np. po zmianie języka GUI).
 */
void ChartsManager::setSeriesName(ChartType type, const QString &name) {
    charts[type].series->setName(name);
}

/**
 * Funkcja pozwala na dynamiczną zmianę tytułu osi X (np. po zmianie języka GUI).
 */

void ChartsManager::setXAxisTitle(ChartType type, const QString &title) {
        charts[type].axisX->setTitleText(title);
}
