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

#include "clock.h"

namespace benchmark {
int64_t Clock::Now() {
  static const auto START_TIME = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - START_TIME);
  return static_cast<int64_t>(us.count());
}

Clock::Clock() {
  startTime = Now();
}

void Clock::reset() {
  startTime = Now();
}

int64_t Clock::elapsedTime() const {
  return Now() - startTime;
}

}  //namespace benchmark