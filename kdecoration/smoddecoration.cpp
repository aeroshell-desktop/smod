/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2018 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 * SPDX-FileCopyrightText: 2021 Paul McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "smoddecoration.h"

#include "smodbutton.h"
#include "smodsettingsprovider.h"

#include "frametexture.h"

#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationShadow>

#include <KColorUtils>
#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>

K_PLUGIN_FACTORY_WITH_JSON(SMODDecoFactory, "smod.json", registerPlugin<SMOD::Decoration>(); registerPlugin<SMOD::Button>();)

namespace SMOD
{

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;

static SizingMargins g_sizingmargins;
static QString g_themeName = "Aero";
static int g_sDecoCount = 0;
static std::shared_ptr<KDecoration3::DecorationShadow> g_smod_shadow, g_smod_shadow_unfocus;

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
{
    g_sDecoCount++;
}

Decoration::~Decoration()
{
    g_sDecoCount--;
    if (g_sDecoCount == 0) {
        // last deco destroyed, clean up shadow
        g_smod_shadow.reset();
        g_smod_shadow_unfocus.reset();
    }
}

void Decoration::paint(QPainter *painter, const QRectF &repaintRegion)
{
    paintOuterBorder(painter, repaintRegion);
    paintSideHighlights(painter, repaintRegion);
    paintTitleBar(painter, repaintRegion);

    // TODO FIXME: shadow hates being updated before or after painting in some windows
    //             what the flip
    QTimer::singleShot(0, this, [&] {
        updateShadow();
    });
}

SizingMargins Decoration::sizingMargins() const
{
    return g_sizingmargins;
}

InternalSettingsPtr Decoration::internalSettings() const
{
    return m_internalSettings;
}

QString Decoration::getButtonGroupStr(Button *button) const
{
    if (!m_leftButtons || !m_rightButtons) {
        qWarning() << "smod: button groups not initialized (how was this even called), returning...";
        return "";
    }

    if (m_leftButtons->buttons().indexOf(button) != -1)
        return "left";
    else if (m_rightButtons->buttons().indexOf(button) != -1)
        return "right";

    return "";
}

int Decoration::titlebarHeight() const
{
    return internalSettings()->titlebarSize();
}

int Decoration::captionHeight() const
{
    return hideTitleBar() ? borderTop() : borderTop() - settings()->smallSpacing() * 4 - 1;
}

QColor Decoration::titleColor(bool active) const
{
    return active ? m_activeFontColor : m_inactiveFontColor;
}

QString Decoration::themeName()
{
    return SMOD::currentlyRegisteredPath;
}

QPixmap Decoration::close_glow()
{
    return QPixmap(QStringLiteral(":/effects/smodglow/textures/close"));
}

QPixmap Decoration::maximize_glow()
{
    return QPixmap(QStringLiteral(":/effects/smodglow/textures/maximize"));
}

QPixmap Decoration::minimize_glow()
{
    return QPixmap(QStringLiteral(":/effects/smodglow/textures/minimize"));
}

int Decoration::decorationCount()
{
    return g_sDecoCount;
}

bool Decoration::glowEnabled()
{
    if (g_sizingmargins.loaded()) {
        return g_sizingmargins.commonSizing().enable_glow;
    } else {
        return false;
    }
}

QRect Decoration::buttonRect(KDecoration3::DecorationButtonType button) const
{
    int width = 0;
    int intendedWidth = g_sizingmargins.buttonSizingFor(SMOD::Maximize).width;
    int height = titlebarHeight() - 1;

    switch (button) {
    case KDecoration3::DecorationButtonType::Menu:
        width = 16;
        height = titlebarHeight();
        break;
    // TODO: make the spacer size changeable
    case KDecoration3::DecorationButtonType::Spacer:
        width = 8;
        break;

    default:
        intendedWidth = g_sizingmargins.buttonSizingFor((SMOD::ButtonTypes)button).width;
        break;
    }

    if (button != KDecoration3::DecorationButtonType::Menu && button != KDecoration3::DecorationButtonType::Spacer) {
        width = (int)((float)titlebarHeight() * ((float)intendedWidth / 21.0) + 0.5f);
    }

    return QRect(0, 0, width, height);
}

bool Decoration::init()
{
    const auto c = window();

    // use DBus connection to update on SMOD configuration change
    auto dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(),
                 QStringLiteral("/KGlobalSettings"),
                 QStringLiteral("org.kde.KGlobalSettings"),
                 QStringLiteral("notifyChange"),
                 this,
                 SLOT(reconfigure()));

