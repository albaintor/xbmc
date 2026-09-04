/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "PlayerVideoSettings.h"

#include "PlayerIds.h"
#include "ServiceBroker.h"
#include "application/ApplicationComponents.h"
#include "application/ApplicationPlayer.h"
#include "cores/VideoSettings.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSettings.h"
#include "settings/SettingsComponent.h"
#include "utils/MathUtils.h"
#include "utils/Variant.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

using namespace JSONRPC;

namespace
{
constexpr std::array<std::string_view, 23> VIDEO_SETTING_PROPERTIES{
    "audiodelay",       "volumeamplification",
    "subtitledelay",    "subtitleverticalposition",
    "viewmode",         "zoom",
    "pixelratio",       "verticalshift",
    "nonlinearstretch", "interlacemethod",
    "scalingmethod",    "brightness",
    "contrast",         "gamma",
    "noisereduction",   "sharpness",
    "postprocess",      "tonemapmethod",
    "tonemapparam",     "stereomode",
    "stereoinvert",     "orientation",
    "centermixlevel",
};

bool IsVideoSettingProperty(const std::string& property)
{
  return std::ranges::find(VIDEO_SETTING_PROPERTIES, property) != VIDEO_SETTING_PROPERTIES.end();
}

const char* ViewModeName(int mode)
{
  switch (static_cast<ViewMode>(mode))
  {
    case ViewModeNormal:
      return "normal";
    case ViewModeZoom:
      return "zoom";
    case ViewModeStretch4x3:
      return "stretch4x3";
    case ViewModeWideZoom:
      return "widezoom";
    case ViewModeStretch16x9:
      return "stretch16x9";
    case ViewModeOriginal:
      return "original";
    case ViewModeStretch16x9Nonlin:
      return "stretch16x9nonlin";
    case ViewModeZoom120Width:
      return "zoom120width";
    case ViewModeZoom110Width:
      return "zoom110width";
    case ViewModeCustom:
    default:
      return "custom";
  }
}

int ViewModeValue(const std::string& mode)
{
  if (mode == "normal")
    return ViewModeNormal;
  if (mode == "zoom")
    return ViewModeZoom;
  if (mode == "stretch4x3")
    return ViewModeStretch4x3;
  if (mode == "widezoom")
    return ViewModeWideZoom;
  if (mode == "stretch16x9")
    return ViewModeStretch16x9;
  if (mode == "original")
    return ViewModeOriginal;
  if (mode == "stretch16x9nonlin")
    return ViewModeStretch16x9Nonlin;
  if (mode == "zoom120width")
    return ViewModeZoom120Width;
  if (mode == "zoom110width")
    return ViewModeZoom110Width;
  if (mode == "custom")
    return ViewModeCustom;
  return -1;
}

bool GetPropertyFromSettings(const CVideoSettings& settings,
                             const std::string& property,
                             CVariant& result)
{
  if (property == "audiodelay")
    result = settings.m_AudioDelay;
  else if (property == "volumeamplification")
    result = settings.m_VolumeAmplification;
  else if (property == "subtitledelay")
    result = settings.m_SubtitleDelay;
  else if (property == "subtitleverticalposition")
    result = settings.m_subtitleVerticalPosition;
  else if (property == "viewmode")
    result = ViewModeName(settings.m_ViewMode);
  else if (property == "zoom")
    result = settings.m_CustomZoomAmount;
  else if (property == "pixelratio")
    result = settings.m_CustomPixelRatio;
  else if (property == "verticalshift")
    result = settings.m_CustomVerticalShift;
  else if (property == "nonlinearstretch")
    result = settings.m_CustomNonLinStretch;
  else if (property == "interlacemethod")
    result = static_cast<int>(settings.m_InterlaceMethod);
  else if (property == "scalingmethod")
    result = static_cast<int>(settings.m_ScalingMethod);
  else if (property == "brightness")
    result = settings.m_Brightness;
  else if (property == "contrast")
    result = settings.m_Contrast;
  else if (property == "gamma")
    result = settings.m_Gamma;
  else if (property == "noisereduction")
    result = settings.m_NoiseReduction;
  else if (property == "sharpness")
    result = settings.m_Sharpness;
  else if (property == "postprocess")
    result = settings.m_PostProcess;
  else if (property == "tonemapmethod")
    result = static_cast<int>(settings.m_ToneMapMethod);
  else if (property == "tonemapparam")
    result = settings.m_ToneMapParam;
  else if (property == "stereomode")
    result = settings.m_StereoMode;
  else if (property == "stereoinvert")
    result = settings.m_StereoInvert;
  else if (property == "orientation")
    result = settings.m_Orientation;
  else if (property == "centermixlevel")
    result = settings.m_CenterMixLevel;
  else
    return false;

  return true;
}

void GetAllProperties(const CVideoSettings& settings, CVariant& result)
{
  result = CVariant(CVariant::VariantTypeObject);
  for (const std::string_view property : VIDEO_SETTING_PROPERTIES)
  {
    const std::string name{property};
    GetPropertyFromSettings(settings, name, result[name]);
  }
}

float ClampAndRound(float value, float range, float step)
{
  value = MathUtils::RoundF(value, step);
  return std::clamp(value, -range, range);
}
} // namespace

