/*
 *  Copyright (C) 2012-2025 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "JNIXBMCCastManager.h"

#include "CompileInfo.h"
#include "utils/log.h"

using namespace jni;

static std::string className = std::string(CCompileInfo::GetClass()) + "/XBMCCastManager";

void CJNIXBMCCastManager::RegisterNatives(JNIEnv* env)
{
  jclass cClass = env->FindClass(className.c_str());
  if (cClass)
  {
    JNINativeMethod methods[] = {
        {"_nativeOnCastSessionChanged", "(I)V",
         reinterpret_cast<void*>(&CJNIXBMCCastManager::_nativeOnCastSessionChanged)},
    };

    env->RegisterNatives(cClass, methods, sizeof(methods) / sizeof(methods[0]));
  }
}

void CJNIXBMCCastManager::_nativeOnCastSessionChanged(JNIEnv* env, jobject thiz, jboolean connected)
{
  OnCastSessionChanged(connected == JNI_TRUE);
}

void CJNIXBMCCastManager::OnCastSessionChanged(bool connected)
{
  CLog::Log(LOGDEBUG, "CJNIXBMCCastManager::OnCastSessionChanged: {}", connected ? "true" : "false");

  // TODO:
  // - If connected: immediately publish current STATE (title/art/position/tracks/volume)
  //   e.g. call CXBMCApp::UpdateSessionMetadata() or your publisher function.
  // - If disconnected: optionally stop pushing.
}
