/*
 *  Copyright (C) 2012-2025 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "JNIXBMCMessageReceivedCallback.h"

#include "CompileInfo.h"
#include "utils/log.h"

#include <androidjni/jutils-details.hpp>

using namespace jni;

static std::string className = std::string(CCompileInfo::GetClass()) + "/CJNIXBMCMessageReceivedCallback";

void CJNIXBMCMessageReceivedCallback::RegisterNatives(JNIEnv* env)
{
  jclass cClass = env->FindClass(className.c_str());
  if (cClass)
  {
    JNINativeMethod methods[] = {
        {"_nativeOnCastMessage", "(I)V",
         reinterpret_cast<void*>(&CJNIXBMCMessageReceivedCallback::_nativeOnCastMessage)},
    };

    env->RegisterNatives(cClass, methods, sizeof(methods) / sizeof(methods[0]));
  }
}

void CJNIXBMCMessageReceivedCallback::_nativeOnCastMessage(JNIEnv* env, jobject thiz, jstring nameSpace, jstring message)
{
  // Receiver->Kodi commands (JSON)
  DispatchCastCommandJSON(jcast<std::string>(jhstring::fromJNI(message)));
}

void CJNIXBMCMessageReceivedCallback::DispatchCastCommandJSON(const std::string& message)
{
  CLog::Log(LOGDEBUG, "CJNIXBMCMessageReceivedCallback::DispatchCastCommandJSON: {}", message);  

  // TODO: parse json using Kodi JSON facilities (CVariant etc.)
  // Expected examples:
  // { "type":"CMD", "cmd":"PLAYPAUSE" }
  // { "type":"CMD", "cmd":"VOLUME_SET", "value":0.75 }
  // { "type":"CMD", "cmd":"AUDIO_TRACK_SET", "streamIndex":2 }
  //
  // Then dispatch:
  // - PLAYPAUSE -> Toggle play/pause
  // - STOP -> Stop
  // - SEEK -> Seek
  // - VOLUME_SET/DELTA -> Set volume
  // - AUDIO_TRACK_SET -> set audio stream
  // - SUBTITLE_TRACK_SET -> set subtitle
  //
  // After applying, trigger a STATE publish (call your UpdateSessionMetadata()).
}

