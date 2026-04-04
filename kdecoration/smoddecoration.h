/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "sizingmargins.h"
#include "smod.h"
#include "smodsettings.h"

#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/Decoration>
#include <KDecoration3/DecorationSettings>

#include <QByteArray>
#include <QPalette>
#include <QVariant>
#include <QVariantAnimation>

// This is absolutely needed in Qt6
// even though it absolutely wasn't needed in Qt5
// funny
#if defined(MYSHAREDLIB_LIBRARY)
#define MYSHAREDLIB_EXPORT Q_DECL_EXPORT
#else
#define MYSHAREDLIB_EXPORT Q_DECL_IMPORT
#endif

#define PS_EXPLORER QStringLiteral("plasmashell_explorer")
#define PLASMASHELL_WM_X11 QStringLiteral("plasmashell plasmashell")
#define PLASMASHELL_WM_WL QStringLiteral("plasmashell org.kde.plasmashell")
#define SETTINGS_WM QStringLiteral("systemsettings systemsettings")
#define AS_KCM QStringLiteral("aeroshell-personalize")
#define OOTB_WM QStringLiteral("atpootb __ATPOOTB")
#define UAC_WM_X11 QStringLiteral("uac-polkit-agent polkit-kde-authentication-agent-1")
#define UAC_WM_WL QStringLiteral(" org.kde.polkit-kde-authentication-agent-1")
#define AS_KCM_WM QStringLiteral("aeroshell-kcmloader aeroshell-kcmloader")

namespace KDecoration3
{
class DecorationButton;
class DecorationButtonGroup;
}

namespace SMOD
{

class Button;

class MYSHAREDLIB_EXPORT Decoration : public KDecoration3::Decoration
{
    Q_OBJECT

public:
    //* constructor
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    //* destructor
    virtual ~Decoration();

    //* paint
    void paint(QPainter *painter, const QRectF &repaintRegion) override;

    SizingMargins sizingMargins() const;
    InternalSettingsPtr internalSettings() const;

    QString getButtonGroupStr(Button *button) const;

    int titlebarHeight() const;
    int captionHeight() const;
    QColor titleColor(bool active) const;

    static QString themeName();
    static QPixmap close_glow();
    static QPixmap maximize_glow();
    static QPixmap minimize_glow();
    static int decorationCount();
    static bool glowEnabled();

    QRect buttonRect(KDecoration3::DecorationButtonType button) const;

    inline bool isMaximized() const;
    inline bool isMaximizedHorizontally() const;
    inline bool isMaximizedVertically() const;

    inline bool isLeftEdge() const;
    inline bool isRightEdge() const;
    inline bool isTopEdge() const;
    inline bool isBottomEdge() const;

    inline bool hideTitleBar() const;
    inline bool hideIcon() const;
    inline bool hideCaption() const;
    inline bool hideInnerBorder() const;

    inline bool isGadgetExplorer() const;
    inline bool isPersonalizeKCM() const;
    inline bool isPolkit() const;
    inline bool isOOTB() const;

Q_SIGNALS:
    void buttonHoverStatus(KDecoration3::DecorationButtonType button, bool hovered, QPoint pos);

public Q_SLOTS:
    bool init() override;

private Q_SLOTS:
    void reconfigure();
    void recalculateBorders();
    void recalculateTitleBar();
    void recalculateSizes();
    void updateButtonsGeometry();
    void updateButtonsGeometryDelayed();
    void updateBlur();

private:
    void createButtons();

    void paintSideHighlights(QPainter *painter, const QRectF &repaintRegion);
    void paintOuterBorder(QPainter *painter, const QRectF &repaintRegion);
    void paintTitleBar(QPainter *painter, const QRectF &repaintRegion);

    std::shared_ptr<KDecoration3::DecorationShadow> createShadow(bool active);
    void updateShadow(bool reconfigured = false);

    //*@name border size
    //@{
    inline bool hasBorders() const;
    inline bool hasNoBorders() const;
    inline bool hasNoSideBorders() const;
    //@}

    InternalSettingsPtr m_internalSettings;
    KDecoration3::DecorationButtonGroup *m_leftButtons = nullptr;
    KDecoration3::DecorationButtonGroup *m_rightButtons = nullptr;

    //*frame corner radius, scaled according to DPI
    qreal m_scaledCornerRadius = 3;
    QColor m_activeFontColor;
    QColor m_inactiveFontColor;
};

bool Decoration::hasBorders() const
{
    if (m_internalSettings && m_internalSettings->mask() & BorderSize) {
        return m_internalSettings->borderSize() > InternalSettings::BorderNoSides;
    } else {
        return settings()->borderSize() > KDecoration3::BorderSize::NoSides;
    }
}

bool Decoration::hasNoBorders() const
{
    if (m_internalSettings && m_internalSettings->mask() & BorderSize) {
        return m_internalSettings->borderSize() == InternalSettings::BorderNone;
    } else {
        return settings()->borderSize() == KDecoration3::BorderSize::None;
    }
}

bool Decoration::hasNoSideBorders() const
{
    if (m_internalSettings && m_internalSettings->mask() & BorderSize) {
        return m_internalSettings->borderSize() == InternalSettings::BorderNoSides;
    } else {
        return settings()->borderSize() == KDecoration3::BorderSize::NoSides;
    }
}

bool Decoration::isMaximized() const
{
    return window()->isMaximized();
}

bool Decoration::isMaximizedHorizontally() const
{
    return window()->isMaximizedHorizontally();
}

bool Decoration::isMaximizedVertically() const
{
    return window()->isMaximizedVertically();
}

bool Decoration::hideTitleBar() const
{
    return m_internalSettings->hideTitleBar() && !window()->isShaded();
}

bool Decoration::isGadgetExplorer() const
{
    const auto c = window();
    if (c->caption() == PS_EXPLORER && (c->windowClass() == PLASMASHELL_WM_X11 || c->windowClass() == PLASMASHELL_WM_WL))
        return true;
    return false;
}
bool Decoration::isPersonalizeKCM() const
{
    if ((window()->windowClass() == SETTINGS_WM || window()->windowClass() == AS_KCM_WM) && window()->caption().startsWith(AS_KCM))
        return true;
    return false;
}
bool Decoration::isOOTB() const
{
    return window()->windowClass() == OOTB_WM;
}

bool Decoration::isPolkit() const
{
    const auto c = window();
    if ((c->windowClass() == UAC_WM_X11) || c->windowClass() == UAC_WM_WL)
        return true;
    return false;
}
bool Decoration::hideIcon() const
{
    if (isPersonalizeKCM() || isGadgetExplorer() || isPolkit() || isOOTB())
        return true;
    return m_internalSettings->hideIcon() && !window()->isShaded();
}

bool Decoration::hideCaption() const
{
    // Personalization page
    if (isPersonalizeKCM() || isGadgetExplorer() || isOOTB())
        return true;
    return m_internalSettings->hideCaption() && !window()->isShaded();
}

bool Decoration::hideInnerBorder() const
{
    // Personalization page
    if (isPersonalizeKCM() || isGadgetExplorer() || isOOTB())
        return true;
    return m_internalSettings->hideInnerBorder() && !window()->isShaded();
}

}
