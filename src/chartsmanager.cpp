#include "../inc/chartsmanager.h"

ChartsManager::ChartsManager(QObject *parent) : QObject{parent}{}

/**
 *
 * Tworzy wykres o zadanym typie i konfiguruje jego oś X i Y. Następnie dodaje wykres do wskazanego
 * layoutu w interfejsie Qt. Funkcja ustawia kolory linii, tytuł, zakresy osi oraz antyaliasing.
 *
 * @param type Typ wykresu (np. PWM, RPM, Current, itd.).
 * @param targetLayout Layout, do którego ma zostać dodany wykres.
 * @param title Tytuł wykresu wyświetlany u góry.
 * @param yLabel Opis osi Y (np. "obr/min", "V", "mA").
 * @param yMax Maksymalna wartość zakresu osi Y.
 * @param xRange Zakres osi X w sekundach (domyślnie 5).
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
 *
 * Dodaje pojedynczy punkt do wykresu danego typu. Jeśli zakres X został przekroczony, oś X
 * przesuwa się w prawo. Stare punkty poza aktualnym zakresem są usuwane.
 *
 * @param type Typ wykresu.
 * @param time Czas pomiaru (np. czas działania aplikacji w sekundach).
 * @param value Wartość pomiaru (np. prąd, RPM).
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
 *
 * Funkcja działa iteracyjnie, usuwając punkty z początku serii do momentu, aż wszystkie znajdują się
 * w aktualnym oknie czasowym wykresu (domyślnie ostatnie 5 sekund).
 *
 * @param series Seria danych (QLineSeries), z której mają zostać usunięte stare punkty.
 * @param currentTime Aktualny czas (w sekundach) używany do określenia okna czasowego.
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

void ChartsManager::setTitle(ChartType type, const QString &title) {
    charts[type].chart->setTitle(title);
}

void ChartsManager::setSeriesName(ChartType type, const QString &name) {
    charts[type].series->setName(name);
}