    auto s = settings();
    // a change in font might cause the borders to change
    connect(s.get(), &KDecoration3::DecorationSettings::fontChanged, this, &Decoration::recalculateSizes);

    // buttons
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);

    // full reconfiguration
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::reconfigure);
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, SettingsProvider::self(), &SettingsProvider::reconfigure, Qt::UniqueConnection);
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

    connect(c, &KDecoration3::DecoratedWindow::activeChanged, this, [&] {
        update();
    });

    connect(c, &KDecoration3::DecoratedWindow::captionChanged, this, &Decoration::recalculateSizes);

    connect(c, &KDecoration3::DecoratedWindow::maximizedHorizontallyChanged, this, &Decoration::recalculateSizes);
    connect(c, &KDecoration3::DecoratedWindow::maximizedVerticallyChanged, this, &Decoration::recalculateSizes);
    connect(c, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::recalculateSizes);
    connect(c, &KDecoration3::DecoratedWindow::shadedChanged, this, &Decoration::recalculateSizes);

    connect(c, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::recalculateSizes);
    connect(c, &KDecoration3::DecoratedWindow::heightChanged, this, &Decoration::recalculateSizes);

    reconfigure();
    createButtons();

    return true;
}

void Decoration::reconfigure()
{
    m_internalSettings = SettingsProvider::self()->internalSettings(this);

    SMOD::registerResource(m_internalSettings->decorationTheme());
    g_sizingmargins.loadSizingMargins();

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    const KConfigGroup cg(config, QStringLiteral("KDE"));

    const KConfigGroup wmConfig(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), QStringLiteral("WM"));

    m_activeFontColor = wmConfig.readEntry("activeForeground", QColor(0, 0, 0, 255));
    m_inactiveFontColor = wmConfig.readEntry("inactiveForeground", QColor(20, 19, 18, 255));

    recalculateBorders();
    recalculateTitleBar();
    updateShadow(true);
    updateButtonsGeometryDelayed();
    update();

    // Reload smodglow
    {
        QDBusMessage message = QDBusMessage::createMethodCall("org.kde.KWin", "/Effects", "", "reconfigureEffect");
        QList<QVariant> args;
        args.append("smodglow");
        message.setArguments(args);
        QDBusConnection::sessionBus().send(message);
    }
}

void Decoration::recalculateBorders()
{
    const auto c = window();
    auto s = settings();

    // left, right and bottom borders
    int left = isMaximized() ? 0 : sizingMargins().frameLeftSizing().width;
    int right = isMaximized() ? 0 : sizingMargins().frameRightSizing().width;
    int bottom = (c->isShaded() || isMaximized()) ? 0 : sizingMargins().frameBottomSizing().height;

    // Increase titlebar height if the font is too large for the configured size
    QString testString = "Message Box qd";
    QFontMetrics fm(s->font());
    QRect bounds = fm.boundingRect(testString);
    CommonSizing commonSizing = sizingMargins().commonSizing();

    int topPadding = commonSizing.titlebar_padding_normal;
    if (isMaximized()) {
        topPadding = commonSizing.titlebar_padding_maximized;
    }

    int top = qMax(titlebarHeight(), bounds.height()) + topPadding + 1;
    if (hideTitleBar()) {
        top = bottom;
    }

    {
        // Hide inner borders
        FrameMargins t_m = sizingMargins().topSide();
        FrameMargins l_m = sizingMargins().leftSide();
        FrameMargins r_m = sizingMargins().rightSide();
        FrameMargins b_m = sizingMargins().bottomSide();

        if (hideInnerBorder()) {
            left = left < l_m.margin_right ? 0 : left - l_m.margin_right;
            right = right < r_m.margin_left ? 0 : right - r_m.margin_left;
            top = top < t_m.margin_bottom ? 0 : top - t_m.margin_bottom;
            bottom = bottom < b_m.margin_top ? 0 : bottom - b_m.margin_top;
        }
    }

    left = qMax(0, left);
    right = qMax(0, right);
    top = qMax(0, top);
    bottom = qMax(0, bottom);
    setBorders(QMargins(left, top, right, bottom));

    // extended sizes
    const int extSize = s->largeSpacing();
    int extSides = 0;
    int extBottom = 0;
    if (hasNoBorders()) {
        if (!isMaximizedHorizontally()) {
            extSides = extSize;
        }

        if (!isMaximizedVertically()) {
            extBottom = extSize;
        }
    } else if (hasNoSideBorders() && !isMaximizedHorizontally()) {
        extSides = extSize;
    }

    setResizeOnlyBorders(QMargins(extSides, 0, extSides, extBottom));
}

