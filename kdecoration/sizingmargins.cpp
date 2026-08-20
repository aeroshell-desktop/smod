#include "sizingmargins.h"

namespace SMOD
{

SizingMargins::SizingMargins()
{
}

SizingMargins::~SizingMargins()
{
}

void SizingMargins::loadSizingMargins()
{
    QSettings settings(":/decoration/sizingmargins", QSettings::IniFormat);
    // GlowSizing
    m_glowSizing.margin_left   = settings.value("Glow/margin_left",   24).toInt();
    m_glowSizing.margin_right  = settings.value("Glow/margin_right",  25).toInt();
    m_glowSizing.margin_top    = settings.value("Glow/margin_top",    17).toInt();
    m_glowSizing.margin_bottom = settings.value("Glow/margin_bottom", 18).toInt();

    m_glowSizing.active_opacity   = settings.value("Glow/active_opacity",   255).toInt() / 255.0;
    m_glowSizing.inactive_opacity = settings.value("Glow/inactive_opacity", 179).toInt() / 255.0;

    // ShadowSizing
    m_shadowSizing.margin_left    = settings.value("Shadow/margin_left",    30).toInt();
    m_shadowSizing.margin_top     = settings.value("Shadow/margin_top",     31).toInt();
    m_shadowSizing.margin_right   = settings.value("Shadow/margin_right",   29).toInt();
    m_shadowSizing.margin_bottom  = settings.value("Shadow/margin_bottom",  37).toInt();
    m_shadowSizing.padding_left   = settings.value("Shadow/padding_left",   14).toInt();
    m_shadowSizing.padding_top    = settings.value("Shadow/padding_top",    14).toInt();
    m_shadowSizing.padding_right  = settings.value("Shadow/padding_right",  20).toInt();
    m_shadowSizing.padding_bottom = settings.value("Shadow/padding_bottom", 20).toInt();

    // CommonSizing
    m_commonSizing.height                       = settings.value("Common/height",                      21).toInt();
    m_commonSizing.titlebar_padding_normal      = settings.value("Common/titlebar_padding_normal",      8).toInt();
    m_commonSizing.titlebar_padding_maximized   = settings.value("Common/titlebar_padding_maximized",   0).toInt();
    m_commonSizing.corner_radius                = settings.value("Common/corner_radius",                6).toInt();
    m_commonSizing.caption_button_spacing       = settings.value("Common/caption_button_spacing",       0).toInt();
    m_commonSizing.alternative                  = settings.value("Common/alternative",                  false).toBool();
    m_commonSizing.enable_glow                  = settings.value("Common/enable_glow",                  false).toBool();
    m_commonSizing.caption_button_align_vcenter = settings.value("Common/caption_button_align_vcenter", false).toBool();
    m_commonSizing.group_buttons                = settings.value("Common/group_buttons",                false).toBool();

    // CloseLoneSizing
    m_buttonSizingMargins[SMOD::CloseLone].width = settings.value("CloseLone/width", 49).toInt();
    m_buttonSizingMargins[SMOD::CloseLone].margin_left = settings.value("CloseLone/margin_left", 20).toInt();
    m_buttonSizingMargins[SMOD::CloseLone].margin_top = settings.value("CloseLone/margin_top", 6).toInt();
    m_buttonSizingMargins[SMOD::CloseLone].margin_right = settings.value("CloseLone/margin_right", 20).toInt();
    m_buttonSizingMargins[SMOD::CloseLone].margin_bottom = settings.value("CloseLone/margin_bottom", 8).toInt();
    m_buttonSizingMargins[SMOD::CloseLone].content_left = settings.value("CloseLone/content_left", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::CloseLone].content_right = settings.value("CloseLone/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::CloseLone].content_top = settings.value("CloseLone/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::CloseLone].content_bottom = settings.value("CloseLone/content_bottom", 4).toInt() - 1;

    // CloseSizing
    m_buttonSizingMargins[SMOD::Close].width = settings.value("Close/width", 49).toInt();
    m_buttonSizingMargins[SMOD::Close].margin_left = settings.value("Close/margin_left", 20).toInt();
    m_buttonSizingMargins[SMOD::Close].margin_top = settings.value("Close/margin_top", 6).toInt();
    m_buttonSizingMargins[SMOD::Close].margin_right = settings.value("Close/margin_right", 20).toInt();
    m_buttonSizingMargins[SMOD::Close].margin_bottom = settings.value("Close/margin_bottom", 8).toInt();
    m_buttonSizingMargins[SMOD::Close].content_left = settings.value("Close/content_left", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::Close].content_right = settings.value("Close/content_right", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::Close].content_top = settings.value("Close/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::Close].content_bottom = settings.value("Close/content_bottom", 4).toInt() - 1;

    // MaximizeSizing
    m_buttonSizingMargins[SMOD::Maximize].width = settings.value("Maximize/width", 27).toInt();
    m_buttonSizingMargins[SMOD::Maximize].margin_left = settings.value("Maximize/margin_left", 12).toInt();
    m_buttonSizingMargins[SMOD::Maximize].margin_top = settings.value("Maximize/margin_top", 6).toInt();
    m_buttonSizingMargins[SMOD::Maximize].margin_right = settings.value("Maximize/margin_right", 12).toInt();
    m_buttonSizingMargins[SMOD::Maximize].margin_bottom = settings.value("Maximize/margin_bottom", 8).toInt();
    m_buttonSizingMargins[SMOD::Maximize].content_left = settings.value("Maximize/content_left", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::Maximize].content_right = settings.value("Maximize/content_right", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::Maximize].content_top = settings.value("Maximize/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::Maximize].content_bottom = settings.value("Maximize/content_bottom", 4).toInt() - 1;

    // MinimizeSizing
    m_buttonSizingMargins[SMOD::Minimize].width = settings.value("Minimize/width", 29).toInt();
    m_buttonSizingMargins[SMOD::Minimize].margin_left = settings.value("Minimize/margin_left", 12).toInt();
    m_buttonSizingMargins[SMOD::Minimize].margin_top = settings.value("Minimize/margin_top", 6).toInt();
    m_buttonSizingMargins[SMOD::Minimize].margin_right = settings.value("Minimize/margin_right", 12).toInt();
    m_buttonSizingMargins[SMOD::Minimize].margin_bottom = settings.value("Minimize/margin_bottom", 8).toInt();
    m_buttonSizingMargins[SMOD::Minimize].content_left = settings.value("Minimize/content_left", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::Minimize].content_right = settings.value("Minimize/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::Minimize].content_top = settings.value("Minimize/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::Minimize].content_bottom = settings.value("Minimize/content_bottom", 4).toInt() - 1;

    // These next 5 buttons will just use the minimize button
    // sizing margins, except for the content parts.

    // HelpSizing
    m_buttonSizingMargins[SMOD::ContextHelp].width = settings.value("Help/width", 29).toInt();
    m_buttonSizingMargins[SMOD::ContextHelp].margin_left = m_buttonSizingMargins[SMOD::Minimize].margin_left;
    m_buttonSizingMargins[SMOD::ContextHelp].margin_top = m_buttonSizingMargins[SMOD::Minimize].margin_top;
    m_buttonSizingMargins[SMOD::ContextHelp].margin_right = m_buttonSizingMargins[SMOD::Minimize].margin_right;
    m_buttonSizingMargins[SMOD::ContextHelp].margin_bottom = m_buttonSizingMargins[SMOD::Minimize].margin_bottom;
    m_buttonSizingMargins[SMOD::ContextHelp].content_left = settings.value("Help/content_left", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::ContextHelp].content_right = settings.value("Help/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::ContextHelp].content_top = settings.value("Help/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::ContextHelp].content_bottom = settings.value("Help/content_bottom", 4).toInt() - 1;

    // OverlapSizing
    m_buttonSizingMargins[SMOD::KeepAbove].width = settings.value("Overlap/width", 29).toInt();
    m_buttonSizingMargins[SMOD::KeepAbove].margin_left = m_buttonSizingMargins[SMOD::Minimize].margin_left;
    m_buttonSizingMargins[SMOD::KeepAbove].margin_top = m_buttonSizingMargins[SMOD::Minimize].margin_top;
    m_buttonSizingMargins[SMOD::KeepAbove].margin_right = m_buttonSizingMargins[SMOD::Minimize].margin_right;
    m_buttonSizingMargins[SMOD::KeepAbove].margin_bottom = m_buttonSizingMargins[SMOD::Minimize].margin_bottom;
    m_buttonSizingMargins[SMOD::KeepAbove].content_left = settings.value("Shade/content_left", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::KeepAbove].content_right = settings.value("Shade/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::KeepAbove].content_top = settings.value("Shade/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::KeepAbove].content_bottom = settings.value("Shade/content_bottom", 4).toInt() - 1;

    // UnderlapSizing
    m_buttonSizingMargins[SMOD::KeepBelow].width = settings.value("Underlap/width", 29).toInt();
    m_buttonSizingMargins[SMOD::KeepBelow].margin_left = m_buttonSizingMargins[SMOD::Minimize].margin_left;
    m_buttonSizingMargins[SMOD::KeepBelow].margin_top = m_buttonSizingMargins[SMOD::Minimize].margin_top;
    m_buttonSizingMargins[SMOD::KeepBelow].margin_right = m_buttonSizingMargins[SMOD::Minimize].margin_right;
    m_buttonSizingMargins[SMOD::KeepBelow].margin_bottom = m_buttonSizingMargins[SMOD::Minimize].margin_bottom;
    m_buttonSizingMargins[SMOD::KeepBelow].content_left = settings.value("Shade/content_left", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::KeepBelow].content_right = settings.value("Shade/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::KeepBelow].content_top = settings.value("Shade/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::KeepBelow].content_bottom = settings.value("Shade/content_bottom", 4).toInt() - 1;

    // ShadeSizing
    m_buttonSizingMargins[SMOD::Shade].width = settings.value("Shade/width", 29).toInt();
    m_buttonSizingMargins[SMOD::Shade].margin_left = m_buttonSizingMargins[SMOD::Minimize].margin_left;
    m_buttonSizingMargins[SMOD::Shade].margin_top = m_buttonSizingMargins[SMOD::Minimize].margin_top;
    m_buttonSizingMargins[SMOD::Shade].margin_right = m_buttonSizingMargins[SMOD::Minimize].margin_right;
    m_buttonSizingMargins[SMOD::Shade].margin_bottom = m_buttonSizingMargins[SMOD::Minimize].margin_bottom;
    m_buttonSizingMargins[SMOD::Shade].content_left = settings.value("Shade/content_left", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::Shade].content_right = settings.value("Shade/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::Shade].content_top = settings.value("Shade/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::Shade].content_bottom = settings.value("Shade/content_bottom", 4).toInt() - 1;

    // PinSizing
    m_buttonSizingMargins[SMOD::OnAllDesktops].width = settings.value("Pin/width", 29).toInt();
    m_buttonSizingMargins[SMOD::OnAllDesktops].margin_left = m_buttonSizingMargins[SMOD::Minimize].margin_left;
    m_buttonSizingMargins[SMOD::OnAllDesktops].margin_top = m_buttonSizingMargins[SMOD::Minimize].margin_top;
    m_buttonSizingMargins[SMOD::OnAllDesktops].margin_right = m_buttonSizingMargins[SMOD::Minimize].margin_right;
    m_buttonSizingMargins[SMOD::OnAllDesktops].margin_bottom = m_buttonSizingMargins[SMOD::Minimize].margin_bottom;
    m_buttonSizingMargins[SMOD::OnAllDesktops].content_left = settings.value("Pin/content_left", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::OnAllDesktops].content_right = settings.value("Pin/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::OnAllDesktops].content_top = settings.value("Pin/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::OnAllDesktops].content_bottom = settings.value("Pin/content_bottom", 4).toInt() - 1;

    // MenuSizing
    m_buttonSizingMargins[SMOD::ApplicationMenu].width = settings.value("Menu/width", 29).toInt();
    m_buttonSizingMargins[SMOD::ApplicationMenu].margin_left = m_buttonSizingMargins[SMOD::Minimize].margin_left;
    m_buttonSizingMargins[SMOD::ApplicationMenu].margin_top = m_buttonSizingMargins[SMOD::Minimize].margin_top;
    m_buttonSizingMargins[SMOD::ApplicationMenu].margin_right = m_buttonSizingMargins[SMOD::Minimize].margin_right;
    m_buttonSizingMargins[SMOD::ApplicationMenu].margin_bottom = m_buttonSizingMargins[SMOD::Minimize].margin_bottom;
    m_buttonSizingMargins[SMOD::ApplicationMenu].content_left = settings.value("Menu/content_left", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::ApplicationMenu].content_right = settings.value("Menu/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::ApplicationMenu].content_top = settings.value("Menu/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::ApplicationMenu].content_bottom = settings.value("Menu/content_bottom", 4).toInt() - 1;

    // CaptureExcludeSizing
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].width = settings.value("CaptureExclude/width", 29).toInt();
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].margin_left = m_buttonSizingMargins[SMOD::Minimize].margin_left;
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].margin_top = m_buttonSizingMargins[SMOD::Minimize].margin_top;
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].margin_right = m_buttonSizingMargins[SMOD::Minimize].margin_right;
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].margin_bottom = m_buttonSizingMargins[SMOD::Minimize].margin_bottom;
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].content_left = settings.value("CaptureExclude/content_left", 3).toInt() - 1;
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].content_right = settings.value("CaptureExclude/content_right", 1).toInt() - 1;
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].content_top = settings.value("CaptureExclude/content_top", 2).toInt() - 1;
    m_buttonSizingMargins[SMOD::ExcludeFromCapture].content_bottom = settings.value("CaptureExclude/content_bottom", 4).toInt() - 1;

    m_frameLeftSizing.width = settings.value("FrameLeft/width", 8).toInt();
    m_frameLeftSizing.inset = settings.value("FrameLeft/inset", 2).toInt();
    m_frameRightSizing.width = settings.value("FrameRight/width", 8).toInt();
    m_frameRightSizing.inset = settings.value("FrameRight/inset", 2).toInt();
    m_frameBottomSizing.height = settings.value("FrameBottom/height", 8).toInt();

    m_topLeftCorner.width = settings.value("TopLeft/width", 9).toInt();
    m_topLeftCorner.margin_left = settings.value("TopLeft/margin_left", 6).toInt();
    m_topLeftCorner.margin_right = settings.value("TopLeft/margin_right", 2).toInt();
    m_topLeftCorner.margin_top = settings.value("TopLeft/margin_top", 6).toInt();
    m_topLeftCorner.margin_bottom = settings.value("TopLeft/margin_bottom", 2).toInt();

    m_topRightCorner.width = settings.value("TopRight/width", 9).toInt();
    m_topRightCorner.margin_left = settings.value("TopRight/margin_left", 2).toInt();
    m_topRightCorner.margin_right = settings.value("TopRight/margin_right", 6).toInt();
    m_topRightCorner.margin_top = settings.value("TopRight/margin_top", 6).toInt();
    m_topRightCorner.margin_bottom = settings.value("TopRight/margin_bottom", 2).toInt();

    m_bottomLeftCorner.width = settings.value("BottomLeft/width", 9).toInt();
    m_bottomLeftCorner.margin_left = settings.value("BottomLeft/margin_left", 6).toInt();
    m_bottomLeftCorner.margin_right = settings.value("BottomLeft/margin_right", 2).toInt();
    m_bottomLeftCorner.margin_top = settings.value("BottomLeft/margin_top", 2).toInt();
    m_bottomLeftCorner.margin_bottom = settings.value("BottomLeft/margin_bottom", 6).toInt();

    m_bottomRightCorner.width = settings.value("BottomRight/width", 9).toInt();
    m_bottomRightCorner.margin_left = settings.value("BottomRight/margin_left", 2).toInt();
    m_bottomRightCorner.margin_right = settings.value("BottomRight/margin_right", 6).toInt();
    m_bottomRightCorner.margin_top = settings.value("BottomRight/margin_top", 2).toInt();
    m_bottomRightCorner.margin_bottom = settings.value("BottomRight/margin_bottom", 6).toInt();

    m_leftSide.margin_left = settings.value("Left/margin_left", 2).toInt();
    m_leftSide.margin_right = settings.value("Left/margin_right", 6).toInt();
    m_rightSide.margin_left = settings.value("Right/margin_left", 6).toInt();
    m_rightSide.margin_right = settings.value("Right/margin_right", 2).toInt();
    m_topSide.margin_top = settings.value("Top/margin_top", 6).toInt();
    m_topSide.margin_bottom = settings.value("Top/margin_bottom", 2).toInt();
    m_bottomSide.margin_top = settings.value("Bottom/margin_top", 2).toInt();
    m_bottomSide.margin_bottom = settings.value("Bottom/margin_bottom", 6).toInt();

    m_loaded = true;

}

