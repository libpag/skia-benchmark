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

#include "ParticleBench.h"
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>
#include "include/core/SkPath.h"
#include "tools/Clock.h"

namespace benchmark {
static constexpr int64_t FLUSH_INTERVAL = 300000;
static constexpr float FPS_BACKGROUND_HEIGHT = 50.f;
static constexpr float STATUS_WIDTH = 250.f;
static constexpr float FONT_SIZE = 40.f;

static bool DrawStatusFlag = true;
static size_t InitDrawCount = 0;
static float TargetFPS = 60.0f;
static size_t MaxDrawCount = 1000000;
static size_t IncreaseStep = 1000;

static std::string ToString(GraphicType type) {
  switch (type) {
    case GraphicType::Rect:
      return "Rect";
    case GraphicType::Circle:
      return "Circle";
    case GraphicType::RRect:
      return "RRect";
    case GraphicType::Oval:
      return "Oval";
    case GraphicType::Star:
      return "Star";
    default:
      return "Unknown";
  }
}

ParticleBench::ParticleBench(GraphicType type)
    : Bench("ParticleBench-" + ToString(type)), graphicType(type) {
}

void ParticleBench::onDraw(SkCanvas* canvas, const AppHost* host) {
  Init(host);
  AnimateRects(host);
  DrawGraphics(canvas);
  DrawStatus(canvas, host);
}

void ParticleBench::Init(const AppHost* host) {
  auto hostWidth = static_cast<float>(host->width());
  auto hostHeight = static_cast<float>(host->height());
  if (width == hostWidth && height == hostHeight && !host->isFirstFrame()) {
    return;
  }
  width = hostWidth;
  height = hostHeight;
  status = {};
  drawCount = std::max(static_cast<size_t>(1), InitDrawCount);
  maxDrawCountReached = false;
  perfData = {};
  fpsFont = SkFont(host->getTypeFace("default"), 40 * host->density());

  for (auto i = 0; i < 3; ++i) {
    SkColor4f color = SkColors::kBlack;
    color[i] = 1.f;
    paints[i].setColor4f(color);
    paints->setAntiAlias(false);
  }
  startRect = SkRect::MakeWH(25.f * host->density(), 25.f * host->density());
  rects.resize(MaxDrawCount);
  std::mt19937 rectRng(18);
  std::mt19937 speedRng(36);
  std::uniform_real_distribution<float> rectDistribution(0, 1);
  std::uniform_real_distribution<float> speedDistribution(-1, 1);
  for (size_t i = 0; i < MaxDrawCount; i++) {
    const auto size = (5.f + rectDistribution(rectRng) * 20.f) * host->density();
    auto& item = rects[i];
    if (graphicType == GraphicType::Oval) {
      const float ratio = 0.5f + rectDistribution(rectRng);
      item.rect.setXYWH(-size, -size, size, ratio * size);
    } else {
      item.rect.setXYWH(-size, -size, size, size);
    }
    item.speedX = speedDistribution(speedRng) * 5.0f;
    item.speedY = speedDistribution(speedRng) * 5.0f;
  }
}

void ParticleBench::AnimateRects(const AppHost* host) {
  if (!maxDrawCountReached) {
    auto halfDrawInterval = static_cast<int64_t>(500000 / TargetFPS);
    auto drawTime = host->lastDrawTime();
    auto idleTime = halfDrawInterval * 2 - drawTime;
    if (idleTime > 0) {
      auto factor = static_cast<double>(idleTime > halfDrawInterval ? drawTime : idleTime) /
                    static_cast<double>(halfDrawInterval);
      auto step = static_cast<int64_t>(IncreaseStep * factor);
      drawCount = std::min(drawCount + static_cast<size_t>(step), MaxDrawCount);
    }
  }
  auto startX = host->mouseX();
  auto startY = host->mouseY();
  auto screenRect = SkRect::MakeWH(width, height);
  if (!screenRect.contains(startX, startY)) {
    startX = screenRect.centerX();
    startY = screenRect.centerY();
  }
  startRect.offsetTo(startX - startRect.width() * 0.5f, startY - startRect.height() * 0.5f);
  for (size_t i = 0; i < drawCount; i++) {
    auto& item = rects[i];
    auto& rect = item.rect;
    if (rect.right() <= 0 || rect.left() >= width || rect.bottom() <= 0 || rect.top() >= height) {
      auto offsetX = rect.width() * 0.5f;
      auto offsetY = rect.height() * 0.5f;
      rect.offsetTo(startX - offsetX, startY - offsetY);
    } else {
      rect.offset(item.speedX, item.speedY);
    }
  }
}

void ParticleBench::DrawRects(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    canvas->drawRect(rects[i].rect, paints[i % 3]);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawStatus(SkCanvas* canvas, const AppHost* host) {
  auto currentTime = Clock::Now();
  if (lastFlushTime == -1) {
    lastFlushTime = currentTime;
  }
  auto flushInterval = currentTime - lastFlushTime;
  if (flushInterval > FLUSH_INTERVAL) {
    auto fps = host->currentFPS();
    if (fps > 0.0f) {
      currentFPS = fps;
      auto drawTime = host->averageDrawTime();
      if (!maxDrawCountReached) {
        if ((currentFPS < TargetFPS - 0.5f &&
             drawTime > static_cast<int64_t>(1000000 / TargetFPS) - 2000) ||
            drawCount >= MaxDrawCount) {
          maxDrawCountReached = true;
        }
      }
      status.clear();
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(1) << currentFPS;
      status.push_back("FPS: " + oss.str());
      oss.str("");
      oss << std::fixed << std::setprecision(1) << static_cast<float>(drawTime) / 1000.f;
      status.push_back("Time: " + oss.str());
      oss.str("");
      oss << drawCount;
      auto countInfo = oss.str();
      if (maxDrawCountReached) {
        countInfo = "[" + countInfo + "]";
      }
      status.push_back("Count: " + countInfo);
      if (currentFPS > 59.f) {
        fpsColor = SkColors::kGreen;
      } else if (currentFPS > 29.f) {
        fpsColor = SkColor4f{1.f, 1.f, 0.f, 1.f};
      } else {
        fpsColor = SkColor4f{0.91f, 0.31f, 0.28f, 1.f};
      }
      lastFlushTime = currentTime - (flushInterval % FLUSH_INTERVAL);
    }
  }
  perfData.fps = currentFPS;
  perfData.drawTime = static_cast<float>(host->averageDrawTime()) / 1000.f;
  perfData.drawCount = drawCount;
  if (!DrawStatusFlag) {
    return;
  }
  SkPaint paint = {};
  paint.setColor4f(SkColor4f{0.32f, 0.42f, 0.62f, 0.9f});
  auto backgroundRect =
      SkRect::MakeWH(static_cast<float>(width), FPS_BACKGROUND_HEIGHT * host->density());
  canvas->drawRect(backgroundRect, paint);
  auto top = FONT_SIZE * host->density();
  paint.setColor(fpsColor);
  float left = STATUS_WIDTH * host->density() / 2;
  for (auto& line : status) {
    canvas->drawString(line.c_str(), left, top, fpsFont, paint);
    left += STATUS_WIDTH * host->density();
  }
}

void ParticleBench::DrawCircle(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    auto& item = rects[i];
    auto& rect = item.rect;
    canvas->drawCircle(rect.centerX(), rect.centerY(), rect.width() * 0.5f, paints[i % 3]);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawRRect(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    auto& item = rects[i];
    auto& rect = item.rect;
    const float radius = rect.width() * 0.2f;
    canvas->drawRoundRect(rect, radius, radius, paints[i % 3]);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawOval(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    auto& item = rects[i];
    auto& rect = item.rect;
    canvas->drawOval(rect, paints[i % 3]);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawStar(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    auto& item = rects[i];
    auto& rect = item.rect;
    SkPaint paint = paints[i % 3];
    const int points = 5;
    const float outerRadius = rect.width() * 0.5f;
    const float innerRadius = outerRadius * 0.382f;
    SkPath path;
    const float angleStep = static_cast<float>(M_PI) / points;
    const float centerX = rect.centerX();
    const float centerY = rect.centerY();
    for (int j = 0; j < points * 2; j++) {
      const float radius = (j % 2 == 0) ? outerRadius : innerRadius;
      const float angle = static_cast<float>(j) * angleStep;
      const float x = centerX + radius * std::sin(angle);
      const float y = centerY - radius * std::cos(angle);
      if (j == 0) {
        path.moveTo(x, y);
      } else {
        path.lineTo(x, y);
      }
    }
    path.close();
    canvas->drawPath(path, paint);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawGraphics(SkCanvas* canvas) const {
  switch (graphicType) {
    case GraphicType::Rect:
      DrawRects(canvas);
      break;
    case GraphicType::Circle:
      DrawCircle(canvas);
      break;
    case GraphicType::RRect:
      DrawRRect(canvas);
      break;
    case GraphicType::Oval:
      DrawOval(canvas);
      break;
    case GraphicType::Star:
      DrawStar(canvas);
      break;
    default:
      DrawRects(canvas);
      break;
  }
}

void ParticleBench::ShowPerfData(bool status) {
  DrawStatusFlag = status;
}

void ParticleBench::SetInitDrawCount(size_t count) {
  InitDrawCount = count;
}

void ParticleBench::SetMaxDrawCount(size_t count) {
  MaxDrawCount = count;
}

void ParticleBench::SetStepDrawCount(size_t count) {
  IncreaseStep = count;
}

void ParticleBench::SetTargetFPS(float fps) {
  TargetFPS = fps;
}

bool ParticleBench::isMaxDrawCountReached() const {
  return maxDrawCountReached;
}

PerfData ParticleBench::getPerfData() const {
  return perfData;
}

}  // namespace benchmark