void Decoration::recalculateTitleBar()
{
    // The titlebar rect has margins around it so the window can be resized by dragging a decoration edge.
    auto s = settings();
    const auto c = window();

    const bool maximized = isMaximized();
    const int pos = maximized ? 0 : s->smallSpacing() * 2;
    const QRect rect(pos, pos, maximized ? c->width() : c->width() - 2 * s->smallSpacing() * 2, maximized ? borderTop() : borderTop() - s->smallSpacing() * 2);

    setTitleBar(rect);
}

void Decoration::recalculateSizes()
{
    recalculateTitleBar();
    recalculateBorders();

    if (m_leftButtons && m_rightButtons) {
        updateButtonsGeometry();
    }

    updateBlur();
    update();
}

void Decoration::updateButtonsGeometry()
{
    const auto s = settings();

    const int vPadding = isMaximized() ? -1 : 1;

    // left buttons positioning
    // TODO: get a 7 VM and look if this is accurate behavior later
    //       my damn Vista VM hates custom msstyles bro
    if (m_leftButtons) {
        m_leftButtons->setSpacing(g_sizingmargins.commonSizing().caption_button_spacing);

        const int startingX = borderLeft() + (hideInnerBorder() ? sizingMargins().leftSide().margin_right : 0);

        if (!g_sizingmargins.commonSizing().caption_button_align_vcenter) {
            m_leftButtons->setPos(QPointF(startingX + (isMaximized() ? 4 : 0) - g_sizingmargins.frameLeftSizing().inset, vPadding));
        } else {
            m_leftButtons->setPos(QPointF(startingX, borderTop() / 2.0f - m_leftButtons->geometry().height() / 2.0f));
        }

        if (!m_leftButtons->buttons().isEmpty()) {
            for (QPointer<KDecoration3::DecorationButton> button : m_leftButtons->buttons()) {
                static_cast<Button *>(button.data())->reconfigure();
            }
        }
    }

    // right buttons positioning
    if (m_rightButtons) {
        m_rightButtons->setSpacing(g_sizingmargins.commonSizing().caption_button_spacing);

        const int startingX =
            size().width() - borderRight() - m_rightButtons->geometry().width() - (hideInnerBorder() ? sizingMargins().rightSide().margin_left : 0);

        if (!g_sizingmargins.commonSizing().caption_button_align_vcenter) {
            m_rightButtons->setPos(QPointF(startingX - (isMaximized() ? 2 : 0) + g_sizingmargins.frameRightSizing().inset, vPadding));
        } else {
            m_rightButtons->setPos(QPointF(startingX, borderTop() / 2.0f - m_rightButtons->geometry().height() / 2.0f));
        }

        if (!m_rightButtons->buttons().isEmpty()) {
            for (QPointer<KDecoration3::DecorationButton> button : m_rightButtons->buttons()) {
                static_cast<Button *>(button.data())->reconfigure();
            }
        }
    }
}

void Decoration::updateButtonsGeometryDelayed()
{
    QTimer::singleShot(0, this, [&] {
        updateButtonsGeometry();
        update();
    });
}

void Decoration::updateBlur()
{
    auto margins = sizingMargins().commonSizing();
    const int radius = isMaximized() ? 0 : margins.corner_radius + 1;

    QPainterPath path;
    path.addRoundedRect(rect(), radius, radius);

    setBlurRegion(QRegion(path.toFillPolygon().toPolygon()));
}

void Decoration::createButtons()
{
    m_leftButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_rightButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Right, this, &Button::create);
    updateButtonsGeometry();
}

