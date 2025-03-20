/////////////////////////////////////////////////////////////////////////////////////////////////
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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include "WGLInterface.h"

namespace {
struct PixelFormat {
  int format;
  int sampleCnt;
  int choosePixelFormatRank;
};

bool pfLess(const PixelFormat& a, const PixelFormat& b) {
  if (a.sampleCnt < b.sampleCnt) {
    return true;
  } else if (b.sampleCnt < a.sampleCnt) {
    return false;
  } else if (a.choosePixelFormatRank < b.choosePixelFormatRank) {
    return true;
  }
  return false;
}

template <typename T, typename K, typename LESS>
int search(const T base[], int count, const K& key, size_t elemSize, const LESS& less) {
  assert(count >= 0);
  if (count <= 0) {
    return ~0;
  }

  int lo = 0;
  int hi = count - 1;

  while (lo < hi) {
    int mid = lo + ((hi - lo) >> 1);
    const T* elem = (const T*)((const char*)base + mid * elemSize);

    if (less(*elem, key)) lo = mid + 1;
    else
      hi = mid;
  }

  const T* elem = (const T*)((const char*)base + hi * elemSize);
  if (less(*elem, key)) {
    hi += 1;
    hi = ~hi;
  } else if (less(key, *elem)) {
    hi = ~hi;
  }
  return hi;
}

template <typename T, bool(LESS)(const T&, const T&)>
int search(const T base[], int count, const T& target, size_t elemSize) {
  return search(base, count, target, elemSize, [](const T& a, const T& b) { return LESS(a, b); });
}
}  // namespace

namespace skiawindow {
static int selectFormat(const int formats[], int formatCount, HDC dc, int desiredSampleCount) {
  if (formatCount <= 0) {
    return -1;
  }
  PixelFormat desiredFormat = {
      0,
      desiredSampleCount,
      0,
  };

  auto wglInterface = WGLInterface::Get();

  std::vector<PixelFormat> rankedFormats;
  rankedFormats.resize(formatCount);
  for (int i = 0; i < formatCount; ++i) {
    static constexpr int kQueryAttr = WGL_SAMPLES;
    int numSamples;
    wglInterface->wglGetPixelFormatAttribiv(dc, formats[i], 0, 1, &kQueryAttr, &numSamples);
    rankedFormats[i].format = formats[i];
    rankedFormats[i].sampleCnt = std::max(1, numSamples);
    rankedFormats[i].choosePixelFormatRank = i;
  }
  std::sort(rankedFormats.begin(), rankedFormats.end(), pfLess);
  int idx = search<PixelFormat, pfLess>(rankedFormats.data(), rankedFormats.size(), desiredFormat,
                                        sizeof(PixelFormat));
  if (idx < 0) {
    idx = ~idx;
  }

  if (desiredSampleCount == 1 && rankedFormats[idx].sampleCnt != 1) {
    return -1;
  }
  return rankedFormats[idx].format;
}

static void GetPixelFormatsToTry(HDC deviceContext, int msaaSampleCount, int formatsToTry[2]) {
  auto wglInterface = WGLInterface::Get();
  if (!wglInterface->pixelFormatSupport) {
    return;
  }
  std::vector<int> intAttributes{WGL_DRAW_TO_WINDOW,
                                 TRUE,
                                 WGL_DOUBLE_BUFFER,
                                 TRUE,
                                 WGL_ACCELERATION,
                                 WGL_FULL_ACCELERATION,
                                 WGL_SUPPORT_OPENGL,
                                 TRUE,
                                 WGL_COLOR_BITS,
                                 24,
                                 WGL_ALPHA_BITS,
                                 8,
                                 WGL_STENCIL_BITS,
                                 8,
                                 0,
                                 0};
  constexpr float floatAttributes[] = {0, 0};
  if (wglInterface->multiSampleSupport && msaaSampleCount > 0) {
    intAttributes.push_back(WGL_SAMPLE_BUFFERS);
    intAttributes.push_back(true);
    intAttributes.push_back(WGL_SAMPLES);
    intAttributes.push_back(msaaSampleCount);
    unsigned int num;
    int formats[64];
    wglInterface->wglChoosePixelFormat(deviceContext, intAttributes.data(), floatAttributes, 64,
                                       formats, &num);
    num = std::min(num, 64U);
    formatsToTry[0] = selectFormat(formats, num, deviceContext, msaaSampleCount);
  }

  auto format = formatsToTry[0] ? &formatsToTry[0] : &formatsToTry[1];
  unsigned numFormats = 0;
  wglInterface->wglChoosePixelFormat(deviceContext, intAttributes.data(), floatAttributes, 1,
                                     format, &numFormats);
}

static HGLRC CreateGLContext(HDC deviceContext, HGLRC sharedContext, bool glCoreProfile) {
  auto oldDeviceContext = wglGetCurrentDC();
  auto oldGLContext = wglGetCurrentContext();

  HGLRC glContext = nullptr;
  auto wglInterface = WGLInterface::Get();
  if (glCoreProfile && wglInterface->createContextAttribsSupport) {
    const int coreProfileAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION,
        wglInterface->glMajorVersion,
        WGL_CONTEXT_MINOR_VERSION,
        wglInterface->glMinorVersion,
        WGL_CONTEXT_PROFILE_MASK,
        WGL_CONTEXT_CORE_PROFILE_BIT,
        0,
    };

    glContext =
        wglInterface->wglCreateContextAttribs(deviceContext, sharedContext, coreProfileAttribs);
  }

  if (glContext == nullptr) {
    glContext = wglCreateContext(deviceContext);
    if (glContext == nullptr) {
      std::cout << "CreateGLContext() wglCreateContext failed.\n";
      return nullptr;
    }
    if (sharedContext != nullptr && !wglShareLists(sharedContext, glContext)) {
      wglDeleteContext(glContext);
      return nullptr;
    }
  }

  wglMakeCurrent(oldDeviceContext, oldGLContext);
  return glContext;
}

HGLRC CreateWGLContext(HDC deviceContext, int msaaSampleCount, HGLRC sharedContext,
                       bool glCoreProfile) {
  auto set = false;
  int pixelFormatsToTry[2] = {-1, -1};
  GetPixelFormatsToTry(deviceContext, msaaSampleCount, pixelFormatsToTry);
  for (auto f = 0; !set && pixelFormatsToTry[f] && f < 2; ++f) {
    PIXELFORMATDESCRIPTOR descriptor;
    DescribePixelFormat(deviceContext, pixelFormatsToTry[f], sizeof(descriptor), &descriptor);
    set = SetPixelFormat(deviceContext, pixelFormatsToTry[f], &descriptor);
  }

  if (!set) {
    return nullptr;
  }
  return CreateGLContext(deviceContext, sharedContext, glCoreProfile);
}

}  // namespace skiawindow
