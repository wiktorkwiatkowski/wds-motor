#include "../inc/mainwindow.h"
#include "../ui/ui_mainwindow.h"

/**
 * @brief Konstruktor klasy MainWindow.
 *
 * Inicjalizuje interfejs użytkownika, nawiązuje połączenie z ESP32, ustawia kolory etykiet,
 * uruchamia timer aktualizacji oraz inicjalizuje i wyświetla wykresy w podanych kontenerach.
 *
 * @param portName Nazwa portu szeregowego, z którym należy się połączyć (np. "COM3", "/dev/ttyUSB0").
 * @param parent Obiekt nadrzędny Qt (zwykle nullptr).
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    serialReader(new SerialReader(this)), charts(new ChartsManager(this)){
    ui->setupUi(this);
    ui->label_8->setText("nie połączono");
    ui->label_8->setStyleSheet("color: red; font-weight: bold;");
    refreshSerialPortList();
    // Połączenie sygnału odebrania danych z funkcją obsługującą aktualizację GUI
    connect(serialReader, &SerialReader::newDataReceived, this, &MainWindow::handleNewSerialData);

    // Obsługa błędów komunikacji szeregowej
    connect(serialReader, &SerialReader::errorOccurred, this, &MainWindow::handleSerialError);

    // Obsługa błedu przerwania połączenia
    connect(serialReader, &SerialReader::portDisconnected, this, &MainWindow::handlePortDisconnected);


    // Obsługa zmiany wartości suwaka PWM
    connect(ui->SliderPWMManual, &QSlider::valueChanged, this, &MainWindow::on_sliderPWMManual_valueChanged);

    connect(ui->pushButtonStartStop, &QPushButton::clicked, this, &MainWindow::on_buttonStartStop_clicked);
    connect(ui->pushButtonToggleMode, &QPushButton::clicked, this, &MainWindow::on_buttonToggleMode_clicked);
    connect(ui->pushButtonSetRPM, &QPushButton::clicked, this, &MainWindow::on_buttonSetRPM_clicked);

    // Połącz port
    connect(ui->pushButtonConnectPort, &QPushButton::clicked, this, &MainWindow::on_ConnectPortClicked);
    // Odswieżenie portu
    connect(ui->pushButtonRefreshPorts, &QPushButton::clicked, this, &MainWindow::refreshSerialPortList);

    // Wyłącz opcję auto
    ui->lineEditTargetRPM->setVisible(false);
    ui->pushButtonSetRPM->setVisible(false);
    ui->labelSetRPMAuto->setVisible(false);
    ui->labelrpermin->setVisible(false);
    ui->widgetSetrpermin->setVisible(false);



    // Obsługa przycisku "Zapisz wartości"
    connect(ui->buttonSavePID, &QPushButton::clicked, this, [this]() {

        QString kpText = ui->editKp->text();
        QString kiText = ui->editKi->text();
        QString kdText = ui->editKd->text();


        // Jeśli pole nie jest puste, konwertuj i wyślij
        if (!kpText.isEmpty()) {
            float kp = kpText.toFloat();
            serialReader->sendData(DataType::Kp, kp);
        }
        if (!kiText.isEmpty()) {
            float ki = kiText.toFloat();
            serialReader->sendData(DataType::Ki, ki);
        }
        if (!kdText.isEmpty()) {
            float kd = kdText.toFloat();
            serialReader->sendData(DataType::Kd, kd);
        }

        // Czyść tylko pola, które użytkownik edytował
        if (!kpText.isEmpty()) ui->editKp->clear();
        if (!kiText.isEmpty()) ui->editKi->clear();
        if (!kdText.isEmpty()) ui->editKd->clear();
    });
    QDoubleValidator *validator = new QDoubleValidator(this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setDecimals(4); // np. 4 miejsca po przecinku
    // Wymuś kropkę jako separator dziesiętny
    validator->setLocale(QLocale::English);  // lub QLocale(QLocale::English)


    ui->editKp->setValidator(validator);
    ui->editKi->setValidator(validator);
    ui->editKd->setValidator(validator);
    validator->setBottom(0.0); // tylko dodatnie wartości



    setLabelsColors();
    // Inicjalizacja wykresów
    charts->setupChart(ChartType::PWM, ui->widgetPWMGraph->layout(), "PWM", "PWM [%]", 110, 5, false);
    charts->setupChart(ChartType::RPM, ui->widgetRPMGraph->layout(), "RPM", "obr/min", 700, 5, false);
    charts->setupChart(ChartType::Voltage, ui->widgetVoltageGraph->layout(), "Napięcie", "V", 8.5, 5, false);
    charts->setupChart(ChartType::Current, ui->widgetCurrentGraph->layout(), "Prąd", "mA", 800, 5, false );
    charts->setupChart(ChartType::Power, ui->widgetPowerGraph->layout(), "Moc", "mW", 5400, 5, false);

    // Timer do cyklicznej aktualizacji GUI i wykresów
    updateChartsTimer = new QTimer(this);
    updateChartsTimer->setInterval(10); // co 10 ms
    connect(updateChartsTimer, &QTimer::timeout, this, &MainWindow::updateCharts);
    updateChartsTimer->start();

    updateGUITimer = new QTimer(this);
    updateGUITimer->setInterval(500); // co 10 ms
    connect(updateGUITimer, &QTimer::timeout, this, &MainWindow::updateGUI);
    updateGUITimer->start();
    // Start timera do pomiaru czasu oraz początku uruchomienia aplikacji
    elapsed.start();
}

/**
 * @brief Destruktor klasy MainWindow.
 *
 * Zatrzymuje komunikację szeregową i zwalnia zasoby GUI.
 */
