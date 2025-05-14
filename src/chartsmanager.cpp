#include "../inc/chartsmanager.h"

ChartsManager::ChartsManager(QObject *parent) : QObject{parent}{}

void ChartsManager::setupChart(ChartType type, QLayout *targetLayout, const QString &title, const QString &yLabel, float yMax, int xRange) {
    ChartComponents components;
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
    components.axisX->setTitleText("Czas [s]");
    components.chart->addAxis(components.axisX, Qt::AlignBottom);
    components.series->attachAxis(components.axisX);

    components.axisY->setRange(0, yMax);
    components.axisY->applyNiceNumbers();
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

void ChartsManager::addPoint(ChartType type, qreal time, qreal value) {
    if (!charts.contains(type)) return;

    auto &c = charts[type];
    c.series->append(time, value);

    // Jeśli czas przekracza 5s, przesuwaj oś X
    if (time > c.xRange) {
        c.axisX->setRange(time - c.xRange, time);
    }
    // Przesuwanie osi X tylko co 1 sek
    // if (time - c.lastXAxisUpdateTime >= 1.0) {
    //     if (time > c.xRange) {
    //         c.axisX->setRange(time - c.xRange, time);
    //     } else {
    //         c.axisX->setRange(0, c.xRange);
    //     }
    //     c.lastXAxisUpdateTime = time;
    // }

    removeOldPoints(c.series, time);
}

void ChartsManager::removeOldPoints(QLineSeries *series, qreal currentTime) {
    // Usuwanie starych punktów spoza zakresu ostatnich 5 sekund
    // Jeśli są jakiekolwiek punkty oraz wartość pierwszego punktu na osi X jest
    // starsza niż ostatnie 5 sekund to go usuń. I tak w pętli do momentu pozbycia
    // się punktów, które przekroczyły zadany czas, w tym przypadku 5 sekund.
    while (!series->points().isEmpty() && series->points().first().x() < (currentTime - 5.0)) {
        series->remove(0);
    }
}
