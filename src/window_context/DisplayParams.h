//
//  Tencent is pleased to support the open source community by making skia-benchmark available.
//
//  Copyright (C) 2025 THL A29 Limited, a Tencent company. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
//  in compliance with the License. You may obtain a copy of the License at
//
//      https://opensource.org/licenses/BSD-3-Clause
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "include/core/SkColorSpace.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/ganesh/GrContextOptions.h"

namespace skiawindow {

struct DisplayParams {
  DisplayParams()
      : colorType(kN32_SkColorType), colorSpace(nullptr), MSAASampleCount(1),
        surfaceProps(0, kRGB_H_SkPixelGeometry), disableVsync(false),
        delayDrawableAcquisition(false), enableBinaryArchive(false),
        createProtectedNativeBackend(false) {
  }

  SkColorType colorType;
  sk_sp<SkColorSpace> colorSpace;
  int MSAASampleCount;
  GrContextOptions grContextOptions;
  SkSurfaceProps surfaceProps;
  bool disableVsync;
  bool delayDrawableAcquisition;
  bool enableBinaryArchive;
  bool createProtectedNativeBackend = false;
};

}  // namespace skiawindow