bool CPlayerVideoSettings::IsProperty(const std::string& property)
{
  return IsVideoSettingProperty(property);
}

JSONRPC_STATUS CPlayerVideoSettings::GetProperty(const std::string& property, CVariant& result)
{
  if (!IsProperty(property))
    return InvalidParams;

  const auto appPlayer = CServiceBroker::GetAppComponents().GetComponent<CApplicationPlayer>();
  if (!appPlayer->IsPlayingVideo())
    return FailedToExecute;

  return GetPropertyFromSettings(appPlayer->GetVideoSettings(), property, result) ? OK
                                                                                  : InvalidParams;
}

JSONRPC_STATUS CPlayerVideoSettings::SetVideoSettings(const std::string& method,
                                                      ITransportLayer* transport,
                                                      IClient* client,
                                                      const CVariant& parameterObject,
                                                      CVariant& result)
{
  if (PlayerForId(PLAYLIST::Id{parameterObject["playerid"].asInteger32()}) != Video)
    return FailedToExecute;

  const auto appPlayer = CServiceBroker::GetAppComponents().GetComponent<CApplicationPlayer>();
  if (!appPlayer->IsPlayingVideo())
    return FailedToExecute;

  const CVariant& values = parameterObject["settings"];
  CVideoSettings settings = appPlayer->GetVideoSettings();
  const auto advancedSettings = CServiceBroker::GetSettingsComponent()->GetAdvancedSettings();

  bool viewModeChanged = false;
  bool customViewModeChanged = false;
  bool volumeAmplificationChanged = false;
  bool subtitleVerticalPositionChanged = false;

  if (values.isMember("audiodelay"))
  {
    settings.m_AudioDelay =
        ClampAndRound(values["audiodelay"].asFloat(), advancedSettings->m_videoAudioDelayRange,
                      advancedSettings->m_videoAudioDelayStep);
  }
  if (values.isMember("volumeamplification"))
  {
    settings.m_VolumeAmplification =
        std::clamp(values["volumeamplification"].asFloat(), VOLUME_DRC_MINIMUM * 0.01f,
                   VOLUME_DRC_MAXIMUM * 0.01f);
    volumeAmplificationChanged = true;
  }
  if (values.isMember("subtitledelay"))
  {
    settings.m_SubtitleDelay =
        ClampAndRound(values["subtitledelay"].asFloat(), advancedSettings->m_videoSubsDelayRange,
                      advancedSettings->m_videoSubsDelayStep);
  }
  if (values.isMember("subtitleverticalposition"))
  {
    settings.m_subtitleVerticalPosition = values["subtitleverticalposition"].asInteger32();
    settings.m_subtitleVerticalPositionSave = false;
    subtitleVerticalPositionChanged = true;
  }
  if (values.isMember("viewmode"))
  {
    const int mode = ViewModeValue(values["viewmode"].asString());
    if (mode < 0)
      return InvalidParams;
    settings.m_ViewMode = mode;
    viewModeChanged = true;
  }
  if (values.isMember("zoom"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_ZOOM))
      return FailedToExecute;
    settings.m_CustomZoomAmount = values["zoom"].asFloat();
    customViewModeChanged = true;
  }
  if (values.isMember("pixelratio"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_PIXEL_RATIO))
      return FailedToExecute;
    settings.m_CustomPixelRatio = values["pixelratio"].asFloat();
    customViewModeChanged = true;
  }
  if (values.isMember("verticalshift"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_VERTICAL_SHIFT))
      return FailedToExecute;
    settings.m_CustomVerticalShift = values["verticalshift"].asFloat();
    customViewModeChanged = true;
  }
  if (values.isMember("nonlinearstretch"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_NONLINSTRETCH))
      return FailedToExecute;
    settings.m_CustomNonLinStretch = values["nonlinearstretch"].asBoolean();
    customViewModeChanged = true;
  }
  if (values.isMember("interlacemethod"))
  {
    const auto value = static_cast<EINTERLACEMETHOD>(values["interlacemethod"].asInteger32());
    if (!appPlayer->Supports(value))
      return FailedToExecute;
    settings.m_InterlaceMethod = value;
  }
  if (values.isMember("scalingmethod"))
  {
    const auto value = static_cast<ESCALINGMETHOD>(values["scalingmethod"].asInteger32());
    if (!appPlayer->Supports(value))
      return FailedToExecute;
    settings.m_ScalingMethod = value;
  }
  if (values.isMember("brightness"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_BRIGHTNESS))
      return FailedToExecute;
    settings.m_Brightness = values["brightness"].asFloat();
  }
  if (values.isMember("contrast"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_CONTRAST))
      return FailedToExecute;
    settings.m_Contrast = values["contrast"].asFloat();
  }
  if (values.isMember("gamma"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_GAMMA))
      return FailedToExecute;
    settings.m_Gamma = values["gamma"].asFloat();
  }
  if (values.isMember("noisereduction"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_NOISE))
      return FailedToExecute;
    settings.m_NoiseReduction = values["noisereduction"].asFloat();
  }
  if (values.isMember("sharpness"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_SHARPNESS))
      return FailedToExecute;
    settings.m_Sharpness = values["sharpness"].asFloat();
  }
  if (values.isMember("postprocess"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_POSTPROCESS))
      return FailedToExecute;
    settings.m_PostProcess = values["postprocess"].asBoolean();
  }
  if (values.isMember("tonemapmethod"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_TONEMAP))
      return FailedToExecute;
    settings.m_ToneMapMethod = static_cast<ETONEMAPMETHOD>(values["tonemapmethod"].asInteger32());
  }
  if (values.isMember("tonemapparam"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_TONEMAP))
      return FailedToExecute;
    settings.m_ToneMapParam = values["tonemapparam"].asFloat();
  }
  if (values.isMember("stereomode"))
    settings.m_StereoMode = values["stereomode"].asInteger32();
  if (values.isMember("stereoinvert"))
    settings.m_StereoInvert = values["stereoinvert"].asBoolean();
  if (values.isMember("orientation"))
  {
    if (!appPlayer->Supports(RENDERFEATURE_ROTATION))
      return FailedToExecute;
    settings.m_Orientation = values["orientation"].asInteger32();
  }
  if (values.isMember("centermixlevel"))
    settings.m_CenterMixLevel = values["centermixlevel"].asInteger32();

  if (customViewModeChanged && !viewModeChanged)
    settings.m_ViewMode = ViewModeCustom;

  appPlayer->SetVideoSettings(settings);

  if (volumeAmplificationChanged)
  {
    appPlayer->SetDynamicRangeCompression(std::lround(settings.m_VolumeAmplification * 100.0f));
  }
  if (subtitleVerticalPositionChanged)
  {
    appPlayer->SetSubtitleVerticalPosition(settings.m_subtitleVerticalPosition, false);
  }
  if (viewModeChanged || customViewModeChanged)
  {
    appPlayer->SetRenderViewMode(settings.m_ViewMode, settings.m_CustomZoomAmount,
                                 settings.m_CustomPixelRatio, settings.m_CustomVerticalShift,
                                 settings.m_CustomNonLinStretch);
  }

  GetAllProperties(appPlayer->GetVideoSettings(), result);
  return OK;
}
