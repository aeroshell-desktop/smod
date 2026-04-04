/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "smodbutton.h"

#include "frametexture.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KIconLoader>

#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>

namespace SMOD
{

static QImage hoverImage(const QImage &image, const QImage &hoverImage, qreal hoverProgress)
{
    if (hoverProgress <= 0.5 / 256) {
        return image;
    }

    if (hoverProgress >= 1.0 - 0.5 / 256) {
        return hoverImage;
    }

    QImage result = image;
    QImage over = hoverImage;
    QColor alpha = Qt::black;
    alpha.setAlphaF(hoverProgress);
    QPainter p;
    p.begin(&over);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(image.rect(), alpha);
    p.end();
    p.begin(&result);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.fillRect(image.rect(), alpha);
    p.setCompositionMode(QPainter::CompositionMode_Plus);
    p.drawImage(0, 0, over);
    p.end();

    return result;
}

// for plugin registration only
Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<KDecoration3::DecorationButtonType>(), args.at(1).value<Decoration *>(), parent)
{
}

//__________________________________________________________________
Button *Button::create(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
{
    if (auto d = qobject_cast<Decoration *>(decoration)) {
        Button *b = new Button(type, d, parent);
        const auto c = d->window();

        b->setVisible(true);

        switch (type) {
        case KDecoration3::DecorationButtonType::Close:
            b->setEnabled(c->isCloseable());
            break;
        case KDecoration3::DecorationButtonType::Maximize:
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
                b->update();
            });
            break;
        case KDecoration3::DecorationButtonType::Minimize:
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
                b->update();
            });
            break;
        case KDecoration3::DecorationButtonType::ContextHelp:
            b->setVisible(c->providesContextHelp());
            QObject::connect(c, &KDecoration3::DecoratedWindow::providesContextHelpChanged, b, &SMOD::Button::setVisible);
            break;

        case KDecoration3::DecorationButtonType::Shade:
            b->setEnabled(c->isShadeable());
            QObject::connect(c, &KDecoration3::DecoratedWindow::shadedChanged, b, &SMOD::Button::setToggled);
            QObject::connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b, &SMOD::Button::setEnabled);
            break;
        case KDecoration3::DecorationButtonType::KeepBelow:
            b->setToggled(c->isKeepBelow());
            QObject::connect(c, &KDecoration3::DecoratedWindow::keepBelowChanged, b, &SMOD::Button::setToggled);
            break;
        case KDecoration3::DecorationButtonType::KeepAbove:
            b->setToggled(c->isKeepAbove());
            QObject::connect(c, &KDecoration3::DecoratedWindow::keepAboveChanged, b, &SMOD::Button::setToggled);
            break;

        case KDecoration3::DecorationButtonType::OnAllDesktops:
            b->setToggled(c->isOnAllDesktops());
            QObject::connect(c, &KDecoration3::DecoratedWindow::onAllDesktopsChanged, b, &SMOD::Button::setToggled);
            break;

        case KDecoration3::DecorationButtonType::ApplicationMenu:
            b->setEnabled(c->hasApplicationMenu());
            QObject::connect(c, &KDecoration3::DecoratedWindow::hasApplicationMenuChanged, b, &SMOD::Button::setEnabled);
            break;

        case KDecoration3::DecorationButtonType::ExcludeFromCapture:
            b->setToggled(c->isExcludedFromCapture());
            QObject::connect(c, &KDecoration3::DecoratedWindow::excludeFromCaptureChanged, b, &SMOD::Button::setToggled);
            break;

        case KDecoration3::DecorationButtonType::Menu:
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
    Q_UNUSED(repaintRegion)

    if (!decoration()) {
        return;
    }

    painter->save();
    auto deco = qobject_cast<Decoration *>(decoration());
    int titlebarHeight = deco->titlebarHeight();

    // QRect g = geometry().toRect();
    // if (deco->window()){
    //     qreal scale = deco->window()->scale();
    //
    //     qDebug() << scale;
    //
    //     QRect scaledTargetRect = g;
    //     scaledTargetRect.setWidth(scaledTargetRect.width() * scale);
    //     scaledTargetRect.setHeight(scaledTargetRect.height() * scale);
    //     scaledTargetRect.setX(scaledTargetRect.x() * scale);
    //     scaledTargetRect.setY(scaledTargetRect.y() * scale);
    //
    //     g = scaledTargetRect;
    //
    //     qreal scaleFactor = 1.0/scale;
    //     painter->scale(scaleFactor, scaleFactor);
    // }

    // menu button
    if (type() == KDecoration3::DecorationButtonType::Menu) {
        const auto c = deco->window();
        QSizeF iconSize = geometry().size();
        QRectF iconRect(geometry().topLeft(), iconSize);

        const int vPadding = c->isMaximized() ? 1 : (decoration()->settings()->smallSpacing() * 2) - 1;
        const int hPadding = c->isMaximized() ? -2 : 0;

        painter->translate(QPointF(hPadding, vPadding));

        iconRect.translate(0, (titlebarHeight - iconSize.height()) / 2);
        c->icon().paint(painter, iconRect.toRect());

    } else if (type() != KDecoration3::DecorationButtonType::Spacer) {
        QRect g = geometry().toRect();
        qreal w = g.width();
        qreal h = g.height();

        // sizing margins
        int l = m_sizingInfo.margin_left, t = m_sizingInfo.margin_top;
        int r = m_sizingInfo.margin_right, b = m_sizingInfo.margin_bottom;

        // content margins
        int c_l = m_sizingInfo.content_left, c_t = m_sizingInfo.content_top;
        int c_r = m_sizingInfo.content_right, c_b = m_sizingInfo.content_bottom;

        const auto c = decoration()->window();

        painter->translate(g.topLeft());

        // pswin was here
        QPixmap glyph, glyphHover, glyphActive;
        QPixmap normal, hover, active;

        QImage normalImg, hoverImg, activeImg;
        QPoint glyphOffset;
        QString glyphType = m_data.glyphName, dpiScale = "";
        QString textureName = m_data.textureName;

        if (titlebarHeight >= 22 && titlebarHeight < 25) {
            dpiScale = "@1.25x";
        } else if (titlebarHeight >= 25 && titlebarHeight < 27) {
            dpiScale = "@1.5x";
        } else if (titlebarHeight >= 27) {
            dpiScale = "@2x";
        }

        int leftoverW = 0;
        int leftoverH = 0;

        // load the textures normally first
        {
            if (!c->isActive()) {
                textureName += "-unfocus";
            }

            normal = QPixmap(":/smod/decoration/" + textureName + dpiScale);
            hover = QPixmap(":/smod/decoration/" + textureName + "-hover" + dpiScale);
            active = QPixmap(":/smod/decoration/" + textureName + "-active" + dpiScale);
        }

        leftoverW = w - c_l - c_r;
        if (leftoverW < 0) {
            leftoverW = 0;
        }

        leftoverH = (titlebarHeight - 1) - c_t - c_b;
        if (leftoverH < 0) {
            leftoverH = 0;
        }

        if (textureName == "maximize") {
            if (titlebarHeight == 18) {
                l--;
            } else if (titlebarHeight == 17) {
                l -= 2;
            }
        } else if (textureName == "minimize") {
            if (titlebarHeight == 18 || titlebarHeight == 17) {
                l--;
            }
        }

        {
            if (!isEnabled()) {
                glyphType += "-inactive";
            }

            glyph = QPixmap(":/smod/decoration/" + glyphType + "-glyph" + dpiScale);
            glyphHover = QPixmap(":/smod/decoration/" + glyphType + "-hover-glyph" + dpiScale);
            glyphActive = QPixmap(":/smod/decoration/" + glyphType + "-active-glyph" + dpiScale);
        }

        switch (type()) {
        case KDecoration3::DecorationButtonType::Maximize:
            if (deco && deco->isMaximized()) {
                glyphOffset = QPoint(c_l + ceil((leftoverW - glyph.width()) / 2.0), c_t + ceil((leftoverH - glyph.height()) / 2.0));

            } else {
                glyphOffset = QPoint(c_l + ceil((leftoverW - glyph.width()) / 2.0), c_t + ceil((leftoverH - glyph.height()) / 2.0));
            }
            break;

        case KDecoration3::DecorationButtonType::Close:
            glyphOffset = QPoint(c_l + ceil((leftoverW - glyph.width()) / 2.0), c_t + ceil((leftoverH - glyph.height()) / 2.0));
            break;

        default:
            glyphOffset = QPoint(c_l + ceil((leftoverW - glyph.width()) / 2.0), c_t + ceil((leftoverH - glyph.height()) / 2.0));
            break;
        }

        QImage image, hImage, aImage;

        image = normal.toImage();
        hImage = hover.toImage();
        aImage = active.toImage();

        FrameTexture btn(l, r, t, b, w, h, &normal);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        if (!isPressed() && !m_isToggled) {
            image = hoverImage(image, hImage, m_hoverProgress);
            normal.convertFromImage(image);
            // render button texture
            btn.render(painter);
            // render glyph
            painter->drawPixmap(glyphOffset.x(), glyphOffset.y(), glyph.width(), glyph.height(), isHovered() ? glyphHover : glyph);
        } else {
            normal.convertFromImage(aImage);
            btn.render(painter);
            painter->drawPixmap(glyphOffset.x(), glyphOffset.y(), glyph.width(), glyph.height(), glyphActive);
        }
    }

    painter->restore();

    return;
}

