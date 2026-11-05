#include <QCoreApplication>
#include <QCommandLineParser>
#include <QProcess>
#include <iostream>
#include "tagengine.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("ktag");
    app.setApplicationVersion("1.0.0");

    const QStringList args = app.arguments();
    if (args.size() < 2) {
        std::cout << "ktag: macOS Finder Color Tags for Dolphin & KDE Plasma" << std::endl;
        std::cout << "Uso:" << std::endl;
        std::cout << "  ktag set <color> <archivos...>" << std::endl;
        std::cout << "  ktag remove <archivos...>" << std::endl;
        std::cout << "  ktag get <archivo>" << std::endl;
        std::cout << "  ktag setup-places" << std::endl;
        std::cout << "  ktag clean" << std::endl;
        std::cout << "\nColores disponibles: red, orange, yellow, green, blue, purple, gray" << std::endl;
        return 0;
    }

    const QString cmd = args.at(1).toLower();

    if (cmd == "setup-places") {
        if (TagEngine::setupPlaces()) {
            std::cout << "Etiquetas añadidas correctamente a la barra de Lugares de Dolphin." << std::endl;
            return 0;
        } else {
            std::cerr << "Error al configurar lugares de Dolphin." << std::endl;
            return 1;
        }
    } else if (cmd == "clean") {
        int cleaned = TagEngine::cleanBrokenTags();
        std::cout << "Enlaces rotos eliminados: " << cleaned << std::endl;
        return 0;
    } else if (cmd == "set") {
        if (args.size() < 4) {
            std::cerr << "Uso: ktag set <color> <archivos...>" << std::endl;
            return 1;
        }
        const QString color = args.at(2);
        const auto *tag = TagEngine::findTag(color);
        if (!tag) {
            std::cerr << "Color no válido: " << qPrintable(color) << std::endl;
            return 1;
        }

        int count = 0;
        for (int i = 3; i < args.size(); ++i) {
            if (TagEngine::tagFile(args.at(i), color)) {
                count++;
            }
        }

        if (count > 0) {
            // Optional feedback notification
            QString msg = (count == 1) ? QString("1 archivo etiquetado como %1").arg(tag->folderName)
                                       : QString("%1 archivos etiquetados como %2").arg(count).arg(tag->folderName);
            QProcess::startDetached("notify-send", {"-a", "Dolphin", "-i", "tag", "Etiquetas de color", msg});
        }
        return 0;
    } else if (cmd == "remove") {
        if (args.size() < 3) {
            std::cerr << "Uso: ktag remove <archivos...>" << std::endl;
            return 1;
        }
        int count = 0;
        for (int i = 2; i < args.size(); ++i) {
            if (TagEngine::removeTags(args.at(i))) {
                count++;
            }
        }
        if (count > 0) {
            QProcess::startDetached("notify-send", {"-a", "Dolphin", "-i", "tag", "Etiquetas de color", "Etiquetas eliminadas"});
        }
        return 0;
    } else if (cmd == "get") {
        if (args.size() < 3) {
            std::cerr << "Uso: ktag get <archivo>" << std::endl;
            return 1;
        }
        const QString tag = TagEngine::getFileTag(args.at(2));
        if (!tag.isEmpty()) {
            std::cout << qPrintable(tag) << std::endl;
        } else {
            std::cout << "none" << std::endl;
        }
        return 0;
    }

    std::cerr << "Comando desconocido: " << qPrintable(cmd) << std::endl;
    return 1;
}
