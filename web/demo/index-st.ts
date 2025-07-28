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


import Benchmark from './wasm/Benchmark';
import {SkiaView, startDraw, updateSize, setCanvasDefaultSize, setupCoordinateConversion} from "./common";

let skiaView: SkiaView;

if (typeof window !== 'undefined') {
    window.onload = async () => {
        try {
            setupCoordinateConversion('benchmark');
            const module = await Benchmark({locateFile: (file: string) => './wasm/' + file});
            if (module === undefined) {
                throw new Error("Benchmark init failed. Please check the .wasm file path!.");
            }
            skiaView = module.SkiaView.MakeFrom('#benchmark');
            var fontPath = "../../resources/font/SFNSRounded.ttf";
            const fontBuffer = await fetch(fontPath).then((response) => response.arrayBuffer());
            const fontUIntArray = new Uint8Array(fontBuffer);
            var emojiFontPath = "../../resources/font/NotoColorEmoji.ttf";
            const emojiFontBuffer = await fetch(emojiFontPath).then((response) => response.arrayBuffer());
            const emojiFontUIntArray = new Uint8Array(emojiFontBuffer);
            skiaView.registerFonts(fontUIntArray, emojiFontUIntArray);
            setCanvasDefaultSize(skiaView);
            startDraw(skiaView);
        } catch (error) {
            console.error(error);
            throw new Error("Benchmark init failed. Please check the .wasm file path!.");
        }
    };

    window.onresize = () => {
        window.setTimeout(() => updateSize(skiaView), 300);
    };
}