qreal Button::hoverProgress() const
{
    return m_hoverProgress;
}

void Button::setHoverProgress(qreal hoverProgress)
{
    if (m_hoverProgress != hoverProgress) {
        m_hoverProgress = hoverProgress;

        if (qobject_cast<Decoration *>(decoration())) {
            update(geometry().adjusted(-32, -32, 32, 32));
        }
    }
}

bool Button::isToggled() const
{
    return m_isToggled;
}

void Button::setToggled(bool toggled)
{
    m_isToggled = toggled;
}

void Button::updateGeometry()
{
    auto d = qobject_cast<Decoration *>(decoration());
    QRect buttonRect = d->buttonRect(type());
    setGeometry(buttonRect);
}

void Button::reconfigure()
{
    Decoration *deco = qobject_cast<Decoration *>(decoration());
    if (deco && type() == KDecoration3::DecorationButtonType::Menu) {
        setVisible(!deco->hideIcon());
    }

    if (isVisible()) {
        updateGeometry();
    }

    m_smodType = (SMOD::ButtonTypes)type();
    if (SMOD::buttonData.contains(m_smodType)) {
        KDecoration3::DecoratedWindow *window = nullptr;

        if (deco) {
            window = deco->window();
        }

        if (m_smodType == SMOD::Close && window && !(window->isMinimizeable() || window->isMaximizeable() || window->providesContextHelp())) {
            m_data = SMOD::buttonData.value(SMOD::CloseLone);
        } else {
            m_data = SMOD::buttonData.value(m_smodType);
        }
    }

    if (deco) {
        m_sizingInfo = deco->sizingMargins().buttonSizingFor(m_smodType);
    }
}

