//////////////////////////////////////////////////////////////////////////////
// breezeconfigurationui.cpp
// -------------------
//
// SPDX-FileCopyrightText: 2009 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// SPDX-License-Identifier: MIT
//////////////////////////////////////////////////////////////////////////////

#include "smodconfigwidget.h"
#include "../smod.h"
#include "smodexceptionlist.h"

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QFontDatabase>
#include <QRegularExpression>
#include <QStandardPaths>

namespace SMOD
{

//_________________________________________________________
ConfigWidget::ConfigWidget(QObject *parent, const KPluginMetaData &data, const QVariantList & /*args*/)
    : KCModule(parent, data)
    , m_configuration(KSharedConfig::openConfig(QStringLiteral("smodrc")))
    , m_changed(false)
{
    // configuration
    m_ui.setupUi(widget());

    // track ui changes
    connect(m_ui.titleAlignment, SIGNAL(currentIndexChanged(int)), SLOT(updateChanged()));
    connect(m_ui.titlebarSize, SIGNAL(valueChanged(int)), SLOT(updateChanged()));
    connect(m_ui.enableShadow, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged);
    connect(m_ui.invertTextColor, &QAbstractButton::clicked, this, &ConfigWidget::updateChanged);

    // track exception changes
    connect(m_ui.exceptions, &ExceptionListWidget::changed, this, &ConfigWidget::updateChanged);
    // set formatting
    // m_ui.drawBorderOnMaximizedWindowsHelpLabel->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    m_themeChanged = false;
    m_ui.themeList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui.themeList->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

//_________________________________________________________
void ConfigWidget::load()
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // assign to ui
    m_ui.titleAlignment->setCurrentIndex(m_internalSettings->titleAlignment());
    m_ui.titlebarSize->setValue(m_internalSettings->titlebarSize());
    m_ui.enableShadow->setChecked(m_internalSettings->enableShadow());
    m_ui.invertTextColor->setChecked(m_internalSettings->invertTextColor());

    // load exceptions
    ExceptionList exceptions;
    exceptions.readConfig(m_configuration);
    m_ui.exceptions->setExceptions(exceptions.get());
    setNeedsSave(false);

    m_ui.hideWidget->setVisible(false);
    // fill list
    QStringList decoration_dirpaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, SMOD::DECORATIONS_PATH, QStandardPaths::LocateDirectory);
    QStringList all_files;

    for (const QString &decoration_dirpath : decoration_dirpaths) {
        QDir decoration_dir(decoration_dirpath);
        all_files += decoration_dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    }

    all_files.removeDuplicates();
    all_files.erase(std::remove_if(all_files.begin(),
                                   all_files.end(),
                                   [](const QString &a) {
                                       return !a.endsWith(".smod.rcc");
                                   }),
                    all_files.end());
    all_files.replaceInStrings(QRegularExpression("\\.smod\\.rcc$"), "");

    QStringListModel *listModel = new QStringListModel(this);
    listModel->setStringList(all_files);
    m_ui.themeList->setModel(listModel);

    connect(m_ui.themeList->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(themeChanged(QModelIndex, QModelIndex)));
    int index = all_files.indexOf(m_internalSettings->decorationTheme());
    QModelIndex m_index = listModel->index(index, 0);
    if (m_index.isValid())
        m_ui.themeList->setCurrentIndex(m_index);
}

//_________________________________________________________
void ConfigWidget::save()
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->load();

    // apply modifications from ui
    m_internalSettings->setTitleAlignment(m_ui.titleAlignment->currentIndex());
    m_internalSettings->setTitlebarSize(m_ui.titlebarSize->value());
    m_internalSettings->setEnableShadow(m_ui.enableShadow->isChecked());
    m_internalSettings->setInvertTextColor(m_ui.invertTextColor->isChecked());

    auto model = m_ui.themeList->model();
    QString theme = model->data(m_ui.themeList->currentIndex(), Qt::DisplayRole).toString();
    m_internalSettings->setDecorationTheme(theme);

    // save configuration
    m_internalSettings->save();

    // get list of exceptions and write
    InternalSettingsList exceptions(m_ui.exceptions->exceptions());
    ExceptionList(exceptions).writeConfig(m_configuration);

    // sync configuration
    m_configuration->sync();
    setNeedsSave(false);

    // needed to tell kwin to reload when running from external kcmshell
    {
        QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(message);
    }

    // needed for SMOD style to reload shadows
    {
        QDBusMessage message(QDBusMessage::createSignal("/BreezeDecoration", "org.kde.Breeze.Style", "reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);
    }
}

//_________________________________________________________
void ConfigWidget::defaults()
{
    // create internal settings and load from rc files
    m_internalSettings = InternalSettingsPtr(new InternalSettings());
    m_internalSettings->setDefaults();

    // assign to ui
    m_ui.titleAlignment->setCurrentIndex(m_internalSettings->titleAlignment());
    m_ui.titlebarSize->setValue(m_internalSettings->titlebarSize());
    m_ui.enableShadow->setChecked(m_internalSettings->enableShadow());
    m_ui.invertTextColor->setChecked(m_internalSettings->invertTextColor());

    auto model = m_ui.themeList->model();
    QModelIndex m_index =
        model->match(model->index(0, 0), Qt::DisplayRole, QVariant::fromValue(m_internalSettings->decorationTheme()), -1, Qt::MatchExactly).at(0);
    if (m_index.isValid()) {
        m_ui.themeList->setCurrentIndex(m_index);
    }
}

void ConfigWidget::themeChanged(QModelIndex index, QModelIndex previous)
{
    if (!m_internalSettings) {
        return;
    }

    if (index != previous && index.row() != -1 && previous.row() != -1) {
        m_themeChanged = true;
        updateChanged();
    }
}
//_______________________________________________
void ConfigWidget::updateChanged()
{
    // check configuration
    if (!m_internalSettings) {
        return;
    }

    // track modifications
    bool modified(false);

    if (m_ui.titleAlignment->currentIndex() != m_internalSettings->titleAlignment()) {
        modified = true;
    } else if (m_ui.titlebarSize->value() != m_internalSettings->titlebarSize()) {
        modified = true;
    } else if (m_ui.enableShadow->isChecked() != m_internalSettings->enableShadow()) {
        modified = true;
    } else if (m_ui.invertTextColor->isChecked() != m_internalSettings->invertTextColor()) {
        modified = true;
    } else if (m_ui.exceptions->isChanged()) {
        modified = true;
    } else if (m_themeChanged) {
        modified = true;
    }

    setNeedsSave(modified);
    m_themeChanged = false;
}

}
