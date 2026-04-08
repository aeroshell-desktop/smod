/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#include "smodbutton.h"

#include "frametexture.h"
#include "smod.h"

#include <KColorUtils>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/kdecoration3/decorationdefines.h>
#include <KIconLoader>

#include <QCursor>
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
    if (auto d = qobject_cast<SMOD::Decoration *>(decoration)) {
        Button *b = new Button(type, d, parent);
        const auto c = d->window();

        b->setVisible(true);
        b->setAcceptedButtons(Qt::LeftButton);
        connect(b, &KDecoration3::DecorationButton::visibilityChanged, d, &SMOD::Decoration::requestUpdateButtonPositions);

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

        d->requestUpdateButtonPositions();
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

    painter->save();

    // menu button
    if (type() == KDecoration3::DecorationButtonType::Menu) {
        const auto c = deco->window();
        QSizeF iconSize = geometry().size();
        QRectF iconRect(geometry().topLeft(), iconSize);

        painter->translate(QPointF(0, c->isMaximized() ? 1 : (decoration()->settings()->smallSpacing() * 2) - 1));

        iconRect.translate(0, (titlebarHeight - iconSize.height()) / 2);
        c->icon().paint(painter, iconRect.toRect());

    } else if (type() != KDecoration3::DecorationButtonType::Spacer) {
        QRect g = geometry().toRect();
        qreal w = g.width();
        qreal h = g.height();

        // sizing margins
        int l = m_sizingInfo.margin_left, r = m_sizingInfo.margin_right;
        int t = m_sizingInfo.margin_top, b = m_sizingInfo.margin_bottom;

        // content margins
        int c_l = 0, c_r = 0;
        int c_t = m_sizingInfo.content_top, c_b = m_sizingInfo.content_bottom;

        if (m_isFlipped) {
            c_l = m_sizingInfo.content_right;
            c_r = m_sizingInfo.content_left;
        } else if (m_isMirrored) {
            c_l = c_r = m_sizingInfo.content_left;
        }

        const auto c = decoration()->window();

        // configure painter
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter->translate(g.topLeft());

        QString textureName = m_data.textureName, glyphName = m_data.glyphName;
        QPoint glyphOffset;

        // TODO FIXME: take KDecoration3::DecoratedWindow::scale() into consideration too
        if (titlebarHeight >= 22 && titlebarHeight < 25) {
            m_dpiScale = "@1.25x";
        } else if (titlebarHeight >= 25 && titlebarHeight < 27) {
            m_dpiScale = "@1.5x";
        } else if (titlebarHeight >= 27) {
            m_dpiScale = "@2x";
        }

        // change texture according to position
        if (m_smodType != Close && m_smodType != CloseLone) {
            switch (m_posInList) {
            case Lone:
            case First:
            case Last:
                textureName = "minimize";
                break;
            case Middle:
                textureName = "maximize";
                break;
            }
        }

        // offset the painter if maximized
        // don't wanna offset the group because we'd also be moving the hitbox that way
        if (c->isMaximized()) {
            painter->translate(-2, 0);

            if (m_smodType == SMOD::Maximize) {
                glyphName = "restore";
            }
        }

        // load the textures
        {
            QString t = textureName;
            QString g = glyphName;

            if (!c->isActive()) {
                t += "-unfocus";
            }

            if (!isEnabled()) {
                g += "-inactive";
            }

            if (m_currentTextureName != t || m_currentGlyphName != g || m_posInList != m_prevPos) {
                m_currentTextureName = t;
                m_currentGlyphName = g;
                loadPixmaps();
            }
        }

        // glyph margins i think
        {
            int leftoverW = w - c_l - c_r;
            if (leftoverW < 0) {
                leftoverW = 0;
            }

            int leftoverH = (titlebarHeight - 1) - c_t - c_b;
            if (leftoverH < 0) {
                leftoverH = 0;
            }

            // automatic scaling or smthing
            if (textureName == "maximize") {
                if (titlebarHeight < 19) {
                    l -= (19 - titlebarHeight);
                }
            } else if (textureName == "minimize") {
                if (titlebarHeight < 19) {
                    l--;
                }
            }

            // set glyph offset
            if (type() == KDecoration3::DecorationButtonType::Close) {
                glyphOffset = QPoint(c_l + ceil((leftoverW - m_glyph.width()) / 2.0), c_t + ceil((leftoverH - m_glyph.height()) / 2.0));

            } else if (textureName == "maximize") {
                if (deco && deco->isMaximized()) {
                    glyphOffset = QPoint(c_l + ceil((leftoverW - m_glyph.width()) / 2.0), c_t + ceil((leftoverH - m_glyph.height()) / 2.0));

                } else {
                    glyphOffset = QPoint(c_l + ceil((leftoverW - m_glyph.width()) / 2.0), c_t + ceil((leftoverH - m_glyph.height()) / 2.0));
                }

            } else if (textureName == "minimize") {
                glyphOffset = QPoint(c_l + ceil((leftoverW - m_glyph.width()) / 2.0), c_t + ceil((leftoverH - m_glyph.height()) / 2.0));
            }
        }

        // for animations
        QImage image, hImage, aImage;

        image = m_normal.toImage();
        hImage = m_hover.toImage();
        aImage = m_active.toImage();

        QPixmap final = m_normal;

        // TODO: switch to Borealis::Texture for HiDPI support. Also maybe
        //       to use the msstyles atlas directly too, which will make
        //       doing SMOD themes and the msstyles migration easier
        FrameTexture btn(l, r, t, b, w, h, &final);

        if (!isPressed() && !m_isToggled) {
            image = hoverImage(image, hImage, m_hoverProgress);
            final.convertFromImage(image);
            // render button texture
            btn.render(painter);
            // render glyph
            painter->drawPixmap(glyphOffset.x(), glyphOffset.y(), m_glyph.width(), m_glyph.height(), isHovered() ? m_glyphHover : m_glyph);
        } else {
            final.convertFromImage(aImage);
            btn.render(painter);
            painter->drawPixmap(glyphOffset.x(), glyphOffset.y(), m_glyph.width(), m_glyph.height(), m_glyphActive);
        }
    }

    /*QString posTxt;
    switch (m_posInList) {
    case First:
        posTxt = "first";
        break;
    case Middle:
        posTxt = "middle";
        break;
    case Last:
        posTxt = "last";
        break;
    case Lone:
        posTxt = "lone";
        break;
    }
    painter->drawText(QRectF(QPointF(0, 0), size()), m_currentTextureName, QTextOption());*/

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
            update();
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

Button::Position Button::positionInList()
{
    return m_posInList;
}

void Button::setPositionInList(Position position)
{
    m_posInList = position;
    update();
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

void Button::loadPixmaps()
{
    m_glyph = QPixmap(":/smod/decoration/" + m_currentGlyphName + "-glyph" + m_dpiScale);
    m_glyphHover = QPixmap(":/smod/decoration/" + m_currentGlyphName + "-hover-glyph" + m_dpiScale);
    m_glyphActive = QPixmap(":/smod/decoration/" + m_currentGlyphName + "-active-glyph" + m_dpiScale);

    QList<QPixmap> pixmapsToMod{QPixmap(":/smod/decoration/" + m_currentTextureName + m_dpiScale),
                                QPixmap(":/smod/decoration/" + m_currentTextureName + "-hover" + m_dpiScale),
                                QPixmap(":/smod/decoration/" + m_currentTextureName + "-active" + m_dpiScale)};

    QList<QPixmap> moddedPixmaps;

    // reset
    m_isFlipped = false;
    m_isMirrored = false;

    if (auto deco = static_cast<Decoration *>(decoration()); deco && deco->sizingMargins().commonSizing().group_buttons) {
        for (int i = 0; i < pixmapsToMod.length(); i++) {
            QPixmap pixmap = pixmapsToMod.at(i);
            QImage img;

            if (pixmap.isNull()) {
                continue;
            }

            // modify according to position and texture name
            switch (m_posInList) {
            case Lone: {
                if (m_currentTextureName.contains("minimize")) {
                    img = pixmap.copy(0, 0, round(pixmap.width() / 2), pixmap.height()).toImage();
                    img.flip(Qt::Horizontal);

                    QPixmap mergedPixmap = pixmap;
                    QPainter painter;

                    if (painter.begin(&mergedPixmap)) {
                        QRect rightRect(img.width() + 1, 0, img.width(), img.height());
                        painter.save();
                        painter.setCompositionMode(QPainter::CompositionMode_Clear);
                        painter.eraseRect(rightRect);
                        painter.restore();
                        painter.drawImage(rightRect, img);
                        painter.end();
                        pixmap = mergedPixmap;
                        m_isMirrored = true;
                    } else {
                        qWarning() << "smod: could not mirror minimize button pixmap";
                    }
                }
                break;
            }

            case First: {
                if (m_smodType == SMOD::Close) {
                    img = pixmap.toImage();
                    img.flip(Qt::Horizontal);
                    pixmap.convertFromImage(img);
                    m_isFlipped = true;
                }
                break;
            }

            case Last: {
                if (m_smodType != SMOD::Close) {
                    img = pixmap.toImage();
                    img.flip(Qt::Horizontal);
                    pixmap.convertFromImage(img);
                    m_isFlipped = true;
                }
                break;
            }

            case Middle: {
                if (m_smodType == SMOD::Close) {
                    img = pixmap.copy(0, 0, round(pixmap.width() / 2), pixmap.height()).toImage();
                    img.flip(Qt::Horizontal);

                    QPixmap mergedPixmap = pixmap;
                    QPainter painter;

                    if (painter.begin(&mergedPixmap)) {
                        QRect rightRect(img.width() + 1, 0, img.width(), img.height());
                        painter.save();
                        painter.setCompositionMode(QPainter::CompositionMode_Clear);
                        painter.eraseRect(rightRect);
                        painter.restore();
                        painter.drawImage(rightRect, img);
                        painter.end();
                        pixmap = mergedPixmap;
                        m_isMirrored = true;
                    } else {
                        qWarning() << "smod: could not mirror close button pixmap";
                    }
                }
                break;
            }
            }

            moddedPixmaps.append(pixmap);
        }
    }

    if (moddedPixmaps.isEmpty()) {
        moddedPixmaps = pixmapsToMod;
    }

    if (moddedPixmaps.length() >= 1) {
        m_normal = moddedPixmaps.at(0);
    } else {
        m_normal = pixmapsToMod.at(0);
    }

    if (moddedPixmaps.length() >= 2) {
        m_hover = moddedPixmaps.at(1);
    } else {
        m_hover = pixmapsToMod.at(1);
    }

    if (moddedPixmaps.length() >= 3) {
        m_active = moddedPixmaps.at(2);
    } else {
        m_active = pixmapsToMod.at(2);
    }
}

} // namespace
