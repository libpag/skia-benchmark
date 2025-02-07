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

namespace benchmark {
/**
 * AppHost provides information about the current app context.
 */
class AppHost {
 public:
  /**
   * Creates an AppHost with the given width, height and density. The width and height are in
   * pixels, and the density is the ratio of physical pixels to logical pixels.
   */
  explicit AppHost(int width = 1280, int height = 720, float density = 1.0f);

  virtual ~AppHost() = default;

  /**
   * Returns the width of the screen.
   */
  int width() const {
    return _width;
  }

  /**
   * Returns the height of the screen.
   */
  int height() const {
    return _height;
  }

  /**
   * Returns the density of the screen.
   */
  float density() const {
    return _density;
  }

  /**
   * Updates the screen size and density. The default values are 1280x720 and 1.0. The width and
   * height are in pixels, and the density is the ratio of physical pixels to logical pixels.
   * Returns true if the screen size or density has changed.
   */
  bool updateScreen(int width, int height, float density);

 private:
  int _width = 1024;
  int _height = 720;
  float _density = 1.0f;
};
}  // namespace benchmark
