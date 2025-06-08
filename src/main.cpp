/**
 * @file main.cpp
 * @brief Główna funkcja uruchamiająca aplikację sterującą silnikiem.
 *
 * Plik zawiera funkcję main(), od której rozpoczyna się działanie programu.
 *
 * Program realizuje następujący przepływ:
 * - tworzy obiekt QApplication niezbędny do obsługi interfejsu graficznego Qt,
 * - ładuje domyślne tłumaczenie interfejsu (język angielski),
 * - uruchamia główne okno aplikacji (MainWindow), które:
 *   - pozwala użytkownikowi wybrać port szeregowy do komunikacji z mikrokontrolerem,
 *   - umożliwia wizualizację parametrów pracy silnika (RPM, prąd, napięcie, moc, PWM),
 *   - pozwala sterować pracą silnika (Start/Stop, tryb ręczny/automatyczny, PID).
 *
 * Program kończy działanie, gdy użytkownik zamknie główne okno aplikacji.
 *
 * @see MainWindow
 */

#include "../inc/mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QTranslator translator;

    if (translator.load("../../i18n/wds_motor_en_US.qm")) {
        qDebug() << "Translator loaded";
        a.installTranslator(&translator);
    } else {
        qDebug() << "Failed to load translator";
    }

    MainWindow w;
    w.setWindowTitle(QObject::tr("Sterowanie silnikiem"));
    w.show();
    return a.exec();
}
