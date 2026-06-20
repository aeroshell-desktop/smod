/*
 * based on Kwin Feedback effect
 * based on StartupId in KRunner by Lubos Lunak
 *
 * SPDX-FileCopyrightText: 2001 Lubos Lunak <l.lunak@kde.org>
 * SPDX-FileCopyrightText: 2010 Martin Gräßlin <mgraesslin@kde.org>
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "smodglow.h"

#include <KConfig>
#include <KConfigGroup>


#define TESTING_NEW_DPI 0
#define RIGHT_SIDE_ORIGIN 0


// internally the SMOD decoration is in the Breeze namespace
// use a typedef to avoid confusion


//Q_LOGGING_CATEGORY(KWIN_EFFECT_SMODWINDOWBUTTONS, "kwin.effect.smodglow", QtWarningMsg)

static void ensureResources()
{
    Q_INIT_RESOURCE(smodglow);
}

namespace KWin
{

SmodGlowEffect::SmodGlowEffect()
{
    setupEffectHandlerConnections();

    reconfigure(ReconfigureAll);

    // NOTE is this needed?
    //effects->makeOpenGLContextCurrent();

    m_shader = ShaderManager::instance()->generateShaderFromFile(
        ShaderTrait::MapTexture,
        QString(),
        QStringLiteral(":/effects/smodglow/shaders/shader.frag")
    );
    /*m_shader = ShaderManager::instance()->generateCustomShader(
        ShaderTrait::MapTexture,
        QByteArray(),
        SmodDecoration::glow_shader()
    );*/
}

SmodGlowEffect::~SmodGlowEffect()
{
}

bool SmodGlowEffect::supported()
{
    return effects->isOpenGLCompositing();
}



void SmodGlowEffect::reconfigure(Effect::ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    ensureResources();


    /*m_active = SMOD::registerResource(SmodDecoration::themeName(), currentlyRegisteredPath);
    currentlyRegisteredPath = SmodDecoration::themeName();*/

    loadTextures();

    if (!isActive())
    {
        qDebug() << "kwin_effect_smodglow: SMOD RCC \"smodgloweffecttextures\" not found!";

        return;
    }


    const auto windowlist = effects->stackingOrder();

    for (EffectWindow *window : windowlist)
    {
        unregisterWindow(window);
        registerWindow(window);
    }
}

void SmodGlowEffect::setupEffectHandlerConnections()
{
    connect(effects, &EffectsHandler::windowAdded, this, &SmodGlowEffect::windowAdded, Qt::UniqueConnection);
    connect(effects, &EffectsHandler::windowClosed, this, &SmodGlowEffect::windowClosed, Qt::UniqueConnection);
#ifndef BUILD_KF6
    connect(effects, &EffectsHandler::windowMaximizedStateChanged, this, &SmodGlowEffect::windowMaximizedStateChanged, Qt::UniqueConnection);
    connect(effects, &EffectsHandler::windowMinimized, this, &SmodGlowEffect::windowMinimized, Qt::UniqueConnection);
    connect(effects, &EffectsHandler::windowStartUserMovedResized, this, &SmodGlowEffect::windowStartUserMovedResized, Qt::UniqueConnection);
    connect(effects, &EffectsHandler::windowFullScreenChanged, this, &SmodGlowEffect::effectWindowFullScreenChanged, Qt::UniqueConnection);
    connect(effects, &EffectsHandler::windowDecorationChanged, this, &SmodGlowEffect::windowDecorationChanged, Qt::UniqueConnection);
#endif
}

void SmodGlowEffect::setupEffectWindowConnections(const EffectWindow *w)
{
#ifdef BUILD_KF6
    connect(w, &EffectWindow::windowMaximizedStateChanged, this, &SmodGlowEffect::windowMaximizedStateChanged, Qt::UniqueConnection);
    connect(w, &EffectWindow::minimizedChanged, this, &SmodGlowEffect::windowMinimized, Qt::UniqueConnection);
    connect(w, &EffectWindow::windowStartUserMovedResized, this, &SmodGlowEffect::windowStartUserMovedResized, Qt::UniqueConnection);
    connect(w, &EffectWindow::windowFullScreenChanged, this, &SmodGlowEffect::effectWindowFullScreenChanged, Qt::UniqueConnection);
    connect(w, &EffectWindow::windowDecorationChanged, this, &SmodGlowEffect::windowDecorationChanged, Qt::UniqueConnection);
#endif
}

