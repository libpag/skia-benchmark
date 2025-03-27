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

#include "base/Bench.h"
#include <iostream>
#include <unordered_map>
#include "benchmark/ParticleBench.h"

namespace benchmark {
static std::vector<Bench*> drawers = {
    new ParticleBench(GraphicType::Rect), new ParticleBench(GraphicType::Circle),
    new ParticleBench(GraphicType::Oval), new ParticleBench(GraphicType::RRect),
    new ParticleBench(GraphicType::ComplexGraphic)};

static std::vector<std::string> GetDrawerNames() {
  std::vector<std::string> names;
  for (const auto& drawer : drawers) {
    names.push_back(drawer->name());
  }
  return names;
}

static std::unordered_map<std::string, Bench*> GetDrawerMap() {
  std::unordered_map<std::string, Bench*> map;
  for (const auto& drawer : drawers) {
    map[drawer->name()] = drawer;
  }
  return map;
}

int Bench::Count() {
  return static_cast<int>(drawers.size());
}

const std::vector<std::string>& Bench::Names() {
  static auto names = GetDrawerNames();
  return names;
}

Bench* Bench::GetByIndex(int index) {
  if (index < 0 || index >= Count()) {
    return nullptr;
  }
  return drawers[static_cast<size_t>(index)];
}

Bench* Bench::GetByName(const std::string& name) {
  static auto drawerMap = GetDrawerMap();
  auto it = drawerMap.find(name);
  if (it == drawerMap.end()) {
    return nullptr;
  }
  return it->second;
}

Bench::Bench(std::string name) : _name(std::move(name)) {
}

void Bench::draw(SkCanvas* canvas, const AppHost* host) {
  if (canvas == nullptr) {
    std::cout << "Drawer::draw() canvas is nullptr!\n";
    return;
  }
  if (host == nullptr) {
    std::cout << "Drawer::draw() appHost is nullptr!\n";
    return;
  }
  canvas->save();
  onDraw(canvas, host);
  canvas->restore();
}
}  // namespace benchmark