MainWindow::~MainWindow() {
    // Zamknięcie portu szeregowego i zwolnienie pamięci interfejsu
    serialReader->stop();
    delete ui;
}

/**
 * @brief Aktualizuje lokalne dane na podstawie odebranej ramki.
 *
 * Funkcja jest wywoływana przez `SerialReader`, gdy przychodzą nowe dane z ESP32.
 * Dane są zapisywane do `latestData` i przetwarzane później w `updateGUIAndCharts()`.
 *
 * @param data Struktura zawierająca dane z ramki (RPM, PWM, napięcie, prąd, moc, PID, tryb).
 */
void MainWindow::handleNewSerialData(const SerialData &data) {
    // qDebug() << "RPM:" << data.rpm << "PWM:" << data.pwm
    //          << "Current:" << data.current << "Voltage:" << data.voltage
    //          << "Power:" << data.power << "Kp:" << data.kp << "Ki:" << data.ki
    //          << "Kd:" << data.kd << "Mode:" << data.mode;
    latestData = data;
}

/**
 * @brief Obsługuje błędy komunikacji szeregowej.
 *
 * Wyświetla komunikat błędu w konsoli debug Qt.
 *
 * @param error Treść błędu.
 */
void MainWindow::handleSerialError(const QString &error) {
    qDebug() << "Błąd:" << error;
}

/**
 * @brief Obsługuje zmianę wartości suwaka PWM przez użytkownika.
 *
 * Aktualizuje etykietę wyświetlającą wartość wypełnienia PWM oraz wysyła przeskalowaną
 * wartość do ESP32 jako sygnał sterujący.
 *
 * @param value Wartość wypełnienia PWM w zakresie 0–100%.
 */
void MainWindow::on_sliderPWMManual_valueChanged(float value) {
    // Wyświetl wartość na etykiecie
    ui->labelPWMManualValue->setText(QString::number(value) + "%");

    // Skalowanie: 100% -> 25 jednostek
    float scaledValue = value * 2.55f;

    // Wyślij do ESP32
    serialReader->sendData(PWM, scaledValue);
}

/**
 * @brief Ustawia kolory etykiet z wartościami pomiarowymi w GUI.
 *
 * Każda wartość ma przypisany kolor zgodny z kolorem linii na wykresie.
 */
void MainWindow::setLabelsColors() const{
    ui->labelColorBoxV->setStyleSheet("background-color: blue;");
    ui->labelColorBoxC->setStyleSheet("background-color: red;");
    ui->labelColorBoxP->setStyleSheet("background-color: green;");
    ui->labelColorBoxRPM->setStyleSheet("background-color: orange;");
    ui->labelColorBoxPWM->setStyleSheet("background-color: purple;");

}

/**
 * @brief Aktualizuje GUI i wykresy na podstawie ostatnio odebranych danych.
 *
 * Funkcja wywoływana co 10 ms przez timer. Ustawia etykiety z aktualnymi wartościami
 * pomiarowymi oraz dodaje nowe punkty do wykresów z uwzględnieniem czasu działania aplikacji.
 */
void MainWindow::updateCharts() const{
    // Aktualizacja wykresów
    qreal t = elapsed.elapsed() / 1000.0;

    charts->addPoint(ChartType::PWM, t, latestData.pwm / 2.55f);
    charts->addPoint(ChartType::RPM, t, latestData.rpm);
    charts->addPoint(ChartType::Current, t, latestData.current);
    charts->addPoint(ChartType::Voltage, t, latestData.voltage);
    charts->addPoint(ChartType::Power, t, latestData.power);

}

