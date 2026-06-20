#pragma once

#include <chrono>

#include <QObject>
#include <QPointer>
#include <QPropertyAnimation>

#ifdef BUILD_KF6
#include "window.h"
#include "effect/effecthandler.h"
#include "effect/effectwindow.h"
#include "opengl/glshadermanager.h"
#include "opengl/glshader.h"
#include "opengl/gltexture.h"
#include "core/renderviewport.h"
#include "core/pixelgrid.h"
#else
#include <kwineffects.h>
#include <kwinglutils.h>
#endif

#include <SMOD/Decoration/SMODDecoration>

// TODO remove "+ 1.0" when I fix the textures
#define MINMAXGLOW_SML 9.0f
#define MINMAXGLOW_SMT 8.0f
#define CLOSEGLOW_SML  9.0f
#define CLOSEGLOW_SMT  8.0f

namespace KWin
{

enum WindowButtonsDPI
{
    DPI_100_PERCENT,
    DPI_125_PERCENT,
    DPI_150_PERCENT,
    DPI_175_PERCENT
};

class GlowHandler;

class SmodGlowEffect : public Effect
{
    Q_OBJECT

public:
    SmodGlowEffect();
    ~SmodGlowEffect() override;

    void reconfigure(ReconfigureFlags flags) override;
    void prePaintWindow(RenderView *view, EffectWindow *w, WindowPrePaintData &data) override;
#ifdef BUILD_KF6
    void paintWindow(const RenderTarget &renderTarget, const RenderViewport &viewport, EffectWindow *w, int mask, const Region &region, WindowPaintData &data)
        override;
#else
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
#endif
    void postPaintScreen() override;

    static bool supported();

    bool isActive() const override
    {
        return m_active && m_shader;
    }

    int requestedEffectChainPosition() const override
    {
        return 23;
    }

private Q_SLOTS:
    void windowAdded(EffectWindow *w);
    void windowClosed(EffectWindow *w);
    void windowMaximizedStateChanged(EffectWindow *w, bool horizontal, bool vertical);
    void windowMinimized(EffectWindow *w);
    void windowStartUserMovedResized(EffectWindow *w);
    void windowDecorationChanged(EffectWindow *w);
    void effectWindowFullScreenChanged(EffectWindow* w);

private:
    void setupEffectHandlerConnections();
    void setupEffectWindowConnections(const EffectWindow *w);
    void registerWindow(const EffectWindow *w);
    void unregisterWindow(const EffectWindow *w);
    void loadTextures();
    void stopAllAnimations(const EffectWindow *w);
    QString currentlyRegisteredPath;


    bool m_resourcesFound = false;
    bool m_active = false;
    int previousDecorationCount = 0;
    GLTexture *m_preferred_texture;
    std::unique_ptr<GLTexture> m_texture_minimize, m_texture_maximize, m_texture_close;
    std::unique_ptr<GLShader> m_shader;
    QHash<const EffectWindow*, GlowHandler*> windows = QHash<const EffectWindow*, GlowHandler*>();
    Region m_prevPaint = Region();
    QMatrix4x4 colorMatrix(const float &brightness, const float &saturation) const;

    WindowButtonsDPI m_current_dpi = DPI_100_PERCENT, m_next_dpi = DPI_100_PERCENT;
    bool m_needsDpiChange = false;
};

class GlowAnimationHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress);

public:
    GlowAnimationHandler(QObject *parent = nullptr) : QObject(parent), m_hoverProgress(0.0) {};

    void startHoverAnimation(qreal endValue)
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

            QObject::connect(hoverAnimation, &QAbstractAnimation::finished, this, &GlowAnimationHandler::animFinished, Qt::UniqueConnection);
        } else {
            return;
        }

        hoverAnimation->setEasingCurve(QEasingCurve::OutQuad);
        hoverAnimation->setStartValue(m_hoverProgress);
        hoverAnimation->setEndValue(endValue);
        //hoverAnimation->setDuration(0.75 + qRound(100 * qAbs(m_hoverProgress - endValue)));
        hoverAnimation->setDuration( //(int)std::chrono::milliseconds(
#ifdef BUILD_KF6
            (int)(0.75 + qRound(100 * qAbs(m_hoverProgress - endValue))) * effects->animationTimeFactor()
#else
            (int)SmodGlowEffect::animationTime(0.75 + qRound(100 * qAbs(m_hoverProgress - endValue)))
#endif
        //)
        );

        hoverAnimation->start();
        Q_EMIT animStarted();
    }

    void stopHoverAnimation()
    {
        QPropertyAnimation *hoverAnimation = m_hoverAnimation.data();

        if (hoverAnimation) {
            hoverAnimation->stop();
            setHoverProgress(0.0);
            //Q_EMIT animFinished();
        }
    }

    qreal hoverProgress() const
    {
        return m_hoverProgress;
    };

    void setHoverProgress(qreal hoverProgress)
    {
        if (m_hoverProgress != hoverProgress) {
            m_hoverProgress = hoverProgress;
        }
    }
    ~GlowAnimationHandler()
    {
        if (!m_hoverAnimation.isNull()) {
            delete m_hoverAnimation;
        }
    }

    QPointer<QPropertyAnimation> m_hoverAnimation = QPointer<QPropertyAnimation>();
    qreal m_hoverProgress = 0.0;
    QPoint pos = QPoint();