void SmodGlowEffect::registerWindow(const EffectWindow *w)
{
    if (!w || windows.contains(w) || !w->hasDecoration()) {
        return;
    }

    // In order to access our custom signal we need to cast to the correct class
    SMOD::Decoration *smoddecoration = qobject_cast<SMOD::Decoration *>(w->decoration());

    // if the cast was unsuccessful (the loaded decoration plugin is not SMOD) then return
    if (!smoddecoration) {
        return;
    }

    // Attempt to connect to the decoration signal.

#if TESTING_NEW_DPI
    auto connection = connect(smoddecoration,
                              &SMOD::Decoration::buttonHoveredChanged,
                              this,
                              [w, this](KDecoration3::DecorationButtonType button, bool hovered, QPoint pos, int dpi) {
#else
    auto connection =
        connect(smoddecoration, &SMOD::Decoration::buttonHoverStatus, this, [w, this](KDecoration3::DecorationButtonType button, bool hovered, QPoint pos) {
            int dpi = m_current_dpi;
#endif
                                  GlowAnimationHandler *anim;

                                  switch (button) {
                                  case KDecoration3::DecorationButtonType::ExcludeFromCapture: {
                                      anim = this->windows.value(w)->m_captureExclude;

#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::ApplicationMenu: {
                                      anim = this->windows.value(w)->m_menu;

#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::OnAllDesktops: {
                                      anim = this->windows.value(w)->m_pin;

#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::Shade: {
                                      anim = this->windows.value(w)->m_shade;

#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::KeepBelow: {
                                      anim = this->windows.value(w)->m_underlap;

#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::KeepAbove: {
                                      anim = this->windows.value(w)->m_overlap;

#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::ContextHelp: {
                                      anim = this->windows.value(w)->m_help;

#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::Minimize: {
                                      anim = this->windows.value(w)->m_min;

#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::Maximize: {
                                      anim = this->windows.value(w)->m_max;
#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT));
#else
                anim->pos = pos - QPoint(MINMAXGLOW_SML, MINMAXGLOW_SMT);
#endif

                                      break;
                                  }
                                  case KDecoration3::DecorationButtonType::Close: {
                                      anim = this->windows.value(w)->m_close;
#if RIGHT_SIDE_ORIGIN
                                      anim->pos = -(pos + QPoint(CLOSEGLOW_SML, CLOSEGLOW_SMT));
#else
                anim->pos = pos - QPoint(CLOSEGLOW_SML, CLOSEGLOW_SMT);
#endif

                                      break;
                                  }
                                  default: {
                                      return;
                                  }
                                  }

                                  if (dpi != m_current_dpi) {
                                      m_next_dpi = (WindowButtonsDPI)dpi;
                                      m_needsDpiChange = true;
                                  }

                                  anim->startHoverAnimation(hovered ? 1.0 : 0.0);
                              });

    if (!connection) {
        return;
    }

    auto glowhandler = new GlowHandler(this);
    glowhandler->m_decoration_connection = connection;
    windows.insert(w, glowhandler);

    setupEffectWindowConnections(w);
}

void SmodGlowEffect::unregisterWindow(const EffectWindow *w)
{
    if (windows.contains(w)) {
        windows.value(w)->stopAll();
        disconnect(windows.value(w)->m_decoration_connection);
        delete windows.value(w);
        windows.remove(w);
    }
}

void SmodGlowEffect::stopAllAnimations(const EffectWindow *w)
{
    if (windows.contains(w)) {
        windows.value(w)->stopAll();
    }
}

void SmodGlowEffect::prePaintWindow(RenderView *view, EffectWindow *w, WindowPrePaintData &data)
{
    effects->prePaintWindow(view, w, data);

    if (w->isUserResize()) {
        stopAllAnimations(w);
        return;
    }
    if (!windows.contains(w)) {
        return;
    }

    if (m_needsDpiChange) {
        loadTextures();
    }

    GlowHandler *handler = windows.value(w);

    if (!handler->m_needsRepaint) {
        return;
    }
    SMOD::Decoration *smoddecoration = qobject_cast<SMOD::Decoration *>(w->decoration());

    // if the cast was unsuccessful (the loaded decoration plugin is not SMOD) then return
    if (!smoddecoration) {
        return;
    }

#if RIGHT_SIDE_ORIGIN
    QPoint origin = w->frameGeometry().topLeft().toPoint() + QPoint(w->frameGeometry().width(), 0);
#else
    QPoint origin = w->pos().toPoint();
#endif

    /*qDebug() << "Min texture: " << m_texture_minimize.get()->size();
    qDebug() << "Max texture: " << m_texture_maximize.get()->size();
    qDebug() << "Close texture: " << m_texture_close.get()->size();*/
    QSize captureExclude_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::ExcludeFromCapture).size();
    QSize menu_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::ApplicationMenu).size();
    QSize pin_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::OnAllDesktops).size();
    QSize shade_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::Shade).size();
    QSize underlap_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::KeepAbove).size();
    QSize overlap_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::KeepAbove).size();
    QSize help_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::ContextHelp).size();
    QSize min_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::Minimize).size();
    QSize max_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::Maximize).size();
    QSize close_size = smoddecoration->buttonRect(KDecoration3::DecorationButtonType::Close).size();
    /*qDebug() << "Min button: " << min_size;
    qDebug() << "Max button: " << max_size;
    qDebug() << "Close button: " << close_size;*/

    captureExclude_size += QSize(MINMAXGLOW_SML * 2 + 1, MINMAXGLOW_SMT * 2);
    menu_size += QSize(MINMAXGLOW_SML*2+1, MINMAXGLOW_SMT*2);
    pin_size += QSize(MINMAXGLOW_SML*2+1, MINMAXGLOW_SMT*2);
    shade_size += QSize(MINMAXGLOW_SML*2+1, MINMAXGLOW_SMT*2);
    underlap_size += QSize(MINMAXGLOW_SML*2+1, MINMAXGLOW_SMT*2);
    overlap_size += QSize(MINMAXGLOW_SML*2+1, MINMAXGLOW_SMT*2);
    help_size += QSize(MINMAXGLOW_SML * 2 + 1, MINMAXGLOW_SMT * 2);
    min_size += QSize(MINMAXGLOW_SML*2+1, MINMAXGLOW_SMT*2);
    max_size += QSize(MINMAXGLOW_SML*2+1, MINMAXGLOW_SMT*2);
    close_size += QSize(CLOSEGLOW_SML * 2, CLOSEGLOW_SMT * 2);
    /*qDebug() << "Min size: " << min_size;
    qDebug() << "Max size: " << max_size;
    qDebug() << "Close size: " << close_size;*/
    handler->m_captureExclude_rect = Rect(origin + handler->m_captureExclude->pos, captureExclude_size);
    handler->m_menu_rect = Rect(origin + handler->m_menu->pos, menu_size);
    handler->m_pin_rect = Rect(origin + handler->m_pin->pos, pin_size);
    handler->m_shade_rect = Rect(origin + handler->m_shade->pos, shade_size);
    handler->m_underlap_rect = Rect(origin + handler->m_underlap->pos, underlap_size);
    handler->m_overlap_rect = Rect(origin + handler->m_overlap->pos, overlap_size);
    handler->m_help_rect = Rect(origin + handler->m_help->pos, help_size);
    handler->m_min_rect = Rect(origin + handler->m_min->pos, min_size);
    handler->m_max_rect = Rect(origin + handler->m_max->pos, max_size);
    handler->m_close_rect = Rect(origin + handler->m_close->pos, close_size);

    /*handler->m_min_rect   = Rect(origin + handler->m_min->pos,   m_texture_minimize.get()->size());
    handler->m_max_rect   = Rect(origin + handler->m_max->pos,   m_texture_maximize.get()->size());
    handler->m_close_rect = Rect(origin + handler->m_close->pos, m_texture_close.get()->size());*/

    Region newPaint = Region();
    newPaint |= Rect(handler->m_captureExclude_rect);
    newPaint |= Rect(handler->m_menu_rect);
    newPaint |= Rect(handler->m_pin_rect);
    newPaint |= Rect(handler->m_shade_rect);
    newPaint |= Rect(handler->m_underlap_rect);
    newPaint |= Rect(handler->m_overlap_rect);
    newPaint |= Rect(handler->m_help_rect);
    newPaint |= Rect(handler->m_min_rect);
    newPaint |= Rect(handler->m_max_rect);
    newPaint |= Rect(handler->m_close_rect);

    m_prevPaint = newPaint;
}