void MainWindow::updateGUI() const{
    // Ustawienie wartości w GUI
    ui->lineEditRPMValue->setText(QString::number(latestData.rpm, 'f', 0));
    ui->lineEditCurrentValue->setText(QString::number(latestData.current, 'f', 2));
    ui->lineEditVoltageValue->setText(QString::number(latestData.voltage, 'f', 2));
    ui->lineEditPowerValue->setText(QString::number(latestData.power, 'f', 1));
    ui->lineEditPWMValue->setText(QString::number(latestData.pwm / 2.55f, 'f', 1));

    ui->lineEditKpValue->setText(QString::number(latestData.kp, 'f', 2));
    ui->lineEditKiValue->setText(QString::number(latestData.ki, 'f', 2));
    ui->lineEdiKdValue->setText(QString::number(latestData.kd, 'f', 2));
}

void MainWindow::on_buttonStartStop_clicked() {
    isMotorRunning = !isMotorRunning;
    ui->pushButtonStartStop->setText(isMotorRunning ? "STOP" : "START");

    // Wyślij do ESP32: np. 1 = start, 0 = stop
    serialReader->sendData(DataType::start_stop, isMotorRunning ? 1.0f : 0.0f);
}

void MainWindow::on_buttonToggleMode_clicked() {
    isManualMode = !isManualMode;
    // Zawsze wyłącz silnik przy zmianie trybu
    serialReader->sendData(DataType::PWM, 0.0f);

    ui->pushButtonToggleMode->setText(isManualMode ? "Tryb: Ręczny" : "Tryb: Automatyczny");

    // Jest tryb manualny to wyślij 0, automatyczny 1
    if (isManualMode == 1){
        serialReader->sendData(DataType::mode, 0.0f);
    }else{
        serialReader->sendData(DataType::mode, 1.0f);
    }

    // Pokaż/ukryj elementy odpowiedniego trybu
    ui->SliderPWMManual->setVisible(isManualMode);
    ui->labelPWMManualValue->setVisible(isManualMode);
    ui->labelPWM->setVisible(isManualMode);


    ui->lineEditTargetRPM->setVisible(!isManualMode);
    ui->pushButtonSetRPM->setVisible(!isManualMode);
    ui->labelSetRPMAuto->setVisible(!isManualMode);
    ui->labelrpermin->setVisible(!isManualMode);
    ui->widgetSetrpermin->setVisible(!isManualMode);

}

void MainWindow::on_buttonSetRPM_clicked() {
    bool ok;
    float rpm = ui->lineEditTargetRPM->text().toFloat(&ok);
    if (ok) {
        serialReader->sendData(DataType::RPM, rpm);
        qDebug() << "wysyłam wartość rpm: "<<rpm;
    } else {
        qDebug() << "Nieprawidłowa wartość RPM";
    }
    ui->lineEditTargetRPM->clear();
}

void MainWindow::on_ConnectPortClicked(){
    if (!isPortConnected) {
        QString selectedPort = ui->comboBoxSelectPort->currentText();
        if (selectedPort.isEmpty()) {
            qDebug() << "Nie wybrano portu";
            return;
        }

        // Próbujemy się połączyć
        serialReader->stop();
        serialReader->start(selectedPort);
        if (!serialReader->isOpen()) {
            qDebug() << "Nie udało się połączyć z portem: " << selectedPort;
            ui->label_8->setText("nie połączono");
            ui->label_8->setStyleSheet("color: red; font-weight: bold;");
            return;
        }

        // Zaktualizuj GUI
        currentPortName = selectedPort;
        currentBaudRate = ui->comboBoxBaudRates->currentText().toInt();
        ui->label_8->setText("połączono");
        ui->label_8->setStyleSheet("color: green; font-weight: bold;");
        ui->pushButtonConnectPort->setText("Rozłącz");

        isPortConnected = true;

    } else {
        handlePortDisconnected();
    }
}

void MainWindow::refreshSerialPortList() {
    ui->comboBoxSelectPort->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        ui->comboBoxSelectPort->addItem(info.systemLocation());
    }
}

void MainWindow::handlePortDisconnected() {
    isPortConnected = false;
    ui->label_8->setText("nie połączono");
    ui->label_8->setStyleSheet("color: red; font-weight: bold;");
    ui->pushButtonConnectPort->setText("Połącz");

    // Reset stanu silnika
    isMotorRunning = false;
    ui->pushButtonStartStop->setText("START");

    // Przywróć domyślny tryb ręczny
    isManualMode = true;
    ui->pushButtonToggleMode->setText("Tryb: Ręczny");
    ui->SliderPWMManual->setVisible(true);
    ui->labelPWMManualValue->setVisible(true);
    ui->lineEditTargetRPM->setVisible(false);
    ui->pushButtonSetRPM->setVisible(false);
    ui->labelSetRPMAuto->setVisible(false);
    ui->labelrpermin->setVisible(false);
    ui->widgetSetrpermin->setVisible(false);

    serialReader->stop();
}
