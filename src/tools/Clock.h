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

#pragma once

#include <chrono>

namespace benchmark {
/**
 * A utility class used to compute relative time.
 */
class Clock {
 public:
  /**
    * Returns the number of microseconds since the benchmark runtime was initialized.
    */
  static int64_t Now();

  /**
   * Creates a new Clock object.
   */
  Clock();

  /**
  * Resets the start time of the Clock to Now().
  */
  void reset();

  /**
   * Returns the number of microseconds elapsed since the Clock was initialized or reset.
   */
  int64_t elapsedTime() const;

 private:
  int64_t startTime = 0;
};
}  // namespace benchmark