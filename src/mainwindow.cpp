/**
 * @file mainwindow.cpp
 * @brief Implementacja klasy MainWindow.
 *
 * Klasa MainWindow obsługuje interfejs graficzny aplikacji sterującej silnikiem.
 * Odpowiada za komunikację z mikrokontrolerem (poprzez SerialReader), wizualizację parametrów
 * pracy silnika na wykresach (ChartsManager), obsługę przycisków, suwaków oraz zarządzanie językiem interfejsu.
 */

#include "../inc/mainwindow.h"
#include "../ui/ui_mainwindow.h"

/**
 * @brief Konstruktor klasy MainWindow.
 *
 * Tworzy i konfiguruje interfejs użytkownika oraz inicjalizuje pozostałe komponenty:
 * - SerialReader (komunikacja szeregowa),
 * - ChartsManager (wykresy),
 * - timery do aktualizacji GUI i wykresów.
 * Na końcu uruchamia licznik czasu od startu aplikacji.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    serialReader(new SerialReader(this)), charts(new ChartsManager(this)){
    ui->setupUi(this);

    connectSignals();

    refreshSerialPortList();

    configureInitialMode();

    setupValidators();

    setLabelsColors();

    setupCharts();

    setupTimers();

    elapsed.start();
}

/**
 * @brief Destruktor klasy MainWindow.
 *
 * Zatrzymuje komunikację szeregowa i zwalnia zasoby GUI.
 */
MainWindow::~MainWindow() {
    // Zamknięcie portu szeregowego i zwolnienie pamięci interfejsu
    serialReader->stop();
    delete ui;
}

/**
 * Zapisuje dane w polu latestData, dane te są wykorzystywane do aktualizacji GUI i wykresów.
 */
void MainWindow::handleNewSerialData(const SerialData &data) {
    // qDebug() << "RPM:" << data.rpm << "PWM:" << data.pwm
    //          << "Current:" << data.current << "Voltage:" << data.voltage
    //          << "Power:" << data.power << "Kp:" << data.kp << "Ki:" << data.ki
    //          << "Kd:" << data.kd << "Mode:" << data.mode;
    latestData = data;
}

/**
 * Wyświetla błąd w konsoli debug Qt.
 */
void MainWindow::handleSerialError(const QString &error) {
    qDebug() << "Błąd:" << error;
}

/**
 * Wyświetla nową wartość na etykiecie i wysyła ją do mikrokontrolera w formie skalowanej (0-255).
 */
void MainWindow::on_sliderPWMManual_valueChanged(float value) {
    ui->labelPWMManualValue->setText(QString::number(value) + "%");

    // Skalowanie
    float scaledValue = value * 2.55f;

    // Wyślij do ESP32
    serialReader->sendData(PWM, scaledValue);
}

/**
 * Ułatwia wizualne skojarzenie wartości z liniami na wykresach.
 */
void MainWindow::setLabelsColors() const{
    ui->labelColorBoxV->setStyleSheet("background-color: blue;");
    ui->labelColorBoxC->setStyleSheet("background-color: red;");
    ui->labelColorBoxP->setStyleSheet("background-color: green;");
    ui->labelColorBoxRPM->setStyleSheet("background-color: orange;");
    ui->labelColorBoxPWM->setStyleSheet("background-color: purple;");

}

/**
 * Dodaje nowe punkty do wykresów (PWM, RPM, prąd, napięcie, moc) z aktualnym czasem.
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

/**
 * Wyświetla aktualne wartości parametrów pracy silnika i parametrów PID.
 */
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

/**
 * Przełącza stan pracy silnika i wysyła odpowiednie polecenie do mikrokontrolera.
 */
void MainWindow::on_buttonStartStop_clicked() {
    isMotorRunning = !isMotorRunning;
    ui->pushButtonStartStop->setText(isMotorRunning ? "STOP" : "START");

    serialReader->sendData(DataType::start_stop, isMotorRunning ? 1.0f : 0.0f);
    serialReader->sendData(DataType::PWM, 0.0f);
    serialReader->sendData(DataType::RPM, 0.0f);

    // Wyzeruj suwak PWM po naciśnięciu STOP
    if (!isMotorRunning) {
        ui->SliderPWMManual->setValue(0);
    }
}

