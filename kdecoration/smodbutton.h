/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "sizingmargins.h"
#include "smod.h"

#include "smoddecoration.h"

#include <KDecoration3/DecorationButton>

#include <QHash>
#include <QHoverEvent>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QPropertyAnimation>

class QVariantAnimation;

namespace SMOD
{

class Button : public KDecoration3::DecorationButton
{
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress);

public:
    explicit Button(QObject *parent, const QVariantList &args);
    virtual ~Button() = default;

    //* button creation
    static Button *create(KDecoration3::DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent);

    //* render
    virtual void paint(QPainter *painter, const QRectF &repaintRegion) override;

    // if there is a button next to this button or nah
    enum PositionInfo {
        ImNothingLikeYall,
        LeftSide,
        RightSide,
        BothSides
    };

    qreal hoverProgress() const;
    void setHoverProgress(qreal hoverProgress);

    bool isToggled() const;
    void setToggled(bool toggled);

    void updateGeometry();
    void reconfigure();

Q_SIGNALS:
    void buttonHoverStatus(KDecoration3::DecorationButtonType button, bool hovered, QPoint pos);

protected:
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;

private:
    //* private constructor
    explicit Button(KDecoration3::DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

    void startHoverAnimation(qreal endValue);

    bool m_isToggled = false;

    PositionInfo posInfo = ImNothingLikeYall;
    SMOD::ButtonData m_data;
    SMOD::ButtonTypes m_smodType = SMOD::Custom;
    ButtonSizingMargins m_sizingInfo;

    // animation
    QPointer<QPropertyAnimation> m_hoverAnimation;
    qreal m_hoverProgress;
};

} // namespace