void Decoration::paintSideHighlights(QPainter *painter, const QRectF &repaintRegion)
{
    Q_UNUSED(repaintRegion)

    const auto c = window();

    int SIDEBAR_HEIGHT = qMax(25, (int)(size().height() / 4));
    if (internalSettings()->invertTextColor() && isMaximized()) {
        return;
    }

    painter->setClipRegion(blurRegion());
    painter->setClipping(true);

    // TODO: add the ability to keep sidehighlights
    if (!isMaximized() && !hideInnerBorder()) {
        auto margins_left = sizingMargins().frameLeftSizing();
        auto margins_right = sizingMargins().frameRightSizing();
        QPixmap sidehighlight(":/smod/decoration/sidehighlight" + (!c->isActive() ? QString("-unfocus") : QString("")));
        painter->drawPixmap(margins_left.inset, borderTop(), borderLeft() - margins_left.inset - margins_left.inset, SIDEBAR_HEIGHT, sidehighlight);
        painter->drawPixmap(size().width() - borderRight() + margins_right.inset,
                            borderTop(),
                            borderRight() - margins_right.inset - margins_right.inset,
                            SIDEBAR_HEIGHT,
                            sidehighlight);
    }
    painter->setClipping(false);
}

void Decoration::paintOuterBorder(QPainter *painter, const QRectF &repaintRegion)
{
    Q_UNUSED(repaintRegion);
    bool active = window()->isActive();
    QString s_top(":/smod/decoration/top");
    QString s_left(":/smod/decoration/left");
    QString s_right(":/smod/decoration/right");
    QString s_bottom(":/smod/decoration/bottom");

    if (!internalSettings()->enableShadow()) {
        s_top += QString("_noshadow");
        s_bottom += QString("_noshadow");
    }

    if (!active) {
        s_top += QString("_unfocus");
        s_bottom += QString("_unfocus");
        s_left += QString("_unfocus");
        s_right += QString("_unfocus");
    }

    if (hideInnerBorder()) {
        s_top += QString("_noinner");
        s_bottom += QString("_noinner");
        s_left += QString("_noinner");
        s_right += QString("_noinner");
    }

    // Render the top side, which is always visible
    QPixmap p_top(s_top);
    auto t_m = sizingMargins().topSide();
    auto l_m = sizingMargins().leftSide();
    auto r_m = sizingMargins().rightSide();
    auto b_m = sizingMargins().bottomSide();
    auto tl_m = sizingMargins().topLeftCorner();
    auto tr_m = sizingMargins().topRightCorner();

    qreal modBorderLeft = borderLeft() + (hideInnerBorder() ? l_m.margin_right : 0);
    qreal modBorderRight = borderRight() + (hideInnerBorder() ? r_m.margin_left : 0);
    qreal modBorderTop = borderTop() + (hideInnerBorder() ? t_m.margin_bottom : 0);
    qreal modBorderBottom = borderBottom() + (hideInnerBorder() ? b_m.margin_top : 0);

    FrameTexture top(0,
                     0,
                     isMaximized() ? 0 : t_m.margin_top,
                     t_m.margin_bottom,
                     isMaximized() ? (size().width() - borderLeft() - borderRight()) : (size().width() - modBorderLeft - modBorderRight),
                     isMaximized() ? borderTop() : modBorderTop,
                     &p_top,
                     1.0,
                     false,
                     tl_m.width,
                     isMaximized() ? t_m.margin_top : 0,
                     p_top.width() - tl_m.width - tr_m.width,
                     p_top.height() - (isMaximized() ? t_m.margin_top : 0));

    top.translate(isMaximized() ? borderLeft() : modBorderLeft, 0);
    top.render(painter);

    if (!isMaximized()) // Render the rest of the decoration
    {
        QPixmap p_left(s_left);
        QPixmap p_right(s_right);
        QPixmap p_bottom(s_bottom);

        auto bl_m = sizingMargins().bottomLeftCorner();
        auto br_m = sizingMargins().bottomRightCorner();

        // Corners
        FrameTexture topleft(tl_m.margin_left,
                             tl_m.margin_right,
                             tl_m.margin_top,
                             tl_m.margin_bottom,
                             modBorderLeft,
                             modBorderTop,
                             &p_top,
                             1.0,
                             false,
                             0,
                             0,
                             tl_m.width,
                             p_top.height());

        FrameTexture topright(tr_m.margin_left,
                              tr_m.margin_right,
                              tr_m.margin_top,
                              tr_m.margin_bottom,
                              modBorderRight,
                              modBorderTop,
                              &p_top,
                              1.0,
                              false,
                              p_top.width() - tr_m.width,
                              0,
                              tr_m.width,
                              p_top.height());

        FrameTexture bottomleft(bl_m.margin_left,
                                bl_m.margin_right,
                                bl_m.margin_top,
                                bl_m.margin_bottom,
                                modBorderLeft,
                                modBorderBottom,
                                &p_bottom,
                                1.0,
                                false,
                                0,
                                0,
                                bl_m.width,
                                p_bottom.height());

        FrameTexture bottomright(br_m.margin_left,
                                 br_m.margin_right,
                                 br_m.margin_top,
                                 br_m.margin_bottom,
                                 modBorderRight,
                                 modBorderBottom,
                                 &p_bottom,
                                 1.0,
                                 false,
                                 p_bottom.width() - br_m.width,
                                 0,
                                 br_m.width,
                                 p_bottom.height());
        // Sides
        FrameTexture left(l_m.margin_left, l_m.margin_right, 0, 0, modBorderLeft, size().height() - modBorderBottom - modBorderTop, &p_left);

        FrameTexture right(r_m.margin_left, r_m.margin_right, 0, 0, modBorderRight, size().height() - modBorderBottom - modBorderTop, &p_right);

        FrameTexture bottom(0,
                            0,
                            b_m.margin_top,
                            b_m.margin_bottom,
                            size().width() - modBorderLeft - modBorderRight,
                            modBorderBottom,
                            &p_bottom,
                            1.0,
                            false,
                            bl_m.width,
                            0,
                            p_bottom.width() - bl_m.width - br_m.width,
                            p_bottom.height());

        // Move texture fragments to the appropriate locations
        topright.translate(size().width() - modBorderRight, 0);
        bottomleft.translate(0, size().height() - modBorderBottom);
        bottomright.translate(size().width() - modBorderRight, size().height() - modBorderBottom);
        left.translate(0, modBorderTop);
        right.translate(size().width() - modBorderRight, modBorderTop);
        bottom.translate(modBorderLeft, size().height() - modBorderBottom);

        // Render them all
        topleft.render(painter);
        topright.render(painter);
        bottomleft.render(painter);
        bottomright.render(painter);
        left.render(painter);
        right.render(painter);
        bottom.render(painter);
    }
}

