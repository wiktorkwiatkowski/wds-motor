#include "../inc/mainwindow.h"
#include "../ui/ui_mainwindow.h"

MainWindow::MainWindow(QString portName, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serialReader(new SerialReader(this))
{
    ui->setupUi(this);
    serialReader->start(portName);
    connect(serialReader, &SerialReader::newDataReceived, this, [](const SerialData &data) {
        qDebug() << "RPM:" << data.rpm
                 << "PWM:" << data.pwm
                 << "Current:" << data.current
                 << "Voltage:" << data.voltage
                 << "Power:" << data.power;
    });

    connect(serialReader, &SerialReader::errorOccurred, this, [](const QString &msg) {
        qDebug() << "Błąd:" << msg;
    });

}

MainWindow::~MainWindow()
{
    serialReader->stop();
    delete ui;
}

void MainWindow::on_pushButtonSearchPorts_clicked()
{
    serialReader->sendData(PWM, 250.f);
    qDebug() << "Wcisnąłeś przycisk OK";
}