Q_SIGNALS:
    void animStarted();
    void animFinished();
};

class GlowHandler : public QObject
{
    Q_OBJECT

public:
    GlowHandler(QObject *parent = nullptr) : QObject(parent)
    {
        m_captureExclude = new GlowAnimationHandler(this);
        m_menu = new GlowAnimationHandler(this);
        m_pin = new GlowAnimationHandler(this);

        m_shade = new GlowAnimationHandler(this);
        m_underlap  = new GlowAnimationHandler(this);
        m_overlap = new GlowAnimationHandler(this);

        m_help  = new GlowAnimationHandler(this);
        m_min   = new GlowAnimationHandler(this);
        m_max   = new GlowAnimationHandler(this);
        m_close = new GlowAnimationHandler(this);

        QObject::connect(m_captureExclude, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);
        QObject::connect(m_menu, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);
        QObject::connect(m_pin, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);

        QObject::connect(m_shade, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);
        QObject::connect(m_underlap, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);
        QObject::connect(m_overlap, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);

        QObject::connect(m_help, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);
        QObject::connect(m_min, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);
        QObject::connect(m_max, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);
        QObject::connect(m_close, &GlowAnimationHandler::animStarted, this, &GlowHandler::animStarted, Qt::UniqueConnection);

        QObject::connect(m_underlap, &GlowAnimationHandler::animFinished, this, &GlowHandler::animFinished, Qt::UniqueConnection);
        QObject::connect(m_overlap, &GlowAnimationHandler::animFinished, this, &GlowHandler::animFinished, Qt::UniqueConnection);

        QObject::connect(m_help, &GlowAnimationHandler::animFinished, this, &GlowHandler::animFinished, Qt::UniqueConnection);
        QObject::connect(m_min, &GlowAnimationHandler::animFinished, this, &GlowHandler::animFinished, Qt::UniqueConnection);
        QObject::connect(m_max, &GlowAnimationHandler::animFinished, this, &GlowHandler::animFinished, Qt::UniqueConnection);
        QObject::connect(m_close, &GlowAnimationHandler::animFinished, this, &GlowHandler::animFinished, Qt::UniqueConnection);
    };

    ~GlowHandler()
    {
        delete m_captureExclude;
        delete m_menu;
        delete m_pin;

        delete m_shade;
        delete m_underlap;
        delete m_overlap;

        delete m_help;
        delete m_min;
        delete m_max;
        delete m_close;
    }

    void stopAll()
    {
        m_captureExclude->stopHoverAnimation();
        m_menu->stopHoverAnimation();
        m_pin->stopHoverAnimation();

        m_shade->stopHoverAnimation();
        m_underlap->stopHoverAnimation();
        m_overlap->stopHoverAnimation();

        m_help->stopHoverAnimation();
        m_min->stopHoverAnimation();
        m_max->stopHoverAnimation();
        m_close->stopHoverAnimation();
    }

    GlowAnimationHandler *m_captureExclude = nullptr, *m_pin = nullptr, *m_menu = nullptr;
    GlowAnimationHandler *m_shade = nullptr, *m_underlap = nullptr, *m_overlap = nullptr;
    GlowAnimationHandler *m_help = nullptr, *m_min = nullptr, *m_max = nullptr, *m_close = nullptr;

    Rect m_captureExclude_rect{}, m_pin_rect{}, m_menu_rect{};
    Rect m_shade_rect{}, m_underlap_rect{}, m_overlap_rect{};
    Rect m_help_rect{}, m_min_rect{}, m_max_rect{}, m_close_rect{};

    Region m_minimizePaintRegion{};
    Region m_maximizePaintRegion{};
    Region m_closePaintRegion{};

    QMetaObject::Connection m_decoration_connection = QMetaObject::Connection();
    bool m_needsRepaint = false;

public Q_SLOTS:
    void animStarted()
    {
        m_needsRepaint = true;
    }

    void animFinished()
    {
        // TODO redo this
        if ((m_captureExclude->m_hoverProgress == 0.0) //|| m_min->m_hoverProgress == 1.0)
            && (m_menu->m_hoverProgress == 0.0) //|| m_min->m_hoverProgress == 1.0)
            && (m_pin->m_hoverProgress == 0.0) //|| m_min->m_hoverProgress == 1.0)
            && (m_shade->m_hoverProgress == 0.0) //|| m_min->m_hoverProgress == 1.0)
            && (m_underlap->m_hoverProgress == 0.0) //|| m_min->m_hoverProgress == 1.0)
            && (m_overlap->m_hoverProgress == 0.0) //|| m_min->m_hoverProgress == 1.0)
            && (m_help->m_hoverProgress == 0.0) //|| m_min->m_hoverProgress == 1.0)
            && (m_min->m_hoverProgress == 0.0) //|| m_min->m_hoverProgress == 1.0)
            && (m_max->m_hoverProgress == 0.0) //|| m_max->m_hoverProgress == 1.0)
            && (m_close->m_hoverProgress == 0.0) //|| m_close->m_hoverProgress == 1.0)
        ) {
            m_needsRepaint = false;
        }
    }
};



}