void Button::hoverEnterEvent(QHoverEvent *event)
{
    KDecoration3::DecorationButton::hoverEnterEvent(event);

    if (isHovered()) {
        Q_EMIT buttonHoverStatus(type(), true, geometry().topLeft().toPoint());
        startHoverAnimation(1.0);
    }
}

void Button::hoverLeaveEvent(QHoverEvent *event)
{
    KDecoration3::DecorationButton::hoverLeaveEvent(event);

    if (!isHovered()) {
        Q_EMIT buttonHoverStatus(type(), false, geometry().topLeft().toPoint());
        startHoverAnimation(0.0);
    }
}

// real constructor
Button::Button(KDecoration3::DecorationButtonType type, Decoration *decoration, QObject *parent)
    : DecorationButton(type, decoration, parent)
    , m_hoverProgress(0.0)
{
    // check if it's for gtk
    /*if (QCoreApplication::applicationName() == QStringLiteral("kded6")) {
        m_gtkButton = true;
    }*/

    // connections
    connect(decoration->window(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
    connect(decoration->settings().get(), &KDecoration3::DecorationSettings::reconfigured, this, &Button::reconfigure);

    connect(this, &Button::buttonHoverStatus, decoration, &Decoration::buttonHoverStatus);

    reconfigure();
}

void Button::startHoverAnimation(qreal endValue)
{
    QPropertyAnimation *hoverAnimation = m_hoverAnimation.data();

    if (hoverAnimation) {
        if (hoverAnimation->endValue() == endValue) {
            return;
        }

        hoverAnimation->stop();
    } else if (m_hoverProgress != endValue) {
        hoverAnimation = new QPropertyAnimation(this, "hoverProgress");
        m_hoverAnimation = hoverAnimation;
    } else {
        return;
    }

    hoverAnimation->setEasingCurve(QEasingCurve::OutQuad);
    hoverAnimation->setStartValue(m_hoverProgress);
    hoverAnimation->setEndValue(endValue);
    hoverAnimation->setDuration(1 + qRound(200 * qAbs(m_hoverProgress - endValue)));
    hoverAnimation->start();
}

} // namespace
