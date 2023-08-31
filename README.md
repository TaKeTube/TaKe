# TaKe

A path tracer based on [UCSD CSE 168 renderer Torrey](https://github.com/BachiLi/torrey_public) written in C++ 17. TaKe (竹, たけ) means bamboo in Japanese. It is also a radical in the main author's Chinese last name.

Except the scene parser and some basic mathematical objects classes, **this renderer is implemented from scratch.**

# Build

All the dependencies are included. Use CMake to build.

```
git clone https://github.com/TaKeTube/TaKe.git
mkdir build
cd build
cmake ..
```

It requires compilers that support C++17 (gcc version >= 8, clang version >= 7, Apple Clang version >= 11.0, MSVC version >= 19.14).

# Scenes

You could also download the scenes from the following Google drive link: 
[https://drive.google.com/file/d/1SrGaw6AbyfhPs1NAuRjxSEQmf34DPKhm/view?usp=sharing](https://drive.google.com/file/d/1SrGaw6AbyfhPs1NAuRjxSEQmf34DPKhm/view?usp=sharing).

# Run

Try 

```
cd build
./take scenes/cbox/cbox.xml
```

This will generate an image "image.exr".

To view the image, use [hdrview](https://github.com/wkjarosz/hdrview), or [tev](https://github.com/Tom94/tev).

# Acknowledgement

The renderer is based on [UCSD CSE 168 renderer Torrey](https://github.com/BachiLi/torrey_public). 

It is heavily inspired by [pbrt](https://pbr-book.org/), [mitsuba](http://www.mitsuba-renderer.org/index_old.html), [the ray tracing series](https://raytracing.github.io/), and [darts](https://cs87-dartmouth.github.io/Fall2022/darts-overview.html).

We use [pugixml](https://pugixml.org/) to parse XML files.

We use [stb_image](https://github.com/nothings/stb) and [tinyexr](https://github.com/syoyo/tinyexr) for reading & writing images.

We use [miniz](https://github.com/richgel999/miniz) for compression & decompression.

We use [tinyply](https://github.com/ddiakopoulos/tinyply) for parsing PLY files.

Many scenes in the scenes folder are downloaded from:

- [http://www.mitsuba-renderer.org/download.html](http://www.mitsuba-renderer.org/download.html)
- [https://benedikt-bitterli.me/resources/](https://benedikt-bitterli.me/resources/)
- [https://casual-effects.com/data/](https://casual-effects.com/data/)
