## Introduction
A benchmark project for the [Skia](https://skia.org/) graphics library. It is used to test the 
performance of the Skia graphics library on different platforms.

There is also a related project, [tgfx-benchmark](https://github.com/libpag/tgfx-benchmark), which
tests the performance of the [TGFX](https://github.com/Tencent/tgfx) graphics library using the same
benchmark cases.

You can also run the benchmark online at [tgfx.org](https://tgfx.org/benchmark) to compare the 
performance of TGFX and Skia on the web platform.

## Build Prerequisites

Here are the minimum tools needed to build skia-benchmark on different platforms:

- Xcode 11.0+
- Visual Studio 2019+
- NodeJS 14.14.0+
- Ninja 1.9.0+
- CMake 3.13.0+
- Python3 3.8+


Please note the following additional notices:

- Ensure you have installed the **[Desktop development with C++]** and **[Universal Windows Platform development]** components for VS2019.
- It is **highly recommended** to use the **latest version of CMake**. Many older versions of CMake may have various bugs across different platforms.

## Dependencies

skia-benchmark uses the [**depsync**](https://github.com/domchen/depsync) tool to manage third-party dependencies.

**For macOS platform：**

Run this script from the root of the project:

```
./sync_deps.sh
```

This script will automatically install the necessary tools and sync all third-party repositories.

**For other platforms:**

First, ensure you have the latest version of Node.js installed (you may need to restart your computer afterward).
Then, run the following command to install the depsync tool:

```
npm install -g depsync
```

Then, run `depsync` in the project's root directory.

```
depsync
```

You might need to enter your Git account and password during synchronization. Make sure you’ve
enabled the `git-credential-store` so that `CMakeLists.txt` can automatically trigger synchronization
next time.

## Getting Started

Before building the projects, please carefully follow the instructions in the
[**Build Prerequisites**](https://github.com/libpag/skia-benchmark?tab=readme-ov-file#build-prerequisites)
and [**Dependencies**](https://github.com/libpag/skia-benchmark?tab=readme-ov-file#dependencies) sections.
These will guide you through the necessary steps to set up your development environment.

### macOS
To get started, open the root directory in CLion. Then, go to `File->Settings` and navigate to
`Build, Execution, Deployment->CMake`. Add a new profile with the Release build type. Now you are
ready to build and run the Benchmark target.

If you prefer using XCode IDE, go to the root directory, run the following command or double-click
it:

```
./gen_mac
```

This will generate a project for the native architecture, such as `arm64` for Apple Silicon Macs or
`x64` for Intel Macs. If you want to generate a project for a specific architecture, use the `-a`
option, for example:

```
./gen_mac -a x64
```    

finally, open Xcode and launch the `mac/Skia-Benchmark.xcodeproj`. You are all set!

### Windows

**Note:** If you're using a dedicated graphics card like NVIDIA or AMD, make sure to update your
graphics driver to the latest version to ensure the test results are accurate.

To get started, open the root directory in CLion. Then, go to `File->Settings` and navigate to
`Build, Execution, Deployment->ToolChains`. Set the toolchain to `Visual Studio` with `amd64`
 architecture. It's also recommended to use the `Ninja` generator for CMake to
speed up the build process. You can set this in `Build, Execution, Deployment->CMake` by choosing
`Ninja` in the `Generator` row. Once done, build and run the `Benchmark` target using the Release
configuration.

If you prefer using Visual Studio IDE, open the `x64 Native Tools Command Prompt for VS 2019` and
run the following command in the root directory:

```
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_CONFIGURATION_TYPES="Debug" -B ./win/Debug-x64
```

This will generate a project for the `x64` architecture with the `Debug` configuration. To generate
a project for the same architecture with the `Release` configuration, run the following command:

```
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_CONFIGURATION_TYPES="Release" -B ./win/Release-x64
```

Finally, open the `Skia-Benchmark.sln` file in the `win/Debug-x64/` or `win/Release-x64/` directory, and set
the `Benchmark` project as the startup project. You are all set!

### Web
To get started, go to the `web/` directory and run the following command to install the necessary
node modules:

```
npm install
```

Then, run the following command to build the project:

```
npm run build
```

Finally, run the following command to start the development server:

```
npm run server
```
This will open [http://localhost:8071/index.html](http://localhost:8071/index.html)
in your default browser. You can also open it manually to view the demo.

The above commands build and run a multithreaded version.

>**⚠️** In the multithreaded version, if you modify the filename of the compiled output benchmark.js, you need to search for
> the keyword "benchmark.js" within the benchmark.js file and replace all occurrences of "benchmark.js" with the new filename.
> Failure to do this will result in the program failing to run. Here's an example of how to modify it:

Before modification:

```js
    // filename: benchmark.js
    var worker = new Worker(new URL("benchmark.js", import.meta.url), {
     type: "module",
     name: "em-pthread"
    });
```

After modification:

```js
    // filename: benchmark-test.js
    var worker = new Worker(new URL("benchmark-test.js", import.meta.url), {
     type: "module",
     name: "em-pthread"
    });
```

To build a single-threaded version, just add the suffix ":st" to each command. For example:

```
npm run build:st

npm run serser:st
``` 
This will open [http://localhost:8071/index-st.html](http://localhost:8071/index-st.html)
in your default browser. You can also open it manually to view the demo.

