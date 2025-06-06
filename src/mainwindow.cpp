#include "../inc/mainwindow.h"
#include "../ui/ui_mainwindow.h"

/**
 * @brief Konstruktor klasy MainWindow.
 *
 *  Tworzy interfejs użytkownika oraz inicjalizuje wszystkie elementy aplikacji:
 * - Łączy sygnały z odpowiednimi slotami,
 * - Odswieża listę dostępnych portów,
 * - Ustawia tryb początkowy interfejsu (ręczny, brak połączenia),
 * - Ustawia walidatory pól PID,
 * - Ustawia kolory etykiet,
 * - Tworzy i konfiguruje wykresy,
 * - Konfiguruje timery aktualizujące GUI i wykresy.
 *
 * Na końcu uruchamia zegar odmierzający czas od startu aplikacji.
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
    // Inicjalizacja wykresów
    setupCharts();
    setupTimers();
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
 *
 * Funkcja ta zapisuje dane odebrane z ESP32 w polu latestData.
 * Dane te są następnie wykorzystywane do odświeżania GUI i wykresów.
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
 *
 * Funkcja ta:
 * - wyświetla nową wartość PWM na etykiecie,
 * - przelicza wartość z zakresu 0–100% na 0–255,
 * - wysyła tę wartość do ESP32 jako sygnał sterujący.
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
 *
 */
void MainWindow::setLabelsColors() const{
    ui->labelColorBoxV->setStyleSheet("background-color: blue;");
    ui->labelColorBoxC->setStyleSheet("background-color: red;");
    ui->labelColorBoxP->setStyleSheet("background-color: green;");
    ui->labelColorBoxRPM->setStyleSheet("background-color: orange;");
    ui->labelColorBoxPWM->setStyleSheet("background-color: purple;");

}

/**
 *
 * Funkcja wywoływana co 10 ms przez timer. Dodaje nowe punkty do wykresów z uwzględnieniem czasu działania aplikacji.
 *
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
 *
 * Funkcja ustawia aktualne wartości pomiarowe w polach tekstowych:
 * - RPM,
 * - prąd,
 * - napięcie,
 * - moc,
 * - PWM (przeskalowane),
 * - współczynniki PID (Kp, Ki, Kd).
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
 *
 * Funkcja przełącza stan pracy silnika pomiędzy uruchomieniem a zatrzymaniem.
 * Zmienna isMotorRunning jest przełączana, co powoduje:
 * - zmianę etykiety przycisku na "START" lub "STOP",
 * - wysłanie odpowiedniego sygnału sterującego do ESP32 (1.0 = start, 0.0 = stop).
 */
void MainWindow::on_buttonStartStop_clicked() {
    isMotorRunning = !isMotorRunning;
    ui->pushButtonStartStop->setText(isMotorRunning ? "STOP" : "START");

    // Wyślij do ESP32: np. 1 = start, 0 = stop
    serialReader->sendData(DataType::start_stop, isMotorRunning ? 1.0f : 0.0f);
}

/**
 *
 * Zmienia tryb sterowania z ręcznego na automatyczny (i odwrotnie).
 * Wykonywane czynności:
 * - Zatrzymanie silnika (wysłanie PWM = 0),
 * - Zmiana tekstu przycisku z informacją o aktualnym trybie,
 * - Wysłanie informacji o trybie do ESP32 (0 = manualny, 1 = automatyczny),
 * - Pokazanie lub ukrycie odpowiednich elementów GUI zależnie od trybu.
 */
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
        serialReader->sendData(DataType::RPM, 0.0f);
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

/**
 * @brief Slot obsługujący przycisk ustawiania zadanej wartości RPM.
 *
 * Funkcja odczytuje wartość wpisaną przez użytkownika w polu lineEditTargetRPM,
 * konwertuje ją na float i wysyła jako żądaną prędkość obrotową silnika do ESP32.
 */
void MainWindow::on_buttonSetRPM_clicked() {
    bool ok;
    float rpm = ui->lineEditTargetRPM->text().toFloat(&ok);
    if (ok) {
        serialReader->sendData(DataType::RPM, rpm);
    } else {
        qDebug() << "Nieprawidłowa wartość RPM";
    }
    ui->lineEditTargetRPM->clear();
}

