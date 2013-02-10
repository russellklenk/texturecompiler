# texturecompiler #

Implements Content.js data compiler that takes images in common formats such as PNG, JPEG, TGA, BMP, PSD, GIF, HDR or PIC and splits the data out into two target files. The first is a binary file (.pixels extension) containing the raw interleaved image channel data. The second is a JSON file specifying header information such as format, target, and dimensions and byte offsets for each level in the mip-chain.

This data compiler uses Sean Barrett's excellent stb_image for loading images, the latest version of which is available at:

 * http://nothings.org/stb_image.c

Much of the image processing functionality is based on Ignacio Castaño/NVIDIA Texture Tools public-domain source code, and the ideas presented on Jonathan Blow's Inner Product articles from Game Developer Magazine, available at:

 * http://code.google.com/p/nvidia-texture-tools/
 * http://number-none.com/product/Mipmapping,%20Part%201/index.html
 * http://number-none.com/product/Mipmapping,%20Part%202/index.html


## Implemented Features ##

 * Load PNG, JPEG, TGA, BMP, PSD, GIF, HDR and PIC.
 * Output to OpenGL ES 2.0/WebGL formats:
  * RGB565
  * RGBA4444
  * RGBA5551
  * R8 (LUMINANCE or ALPHA)
  * RG8 (LUMINANCE_ALPHA)
  * RGB8
  * RGBA8
  * R16F/RG16F/RGB16F/RGBA16F (HALF_FLOAT_OES)
  * R32F/RG32F/RGB32F/RGBA32F
 * High-quality image scaling.
 * Image downsampling performed in light-linear space.
 * Automatic resizing of images to power-of-two dimensions if desired.
 * Support for non-power-of-two images.
 * Image output with pre-multiplied alpha.
 * Generation of mip-maps down to 1x1, or a fixed number of levels.


## TODOs ##

 * Investigate ringing issues with Kaiser and Lanczos filters with widths larger than 1.0 and large sample counts.
 * Implement support for offline construction of texture atlases.
 * Implement support for compressed texture formats (S3TC/DXT, PVRTC and ETC).
 * Add additional validations beyond the ones already present.
 * Support output to container formats which have associated metadata built-in.


## Installation ##

Typically, just:

```bash
    > npm install texturecompiler
```

But if you'd like to build things manually, see below.


## Compiling the Source Code ##

To build this code on Windows, you must have the following installed:

    1. Microsoft Visual Studio 2010 (any edition will do)
       Microsoft Visual C++ 2010 Express is available for free:
       http://www.microsoft.com/visualstudio/en-us/products/2010-editions/visual-cpp-express
    2. Python 2.7.x (aka CPython)
       Available from http://www.python.org

The path to the python executable must be in your PATH. Default is C:\Python27\.

On Linux systems, typically:

    > sudo apt-get install build-essential

However, the required compilers (gcc 4.2 or later) and tools are typically installed already, since they are required to build the Node.js runtime.

On Mac OSX, you must install XCode and the XCode command-line tools to get gcc 4.2. The XCode command-line tools are now a separate optional package and require manual installation through the Preferences window of the XCode IDE, however, the required compilers and tools are typically installed already, since they are required to build Node.js.

This module contains native C/C++ source code that must first be compiled into a .node file (that's a shared object file or DLL.) This requires that node-gyp, the Node.js addon build system, be installed. To install node-gyp:

```bash
    > npm install -g node-gyp
```

This installs node-gyp as a global module, so that the node-gyp command is available from the command-line. The Node.js source package corresponding to the currently installed Node.js version is downloaded so that the correct V8 header files can be retrieved for use during the build. Once installed, run:

```bash
    > node build -f
```

This executes the build.js file, which in turn executes node-gyp and copies everything to the correct locations. All source code will be compiled into a file texture_compiler.node in the bin directory.

If you would like to invoke node-gyp manually to rebuild, do the following:

```bash
    > node-gyp configure build
```

This should compile all of the source code into a .node file located in either build/Debug/texture_compiler.node or build/Release/texture_compiler.node.


## Sample Input .texture file ##

```js
{
    "type" : "COLOR",
    "format" : "RGB",
    "target" : "TEXTURE_2D",
    "sourcePath" : "nodejs_logo.png",
    "wrapModeS" : "CLAMP_TO_EDGE",
    "wrapModeT" : "CLAMP_TO_EDGE",
    "minifyFilter" : "LINEAR",
    "magnifyFilter" : "LINEAR",
    "borderMode" : "MIRROR",
    "premultipliedAlpha" : false,
    "forcePowerOfTwo" : false,
    "flipY" : true,
    "buildMipmaps" : false,
    "levelCount" : 0,
    "targetWidth" : 0,
    "targetHeight" : 0
}
```


## Sample Output .texture file ##

```js
{
    "type": "COLOR",
    "target": "TEXTURE_2D",
    "format": "RGB",
    "dataType": "UNSIGNED_BYTE",
    "wrapS": "CLAMP_TO_EDGE",
    "wrapT": "CLAMP_TO_EDGE",
    "magFilter": "LINEAR",
    "minFilter": "LINEAR",
    "hasMipmaps": false,
    "levels": [
        {
            "width": 245,
            "height": 66,
            "byteOffset": 0,
            "byteSize": 48510
        }
    ]
}
```


Another file is output, with the .pixels (default) extension. This binary file contains the raw pixel data for all mip-levels, starting with the highest resolution (level-0). Relevant dimensions and byte offsets can be found within the objects of the 'levels' array of the texture object.


## License ##

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this software dedicate any and all copyright interest in the software to the public domain. We make this dedication for the benefit of the public at large and to the detriment of our heirs and successors. We intend this dedication to be an overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
