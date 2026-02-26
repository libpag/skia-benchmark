/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making skia-benchmark available.
//
//  Copyright (C) 2025 Tencent. All rights reserved.
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
#include "include/core/SkTextBlob.h"
#include "tools/Clock.h"

namespace benchmark {
static constexpr int64_t FLUSH_INTERVAL = 300000;
static constexpr float FPS_BACKGROUND_HEIGHT = 50.f;
static constexpr float STATUS_WIDTH = 250.f;
static constexpr float FONT_SIZE = 40.f;

static bool DrawStatusFlag = true;
static size_t InitDrawCount = 1;
static float TargetFPS = 60.0f;
static size_t MaxDrawCount = 1000000;
static size_t IncreaseStep = 1000;
static bool AntiAliasFlag = true;
static bool StrokeFlag = false;
static SkPaint::Join LineJoinType = SkPaint::Join::kMiter_Join;

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
    case GraphicType::Text:
      return "Text";
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

static SkPath CreateStar(const SkRect& rect) {
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
  return path;
}

std::vector<SkGlyphID> convertTextToGlyphs(const SkFont& font, std::string_view text) {
  size_t glyphCount =
      font.textToGlyphs(text.data(), text.size(), SkTextEncoding::kUTF8, SkSpan<SkGlyphID>());

  std::vector<SkGlyphID> glyphs(glyphCount);
  font.textToGlyphs(text.data(), text.size(), SkTextEncoding::kUTF8, SkSpan<SkGlyphID>(glyphs));

  return glyphs;
}

static GlyphRunData CreateGlyphRun(const AppHost* host,
                                   const std::vector<GraphicData>& graphicDatas) {
  constexpr std::string_view LONG_TEXT =
      R"(君子曰：学不可以已。青，取之于蓝，而青于蓝；冰，水为之，而寒于水。木直中绳，
𫐓以为轮，其曲中规。虽有槁暴，不复挺者，𫐓使之然也。故木受绳则直，金就砺则利，君子博学而日参省乎己，则知明而行无过矣。
故不登高山，不知天之高也；不临深溪，不知地之厚也；不闻先王之遗言，不知学问之大也。干、越、夷、貉之子，生而同声，
长而异俗，教使之然也。诗曰：“嗟尔君子，无恒安息。靖共尔位，好是正直。神之听之，介尔景福。”神莫大于化道，福莫长于无祸。
（此段教材无）吾尝终日而思矣，不如须臾之所学也；吾尝跂而望矣，不如登高之博见也。登高而招，臂非加长也，而见者远；顺风而呼，
声非加疾也，而闻者彰。假舆马者，非利足也，而致千里；假舟楫者，非能水也，而绝江河。君子生非异也，善假于物也。南方有鸟焉，
名曰蒙鸠，以羽为巢，而编之以发，系之苇苕，风至苕折，卵破子死。巢非不完也，所系者然也。西方有木焉，名曰射干，茎长四寸，
生于高山之上，而临百仞之渊，木茎非能长也，所立者然也。蓬生麻中，不扶而直；白沙在涅，与之俱黑。兰槐之根是为芷，其渐之滫，
君子不近，庶人不服。其质非不美也，所渐者然也。)";
  auto font = SkFont(host->getTypeFace("default"), 10.f * host->density());
  std::vector<SkGlyphID> sourceGlyphIDs = convertTextToGlyphs(font, LONG_TEXT);
  if (sourceGlyphIDs.empty()) {
    return {};
  }
  const auto totalCount = graphicDatas.size();
  const auto sourceGlyphCount = sourceGlyphIDs.size();
  std::vector<SkGlyphID> glyphIDs = {};
  std::vector<SkPoint> positions = {};
  glyphIDs.reserve(totalCount);
  positions.reserve(totalCount);
  for (size_t i = 0; i < totalCount; ++i) {
    glyphIDs.push_back(sourceGlyphIDs[i % sourceGlyphCount]);
    positions.push_back(SkPoint::Make(graphicDatas[i].rect.left(), graphicDatas[i].rect.top()));
  }
  return {font, std::move(glyphIDs), std::move(positions)};
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
  drawCount = InitDrawCount;
  maxDrawCountReached = false;
  textSpawnedCount = 0;
  perfData = {};
  fpsFont = SkFont(host->getTypeFace("default"), 40 * host->density());

