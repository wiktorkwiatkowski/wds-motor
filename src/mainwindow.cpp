#include "../inc/mainwindow.h"
#include "../ui/ui_mainwindow.h"

MainWindow::MainWindow(QString portName, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    serialReader(new SerialReader(this)), charts(new ChartsManager(this)){
    ui->setupUi(this);

    // Inicjalizacja wykresu PWM

    // Rozpoczęcie komunikacji z ESP32 przez podany port
    serialReader->start(portName);

    // Połączenie sygnału odebrania danych z funkcją obsługującą aktualizację GUI
    connect(serialReader, &SerialReader::newDataReceived, this, &MainWindow::handleNewSerialData);

    // Obsługa błędów komunikacji szeregowej
    connect(serialReader, &SerialReader::errorOccurred, this, &MainWindow::handleSerialError);

    // Obsługa zmiany wartości suwaka PWM
    connect(ui->SliderPWMManual, &QSlider::valueChanged, this, &MainWindow::on_sliderPWMManual_valueChanged);

    connect(ui->buttonSavePID, &QPushButton::clicked, this, [this]() {
        float kp = ui->editKp->text().toFloat();
        float ki = ui->editKi->text().toFloat();
        float kd = ui->editKd->text().toFloat();

        serialReader->sendData(DataType::Kp, kp);
        serialReader->sendData(DataType::Ki, ki);
        serialReader->sendData(DataType::Kd, kd);
    });

    setLabelsColors();

     // Inicjalizacja wykresu PWM
    charts->setupChart(ChartType::PWM, ui->widgetPWMGraph->layout(), "PWM", "PWM [%]", 100);
    charts->setupChart(ChartType::RPM, ui->widgetRPMGraph->layout(), "RPM", "obr/min", 5000);
    charts->setupChart(ChartType::Voltage, ui->widgetVoltageGraph->layout(), "Napięcie", "V", 15);
    charts->setupChart(ChartType::Current, ui->widgetCurrentGraph->layout(), "Prąd", "mA", 2000);
    charts->setupChart(ChartType::Power, ui->widgetPowerGraph->layout(), "Moc", "mW", 2000);
    // Start timera do pomiaru czasu oraz początku uruchomienia aplikacji
    elapsed.start();
}

MainWindow::~MainWindow() {
    // Zamknięcie portu szeregowego i zwolnienie pamięci interfejsu
    serialReader->stop();
    delete ui;
}

// Funkcja aktualizująca wartości w GUI po odebraniu nowej ramki danych
void MainWindow::handleNewSerialData(const SerialData &data) {
    qDebug() << "RPM:" << data.rpm << "PWM:" << data.pwm
             << "Current:" << data.current << "Voltage:" << data.voltage
             << "Power:" << data.power << "Kp:" << data.kp << "Ki:" << data.ki
             << "Kd:" << data.kd << "Mode:" << data.mode;

    // Ustawianie wartości w odpowiednich etykietach GUI
    ui->labelRPMValue->setText(QString::number(data.rpm, 'f', 0));
    ui->labelCurrentValue->setText(QString::number(data.current, 'f', 2));
    ui->labelVoltageValue->setText(QString::number(data.voltage, 'f', 1));
    ui->labelPowerValue->setText(QString::number(data.power, 'f', 1));
    ui->labelPWMValue->setText(QString::number(data.pwm));

    ui->labelKpValue->setText(QString::number(data.kp, 'f', 2));
    ui->labelKiValue->setText(QString::number(data.ki, 'f', 2));
    ui->labelKdValue->setText(QString::number(data.kd, 'f', 2));

    // Obliczenie czasu w sekundach od uruchomienia aplikacji
    qreal currentTime = elapsed.elapsed() / 1000.0;

    // Dodanie punktu na wykresie (PWM przeskalowane do 0–100%)
    charts->addPoint(ChartType::PWM, currentTime, data.pwm / 2.55f);
}

// Funkcja obsługująca błędy komunikacji szeregowej
void MainWindow::handleSerialError(const QString &error) {
    qDebug() << "Błąd:" << error;
}

void MainWindow::on_sliderPWMManual_valueChanged(float value) {
    // Wyświetl wartość na etykiecie
    ui->labelPWMManualValue->setText(QString::number(value) + "%");

    // Skalowanie: 100% -> 25 jednostek
    float scaledValue = value * 2.55f;

    // Wyślij do ESP32
    serialReader->sendData(PWM, scaledValue);
}

void MainWindow::setLabelsColors() const{
    ui->labelCurrentValue->setStyleSheet("color: red;");
    ui->labelRPMValue->setStyleSheet("color: orange;");
    ui->labelVoltageValue->setStyleSheet("color: blue;");
    ui->labelPowerValue->setStyleSheet("color: green;");
    ui->labelPWMValue->setStyleSheet("color: purple;");

}
