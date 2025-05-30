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

#pragma once

#include <deque>
#include "base/Bench.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"

namespace benchmark {

enum class GraphicType { Rect, Circle, Oval, RRect, Star };

struct GraphicData {
  SkRect rect{0, 0, 1, 1};
  float speedX;
  float speedY;
};

struct DrawParam {
  size_t startCount = 1;
  size_t stepCount = 600;
  float minFPS = 60.0f;
  size_t maxCount = 1000000;
};

struct PerfData {
  float fps = 0.0f;
  float drawTime = 0.0f;
  size_t drawCount = 0;
};

class ParticleBench : public Bench {
 public:
  ParticleBench() : Bench("ParticleBench") {
  }

  explicit ParticleBench(GraphicType type);

  static void ShowPerfData(bool status);

  static void SetInitDrawCount(size_t count);

  static void SetMaxDrawCount(size_t count);

  static void SetStepDrawCount(size_t count);

  static void SetTargetFPS(float fps);

  static void SetAntiAlias(bool aa);

  bool isMaxDrawCountReached() const;

  PerfData getPerfData() const;

 protected:
  void onDraw(SkCanvas* canvas, const AppHost* host) override;

 private:
  void Init(const AppHost* host);

  void AnimateRects(const AppHost* host);

  void DrawRects(SkCanvas* canvas) const;

  void DrawStatus(SkCanvas* canvas, const AppHost* host);

  void DrawCircle(SkCanvas* canvas) const;

  void DrawRRect(SkCanvas* canvas) const;

  void DrawOval(SkCanvas* canvas) const;

  void DrawStar(SkCanvas* canvas) const;

  void DrawGraphics(SkCanvas* canvas) const;

 private:
  float width = 0;   //appHost width
  float height = 0;  //appHost height
  float currentFPS = 0.f;
  size_t drawCount = 1;
  std::vector<GraphicData> graphics = {};
  std::vector<SkPath> paths = {};
  SkRect startRect = SkRect::MakeEmpty();
  SkPaint paints[3];  // red, green, blue solid paints
  int64_t lastFlushTime = -1;
  SkFont fpsFont = {};
  SkColor4f fpsColor = SkColors::kGreen;
  std::vector<std::string> status = {};
  GraphicType graphicType = GraphicType::Rect;
  bool maxDrawCountReached = false;
  PerfData perfData = {};
};

}  // namespace benchmark
