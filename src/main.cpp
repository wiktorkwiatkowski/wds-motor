/**
 * @file main.cpp
 * @brief Uruchamia program i pozwala użytkownikowi wybrać port szeregowy.
 *
 * Plik zawiera główną funkcję programu. Na początku wyświetla okno wyboru portu
 * USB, a po zatwierdzeniu uruchamia główne okno aplikacji z wizualizacją danych
 * i kontrolą silnika.
 */

#include "../inc/mainwindow.h"
#include "../inc/portdialog.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStyle>


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

    // // Tworzymy okno dialogowe do wyboru portu
    // PortDialog dialog;
    // dialog.setWindowTitle("Wybór Portu");

    // // Uruchamienie dialog'u i sprawdzenie czy użytkownik kliknął "Połącz"
    // if (dialog.exec() == QDialog::Accepted) {
    //     QMessageBox::information(
    //         nullptr, QObject::tr("Połączenie"),
    //         QObject::tr("Połączono z portem: %1").arg(dialog.selectedPort()));
    // } else {
    //     // Jeśli użytkownik anulował, koniec aplikacji
    //     return 0;
    // }

    // // Pobranie nazwy wybranego portu
    // QString selectedPort = dialog.selectedPort();
    // qDebug() << "Wybrano port: " << selectedPort;

    // Stworzenie i uruchamienie głównego okna aplikacji
    MainWindow w;
    w.setWindowTitle("Sterowanie silnikiem");
    w.show();
    return a.exec();
}
