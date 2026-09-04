/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "JSONRPC.h"

#include <string>

class CVariant;

namespace JSONRPC
{
class CPlayerVideoSettings
{
public:
  static bool IsProperty(const std::string& property);
  static JSONRPC_STATUS GetProperty(const std::string& property, CVariant& result);
  static JSONRPC_STATUS SetVideoSettings(const std::string& method,
                                         ITransportLayer* transport,
                                         IClient* client,
                                         const CVariant& parameterObject,
                                         CVariant& result);
};
} // namespace JSONRPC
