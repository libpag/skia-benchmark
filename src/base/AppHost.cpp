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

#include "AppHost.h"
#include <iostream>
#include "tools/Clock.h"

namespace benchmark {
AppHost::AppHost(int width, int height, float density)
    : _width(width), _height(height), _density(density) {
}

sk_sp<SkTypeface> AppHost::getTypeFace(const std::string& name) const {
  auto result = typefaces.find(name);
  if (result != typefaces.end()) {
    return result->second;
  }
  return nullptr;
}

bool AppHost::updateScreen(int width, int height, float density) {
  if (width <= 0 || height <= 0) {
    std::cout << "AppHost::updateScreen() width or height is invalid!\n";
    return false;
  }
  if (density < 1.0f) {
    std::cout << "AppHost::updateScreen() density is invalid!\n";
    return false;
  }
  if (width == _width && height == _height && density == _density) {
    return false;
  }
  _width = width;
  _height = height;
  _density = density;
  return true;
}

void AppHost::addTypeface(const std::string& name, sk_sp<SkTypeface> typeface) {
  if (name.empty()) {
    std::cout << "AppHost::addTypeface() name is empty!\n";
    return;
  }
  if (typeface == nullptr) {
    std::cout << "AppHost::addTypeface() typeface is nullptr!\n";
    return;
  }
  if (typefaces.count(name) > 0) {
    std::cout << "AppHost::addTypeface() typeface with name " << name << " aleardy exists!\n";
    return;
  }
  typefaces[name] = std::move(typeface);
}

float AppHost::currentFPS() const {
  if (fpsTimeStamps.size() < 60) {
    return 0.0f;
  }
  auto duration = fpsTimeStamps.back() - fpsTimeStamps.front();
  return static_cast<float>((fpsTimeStamps.size() - 1) * 1000000) / static_cast<float>(duration);
}

int64_t AppHost::averageDrawTime() const {
  if (drawTimes.empty()) {
    return 0;
  }
  int64_t total = 0;
  for (auto& drawTime : drawTimes) {
    total += drawTime;
  }
  return total / static_cast<int64_t>(drawTimes.size());
}

void AppHost::recordFrame(int64_t drawTime) {
  auto currentTime = Clock::Now();
  fpsTimeStamps.push_back(currentTime);
  while (fpsTimeStamps.size() > 60) {
    fpsTimeStamps.pop_front();
  }
  drawTimes.push_back(drawTime);
  while (drawTimes.size() > 60) {
    drawTimes.pop_front();
  }
}

void AppHost::resetFrames() {
  fpsTimeStamps.clear();
  drawTimes.clear();
}

}  // namespace benchmark
