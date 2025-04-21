#include "../inc/portdialog.h"
#include "../ui/ui_portdialog.h"

PortDialog::PortDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PortDialog)
{
    ui->setupUi(this);
    // Połączenie sygnałów z slotami
    connect(ui->pushButtonConnect, &QPushButton::clicked, this, &PortDialog::on_pushButtonConnect_clicked);
    connect(ui->pushButtonRefresh, &QPushButton::clicked, this, &PortDialog::on_pushButtonRefresh_clicked);

    // Inicjalizacja listy portów
    refreshPorts();
}

PortDialog::~PortDialog()
{
    delete ui;
}

// Odświeżenie listy dostępnych portów
void PortDialog::refreshPorts()
{
    ui->comboBoxPort->clear();

    // Pobierz dostępne porty
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // Dodaj porty do ComboBox
    for (const QSerialPortInfo &port : ports) {
        ui->comboBoxPort->addItem(port.systemLocation());
    }
}

// Funkcja wywoływana po naciśnięciu "Połącz"
void PortDialog::on_pushButtonConnect_clicked()
{
    // Pobierz wybrany port z ComboBox
    QString selectedPortName = ui->comboBoxPort->currentText();
    if (selectedPortName.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nie wybrano żadnego portu!"));
        return;
    }
    // Zapisz wybrany port, aby później go użyć w aplikacji
    portName = selectedPortName;

    accept();
}

// Funkcja wywoływana po naciśnięciu "Odśwież"
void PortDialog::on_pushButtonRefresh_clicked()
{
    // Odśwież listę portów
    refreshPorts();
}

// Funkcja do uzyskania wybranego portu
QString PortDialog::selectedPort() const
{
    return portName;
}
