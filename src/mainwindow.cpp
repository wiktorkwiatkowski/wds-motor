#include "../inc/mainwindow.h"
#include "../ui/ui_mainwindow.h"

MainWindow::MainWindow(QString portName, QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    serialReader(new SerialReader(this)),
    pwmSeries(new QLineSeries),
    chart(new QChart),
    chartView(new QChartView),
    axisX(new QValueAxis),
    axisY(new QValueAxis)
{
    ui->setupUi(this);

    // Inicjalizacja wykresu PWM
    setupPWMChart();

    // Rozpoczęcie komunikacji z ESP32 przez podany port
    serialReader->start(portName);

    // Połączenie sygnału odebrania danych z funkcją obsługującą aktualizację GUI
    connect(serialReader, &SerialReader::newDataReceived, this, &MainWindow::handleNewSerialData);

    // Obsługa błędów komunikacji szeregowej
    connect(serialReader, &SerialReader::errorOccurred, this, &MainWindow::handleSerialError);

    // Obsługa zmiany wartości suwaka PWM
    connect(ui->SliderPWMManual, &QSlider::valueChanged, this, &MainWindow::on_sliderPWMManual_valueChanged);

    // Start timera do pomiaru czasu oraz początku uruchomienia aplikacji
    elapsed.start();

}

MainWindow::~MainWindow()
{
    // Zamknięcie portu szeregowego i zwolnienie pamięci interfejsu
    serialReader->stop();
    delete ui;
}

// Funkcja aktualizująca wartości w GUI po odebraniu nowej ramki danych
void MainWindow::handleNewSerialData(const SerialData &data)
{
    qDebug() << "RPM:" << data.rpm
             << "PWM:" << data.pwm
             << "Current:" << data.current
             << "Voltage:" << data.voltage
             << "Power:" << data.power
             << "Kp:" << data.kp
             << "Ki:" << data.ki
             << "Kd:" << data.kd
             << "Mode:"<<data.mode;

    // Ustawianie wartości w odpowiednich etykietach GUI
    ui->labelRPMValue->setText(QString::number(data.rpm, 'f', 0));
    ui->labelCurrentValue->setText(QString::number(data.current, 'f', 2));
    ui->labelVoltageValue->setText(QString::number(data.voltage, 'f', 1));
    ui->labelPowerValue->setText(QString::number(data.power, 'f', 1));

    // Obliczenie czasu w sekundach od uruchomienia aplikacji
    qreal currentTime = elapsed.elapsed() / 1000.0;

    // Dodanie punktu na wykresie (PWM przeskalowane do 0–100%)
    pwmSeries->append(currentTime, data.pwm / 2.55f);

    // Jeśli czas przekracza 5s, przesuwaj oś X
    if (currentTime > 5.0) {
        axisX->setRange(currentTime - 5.0, currentTime);
    }

    // Usuwanie starych punktów spoza zakresu ostatnich 5 sekund
    // Jeśli są jakiekolwiek punkty oraz wartość pierwszego punktu na osi X jest starsza niż ostatnie 5 sekund to go usuń.
    // I tak w pętli do momentu pozbycia się punktów, które przekroczyły zadany czas, w tym przypadku 5 sekund.
    while (!pwmSeries->points().isEmpty() && pwmSeries->points().first().x() < (currentTime - 5.0)) {
        pwmSeries->remove(0);
    }


}

// Funkcja obsługująca błędy komunikacji szeregowej
void MainWindow::handleSerialError(const QString &error)
{
    qDebug() << "Błąd:" << error;
}

void MainWindow::on_sliderPWMManual_valueChanged(float value)
{
    // Wyświetl wartość na etykiecie
    ui->labelPWMManualValue->setText(QString::number(value) + "%");

    // Skalowanie: 100% -> 25 jednostek
    float scaledValue = value * 2.55f;

    // Wyślij do ESP32
    serialReader->sendData(PWM, scaledValue);
}

// Inicjalizacja i konfiguracja wykresu PWM
void MainWindow::setupPWMChart(){
    // Ustawienia serii danych
    pwmSeries->setName("PWM");
    chart->addSeries(pwmSeries);
    chart->setTitle("Wypełnienie PWM w czasie");

    // Oś X, czas [s]
    axisX->setRange(0, 5.0);
    axisX->setLabelFormat("%.1f");
    axisX->setTitleText("Czas [s]");
    chart->addAxis(axisX, Qt::AlignBottom);
    pwmSeries->attachAxis(axisX);

    // Oś Y, PWM [%]
    axisY->setRange(0, 100);
    axisY->applyNiceNumbers();
    axisY->setTitleText("PWM [%]");
    chart->addAxis(axisY, Qt::AlignLeft);
    pwmSeries->attachAxis(axisY);

    // Dodanie wykresu do widoku i aktywacja wygładzania
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Dodanie widoku do layoutu w UI
    ui->widgetPWMGraph->layout()->addWidget(chartView);

}

