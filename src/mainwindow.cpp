#include "../inc/mainwindow.h"
#include "../ui/ui_mainwindow.h"

MainWindow::MainWindow(QString portName, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serialReader(new SerialReader(this))
{
    ui->setupUi(this);
    serialReader->start(portName);
    connect(serialReader, &SerialReader::newDataReceived, this, &MainWindow::handleNewSerialData);

    connect(serialReader, &SerialReader::errorOccurred, this, &MainWindow::handleSerialError);

}

MainWindow::~MainWindow()
{
    serialReader->stop();
    delete ui;
}

void MainWindow::on_pushButtonSearchPorts_clicked()
{
    serialReader->sendData(PWM, 250.f);
    // qDebug() << "Wcisnąłeś przycisk OK";
}

void MainWindow::handleNewSerialData(const SerialData &data)
{
    qDebug() << "RPM:" << data.rpm
             << "PWM:" << data.pwm
             << "Current:" << data.current
             << "Voltage:" << data.voltage
             << "Power:" << data.power
             << "Kp:" << data.kp
             << "Ki:" << data.ki
             << "Kd:" << data.kd;

    // Aktualizacja etykiet w interfejsie
    ui->labelRPMValue->setText(QString::number(data.rpm, 'f', 0));        // np. 1200
    ui->labelCurrentValue->setText(QString::number(data.current, 'f', 2)); // np. 1.23
    ui->labelVoltageValue->setText(QString::number(data.voltage, 'f', 1)); // np. 12.5
    ui->labelPowerValue->setText(QString::number(data.power, 'f', 1));     // np. 25.0
}

// Nowy slot - obsługuje błędy
void MainWindow::handleSerialError(const QString &error)
{
    qDebug() << "Błąd:" << error;
    // Możesz tu później pokazać np. QMessageBox::critical
}