/**
 * Funkcja:
 * - zmienia tryb pracy aplikacji (manualny <-> automatyczny),
 * - zawsze zatrzymuje silnik przy zmianie trybu (wysyła PWM = 0),
 * - wysyła odpowiednie polecenie trybu do mikrokontrolera,
 * - w trybie automatycznym dodatkowo resetuje żądaną prędkość RPM,
 * - ukrywa lub pokazuje odpowiednie elementy GUI w zależności od trybu.
 */
void MainWindow::on_buttonToggleMode_clicked() {
    serialReader->sendData(DataType::PWM, 0.0f);
    isManualMode = !isManualMode;
    // Zawsze wyłącz silnik przy zmianie trybu

    ui->pushButtonToggleMode->setText(isManualMode ? tr("Tryb: Ręczny") : tr("Tryb: Automatyczny"));

    // Jest tryb manualny to wyślij 0, automatyczny 1
    if (isManualMode == 1){
        serialReader->sendData(DataType::mode, 0.0f);
    }else{
        serialReader->sendData(DataType::mode, 1.0f);
        serialReader->sendData(DataType::RPM, 0.0f);
        ui->SliderPWMManual->setValue(0);
    }

    // Pokaż/ukryj elementy odpowiedniego trybu
    ui->SliderPWMManual->setVisible(isManualMode);
    ui->labelPWMManualValue->setVisible(isManualMode);
    ui->labelPWM->setVisible(isManualMode);


    ui->spinBoxTargetRPM->setVisible(!isManualMode);
    ui->pushButtonSetRPM->setVisible(!isManualMode);
    ui->labelSetRPMAuto->setVisible(!isManualMode);
    ui->labelrpermin->setVisible(!isManualMode);
    ui->widgetSetrpermin->setVisible(!isManualMode);

}

/**
 * Odczytuje wartość RPM z pola tekstowego i wysyła ją do mikrokontrolera.
 */
void MainWindow::on_buttonSetRPM_clicked() {
    bool ok;
    int rpm = ui->spinBoxTargetRPM->value();
    serialReader->sendData(DataType::RPM, rpm);
    qDebug()<< "Wartość rpm: "<< rpm;
    ui->spinBoxTargetRPM->clear();
}

/**
 * Nawiązuje lub kończy połączenie z wybranym portem szeregowym w zależności od aktualnego stanu.
 * Po poprawnym połączeniu aktualizuje GUI oraz zapisuje nazwę i prędkość portu.
 * W przypadku rozłączenia wywołuje handlePortDisconnected().
 */
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
            ui->label_8->setText(tr("nie połączono"));
            ui->label_8->setStyleSheet("color: red; font-weight: bold;");
            return;
        }

        // Zaktualizuj GUI
        currentPortName = selectedPort;
        currentBaudRate = ui->comboBoxBaudRates->currentText().toInt();
        ui->label_8->setText(tr("połączono"));
        ui->label_8->setStyleSheet("color: green; font-weight: bold;");
        ui->pushButtonConnectPort->setText(tr("Rozłącz"));

        isPortConnected = true;

    } else {
        handlePortDisconnected();
    }
}

/**
 * Odświeża listę dostępnych portów szeregowych.Funkcja czyści zawartość comboBoxSelectPort i dodaje tylko porty,
 * których nazwa zawiera jedno z poniższych wyrażeń:
 * - "ttyUSB",
 * - "ttyACM",
 * - "COM.
 */
void MainWindow::refreshSerialPortList() {
    ui->comboBoxSelectPort->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {

        // Filtrowanie: pokaż tylko porty zawierające "USB" lub "COM"
        QString name = info.systemLocation();

        if (name.contains("ttyUSB") || name.contains("ttyACM") || name.contains("COM")) {
            ui->comboBoxSelectPort->addItem(info.systemLocation());
        }
    }
}

