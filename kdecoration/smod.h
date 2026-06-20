#ifndef SMOD_H
#define SMOD_H

/*
 * Generic reusable code for SMOD
 */

#include "smodsettings.h"

#include <QFileInfo>
#include <QHash>
#include <QResource>
#include <QStandardPaths>
#include <QString>

namespace SMOD
{

using InternalSettingsPtr = QSharedPointer<InternalSettings>;
using InternalSettingsList = QList<InternalSettingsPtr>;
using InternalSettingsListIterator = QListIterator<InternalSettingsPtr>;

enum ExceptionMask {
    None = 0,
    BorderSize = 1 << 4,
};

const QString DECORATIONS_PATH = QStringLiteral("smod/decorations/");
const QString SMOD_EXTENSION = QStringLiteral(".smod.rcc");
const QString DEFAULT_THEME = QStringLiteral("Aero");

static QString currentlyRegisteredResource = QStringLiteral();
static QString currentlyRegisteredPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, DECORATIONS_PATH + DEFAULT_THEME + SMOD_EXTENSION);

// Because DecorationButton::Type doesn't have a type for a single close button
// the order has to stay the same as DecorationButton::Type.
// Our buttons use negative integers.
enum ButtonTypes {
    CloseLone = -1,
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

struct ButtonData {
    QString glyphName;
    QString textureName;
};

// TODO: mayhaps make a function to fill this list
//       this looks horrible lmao
static QHash<ButtonTypes, ButtonData> buttonData{{Close, ButtonData{QStringLiteral("close"), QStringLiteral("close")}},
                                                 {CloseLone, ButtonData{QStringLiteral("close"), QStringLiteral("close-single")}},
                                                 {Maximize, ButtonData{QStringLiteral("maximize"), QStringLiteral("maximize")}},
                                                 {Minimize, ButtonData{QStringLiteral("minimize"), QStringLiteral("minimize")}},
                                                 {ContextHelp, ButtonData{QStringLiteral("help"), QStringLiteral("minimize")}},
                                                 {KeepBelow, ButtonData{QStringLiteral("underlap"), QStringLiteral("minimize")}},
                                                 {KeepAbove, ButtonData{QStringLiteral("overlap"), QStringLiteral("minimize")}},
                                                 {Shade, ButtonData{QStringLiteral("shade"), QStringLiteral("minimize")}},
                                                 {OnAllDesktops, ButtonData{QStringLiteral("pin"), QStringLiteral("minimize")}},
                                                 {ApplicationMenu, ButtonData{QStringLiteral("menu"), QStringLiteral("minimize")}},
                                                 {ExcludeFromCapture, ButtonData{QStringLiteral("captureExclude"), QStringLiteral("minimize")}}};

inline void registerResource(const QString &name)
{
    if (currentlyRegisteredResource != QStringLiteral()) {
        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, DECORATIONS_PATH + currentlyRegisteredResource + SMOD_EXTENSION);

        if (!path.isEmpty()) {
            printf("smod: Unregistering resource %s\n", path.toStdString().c_str());
            QResource::unregisterResource(path);
        }
    }

    printf("smod: Trying to locate SMOD file for %s\n", name.toStdString().c_str());

    QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, DECORATIONS_PATH + name + SMOD_EXTENSION);
    if (path.isEmpty()) {
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, DECORATIONS_PATH + DEFAULT_THEME + SMOD_EXTENSION);
        printf("smod: File not found, fallback to default theme %s\n", path.toStdString().c_str());
    }

    printf("smod: Registering resource %s\n", path.toStdString().c_str());
    QResource::registerResource(path);
    currentlyRegisteredResource = name;
    currentlyRegisteredPath = path;
}

}

#endif // SMOD_H
