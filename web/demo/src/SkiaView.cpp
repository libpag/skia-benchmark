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

#include "SkiaView.h"
#include <emscripten/val.h>
#include "base/Bench.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/ports/SkFontMgr_data.h"
#include "include/ports/SkFontMgr_empty.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "tools/Clock.h"

static constexpr int64_t FLUSH_INTERVAL = 300000;

namespace benchmark {
sk_sp<SkData> GetDataFromEmscripten(const val& emscriptenData) {
  if (emscriptenData.isUndefined()) {
    return nullptr;
  }
  unsigned int length = emscriptenData["length"].as<unsigned int>();
  if (length == 0) {
    return nullptr;
  }
  auto buffer = new (std::nothrow) uint8_t[length];
  if (buffer) {
    auto memory = val::module_property("HEAPU8")["buffer"];
    auto memoryView =
        emscriptenData["constructor"].new_(memory, reinterpret_cast<uintptr_t>(buffer), length);
    memoryView.call<void>("set", emscriptenData);
    return SkData::MakeFromMalloc(buffer, length);
  }
  return nullptr;
}

EM_BOOL RequestFrameCallback(double, void* userData) {
  auto baseView = static_cast<SkiaView*>(userData);
  if (baseView) {
    baseView->draw();
  }
  return EM_TRUE;
}

EM_BOOL MouseClickCallback(int, const EmscriptenMouseEvent* e, void* userData) {
  auto baseView = static_cast<SkiaView*>(userData);
  if (baseView) {
    double devicePixelRatio = emscripten_get_device_pixel_ratio();
    float x = static_cast<float>(devicePixelRatio) * static_cast<float>(e->clientX);
    float y = static_cast<float>(devicePixelRatio) * static_cast<float>(e->clientY);
    baseView->appHost->mouseMoved(x, y);
    baseView->appHost->resetFrames();
    baseView->drawIndex++;
  }
  return EM_TRUE;
}

EM_BOOL MouseMoveCallBack(int, const EmscriptenMouseEvent* e, void* userData) {
  auto appHost = static_cast<benchmark::AppHost*>(userData);
  if (appHost) {
    double devicePixelRatio = emscripten_get_device_pixel_ratio();
    float x = static_cast<float>(devicePixelRatio) * static_cast<float>(e->clientX);
    float y = static_cast<float>(devicePixelRatio) * static_cast<float>(e->clientY);
    appHost->mouseMoved(x, y);
  }
  return EM_TRUE;
}

EM_BOOL MouseLeaveCallBack(int, const EmscriptenMouseEvent*, void* userData) {
  auto appHost = static_cast<benchmark::AppHost*>(userData);
  if (appHost) {
    appHost->mouseMoved(-1, -1);
  }
  return EM_TRUE;
}

SkiaView::SkiaView(const std::string& canvasID) : canvasID(canvasID) {
  appHost = std::make_shared<benchmark::AppHost>(1024, 720);
  drawIndex = 0;
  emscripten_set_click_callback(canvasID.c_str(), this, EM_TRUE, MouseClickCallback);
  emscripten_set_mousemove_callback(canvasID.c_str(), appHost.get(), EM_TRUE, MouseMoveCallBack);
  emscripten_set_mouseleave_callback(canvasID.c_str(), appHost.get(), EM_TRUE, MouseLeaveCallBack);

  EmscriptenWebGLContextAttributes attrs;
  emscripten_webgl_init_context_attributes(&attrs);
  attrs.majorVersion = 2;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context =
      emscripten_webgl_create_context(canvasID.c_str(), &attrs);
  emscripten_webgl_make_context_current(context);

  auto interface = GrGLMakeNativeInterface();
  skContext = GrDirectContexts::MakeGL(interface);
}

SkiaView::~SkiaView() {
  emscripten_webgl_destroy_context(emscripten_webgl_get_current_context());
}

void SkiaView::updateSize(float devicePixelRatio) {
  if (!canvasID.empty()) {
    int width = 0;
    int height = 0;
    emscripten_get_canvas_element_size(canvasID.c_str(), &width, &height);
    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = 0;
    framebufferInfo.fFormat = GR_GL_RGBA8;

    GrBackendRenderTarget backendRenderTarget =
        GrBackendRenderTargets::MakeGL(width, height, 1, 8, framebufferInfo);

    SkSurfaceProps surfaceProps(0, kRGB_H_SkPixelGeometry);
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();

    skSurface = SkSurfaces::WrapBackendRenderTarget(skContext.get(), backendRenderTarget,
                                                    kBottomLeft_GrSurfaceOrigin, kN32_SkColorType,
                                                    colorSpace, &surfaceProps);
    appHost->updateScreen(width, height, devicePixelRatio);
  }
}

void SkiaView::registerFonts(const val& fontVal, const val& emojiFontVal) {
  auto fontData = GetDataFromEmscripten(fontVal);
  auto mgr = SkFontMgr_New_Custom_Empty();
  if (fontData) {
    auto typeface = mgr->makeFromData(fontData);
    if (typeface) {
      appHost->addTypeface("default", std::move(typeface));
    }
  }

  auto emojiFontData = GetDataFromEmscripten(emojiFontVal);
  if (emojiFontData) {
    auto typeface = mgr->makeFromData(emojiFontData);
    if (typeface) {
      appHost->addTypeface("emoji", std::move(typeface));
    }
  }
}

void SkiaView::startDraw() {
  ParticleBench::ShowPerfData(showPerfDataFlag);
  emscripten_request_animation_frame_loop(RequestFrameCallback, this);
}

void SkiaView::setImagePath(const std::string&) {
}

void SkiaView::draw() {
  auto currentTime = Clock::Now();
  if (appHost->width() <= 0 || appHost->height() <= 0) {
    return;
  }
  if (skSurface == nullptr) {
    printf("skSurface is nullptr\n");
    return;
  }
  auto canvas = skSurface->getCanvas();
  canvas->clear({0.87f, 0.87f, 0.87f, 1.0f});
  auto numBenches = benchmark::Bench::Count();
  auto index = (drawIndex % numBenches);
  auto bench = benchmark::Bench::GetByIndex(index);
  bench->draw(canvas, appHost.get());
  auto particleBench = static_cast<ParticleBench*>(bench);
  if (!showPerfDataFlag) {
    updatePerfInfo(particleBench->getPerfData());
  }
  skContext->flushAndSubmit(skSurface.get(), static_cast<GrSyncCpu>(true));
  auto drawTime = benchmark::Clock::Now() - currentTime;
  appHost->recordFrame(drawTime);
}

void SkiaView::restartDraw() const {
  if (appHost) {
    appHost->resetFrames();
  }
}

void SkiaView::updatePerfInfo(const PerfData& data) const {
  auto jsWindow = emscripten::val::global("window");
  if (!jsWindow.hasOwnProperty("updatePerfInfo")) {
    return;
  }
  static int64_t lastFlushTime = -1;
  const auto currentTime = Clock::Now();
  if (lastFlushTime == -1) {
    lastFlushTime = currentTime;
  }
  if (data.fps > 0.0f) {
    if (const auto flushInterval = currentTime - lastFlushTime; flushInterval > FLUSH_INTERVAL) {
      const auto bench = getBenchByIndex();
      jsWindow.call<void>("updatePerfInfo", data.fps, data.drawTime, data.drawCount,
                          bench->isMaxDrawCountReached());
      lastFlushTime = currentTime - (flushInterval % FLUSH_INTERVAL);
    }
  }
}

void SkiaView::updateDrawParam(const DrawParam& drawParam) const {
  ParticleBench::SetInitDrawCount(drawParam.startCount);
  ParticleBench::SetStepDrawCount(drawParam.stepCount);
  ParticleBench::SetMaxDrawCount(drawParam.maxCount);
  ParticleBench::SetTargetFPS(drawParam.minFPS);
  appHost->resetFrames();
}

void SkiaView::updateGraphicType(int type) {
  drawIndex = type;
  appHost->resetFrames();
}

ParticleBench* SkiaView::getBenchByIndex() const {
  const auto numBenches = benchmark::Bench::Count();
  const auto index = (drawIndex % numBenches);
  const auto bench = benchmark::Bench::GetByIndex(index);
  return static_cast<ParticleBench*>(bench);
}

void SkiaView::showPerfData(bool status) {
  showPerfDataFlag = status;
}

void SkiaView::setAntiAlias(bool aa) {
  ParticleBench::SetAntiAlias(aa);
  appHost->resetFrames();
}

}  // namespace benchmark
int main() {
  return 0;
}