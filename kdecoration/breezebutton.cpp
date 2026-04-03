/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "breezebutton.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KIconLoader>

#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

namespace Breeze
{

using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;
using KDecoration3::DecorationButtonType;

// real constructor
Button::Button(DecorationButtonType type, Decoration *decoration, QObject *parent)
    : DecorationButton(type, decoration, parent)
    , m_animation(new QVariantAnimation(this))
    , m_hoverProgress(0.0)
{
    // setup animation
    // It is important start and end value are of the same type, hence 0.0 and not just 0
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setOpacity(value.toReal());
    });

    // check if it's for gtk
    if (QCoreApplication::applicationName() == QStringLiteral("kded6")) {
        m_gtkButton = true;
    }

    if (SMOD::buttonData.contains((SMOD::ButtonTypes)type)) {
        m_data = SMOD::buttonData.value((SMOD::ButtonTypes)type);
    }

    // connections
    connect(decoration->window(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
    connect(decoration->settings().get(), &KDecoration3::DecorationSettings::reconfigured, this, &Button::reconfigure);

    connect(this, &Button::buttonHoverStatus, decoration, &Decoration::buttonHoverStatus);

    reconfigure();
}

void Button::updateGeometry()
{
    auto d = qobject_cast<Decoration *>(decoration());

    QRect buttonRect = d->buttonRect(type());

    setGeometry(buttonRect);
    setIconSize(buttonRect.size());
}

// for plugin registration only
Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration *>(), parent)
{
    m_flag = FlagStandalone;
    //! icon size must return to !valid because it was altered from the default constructor,
    //! in Standalone mode the button is not using the decoration metrics but its geometry
    m_iconSize = QSize(-1, -1);
}

//__________________________________________________________________
Button *Button::create(DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
{
    if (auto d = qobject_cast<Decoration *>(decoration)) {
        Button *b = new Button(type, d, parent);
        const auto c = d->window();
        switch (type) {
        case DecorationButtonType::Close:
            b->setVisible(true);
            break;

        case DecorationButtonType::Maximize:
            b->setVisible(c->isMaximizeable() || c->isMinimizeable());
            b->setEnabled(c->isMaximizeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::maximizeableChanged, b, [b](bool maximizeable) {
                auto d = qobject_cast<Decoration *>(b->decoration());
                const auto c = d->window();

                if (!c) {
                    return;
                }

                b->setVisible(c->isMaximizeable() || c->isMinimizeable());
                b->setEnabled(maximizeable);
            });
            break;

        case DecorationButtonType::Minimize:
            b->setVisible(c->isMinimizeable() || c->isMaximizeable());
            b->setEnabled(c->isMinimizeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::minimizeableChanged, b, [b](bool minimizeable) {
                auto d = qobject_cast<Decoration *>(b->decoration());
                const auto c = d->window();

                if (!c) {
                    return;
                }

                b->setVisible(c->isMinimizeable() || c->isMaximizeable());
                b->setEnabled(minimizeable);
            });
            break;

        case DecorationButtonType::ContextHelp:
            b->setVisible(c->providesContextHelp());
            QObject::connect(c, &KDecoration3::DecoratedWindow::providesContextHelpChanged, b, &Breeze::Button::setVisible);
            break;

        case DecorationButtonType::Shade:
            b->setEnabled(c->isShadeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b, &Breeze::Button::setEnabled);
            break;

        case DecorationButtonType::KeepAbove:
            b->setVisible(c->isShadeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b, &Breeze::Button::setEnabled);
            break;

        case DecorationButtonType::Menu:
            b->setVisible(!d->hideIcon());
            QObject::connect(c, &KDecoration3::DecoratedWindow::iconChanged, b, [b] {
                b->update();
            });
            break;

        default:
            break;
        }

        return b;
    }

    return nullptr;
}

void Button::paint(QPainter *painter, const QRectF &repaintRegion)
{
    smodPaint(painter, repaintRegion);
    return;
}

void Button::reconfigure()
{
    auto deco = static_cast<Decoration *>(decoration());
    if (deco && type() == DecorationButtonType::Menu) {
        setVisible(!deco->hideIcon());
    }

    if (isVisible()) {
        updateGeometry();
    }
}

} // namespace

#include "breezebutton.moc"