  for (auto i = 0; i < 3; ++i) {
    SkColor4f color = SkColors::kBlack;
    color[i] = 1.f;
    paints[i].setColor4f(color);
    paints[i].setAntiAlias(AntiAliasFlag);
    if (StrokeFlag) {
      paints[i].setStyle(SkPaint::Style::kStroke_Style);
      paints[i].setStrokeWidth(2.0f);
      paints[i].setStrokeJoin(LineJoinType);
    } else {
      paints[i].setStyle(SkPaint::Style::kFill_Style);
    }
  }
  // Soft purple for text
  textPaint.setColor({0.6f, 0.5f, 0.7f, 1.0f});
  startRect = SkRect::MakeWH(20.f * host->density(), 20.f * host->density());
  graphics.resize(MaxDrawCount);
  std::mt19937 rectRng(18);
  std::mt19937 speedRng(36);
  std::uniform_real_distribution<float> rectDistribution(0, 1);
  std::uniform_real_distribution<float> speedDistribution(-1, 1);
  for (size_t i = 0; i < MaxDrawCount; i++) {
    const auto size = (4.f + rectDistribution(rectRng) * 10.f) * host->density();
    auto& graphic = graphics[i];
    if (graphicType == GraphicType::Oval) {
      graphic.rect.setXYWH(-size, -size, size, 0.8f * size);
    } else {
      graphic.rect.setXYWH(-size, -size, size, size);
    }
    graphic.speedX = speedDistribution(speedRng) * 5.0f;
    graphic.speedY = speedDistribution(speedRng) * 5.0f;
  }
  if (graphicType == GraphicType::Star) {
    paths.resize(MaxDrawCount);
    for (size_t i = 0; i < MaxDrawCount; i++) {
      paths[i] = CreateStar(graphics[i].rect);
    }
  }
  if (graphicType == GraphicType::Text) {
    glyphRun = CreateGlyphRun(host, graphics);
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
      if (step < 1) {
        step = 1;
      }
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
  SkRect respawnBounds{0, 0, width, height};
  const auto isTextType = graphicType == GraphicType::Text;
  if (isTextType) {
    // Expand bounds for text clipping test
    respawnBounds.outset(width, height);
  }
  for (size_t i = 0; i < drawCount; i++) {
    auto& graphic = graphics[i];
    auto& rect = graphic.rect;
    bool shouldRespawn;
    if (isTextType) {
      // For Text: new particles use screen rect to respawn from mouse position,
      // already spawned particles use expanded border for clipping test
      auto isNewParticle = i >= textSpawnedCount;
      shouldRespawn = isNewParticle ? !SkRect::Intersects(rect, screenRect)
                                    : !SkRect::Intersects(rect, respawnBounds);
    } else {
      // For other types: simply check screen border
      shouldRespawn = !SkRect::Intersects(rect, respawnBounds);
    }
    if (shouldRespawn) {
      auto offsetX = rect.width() * 0.5f;
      auto offsetY = rect.height() * 0.5f;
      rect.offsetTo(startX - offsetX, startY - offsetY);
    } else {
      rect.offset(graphic.speedX, graphic.speedY);
    }
    if (isTextType) {
      glyphRun.positions[i].set(rect.left(), rect.top());
    }
  }
  if (isTextType) {
    textSpawnedCount = drawCount;
  }
}

void ParticleBench::DrawRects(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    canvas->drawRect(graphics[i].rect, paints[i % 3]);
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
  canvas->resetMatrix();
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
    auto& graphic = graphics[i];
    auto& rect = graphic.rect;
    canvas->drawCircle(rect.centerX(), rect.centerY(), rect.width() * 0.5f, paints[i % 3]);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawRRect(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    auto& graphic = graphics[i];
    auto& rect = graphic.rect;
    const float radius = rect.width() * 0.25f;
    canvas->drawRoundRect(rect, radius, radius, paints[i % 3]);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawOval(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    auto& graphic = graphics[i];
    auto& rect = graphic.rect;
    canvas->drawOval(rect, paints[i % 3]);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawStar(SkCanvas* canvas) const {
  for (size_t i = 0; i < drawCount; i++) {
    auto& graphic = graphics[i];
    canvas->setMatrix(SkMatrix::Translate(graphic.rect.centerX(), graphic.rect.centerY()));
    auto& paint = paints[i % 3];
    canvas->drawPath(paths[i], paint);
  }
  SkPaint paint;
  paint.setColor4f(SkColors::kWhite);
  canvas->drawRect(startRect, paint);
}

void ParticleBench::DrawText(SkCanvas* canvas) const {
  auto subGlyphIDs = SkSpan(glyphRun.glyphs.data(), drawCount);
  auto subPositions = SkSpan(glyphRun.positions.data(), drawCount);
  auto textBlob = SkTextBlob::MakeFromPosGlyphs(subGlyphIDs, subPositions, glyphRun.font);
  canvas->drawTextBlob(textBlob, 0.f, 0.f, textPaint);

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
    case GraphicType::Text:
      DrawText(canvas);
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
  InitDrawCount = std::max(static_cast<size_t>(1), count);
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

void ParticleBench::SetAntiAlias(bool aa) {
  AntiAliasFlag = aa;
}

void ParticleBench::SetStroke(bool stroke) {
  StrokeFlag = stroke;
}

}  // namespace benchmark
