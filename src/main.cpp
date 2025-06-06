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
    // Stworzenie i uruchamienie głównego okna aplikacji
    MainWindow w;
    w.setWindowTitle("Sterowanie silnikiem");
    w.show();
    return a.exec();
}