void Decoration::paintTitleBar(QPainter *painter, const QRectF &repaintRegion)
{
    if (hideTitleBar()) {
        return;
    }

    if (!hideCaption()) {
        painter->save();

        const auto c = window();
        int titleAlignment = internalSettings()->titleAlignment();
        bool invertText = internalSettings()->invertTextColor() && c->isMaximized();

        // TODO: also test for accurate behavior here
        const int left = (m_leftButtons->geometry().x() + m_leftButtons->geometry().width()) + (hideIcon() ? g_sizingmargins.frameLeftSizing().inset : 0) + 2;
        const int right = m_rightButtons->geometry().left() - (g_sizingmargins.frameRightSizing().inset) + 2;

        QRect captionRect(left, 0, right - left, borderTop() + (hideInnerBorder() ? sizingMargins().topSide().margin_bottom : 0));

        QString caption = settings()->fontMetrics().elidedText(c->caption().remove(QRegularExpression(" —.+")), Qt::ElideMiddle, captionRect.width());

        // replace emojis for █
        // fixes a BUG in which the glow is shorter than the actual text when there's emojis
        QTextOption emojiOpt;
        emojiOpt.setFlags(QTextOption::ShowDefaultIgnorables);
        QFontMetrics fm(settings()->font());
        auto rect =
            fm.boundingRect(caption.replace(QRegularExpression("\\p{Extended_Pictographic}", QRegularExpression::UseUnicodePropertiesOption), "█"), emojiOpt);

        // TODO: force active text color if the theme requests it to match Windows 7 behavior
        QColor textColor = titleColor(c->isActive()); // c->color(KDecoration3::ColorGroup::Active, KDecoration3::ColorRole::Foreground);

        captionRect.setHeight(captionRect.height() - 3);
        painter->setFont(settings()->font());
        painter->setPen(textColor);

        QLabel label(caption);
        QPalette palette = label.palette();

        if (invertText) {
            textColor.setRed(255);
            textColor.setGreen(255);
            textColor.setBlue(255);
        }

        palette.setColor(label.backgroundRole(), textColor);
        palette.setColor(label.foregroundRole(), textColor);
        label.setStyleSheet("QLabel { background: #00aaaaaa; }");
        label.setPalette(palette);

        auto font = settings()->font();
        font.setKerning(false);
        label.setFont(font);

        if (titleAlignment == InternalSettings::AlignRight) {
            label.setAlignment(Qt::AlignRight);
        } else if (titleAlignment == InternalSettings::AlignCenter) {
            label.setAlignment(Qt::AlignHCenter);
        } else if (titleAlignment == InternalSettings::AlignCenterFullWidth) {
            captionRect.setX(0);
            captionRect.setWidth(size().width());
            label.setAlignment(Qt::AlignHCenter);
        }

        label.setFixedWidth(captionRect.width());
        label.setFixedHeight(captionRect.height());

        QPixmap glowPixmap(":/smod/decoration/glow");

        auto glowMargins = sizingMargins().glowSizing();
        int l = glowMargins.margin_left;
        int r = glowMargins.margin_right;
        int t = glowMargins.margin_top;
        int b = glowMargins.margin_bottom;
        qreal opacity = c->isActive() ? glowMargins.active_opacity : glowMargins.inactive_opacity;

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

        int glowWidth = rect.width() + 32;
        int glowHeight = rect.height() * 1.2;

        if (glowWidth < l + r) {
            glowWidth = l + r;
        }

        if (glowHeight < t + b) {
            glowHeight = t + b;
        }

        // only render if the caption is not empty
        if (!caption.trimmed().isEmpty()) {
            FrameTexture glow(l, r, t, b, glowWidth, glowHeight, &glowPixmap, opacity);

            if (!invertText) {
                int x = 0;

                // TODO: rtl support
                switch (titleAlignment) {
                case InternalSettings::AlignLeft:
                    x = captionRect.x() + floor(rect.width() / 2) - floor(glowWidth / 2);
                    break;
                case InternalSettings::AlignRight:
                    // QT-BUG: QRect::right() is off by one
                    x = ((captionRect.x() + captionRect.width()) - floor(rect.width() / 2)) - floor(glowWidth / 2);
                    break;
                case InternalSettings::AlignCenter:
                    x = captionRect.center().x() - glowWidth / 2;
                    break;
                case InternalSettings::AlignCenterFullWidth:
                    x = (size().width() / 2) - (glowWidth / 2);
                    break;
                }

                glow.translate(x, captionRect.center().y() - floor(glowHeight / 2) + 1);
                glow.render(painter);
            }

            QPixmap text_pixmap = label.grab();
            painter->drawPixmap(captionRect, text_pixmap);

            if (invertText) {
                painter->setOpacity(0.7);
                painter->drawPixmap(captionRect, text_pixmap);
                painter->setOpacity(1.0);
            }
        }

        painter->restore();
    }

    if (m_leftButtons) {
        m_leftButtons->paint(painter, repaintRegion);
    }

    if (m_rightButtons) {
        m_rightButtons->paint(painter, repaintRegion);
    }
}