/**
 * Funkcja wywoływana w przypadku:
 * - ręcznego kliknięcia "Rozłącz",
 * - nagłego odłączenia urządzenia (sygnał portDisconnected z SerialReader).
 *
 * Działania:
 * - aktualizuje GUI (zmiana etykiety statusu na "nie połączono", zmiana tekstu przycisku "Połącz"),
 * - resetuje stan pracy silnika (wyłącza silnik, ustawia przycisk Start/Stop na "START"),
 * - przywraca domyślny tryb ręczny oraz układ elementów GUI,
 * - resetuje wartość suwaka PWM do 0,
 * - zatrzymuje komunikację przez SerialReader.
 */
void MainWindow::handlePortDisconnected() {
    isPortConnected = false;
    ui->label_8->setText(tr("nie połączono"));
    ui->label_8->setStyleSheet("color: red; font-weight: bold;");
    ui->pushButtonConnectPort->setText(tr("Połącz"));

    // Reset stanu silnika
    isMotorRunning = false;
    ui->pushButtonStartStop->setText("START");

    // Przywróć domyślny tryb ręczny
    isManualMode = true;
    ui->pushButtonToggleMode->setText(tr("Tryb: Ręczny"));
    ui->SliderPWMManual->setVisible(true);
    ui->labelPWMManualValue->setVisible(true);
    ui->spinBoxTargetRPM->setVisible(false);
    ui->pushButtonSetRPM->setVisible(false);
    ui->labelSetRPMAuto->setVisible(false);
    ui->labelrpermin->setVisible(false);
    ui->widgetSetrpermin->setVisible(false);    
    ui->SliderPWMManual->setValue(0);
    serialReader->stop();
}

/**
 * Odczytuje wartości Kp, Ki, Kd i wysyła je do mikrokontrolera.
 */
void MainWindow::on_buttonSavePID_clicked() {
    QString kpText = ui->editKp->text();
    QString kiText = ui->editKi->text();
    QString kdText = ui->editKd->text();

    if (!kpText.isEmpty()) {
        float kp = kpText.toFloat();
        serialReader->sendData(DataType::Kp, kp);
        ui->editKp->clear();
    }
    if (!kiText.isEmpty()) {
        float ki = kiText.toFloat();
        serialReader->sendData(DataType::Ki, ki);
        ui->editKi->clear();
    }
    if (!kdText.isEmpty()) {
        float kd = kdText.toFloat();
        serialReader->sendData(DataType::Kd, kd);
        ui->editKd->clear();
    }
}

/**
 * Funkcja łączy sygnały interfejsu użytkownika (przyciski, suwaki, akcje menu) z odpowiednimi slotami
 * obsługującymi logikę aplikacji. Umożliwia również obsługę komunikacji z mikrokontrolerem (odbiór danych,
 * obsługa błędów, obsługa rozłączenia portu). Funkcja wywoływana jest podczas inicjalizacji MainWindow.
 */
void MainWindow::connectSignals(){
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

    // Obsługa przycisku "Zapisz wartości"
    connect(ui->buttonSavePID, &QPushButton::clicked, this, &MainWindow::on_buttonSavePID_clicked);

    connect(ui->actionPolski, &QAction::triggered, this, &MainWindow::switchToPolish);
    connect(ui->actionAngielski, &QAction::triggered, this, &MainWindow::switchToEnglish);

}

/**
 * Ustawia stan początkowy: tryb ręczny, brak połączenia, ukryte elementy trybu automatycznego.
 */
void MainWindow::configureInitialMode() {
    ui->label_8->setText(tr("nie połączono"));
    ui->label_8->setStyleSheet("color: red; font-weight: bold;");

    ui->spinBoxTargetRPM->setVisible(false);
    ui->pushButtonSetRPM->setVisible(false);
    ui->labelSetRPMAuto->setVisible(false);
    ui->labelrpermin->setVisible(false);
    ui->widgetSetrpermin->setVisible(false);
}

/**
 * Pozwala tylko na wpisywanie dodatnich liczb zmiennoprzecinkowych do 4 miejsc po przecinku.
 */