void SmodGlowEffect::postPaintScreen()
{
    for (auto it = windows.begin(); it != windows.end(); ++it) {
        GlowHandler *handler = it.value();

        if (handler->m_needsRepaint) {
            effects->addRepaint(m_prevPaint);
        }
    }

    effects->postPaintScreen();
}

void SmodGlowEffect::windowAdded(EffectWindow *w)
{
    if (previousDecorationCount == 0 && SMOD::Decoration::decorationCount() != 0) {
        loadTextures();
    }
    previousDecorationCount = SMOD::Decoration::decorationCount();

    m_active = m_active && previousDecorationCount != 0 && SMOD::Decoration::glowEnabled();

    registerWindow(w);
}

void SmodGlowEffect::windowClosed(EffectWindow *w)
{
    if (SMOD::Decoration::decorationCount() == 0) {
        m_active = false;
    }
    unregisterWindow(w);
}

void SmodGlowEffect::windowMaximizedStateChanged(EffectWindow *w, bool horizontal, bool vertical)
{
    Q_UNUSED(horizontal)
    Q_UNUSED(vertical)

    stopAllAnimations(w);
}

void SmodGlowEffect::windowMinimized(EffectWindow *w)
{
    stopAllAnimations(w);
}

void SmodGlowEffect::windowStartUserMovedResized(EffectWindow *w)
{
    stopAllAnimations(w);
}

void SmodGlowEffect::effectWindowFullScreenChanged(EffectWindow *w)
{
    /*
     * When a window goes fullscreen the decoration connection is lost.
     * Just need to register the window again.
     */
    unregisterWindow(w);
    registerWindow(w);
}

void SmodGlowEffect::windowDecorationChanged(EffectWindow *w)
{
    /*
     * Ditto
     */
    unregisterWindow(w);
    registerWindow(w);
}

}