/**
 *
 * Jeśli nie ma aktywnego połączenia, funkcja:
 * - sprawdza, czy wybrano port,
 * - uruchamia komunikację przez SerialReader,
 * - aktualizuje GUI (zielona etykieta, zmiana tekstu przycisku, zapis portu i prędkości).
 *
 * Jeśli port był już połączony, następuje jego rozłączenie przez handlePortDisconnected().
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

/**
 *
 * Czyści zawartość comboBoxSelectPort i dodaje tylko porty,
 * których nazwa zawiera "ttyUSB", "ttyACM" lub "COM".
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
 *
 * Funkcja wykonywana w przypadku:
 * - ręcznego kliknięcia "Rozłącz",
 * - nagłego odłączenia urządzenia.
 *
 * Działania:
 * - zmiana stanu GUI (czerwona etykieta, zmiana tekstu przycisku),
 * - powrót do trybu ręcznego i wyłączenie silnika,
 * - ukrycie elementów trybu automatycznego,
 * - zatrzymanie komunikacji przez serialReader.
 */
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

/**
 *
 * Funkcja odczytuje zawartość pól editKp, editKi oraz editKd.
 * Jeśli dane pole nie jest puste, jego zawartość konwertowana jest na float
 * i przesyłana do ESP32 jako wartość odpowiedniego współczynnika PID.
 * Po wysłaniu zawartość pól zostaje wyczyszczona.
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
 *
 * Przypisuje funkcje do zdarzeń związanych z:
 * - odbiorem danych z ESP32,
 * - błędami i rozłączeniem portu,
 * - zmianą suwaka PWM,
 * - przyciskami GUI (START/STOP, tryb, ustaw RPM, połącz, odśwież porty, zapisz PID).
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
}

/**
 *
 * Funkcja ustawia domyślny stan jako:
 * - brak połączenia (czerwona etykieta),
 * - tryb ręczny aktywny,
 * - ukrycie elementów trybu automatycznego.
 */
void MainWindow::configureInitialMode() {
    ui->label_8->setText("nie połączono");
    ui->label_8->setStyleSheet("color: red; font-weight: bold;");

    ui->lineEditTargetRPM->setVisible(false);
    ui->pushButtonSetRPM->setVisible(false);
    ui->labelSetRPMAuto->setVisible(false);
    ui->labelrpermin->setVisible(false);
    ui->widgetSetrpermin->setVisible(false);
}

/**
 *
 * Wszystkie pola (Kp, Ki, Kd) akceptują tylko dodatnie liczby zmiennoprzecinkowe
 * z maksymalnie 4 miejscami po przecinku.
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
 *
 * Ustawiane są osobne wykresy dla parametrów:
 * - PWM,
 * - RPM,
 * - napięcie,
 * - prąd,
 * - moc.
 *
 * Każdy wykres tworzony jest przez metodę ChartsManager::setupChart()
 * z określonymi parametrami osi Y i tytułem.
 */

void MainWindow::setupCharts() {
    charts->setupChart(ChartType::PWM, ui->widgetPWMGraph->layout(), "PWM", "PWM [%]", 110, 5, false);
    charts->setupChart(ChartType::RPM, ui->widgetRPMGraph->layout(), "RPM", "obr/min", 600, 5, false);
    charts->setupChart(ChartType::Voltage, ui->widgetVoltageGraph->layout(), "Napięcie", "V", 8.5, 5, false);
    charts->setupChart(ChartType::Current, ui->widgetCurrentGraph->layout(), "Prąd", "mA", 800, 5, false);
    charts->setupChart(ChartType::Power, ui->widgetPowerGraph->layout(), "Moc", "mW", 5500, 5, false);

}

/**
 *
 * Tworzone są dwa timery:
 * - updateChartsTimer: odświeża wykresy co 10 ms,
 * - updateGUITimer: aktualizuje pola tekstowe co 500 ms.
 *
 * Timery uruchamiane są natychmiast po konfiguracji.
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
