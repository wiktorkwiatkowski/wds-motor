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
MainWindow::MainWindow(QString portName, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    serialReader(new SerialReader(this)), charts(new ChartsManager(this)){
    ui->setupUi(this);

    // Rozpoczęcie komunikacji z ESP32 przez podany port
    serialReader->start(portName);

    // Połączenie sygnału odebrania danych z funkcją obsługującą aktualizację GUI
    connect(serialReader, &SerialReader::newDataReceived, this, &MainWindow::handleNewSerialData);

    // Obsługa błędów komunikacji szeregowej
    connect(serialReader, &SerialReader::errorOccurred, this, &MainWindow::handleSerialError);

    // Obsługa zmiany wartości suwaka PWM
    connect(ui->SliderPWMManual, &QSlider::valueChanged, this, &MainWindow::on_sliderPWMManual_valueChanged);

    connect(ui->pushButtonStartStop, &QPushButton::clicked, this, &MainWindow::on_buttonStartStop_clicked);
    connect(ui->pushButtonToggleMode, &QPushButton::clicked, this, &MainWindow::on_buttonToggleMode_clicked);
    connect(ui->pushButtonSetRPM, &QPushButton::clicked, this, &MainWindow::on_buttonSetRPM_clicked);

    ui->lineEditTargetRPM->setVisible(false);
    ui->pushButtonSetRPM->setVisible(false);


    // Obsługa przycisku "Zapisz wartości"
    connect(ui->buttonSavePID, &QPushButton::clicked, this, [this]() {

        float kp = ui->editKp->text().toFloat();
        float ki = ui->editKi->text().toFloat();
        float kd = ui->editKd->text().toFloat();

        serialReader->sendData(DataType::Kp, kp);
        serialReader->sendData(DataType::Ki, ki);
        serialReader->sendData(DataType::Kd, kd);

        // Wyczyść pola po zapisaniu
        ui->editKp->clear();
        ui->editKi->clear();
        ui->editKd->clear();
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
    charts->setupChart(ChartType::Power, ui->widgetPowerGraph->layout(), "Moc", "mW", 4800, 5, false);

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
    // Na początku ma być zatrzmany
    serialReader->sendData(start_stop, 0.0f);


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
    ui->pushButtonToggleMode->setText(isManualMode ? "Tryb: Ręczny" : "Tryb: Automatyczny");

    // Pokaż/ukryj elementy odpowiedniego trybu
    ui->SliderPWMManual->setVisible(isManualMode);
    ui->labelPWMManualValue->setVisible(isManualMode);

    ui->lineEditTargetRPM->setVisible(!isManualMode);
    ui->pushButtonSetRPM->setVisible(!isManualMode);
}

void MainWindow::on_buttonSetRPM_clicked() {
    bool ok;
    float rpm = ui->lineEditTargetRPM->text().toFloat(&ok);
    if (ok) {
        serialReader->sendData(DataType::RPM, rpm);
    } else {
        qDebug() << "Nieprawidłowa wartość RPM";
    }
}
