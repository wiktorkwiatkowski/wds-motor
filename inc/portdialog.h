/**
 * @file portdialog.h
 * @brief Deklaracja klasy PortDialog do wyboru portu szeregowego.
 *
 * Klasa reprezentuje proste okno dialogowe z interfejsem Qt, umożliwiające
 * użytkownikowi wybór dostępnego portu USB z listy oraz jego potwierdzenie.
 * Obsługuje także ręczne odświeżanie dostępnych portów.
 */

#ifndef PORTDIALOG_H
#define PORTDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QScreen>

QT_BEGIN_NAMESPACE
namespace Ui {
class PortDialog;
}
QT_END_NAMESPACE

/**
 * @brief Klasa reprezentuje okno dialogowe służące do wyboru portu szeregowego.
 *
 * Umożliwia użytkownikowi wybór dostępnego portu USB z listy oraz odświeżenie
 * tej listy. Po zatwierdzeniu dialogu, wybrany port może być użyty do dalszej
 * komunikacji.
 */
class PortDialog : public QDialog {
    Q_OBJECT

public:
    /**
   * @brief Konstruktor domyślny.
   * @param parent Wskaźnik do obiektu nadrzędnego.
   */
    explicit PortDialog(QWidget *parent = nullptr);

    /**
   * @brief Destruktor.
   */
    ~PortDialog();

    /**
   * @brief Zwraca nazwę wybranego portu.
   * @return Nazwa portu (np. "/dev/ttyUSB0").
   */
    QString selectedPort() const;

private slots:
    /**
   * @brief Slot wywoływany po kliknięciu przycisku "Połącz".
   *
   * Pobiera aktualnie wybrany port i zamyka dialog, jeśli port został poprawnie
   * wybrany.
   */
    void on_pushButtonConnect_clicked();

    /**
   * @brief Slot wywoływany po kliknięciu przycisku "Odśwież".
   *
   * Odświeża listę dostępnych portów szeregowych.
   */
    void on_pushButtonRefresh_clicked();

private:
    /**
   * @brief Odświeża listę dostępnych portów w ComboBoxie.
   */
    void refreshPorts();

    Ui::PortDialog *ui;
    QString portName; ///< Wybrana nazwa portu.
};

#endif // PORTDIALOG_H
