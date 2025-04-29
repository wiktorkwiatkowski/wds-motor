#include "../inc/portdialog.h"
#include "../ui/ui_portdialog.h"

PortDialog::PortDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PortDialog)
{
    ui->setupUi(this);
    // Podłączenie przycisków do funkcji (slotów)
    connect(ui->pushButtonConnect, &QPushButton::clicked, this, &PortDialog::on_pushButtonConnect_clicked);
    connect(ui->pushButtonRefresh, &QPushButton::clicked, this, &PortDialog::on_pushButtonRefresh_clicked);

    // Pierwsze wczytanie dostępnych portów do listy
    refreshPorts();
}

PortDialog::~PortDialog()
{
    delete ui;
}

// Wczytuje dostępne porty szeregowe do comboBoxa
void PortDialog::refreshPorts()
{
    // Wyczyść poprzednie pozycje
    ui->comboBoxPort->clear();

    // Pobierz listę dostępnych portów szeregowych
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // Dodaj każdy port do rozwijanej listy
    for (const QSerialPortInfo &port : ports) {
        ui->comboBoxPort->addItem(port.systemLocation());
    }
}

// Obsługa kliknięcia przycisku "Połącz"
void PortDialog::on_pushButtonConnect_clicked()
{
    // Pobierz aktualnie wybrany port z rozwijanej listy
    QString selectedPortName = ui->comboBoxPort->currentText();

    // Jeśli nic nie wybrano — pokaż ostrzeżenie
    if (selectedPortName.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"), tr("Nie wybrano żadnego portu!"));
        return;
    }

    // Zapisz wybraną nazwę portu, będzie do odczytu przez główne okno
    portName = selectedPortName;

    // Zamknij okno dialogowe i zaakceptuj wybór
    accept();
}

// Obsługa kliknięcia przycisku "Odśwież"
void PortDialog::on_pushButtonRefresh_clicked()
{
    // Ponownie wczytaj listę portów
    refreshPorts();
}

// Zwraca nazwę aktualnie wybranego portu
QString PortDialog::selectedPort() const
{
    return portName;
}
