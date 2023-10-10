# 🎍 TaKe Renderer

A path tracer based on [UCSD CSE 168 renderer Torrey](https://github.com/BachiLi/torrey_public) written in C++ 17. TaKe (竹, たけ) means bamboo in Japanese. It is also a radical in the main author's Chinese last name 管.

This renderer is implemented **from scratch** except for the scene parser and some basic mathematical object classes.

## Key Features

- **Monte Carlo path tracing**
- **Bounding volume hierarchies (BVHs) acceleration**
- **Textures**
- **Microfacet BRDFs**
- **Multiple importance sampling (both One-sample model and Multiple-sample model)**
- **Importance sampled environment light / IBL**
- **Multithreading acceleration**
- **Mitsuba scene format parser**
- **std::variant based polymorphism**

Our primary objective is to seamlessly incorporate the key features from the following projects, all of which **share the same author** with the TaKe renderer:

- **Disney principled BSDF** from [LaJollaTube](https://www.notion.so/LaJollaTube-f13182165ce847088a16b02712a69625?pvs=21)
- **Heterogeneous volumetric rendering** from [LaJollaTube](https://www.notion.so/LaJollaTube-f13182165ce847088a16b02712a69625?pvs=21)
- **ReSTIR DI ([Paper Link](https://research.nvidia.com/publication/2020-07_spatiotemporal-reservoir-resampling-real-time-ray-tracing-dynamic-direct))** from [LajollaPlus](https://www.notion.so/9ec9673eb9c14b44b803a74e797f8dcd?pvs=21)
- **CUDA Acceleration** from [Torrey GPU](https://www.notion.so/53a7a9702ec343d1859a112f0f8d1d84?pvs=21)

## Build

All the dependencies are included. Use CMake to build.

```
git clone https://github.com/TaKeTube/TaKe.git
mkdir build
cd build
cmake ..
```

It requires compilers that support C++17 (gcc version >= 8, clang version >= 7, Apple Clang version >= 11.0, MSVC version >= 19.14).

## Scenes

You could also download the scenes from the following Google drive link: 
[https://drive.google.com/file/d/1SrGaw6AbyfhPs1NAuRjxSEQmf34DPKhm/view?usp=sharing](https://drive.google.com/file/d/1SrGaw6AbyfhPs1NAuRjxSEQmf34DPKhm/view?usp=sharing).

## Run

Try 

```
cd build
./take scenes/cbox/cbox.xml
```

This will generate an image "image.exr".

To view the image, use [hdrview](https://github.com/wkjarosz/hdrview), or [tev](https://github.com/Tom94/tev).

## Gallery

Below are some images rendered by the above renderers:

<img src="gallery\room.png" alt="room" style="zoom: 50%;" />

Self-designed scene made using assets from https://polyhaven.com/

**Disney principled BSDF**

<img src="gallery\array.png" alt="array" style="zoom: 67%;" />

<img src="gallery\lorenz_attractor.png" alt="lorenz_attractor" style="zoom:50%;" />

Self-designed scene procedurally generated by Houdini

**Heterogeneous volumetric rendering**

![colored_smoke](.\gallery\colored_smoke.png)

![box_vol](.\gallery\box_vol.png)

<img src="gallery\hand.png" alt="hand" style="zoom:50%;" />

Self-designed scene

**ReSTIR DI**

<img src="gallery\restir.png" alt="restir" style="zoom: 25%;" />

<img src="gallery\restir_scene.png" alt="restir_scene" style="zoom: 50%;" />

Self-designed scene

## Acknowledgement

The renderer is based on Prof. [Tzu-Mao Li](https://cseweb.ucsd.edu/~tzli/)'s [UCSD CSE 168 renderer Torrey](https://github.com/BachiLi/torrey_public).

It is heavily inspired by [Lajolla](https://github.com/BachiLi/lajolla_public), [pbrt](https://pbr-book.org/), [mitsuba](http://www.mitsuba-renderer.org/index_old.html), [the ray tracing series](https://raytracing.github.io/), and [darts](https://cs87-dartmouth.github.io/Fall2022/darts-overview.html).

We use [pugixml](https://pugixml.org/) to parse XML files.

We use [stb_image](https://github.com/nothings/stb) and [tinyexr](https://github.com/syoyo/tinyexr) for reading & writing images.

We use [miniz](https://github.com/richgel999/miniz) for compression & decompression.

We use [tinyply](https://github.com/ddiakopoulos/tinyply) for parsing PLY files.

Many scenes in the scenes folder are downloaded from:

- [http://www.mitsuba-renderer.org/download.html](http://www.mitsuba-renderer.org/download.html)
- [https://benedikt-bitterli.me/resources/](https://benedikt-bitterli.me/resources/)
- [https://casual-effects.com/data/](https://casual-effects.com/data/)
