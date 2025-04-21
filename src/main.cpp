#include "../inc/mainwindow.h"
#include "../inc/portdialog.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
     QApplication a(argc, argv);
    // QTranslator translator;
    // const QStringList uiLanguages = QLocale::system().uiLanguages();
    // for (const QString &locale : uiLanguages) {
    //     const QString baseName = "wds_motor_" + QLocale(locale).name();
    //     if (translator.load(":/i18n/" + baseName)) {
    //         a.installTranslator(&translator);
    //         break;
    //     }
    // }
    // Tworzymy okno dialogowe do wyboru portu
     PortDialog dialog;
     dialog.setWindowTitle("Wybór Portu");

     // Uruchamiamy dialog
     if (dialog.exec() == QDialog::Accepted) {
         QMessageBox::information( nullptr, QObject::tr("Połączenie"), QObject::tr("Połączono z portem: %1").arg(dialog.selectedPort()));
    } else {
        return 0;
    }

    // Sprawdzamy wybrany port
    QString selectedPort = dialog.selectedPort();
    qDebug() << "Wybrano port: " << selectedPort;
    MainWindow w(dialog.selectedPort());
    w.show();
    return a.exec();
}
