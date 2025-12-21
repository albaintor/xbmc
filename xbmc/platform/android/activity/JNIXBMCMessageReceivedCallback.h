/*
 *  Copyright (C) 2012-2025 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <androidjni/JNIBase.h>

namespace jni
{

class CJNIXBMCMessageReceivedCallback
{
public:
  static void RegisterNatives(JNIEnv* env);

protected:
  static void _nativeOnCastMessage(JNIEnv* env, jobject thiz, jstring nameSpace, jstring message);
  static void DispatchCastCommandJSON(const std::string& message);
};

} // namespace jni
