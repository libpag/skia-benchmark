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

#include <Cocoa/Cocoa.h>
#include <memory>
#include "window_context/WindowContext.h"

namespace skiawindow {

struct DisplayParams;

static inline CGFloat GetBackingScaleFactor(NSView* view) {
#ifdef SK_BUILD_FOR_IOS
  UIScreen* screen = view.window.screen ?: [UIScreen mainScreen];
  return screen.nativeScale;
#else
  NSScreen* screen = view.window.screen ?: [NSScreen mainScreen];
  return screen.backingScaleFactor;
#endif
}

struct MacWindowInfo {
  NSView* mainView;
};

static inline NSOpenGLPixelFormat* GetGLPixelFormat(int sampleCount) {
  constexpr int kMaxAttributes = 19;
  NSOpenGLPixelFormatAttribute attributes[kMaxAttributes];
  int numAttributes = 0;
  attributes[numAttributes++] = NSOpenGLPFAAccelerated;
  attributes[numAttributes++] = NSOpenGLPFAClosestPolicy;
  attributes[numAttributes++] = NSOpenGLPFADoubleBuffer;
  attributes[numAttributes++] = NSOpenGLPFAOpenGLProfile;
  attributes[numAttributes++] = NSOpenGLProfileVersion3_2Core;
  attributes[numAttributes++] = NSOpenGLPFAColorSize;
  attributes[numAttributes++] = 24;
  attributes[numAttributes++] = NSOpenGLPFAAlphaSize;
  attributes[numAttributes++] = 8;
  attributes[numAttributes++] = NSOpenGLPFADepthSize;
  attributes[numAttributes++] = 0;
  attributes[numAttributes++] = NSOpenGLPFAStencilSize;
  attributes[numAttributes++] = 8;
  if (sampleCount > 1) {
    attributes[numAttributes++] = NSOpenGLPFAMultisample;
    attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
    attributes[numAttributes++] = 1;
    attributes[numAttributes++] = NSOpenGLPFASamples;
    attributes[numAttributes++] = sampleCount;
  } else {
    attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
    attributes[numAttributes++] = 0;
  }
  attributes[numAttributes++] = 0;
  SkASSERT(numAttributes <= kMaxAttributes);
  return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
}

std::unique_ptr<WindowContext> MakeGLForMac(const MacWindowInfo&, const DisplayParams&);

}  // namespace skiawindow