GlowSizing SizingMargins::glowSizing() const
{
    return m_glowSizing;
}

ShadowSizing SizingMargins::shadowSizing() const
{
    return m_shadowSizing;
}

CommonSizing SizingMargins::commonSizing() const
{
    return m_commonSizing;
}

ButtonSizingMargins SizingMargins::buttonSizingFor(SMOD::ButtonTypes type) const
{
    return m_buttonSizingMargins.value(type);
}

BorderFrame SizingMargins::frameLeftSizing() const
{
    return m_frameLeftSizing;
}

BorderFrame SizingMargins::frameRightSizing() const
{
    return m_frameRightSizing;
}

BorderFrame SizingMargins::frameBottomSizing() const
{
    return m_frameBottomSizing;
}

FrameMargins SizingMargins::topLeftCorner() const
{
    return m_topLeftCorner;
}

FrameMargins SizingMargins::topRightCorner() const
{
    return m_topRightCorner;
}

FrameMargins SizingMargins::bottomLeftCorner() const
{
    return m_bottomLeftCorner;
}

FrameMargins SizingMargins::bottomRightCorner() const
{
    return m_bottomRightCorner;
}

FrameMargins SizingMargins::leftSide() const
{
    return m_leftSide;
}

FrameMargins SizingMargins::rightSide() const
{
    return m_rightSide;
}

FrameMargins SizingMargins::topSide() const
{
    return m_topSide;
}

FrameMargins SizingMargins::bottomSide() const
{
    return m_bottomSide;
}

bool SizingMargins::loaded() const
{
    return m_loaded;
}
}
