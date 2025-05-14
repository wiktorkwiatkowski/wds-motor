#ifndef CHARTSMANAGER_H
#define CHARTSMANAGER_H

#include <QObject>
#include <QtCharts>
#include <QMap>

enum class ChartType {
    PWM,
    RPM,
    Voltage,
    Current,
    Power
};

class ChartsManager : public QObject
{
    Q_OBJECT
public:
    explicit ChartsManager(QObject *parent = nullptr);
    void setupChart(ChartType type, QLayout *targetLayout, const QString &title, const QString &yLabel, float yMax, int xRange = 5);
    void addPoint(ChartType type, qreal time, qreal value);

private:
    struct ChartComponents {
        QLineSeries *series;
        QChart *chart;
        QChartView *chartView;
        QValueAxis *axisX;
        QValueAxis *axisY;
        qreal lastXAxisUpdateTime = 0.0;
        int xRange;
    };

    QMap<ChartType, ChartComponents> charts;
    void removeOldPoints(QLineSeries *series, qreal currentTime);

};

#endif // CHARTSMANAGER_H