void MainWindow::setupValidators() {
    QDoubleValidator *validator = new QDoubleValidator(this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setDecimals(4);
    validator->setLocale(QLocale::English);
    validator->setBottom(0.0);

    ui->editKp->setValidator(validator);
    ui->editKi->setValidator(validator);
    ui->editKd->setValidator(validator);
}

/**
 * Ustawia zakresy, kolory, tytuły wykresów dla parametrów: PWM, RPM, napięcie, prąd, moc.
 */
void MainWindow::setupCharts() {
    charts->setupChart(ChartType::PWM, ui->widgetPWMGraph->layout(), "PWM", "PWM [%]", 110, 5, false);
    charts->setupChart(ChartType::RPM, ui->widgetRPMGraph->layout(), "RPM", "obr/min", 600, 5, false);
    charts->setupChart(ChartType::Voltage, ui->widgetVoltageGraph->layout(), tr("Napięcie"), "V", 8.5, 5, false);
    charts->setupChart(ChartType::Current, ui->widgetCurrentGraph->layout(), tr("Prąd"), "mA", 800, 5, false);
    charts->setupChart(ChartType::Power, ui->widgetPowerGraph->layout(), tr("Moc"), "mW", 5500, 5, false);

}

/**
 * Timery:
 * - updateChartsTimer -> odświeża wykresy co 10 ms,
 * - updateGUITimer -> odświeża dane w polach tekstowych co 500 ms.
 */
void MainWindow::setupTimers() {
    updateChartsTimer = new QTimer(this);
    updateChartsTimer->setInterval(10);
    connect(updateChartsTimer, &QTimer::timeout, this, &MainWindow::updateCharts);
    updateChartsTimer->start();

    updateGUITimer = new QTimer(this);
    updateGUITimer->setInterval(500);
    connect(updateGUITimer, &QTimer::timeout, this, &MainWindow::updateGUI);
    updateGUITimer->start();
}

/**
 * Ładuje plik tłumaczenia wds_motor_pl.qm i odświeża GUI.
 */
void MainWindow::switchToPolish() {
    qApp->removeTranslator(&translator);
    if (translator.load("../../i18n/wds_motor_pl.qm")) {
        qApp->installTranslator(&translator);
    } else {
        qDebug() << "Failed to load Polish translation!";
    }
    ui->retranslateUi(this); // odświeżenie GUI
    this->setWindowTitle(tr("Sterowanie silnikiem"));
    retranslateCharts();
    ui->pushButtonToggleMode->setText(isManualMode ? tr("Tryb: Ręczny") : tr("Tryb: Automatyczny"));

}

/**
 * Ładuje plik tłumaczenia wds_motor_en_US.qm i odświeża GUI.
 */
void MainWindow::switchToEnglish() {
    qApp->removeTranslator(&translator);
    if (translator.load("../../i18n/wds_motor_en_US.qm")) {
        qApp->installTranslator(&translator);
    } else {
        qDebug() << "Failed to load English translation!";
    }
    ui->retranslateUi(this); // odświeżenie GUI
    this->setWindowTitle(tr("Sterowanie silnikiem"));
    retranslateCharts();
    ui->pushButtonToggleMode->setText(isManualMode ? tr("Tryb: Ręczny") : tr("Tryb: Automatyczny"));

}

/**
 * Funkcja wywoływana automatycznie po zmianie języka GUI.
 */
void MainWindow::retranslateCharts() {
    charts->setTitle(ChartType::Voltage, tr("Napięcie"));
    charts->setSeriesName(ChartType::Voltage, tr("Napięcie"));
    charts->setXAxisTitle(ChartType::Voltage, tr("Czas [s]"));

    charts->setTitle(ChartType::Current, tr("Prąd"));
    charts->setSeriesName(ChartType::Current, tr("Prąd"));
    charts->setXAxisTitle(ChartType::Current, tr("Czas [s]"));

    charts->setTitle(ChartType::Power, tr("Moc"));
    charts->setSeriesName(ChartType::Power, tr("Moc"));
    charts->setXAxisTitle(ChartType::Power, tr("Czas [s]"));

    charts->setXAxisTitle(ChartType::RPM, tr("Czas [s]"));
    charts->setXAxisTitle(ChartType::PWM, tr("Czas [s]"));
}
