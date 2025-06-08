/**
 * @file main.cpp
 * @brief Uruchamia program i pozwala użytkownikowi wybrać port szeregowy.
 *
 * Plik zawiera główną funkcję programu. Na początku wyświetla okno wyboru portu
 * USB, a po zatwierdzeniu uruchamia główne okno aplikacji z wizualizacją danych
 * i kontrolą silnika.
 */

#include "../inc/mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QStyle>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>



/**
 * @brief Główna funkcja programu.
 *
 * Pokazuje okno dialogowe do wyboru portu COM, a następnie otwiera główne okno
 * aplikacji. Jeśli użytkownik anuluje wybór portu, program kończy działanie.
 *
 * @param argc Liczba argumentów przekazanych do programu.
 * @param argv Argumenty przekazane z linii poleceń.
 * @return Kod zakończenia programu (0 = OK).
 */
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QTranslator translator;

    if (translator.load("../../i18n/wds_motor_en_US.qm")) {
        qDebug() << "Translator loaded";
        a.installTranslator(&translator);
    } else {
        qDebug() << "Failed to load translator";
    }

    // Stworzenie i uruchamienie głównego okna aplikacji
    MainWindow w;
    w.setWindowTitle(QObject::tr("Sterowanie silnikiem"));
    w.show();
    return a.exec();
}
