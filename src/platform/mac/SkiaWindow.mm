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

#import "SkiaWindow.h"
#import <QuartzCore/CADisplayLink.h>
#include <cmath>
#include <filesystem>
#include "base/AppHost.h"
#include "base/Bench.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrbackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "platform/mac/GLWindowContext_mac.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "window_context/DisplayParams.h"

@implementation SkiaWindow {
  NSWindow* window;
  NSView* view;
  std::unique_ptr<skiawindow::WindowContext> windowContext;
  std::unique_ptr<benchmark::AppHost> appHost;
  int drawIndex;
  CVDisplayLinkRef displayLink;
}

- (void)dealloc {
  if (displayLink != nil) {
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
  }
  [window release];
  [view release];
  [super dealloc];
}

- (void)windowWillClose:(NSNotification*)notification {
  [NSApp terminate:self];
}

- (void)windowDidResize:(NSNotification*)notification {
  [self updateSize];
}

static CVReturn displayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                                    CVOptionFlags, CVOptionFlags*, void* context) {
  auto self = (SkiaWindow*)context;
  dispatch_async(dispatch_get_main_queue(), ^{
    [self redraw];
  });
  return kCVReturnSuccess;
}

- (void)open {
  NSRect frame = NSMakeRect(0, 0, 1024, 720);
  NSWindowStyleMask styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
  window = [[NSWindow alloc] initWithContentRect:frame
                                       styleMask:styleMask
                                         backing:NSBackingStoreBuffered
                                           defer:NO];
  [window setTitle:@"Skia Benchmark"];
  [window setDelegate:self];
  view = [[NSView alloc] initWithFrame:frame];

  skiawindow::MacWindowInfo info;
  info.mainView = view;
  windowContext = skiawindow::MakeGLForMac(info, skiawindow::DisplayParams());

  [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [view addGestureRecognizer:[[NSClickGestureRecognizer alloc]
                                 initWithTarget:self
                                         action:@selector(handleClick:)]];
  [window setContentView:view];
  [window center];
  [window makeKeyAndOrderFront:nil];

  [self updateSize];
  drawIndex = 0;
  if (@available(macOS 14, *)) {
    displayLink = nil;
    CADisplayLink* caDisplayLink = [view displayLinkWithTarget:self selector:@selector(redraw)];
    [caDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
  } else {
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &displayLinkCallback, self);
    CVDisplayLinkStart(displayLink);
  }
}

- (void)handleClick:(NSClickGestureRecognizer*)gestureRecognizer {
  drawIndex++;
}

- (void)updateSize {
  CGSize size = [view convertSizeToBacking:view.bounds.size];
  auto width = static_cast<int>(round(size.width));
  auto height = static_cast<int>(round(size.height));
  if (appHost == nullptr) {
    appHost = std::make_unique<benchmark::AppHost>();
    auto typeface = SkFontMgr::RefDefault()->matchFamilyStyle("Helvetica", SkFontStyle());
    appHost->addTypeface("default", typeface);
  }
  auto contentScale = static_cast<float>(size.height / view.bounds.size.height);
  auto sizeChanged = appHost->updateScreen(width, height, contentScale);
  if (sizeChanged && windowContext != nullptr) {
    windowContext->resize(width, height);
  }
}

- (void)redraw {
  if (appHost->width() <= 0 || appHost->height() <= 0) {
    return;
  }

  if (windowContext == nullptr) {
    skiawindow::MacWindowInfo info;
    info.mainView = [window contentView];
    windowContext = MakeGLForMac(info, skiawindow::DisplayParams());
  }

  if (windowContext == nullptr) {
    return;
  }

  sk_sp<SkSurface> surface = windowContext->getBackbufferSurface();
  if (surface == nullptr) {
    return;
  }
  auto canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);
  auto numBenches = benchmark::Bench::Count();
  auto index = (drawIndex % numBenches);
  auto bench = benchmark::Bench::GetByIndex(index);
  bench->draw(canvas, appHost.get());
  if (auto dContext = windowContext->directContext()) {
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
  }

  windowContext->swapBuffers();
}
@end