std::shared_ptr<KDecoration3::DecorationShadow> Decoration::createShadow(bool active)
{
    ShadowSizing sizing = sizingMargins().shadowSizing();

    QMargins margins(sizing.margin_left, sizing.margin_top, sizing.margin_right, sizing.margin_bottom);
    QMargins padding(sizing.padding_left, sizing.padding_top, sizing.padding_right, sizing.padding_bottom);

    QImage texture = QImage(active ? ":/smod/decoration/shadow" : ":/smod/decoration/shadow-unfocus");
    QRect innerShadowRect = texture.rect() - margins;

    auto shadow = std::make_shared<KDecoration3::DecorationShadow>();
    shadow->setPadding(padding);
    shadow->setInnerShadowRect(innerShadowRect);
    shadow->setShadow(texture);

    return shadow;
}

void Decoration::updateShadow(bool reconfigured)
{
    if (reconfigured) {
        g_smod_shadow.reset();
        g_smod_shadow_unfocus.reset();
    }

    if (!internalSettings()->enableShadow()) {
        setShadow(std::shared_ptr<KDecoration3::DecorationShadow>(nullptr));
        return;
    }

    if (window()->isActive()) {
        if (!g_smod_shadow) {
            g_smod_shadow = createShadow(true);
        }

        setShadow(g_smod_shadow);
    } else {
        if (!g_smod_shadow_unfocus) {
            g_smod_shadow_unfocus = createShadow(false);
        }

        setShadow(g_smod_shadow_unfocus);
    }
}

} // namespace

#include "smoddecoration.moc"
