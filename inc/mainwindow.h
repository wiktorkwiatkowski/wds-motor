#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QSerialPort>
#include "serialreader.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString portName, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonSearchPorts_clicked();

private:
    Ui::MainWindow *ui;
    SerialReader *serialReader;
};
#endif // MAINWINDOW_H
