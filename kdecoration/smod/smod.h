#pragma once

/*
 * Generic reusable code for SMOD
 */

#include <QStandardPaths>
#include <QResource>
#include <QString>
#include <QFileInfo>

namespace SMOD
{
    const QString DECORATIONS_PATH = "smod/decorations/";
    const QString SMOD_EXTENSION = ".smod.rcc";

    static QString currentlyRegisteredResource = "";
    static QString currentlyRegisteredPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, DECORATIONS_PATH + "Aero" + SMOD_EXTENSION);

    // because we DecorationButton::Type doesn't have a type for a single close button
    // order has to stay the same as DecorationButton::Type
    // our buttons use negative integers
    enum ButtonTypes {
        // main
        CloseLone = -1, // to make converting easier
        Menu = 0,
        ApplicationMenu,
        OnAllDesktops,
        Minimize,
        Maximize,
        Close,
        ContextHelp,
        Shade,
        KeepBelow,
        KeepAbove,
        Custom,
        Spacer,
        ExcludeFromCapture,
    };

    inline void registerResource(const QString &name)
    {
        if(currentlyRegisteredResource != "")
        {
            QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, DECORATIONS_PATH + currentlyRegisteredResource + SMOD_EXTENSION);
            if(!path.isEmpty())
            {
                printf("smod: Unregistering resource %s\n", path.toStdString().c_str());
                QResource::unregisterResource(path);
            }
        }
        printf("smod: Trying to locate SMOD file for %s\n", name.toStdString().c_str());
        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, DECORATIONS_PATH + name + SMOD_EXTENSION);
        if(path.isEmpty())
        {
            path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, DECORATIONS_PATH + "Aero" + SMOD_EXTENSION);
            printf("smod: File not found, fallback to default theme %s\n", path.toStdString().c_str());
        }
        printf("smod: Registering resource %s\n", path.toStdString().c_str());
        QResource::registerResource(path);
        currentlyRegisteredResource = name;
        currentlyRegisteredPath = path;
    }
}
