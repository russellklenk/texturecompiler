/*/////////////////////////////////////////////////////////////////////////////
/// @summary Implements the storage structures and routines for manipulating
/// image data in a generic image container format. This format can store 1D,
/// 2D, 3D and cubemap images, with or without mipmaps, in either integer or
/// floating-point formats, and with or without compression.
///////////////////////////////////////////////////////////////////////////80*/

/*////////////////
//   Includes   //
////////////////*/
#include <limits>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "libimage.hpp"

/*//////////////////////////
//   Using Declarations   //
//////////////////////////*/

/*//////////////////////
//   Implementation   //
//////////////////////*/

/*/////////////////////////////////////////////////////////////////////////80*/

static const float PI          = 3.14159265358979f;

/*/////////////////////////////////////////////////////////////////////////80*/

/// The coefficients for a 5x5 Sobel filter.
static const float Sobel_5x5[] =
{
    -1, -2, 0, 2, 1,
    -2, -3, 0, 3, 2,
    -3, -4, 0, 4, 3,
    -2, -3, 0, 3, 2,
    -1, -2, 0, 2, 1
};

/*/////////////////////////////////////////////////////////////////////////80*/

/// The coefficients for a 7x7 Sobel filter.
static const float Sobel_7x7[] =
{
    -1, -2, -3, 0, 3, 2, 1,
    -2, -3, -4, 0, 4, 3, 2,
    -3, -4, -5, 0, 5, 4, 3,
    -4, -5, -6, 0, 6, 5, 4,
    -3, -4, -5, 0, 5, 4, 3,
    -2, -3, -4, 0, 4, 3, 2,
    -1, -2, -3, 0, 3, 2, 1
};

/*/////////////////////////////////////////////////////////////////////////80*/

/// The coefficients for a 9x9 Sobel filter.
static const float Sobel_9x9[] =
{
    -1, -2, -3, -4, 0, 4, 3, 2, 1,
    -2, -3, -4, -5, 0, 5, 4, 3, 2,
    -3, -4, -5, -6, 0, 6, 5, 4, 3,
    -4, -5, -6, -7, 0, 7, 6, 5, 4,
    -5, -6, -7, -8, 0, 8, 7, 6, 5,
    -4, -5, -6, -7, 0, 7, 6, 5, 4,
    -3, -4, -5, -6, 0, 6, 5, 4, 3,
    -2, -3, -4, -5, 0, 5, 4, 3, 2,
    -1, -2, -3, -4, 0, 4, 3, 2, 1
};

/*/////////////////////////////////////////////////////////////////////////80*/

static inline int32_t repeat_remainder(int32_t a, size_t b)
{
    if (a >= 0) return (a % b);
    return (a + 1) % b + b - 1;
}

/*/////////////////////////////////////////////////////////////////////////80*/

int32_t image::basic_attributes(
    size_t image_count,
    size_t pixel_width,
    size_t pixel_height,
    size_t slice_count,
    size_t faces_count)
{
    int32_t attribs  = image::ATTRIBUTES_NONE;
    if (image_count  < 1) image_count     = 1; // minimum of 1 image
    if (pixel_width  < 1) pixel_width     = 1; // minimum dimension of 1 px.
    if (pixel_height < 1) pixel_height    = 1; // minimum dimension of 1 px.
    if (slice_count  < 1) slice_count     = 1; // minimum of 1 slice
    if (faces_count  < 1) faces_count     = 1; // minimum of 1 face
    if (faces_count  > 1) faces_count     = 6; // force to cubemap
    if (faces_count  > 6) faces_count     = 6; // force to cubemap
    if (image_count  > 1) attribs        |= image::ATTRIBUTES_ARRAY;
    if (faces_count == 6) return attribs |  image::ATTRIBUTES_CUBEMAP;
    if (slice_count  > 1) return attribs |  image::ATTRIBUTES_3D;
    if (pixel_width  > 1 &&
        pixel_height > 1) return attribs |  image::ATTRIBUTES_2D;
    return attribs   | image::ATTRIBUTES_1D;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool image::is_plain_format(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_R8:
        case image::FORMAT_RG8:
        case image::FORMAT_RGB8:
        case image::FORMAT_RGBA8:
        case image::FORMAT_R16:
        case image::FORMAT_RG16:
        case image::FORMAT_RGBA16:
        case image::FORMAT_R16F:
        case image::FORMAT_RG16F:
        case image::FORMAT_RGBA16F:
        case image::FORMAT_R32F:
        case image::FORMAT_RG32F:
        case image::FORMAT_RGBA32F:
            return true;

        default: break;
    }
    return false;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool image::is_float_format(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_R16F:
        case image::FORMAT_RG16F:
        case image::FORMAT_RGBA16F:
        case image::FORMAT_R32F:
        case image::FORMAT_RG32F:
        case image::FORMAT_RGBA32F:
            return true;

        default: break;
    }
    return false;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool image::is_packed_format(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_RGB10A2:
            return true;

        case image::FORMAT_PVRTC1:
        case image::FORMAT_PVRTC2:
            return true;

        default: break;
    }
    return false;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool image::is_compressed_format(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_BC1:
        case image::FORMAT_BC2:
        case image::FORMAT_BC3:
        case image::FORMAT_BC3_XGBR:
        case image::FORMAT_BC3_RXBG:
        case image::FORMAT_BC3_RBXG:
        case image::FORMAT_BC3_XRBG:
        case image::FORMAT_BC3_RGXB:
        case image::FORMAT_BC3_XGXR:
        case image::FORMAT_BC4:
        case image::FORMAT_BC5:
        case image::FORMAT_BC5_XY:
        case image::FORMAT_ATI2N_DXT5:
        case image::FORMAT_PVRTC1:
        case image::FORMAT_PVRTC2:
            return true;

        default: break;
    }
    return false;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool image::is_block_compressed_format(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_BC1:
        case image::FORMAT_BC2:
        case image::FORMAT_BC3:
        case image::FORMAT_BC3_XGBR:
        case image::FORMAT_BC3_RXBG:
        case image::FORMAT_BC3_RBXG:
        case image::FORMAT_BC3_XRBG:
        case image::FORMAT_BC3_RGXB:
        case image::FORMAT_BC3_XGXR:
        case image::FORMAT_BC4:
        case image::FORMAT_BC5:
        case image::FORMAT_BC5_XY:
        case image::FORMAT_ATI2N_DXT5:
            return true;

        default: break;
    }
    return false;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool image::is_pvrtc_compressed_format(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_PVRTC1:
        case image::FORMAT_PVRTC2:
            return true;

        default: break;
    }
    return false;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::face_count(int32_t attributes)
{
    return (attributes & image::ATTRIBUTES_CUBEMAP) ? 6 : 1;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::channel_count(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_UNKNOWN:
            return 0;

        case image::FORMAT_R8:
        case image::FORMAT_R16:
        case image::FORMAT_R16F:
        case image::FORMAT_R32F:
        case image::FORMAT_BC4:
            return 1;

        case image::FORMAT_RG8:
        case image::FORMAT_RG16:
        case image::FORMAT_RG16F:
        case image::FORMAT_RG32F:
        case image::FORMAT_BC3_XGXR:
        case image::FORMAT_BC5:
        case image::FORMAT_BC5_XY:
        case image::FORMAT_ATI2N_DXT5:
            return 2;

        case image::FORMAT_RGB8:
        case image::FORMAT_BC3_XGBR:
        case image::FORMAT_BC3_RXBG:
        case image::FORMAT_BC3_RBXG:
        case image::FORMAT_BC3_XRBG:
        case image::FORMAT_BC3_RGXB:
            return 3;

        case image::FORMAT_RGBA8:
        case image::FORMAT_RGBA16:
        case image::FORMAT_RGBA16F:
        case image::FORMAT_RGBA32F:
        case image::FORMAT_RGB10A2:
        case image::FORMAT_BC1:
        case image::FORMAT_BC2:
        case image::FORMAT_BC3:
        case image::FORMAT_PVRTC1:
        case image::FORMAT_PVRTC2:
            return 4;

        default:
            return 0;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::bytes_per_block(int32_t image_format)
{
    // @note: 1 block is 4x4 = 16 pixels
    // @note: dimensions must be evenly divisible by four (mip levels included)
    switch (image_format)
    {
        case image::FORMAT_BC1:
        case image::FORMAT_BC4:
            return 8;

        case image::FORMAT_BC2:
        case image::FORMAT_BC3:
        case image::FORMAT_BC3_XGXR:
        case image::FORMAT_BC3_XGBR:
        case image::FORMAT_BC3_RXBG:
        case image::FORMAT_BC3_RBXG:
        case image::FORMAT_BC3_XRBG:
        case image::FORMAT_BC3_RGXB:
        case image::FORMAT_BC5:
        case image::FORMAT_BC5_XY:
        case image::FORMAT_ATI2N_DXT5:
            return 16;

        default:
            return 0;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::bytes_per_pixel(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_R8:
            return 1;

        case image::FORMAT_R16:
        case image::FORMAT_R16F:
        case image::FORMAT_RG8:
            return 2;

        case image::FORMAT_RGB8:
            return 3;

        case image::FORMAT_R32F:
        case image::FORMAT_RG16:
        case image::FORMAT_RG16F:
        case image::FORMAT_RGBA8:
        case image::FORMAT_RGB10A2:
            return 4;

        case image::FORMAT_RG32F:
        case image::FORMAT_RGBA16:
        case image::FORMAT_RGBA16F:
            return 8;

        case image::FORMAT_RGBA32F:
            return 16;

        default:
            return 0;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::bytes_per_channel(int32_t image_format)
{
    switch (image_format)
    {
        case image::FORMAT_R8:
        case image::FORMAT_RG8:
        case image::FORMAT_RGB8:
        case image::FORMAT_RGBA8:
            return 1;

        case image::FORMAT_R16:
        case image::FORMAT_R16F:
        case image::FORMAT_RG16:
        case image::FORMAT_RG16F:
        case image::FORMAT_RGBA16:
        case image::FORMAT_RGBA16F:
            return 2;

        case image::FORMAT_R32F:
        case image::FORMAT_RG32F:
        case image::FORMAT_RGBA32F:
            return 4;

        default:
            return 0;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::miplevel_count(
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices)
{
    size_t level_count = 0;
    size_t major_dim   = 0;

    // select largest of (width, height, slices).
    major_dim = (level0_width  > level0_height) ? level0_width  : level0_height;
    major_dim = (level0_slices > major_dim)     ? level0_slices : major_dim;

    // compute levels down to 1 in the major dimension:
    while (major_dim > 0)
    {
        major_dim  >>= 1;
        level_count += 1;
    }
    return level_count;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::miplevel_width(size_t level0_width, size_t level_index)
{
    int level_width = level0_width >> level_index;
    return  (0 == level_width) ? 1  : level_width;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::miplevel_height(size_t level0_height, size_t level_index)
{
    int level_height = level0_height >> level_index;
    return  (0 == level_height)  ? 1  : level_height;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::miplevel_slices(size_t level0_slices, size_t level_index)
{
    int level_slices  = level0_slices >> level_index;
    return (0 == level_slices)    ? 1  : level_slices;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::miplevel_size(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  level_index)
{
    size_t  level_slices = image::miplevel_slices(level0_slices, level_index);
    size_t  slice_size   = image::miplevel_slice_size(
        image_format,
        level0_width,
        level0_height,
        level_index);
    return (slice_size   * level_slices);
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::miplevel_slice_size(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level_index)
{
    size_t  level_width  = image::miplevel_width (level0_width,  level_index);
    size_t  level_height = image::miplevel_height(level0_height, level_index);
    size_t  level_size   = 0;
    bool    compressed   = image::is_compressed_format(image_format);

    if (compressed)
    {
        if (image::FORMAT_PVRTC1 == image_format)
        {
            // see IMG_texture_compression_pvrtc spec for formula.
            // ( max(width, 16) * max(height, 8) * 2 + 7) / 8
            level_width  = CMN_MAX(level_width, 16);
            level_height = CMN_MAX(level_height, 8);
            level_size   = (level_width * level_height * 2 + 7) / 8;
        }
        else if (image::FORMAT_PVRTC2 == image_format)
        {
            // see IMG_texture_compression_pvrtc spec for formula.
            // ( max(width, 8) * max(height, 8) * 4 + 7) / 8
            level_width  = CMN_MAX(level_width,  8);
            level_height = CMN_MAX(level_height, 8);
            level_size   = (level_width * level_height * 4 + 7) / 8;
        }
        else
        {
            // @note: block-compressed textures operate on 4x4 blocks
            // of pixels, so they are padded to be evenly divisible by
            // four. account for that behavior when calculating size.
            level_size =
                ((level_width  + 3) >> 2) *
                ((level_height + 3) >> 2) *
                image::bytes_per_block(image_format);
        }
    }
    else
    {
        level_size =
            level_width  *
            level_height *
            image::bytes_per_pixel(image_format);
    }
    return level_size;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::miplevel_offset(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  level_index)
{
    size_t byte_offset = 0;
    for (size_t index  = 0; index < level_index; ++index)
    {
        byte_offset   += image::miplevel_size(
            image_format,
            level0_width,
            level0_height,
            level0_slices,
            index);
    }
    return  byte_offset;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::miplevel_slice_offset(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level_index,
    size_t  slice_index)
{
    size_t  slice_size = image::miplevel_slice_size(
        image_format,
        level0_width,
        level0_height,
        level_index);
    return (slice_size * slice_index);
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::subimage_size(
    int32_t image_format,
    int32_t attributes,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count)
{
    size_t  face_size = image::subimage_face_size(
        image_format,
        level0_width,
        level0_height,
        level0_slices,
        mipmap_count);
    return (face_size * image::face_count(attributes));
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::subimage_face_size(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count)
{
    size_t face_size = 0;
    if (0 == mipmap_count)
    {
        // compute the number of mipmaps down to 1x1x1.
        mipmap_count = image::miplevel_count(
            level0_width,
            level0_height,
            level0_slices);
    }
    for (size_t level_index = 0; level_index < mipmap_count; ++level_index)
    {
        face_size   += image::miplevel_size(
            image_format,
            level0_width,
            level0_height,
            level0_slices,
            level_index);
    }
    return face_size;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::subimage_offset(
    int32_t image_format,
    int32_t attributes,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count,
    size_t  image_index)
{
    size_t  item_size = image::subimage_size(
        image_format,
        attributes,
        level0_width,
        level0_height,
        level0_slices,
        mipmap_count);
    return (item_size * image_index);
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::subimage_face_offset(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count,
    size_t  face_index)
{
    size_t  face_size = image::subimage_face_size(
        image_format,
        level0_width,
        level0_height,
        level0_slices,
        mipmap_count);
    return (face_size * face_index);
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::pixel_count(
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  level_index,
    size_t  mipmap_count)
{
    if (0 == mipmap_count)
    {
        // compute the total number of mip-levels down to 1x1x1.
        mipmap_count = image::miplevel_count(
            level0_width,
            level0_height,
            level0_slices);
    }

    // sum the number of pixels in each mip-level.
    size_t sum    = 0;
    for (size_t i = level_index; i <= mipmap_count; ++i)
    {
        size_t level_w = image::miplevel_width (level0_width,  i);
        size_t level_h = image::miplevel_height(level0_height, i);
        size_t level_s = image::miplevel_slices(level0_slices, i);
        sum += level_w * level_h * level_s;
    }
    return sum;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::image_size(
    int32_t image_format,
    int32_t attributes,
    size_t  image_count,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count)
{
    size_t  item_size = image::subimage_size(
        image_format,
        attributes,
        level0_width,
        level0_height,
        level0_slices,
        mipmap_count);
    return (item_size * image_count);
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::atlas_size(size_t atlas_entry_count)
{
    size_t names_size = atlas_entry_count * sizeof(uint32_t);
    size_t entry_size = atlas_entry_count * sizeof(image::atlas_entry_t);
    return names_size + entry_size + sizeof(uint32_t);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::get_header(image::container_t *image, image::header_t *out_header)
{
    if (out_header != NULL)
    {
        out_header->format      = ( int32_t) image->format;
        out_header->flags       = ( int32_t) image->flags;
        out_header->items       = (uint32_t) image->items;
        out_header->levels      = (uint32_t) image->levels;
        out_header->width       = (uint32_t) image->width;
        out_header->height      = (uint32_t) image->height;
        out_header->slices      = (uint32_t) image->slices;
        out_header->image_size  = (uint64_t) image->image_size;
        out_header->atlas_size  = (uint64_t) image->atlas_size;
        out_header->reserved[0] = 'I';
        out_header->reserved[1] = 'M';
        out_header->reserved[2] = 'G';
        out_header->reserved[3] = 'C';
        out_header->reserved[4] = 'F';
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::container_from_header(
    image::header_t    *header,
    image::container_t *out_container)
{
    if (out_container != NULL)
    {
        out_container->format     = header->format;
        out_container->flags      = header->flags;
        out_container->items      = header->items;
        out_container->levels     = header->levels;
        out_container->width      = header->width;
        out_container->height     = header->height;
        out_container->slices     = header->slices;
        out_container->image_size = (size_t) header->image_size;
        out_container->atlas_size = (size_t) header->atlas_size;
        out_container->alloc_base = NULL;
        out_container->image_data = NULL;
        out_container->atlas_data = NULL;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::channel_size(size_t channel_width, size_t channel_height)
{
    return channel_width * channel_height * sizeof(float);
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::buffer_size(
    size_t channel_width,
    size_t channel_height,
    size_t channel_count)
{
    size_t channel_size = channel_width * channel_height * sizeof(float);
    return channel_size * channel_count;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::buffer_init_with_memory(
    size_t           channel_width,
    size_t           channel_height,
    size_t           channel_count,
    void            *channel_memory,
    image::buffer_t *out_buffer)
{
    assert(channel_count  >= 1);
    assert(channel_count  <= MAX_IMAGE_CHANNELS);
    assert(channel_memory != NULL);
    size_t num_elms    = channel_width * channel_height;
    float *mem_base    = (float*)   channel_memory;
    float *channels[4] =
    {
        mem_base + (0 * num_elms),
        mem_base + (1 * num_elms),
        mem_base + (2 * num_elms),
        mem_base + (3 * num_elms)
    };
    out_buffer->channel_width   = channel_width;
    out_buffer->channel_height  = channel_height;
    out_buffer->channel_count   = channel_count;
    out_buffer->channel_data    = mem_base;
    for (size_t i = 0; i < channel_count; ++i)
    {
        out_buffer->channels[i] = channels[i];
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::convolution_kernel_init(
    size_t                       window_size,
    image::convolution_kernel_t *out_kernel)
{
    out_kernel->kernel_matrix = NULL;
    out_kernel->window_size   = window_size;
    return window_size * window_size * sizeof(float);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_laplacian_3x3(image::convolution_kernel_t *ck)
{
    float *kernel3x3 = ck->kernel_matrix;
    kernel3x3[0] = +0.0f;  kernel3x3[1] = -1.0f;  kernel3x3[2] = +0.0f;
    kernel3x3[3] = -1.0f;  kernel3x3[4] = +4.0f;  kernel3x3[5] = -1.0f;
    kernel3x3[6] = +0.0f;  kernel3x3[7] = -1.0f;  kernel3x3[8] = +0.0f;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_edge_detect_3x3(image::convolution_kernel_t *ck)
{
    float *kernel3x3 = ck->kernel_matrix;
    kernel3x3[0] = +0.0f;  kernel3x3[1] = +0.0f;  kernel3x3[2] = +0.0f;
    kernel3x3[3] = -1.0f;  kernel3x3[4] = +0.0f;  kernel3x3[5] = +1.0f;
    kernel3x3[6] = +0.0f;  kernel3x3[7] = +0.0f;  kernel3x3[8] = +0.0f;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_sobel_3x3(image::convolution_kernel_t *ck)
{
    float *kernel3x3 = ck->kernel_matrix;
    kernel3x3[0] = -1.0f;  kernel3x3[1] = +0.0f;  kernel3x3[2] = +1.0f;
    kernel3x3[3] = -2.0f;  kernel3x3[4] = +0.0f;  kernel3x3[5] = +2.0f;
    kernel3x3[6] = -1.0f;  kernel3x3[7] = +0.0f;  kernel3x3[8] = +1.0f;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_sobel_5x5(image::convolution_kernel_t *ck)
{
    memcpy(ck->kernel_matrix, Sobel_5x5, sizeof(float) * 5 * 5);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_sobel_7x7(image::convolution_kernel_t *ck)
{
    memcpy(ck->kernel_matrix, Sobel_7x7, sizeof(float) * 7 * 7);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_sobel_9x9(image::convolution_kernel_t *ck)
{
    memcpy(ck->kernel_matrix, Sobel_9x9, sizeof(float) * 9 * 9);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_copy(
    image::convolution_kernel_t       *dst,
    image::convolution_kernel_t const *src)
{
    assert(src->window_size == dst->window_size);
    size_t window_size = src->window_size;
    size_t matrix_size = window_size * window_size * sizeof(float);
    memcpy(dst->kernel_matrix, src->kernel_matrix, matrix_size);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_normalize(image::convolution_kernel_t *kernel)
{
    float  sum = 0.0f;
    float  inv = 0.0f;
    float *mtx = kernel->kernel_matrix;
    size_t win = kernel->window_size;
    size_t len = win * win;
    for (size_t i = 0; i < len; ++i) sum    += fabsf(mtx[i]);
    inv = 1.0f  / sum;
    for (size_t i = 0; i < len; ++i) mtx[i] *= inv;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::convolution_kernel_transpose(image::convolution_kernel_t *kernel)
{
    float *matrix      = kernel->kernel_matrix;
    size_t window_size = kernel->window_size;
    for (size_t i = 0; i < window_size; ++i)
    {
        for (size_t j  = i + 1; j < window_size; ++j)
        {
            size_t ia  = i * window_size + j;
            size_t ib  = j * window_size + i;
            float  t   = matrix[ia];
            matrix[ia] = matrix[ib];
            matrix[ib] = t;
        }
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::convolution_kernel_apply(
    image::convolution_kernel_t *kernel,
    int32_t                      border_mode,
    size_t                       source_x,
    size_t                       source_y,
    size_t                       source_width,
    size_t                       source_height,
    float                       *source_values)
{
    float *matrix = kernel->kernel_matrix;
    size_t window = kernel->window_size;
    size_t offset = kernel->window_size >> 1;
    float  sum    = 0.0f;
    for (size_t i = 0; i < window; ++i)
    {
        size_t sample_y  = (source_y + i) - offset;
        for (size_t e = 0; e  < window; ++e)
        {
            size_t sample_x   =(source_x + e) - offset;
            size_t kernel_idx =(e * window) + i;
            size_t sample_idx = image::sample_index(
                source_width,
                source_height,
                sample_x,
                sample_y,
                border_mode);
            sum += matrix[kernel_idx] * source_values[sample_idx];
        }
    }
    return sum;
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::sample_delta(
    float             x,
    float             scale,
    image::filter_fn  filter_kernel,
    void             *filter_args)
{
    return filter_kernel((x + 0.5f) * scale, filter_args);
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::sample_box(
    float             x,
    float             scale,
    size_t            sample_count,
    image::filter_fn  filter_kernel,
    void             *filter_args)
{
    float   sum   = 0.0f;
    float   nrm   = 1.0f / sample_count;
    for (size_t i = 0; i < sample_count; ++i)
    {
        float p   = (x + (i + 0.5f) * nrm) * scale;
        float v   = filter_kernel(p, filter_args);
        sum      += v;
    }
    return (sum * nrm);
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::sample_triangle(
    float             x,
    float             scale,
    size_t            sample_count,
    image::filter_fn  filter_kernel,
    void             *filter_args)
{
    float   sum   = 0.0f;
    float   nrm   = 1.0f / sample_count;
    for (size_t i = 0; i < sample_count; ++i)
    {
        float o   = (2.0f * i + 1.0f) * nrm;
        float p   = (x    + o - 0.5f) * scale;
        float v   = filter_kernel(p, filter_args);
        float w   = (o    > 1.0f)     ? 2.0f - o : o;
        sum      += (w  * v);
    }
    return (sum * nrm * 2.0f);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::box_args_init(image::box_args_t *out_args)
{
    out_args->filter_width = 0.5;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::box_args_init(
    float              filter_width,
    image::box_args_t *out_args)
{
    out_args->filter_width = filter_width;
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::box_filter(float x, void *args)
{
    image::box_args_t *kargs = (image::box_args_t*)args;
    if (::fabs(x)   <= kargs->filter_width) return 1.0f;
    return 0.0f;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::kaiser_args_init(
    float                 filter_width,
    image::kaiser_args_t *out_args)
{
    out_args->filter_width = filter_width;
    out_args->alpha        = 4.0f;
    out_args->stretch      = 1.0f;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::kaiser_args_init(
    float                 filter_width,
    float                 alpha,
    float                 stretch,
    image::kaiser_args_t *out_args)
{
    out_args->filter_width = filter_width;
    out_args->alpha        = alpha;
    out_args->stretch      = stretch;
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::kaiser_filter(float x, void *args)
{
    image::kaiser_args_t *kargs = (image::kaiser_args_t*)args;
    float a    = kargs->alpha;
    float s    = kargs->stretch;
    float sinc = image::sinc(PI * x * s);
    float t    = x    / kargs->filter_width;
    float omtt = 1.0f - t * t;
    if (omtt  >= 0)
    {
        float  sqrt_omtt = sqrtf(omtt);
        return sinc * image::bessel0(a * sqrt_omtt) / image::bessel0(a);
    }
    return 0.0f;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::lanczos_args_init(
    float                  filter_width,
    image::lanczos_args_t *out_args)
{
    out_args->filter_width = filter_width;
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::lanczos_filter(float x, void *args)
{
    image::lanczos_args_t *kargs = (image::lanczos_args_t*)args;
    float a = fabsf(x);
    if (a < kargs->filter_width)
    {
        float  sinc_a = image::sinc(PI * a);
        float  sinc_b = image::sinc(PI * a / kargs->filter_width);
        return sinc_a * sinc_b;
    }
    return 0;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::filter_1d_init(
    size_t                     scale_value,
    size_t                     sample_count,
    float                      filter_width,
    image::filter_kernel_1d_t *out_kernel_info)
{
    assert(scale_value   > 1);
    assert(sample_count  > 0);
    float  scale  = 1.0f / (float) scale_value;
    float  width  = filter_width * scale;
    size_t window = (size_t) ceilf(2.0f * width);
    size_t bytes  =  sizeof(float) * window;
    out_kernel_info->window_size   = window;
    out_kernel_info->sample_count  = sample_count;
    out_kernel_info->scale_value   = scale;
    out_kernel_info->filter_width  = width;
    out_kernel_info->filter_weights= NULL;
    return bytes;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::compute_filter_weights_1d(
    image::filter_fn           filter_kernel,
    void                      *filter_args,
    image::filter_kernel_1d_t *kernel_weights)
{
    float  inv       = 0.0f;
    float  total     = 0.0f;
    float  offset    = kernel_weights->window_size * 0.5f;
    float *weights   = kernel_weights->filter_weights;
    float  scale     = kernel_weights->scale_value;
    size_t window    = kernel_weights->window_size;
    size_t samples   = kernel_weights->sample_count;
    for (size_t i    = 0;  i < window; ++i)
    {
        float weight = image::sample_box(
            i - offset,
            scale,
            samples,
            filter_kernel,
            filter_args);
        weights[i]  = weight;
        total      += weight;
    }
    inv = 1.0f / total; // normalize
    for (size_t i = 0; i < window; ++i) weights[i] *= inv;
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::apply_filter_vertical_1d(
    image::filter_kernel_1d_t *kernel_weights,
    int32_t                    border_mode,
    size_t                     source_x,
    size_t                     source_y,
    size_t                     source_width,
    size_t                     source_height,
    float                     *source_values)
{
    float *weights = kernel_weights->filter_weights;
    size_t window  = kernel_weights->window_size;
    size_t offset  = kernel_weights->window_size >> 1;
    float  sum     = 0.0f;
    for (size_t i  = 0; i <  window; ++i)
    {
        size_t filter_y  = (source_y + i) - offset;
        size_t src_index = image::sample_index(
            source_width,
            source_height,
            source_x,
            filter_y,
            border_mode);
        sum += weights[i] * source_values[src_index];
    }
    return sum;
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::apply_filter_horizontal_1d(
    image::filter_kernel_1d_t *kernel_weights,
    int32_t                    border_mode,
    size_t                     source_x,
    size_t                     source_y,
    size_t                     source_width,
    size_t                     source_height,
    float                     *source_values)
{
    float *weights = kernel_weights->filter_weights;
    size_t window  = kernel_weights->window_size;
    size_t offset  = kernel_weights->window_size >> 1;
    float  sum     = 0.0f;
    for (size_t i  = 0; i <  window; ++i)
    {
        size_t filter_x  = (source_x + i) - offset;
        size_t src_index = image::sample_index(
            source_width,
            source_height,
            filter_x,
            source_y,
            border_mode);
        sum += weights[i] * source_values[src_index];
    }
    return sum;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::polyphase_1d_init(
    size_t                        source_dimension,
    size_t                        target_dimension,
    size_t                        sample_count,
    float                         filter_width,
    image::polyphase_kernel_1d_t *out_kernel_info)
{
    assert(source_dimension >= 1);
    assert(target_dimension >= 1);
    assert(sample_count     >  0);

    float  scale     = (float)target_dimension / (float) source_dimension;
    float  scale_inv = 1.0f / scale;
    if (scale > 1.0f)
    {
        // we are upsampling.
        scale        = 1.0f;
        sample_count = 1;
    }

    float  width     = filter_width * scale_inv;
    size_t columns   = target_dimension;
    size_t window    =((size_t)  ceilf(2.0f * width)+1);
    size_t bytes     = columns * window * sizeof(float);
    out_kernel_info->window_size    = window;
    out_kernel_info->column_count   = columns;
    out_kernel_info->sample_count   = sample_count;
    out_kernel_info->scale_value    = scale;
    out_kernel_info->scale_inverse  = scale_inv;
    out_kernel_info->filter_width   = width;
    out_kernel_info->filter_weights = NULL;
    return bytes;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::compute_polyphase_matrix_1d(
    image::filter_fn              filter_kernel,
    void                         *filter_args,
    image::polyphase_kernel_1d_t *kernel_weights)
{
    float *weights    = kernel_weights->filter_weights;
    float  width      = kernel_weights->filter_width;
    float  scale      = kernel_weights->scale_value;
    float  scale_inv  = kernel_weights->scale_inverse;
    size_t window     = kernel_weights->window_size;
    size_t samples    = kernel_weights->sample_count;
    size_t columns    = kernel_weights->column_count;
    for (size_t  i    = 0; i < columns; ++i)
    {
        float  total  =  0.0f;
        float  center = (0.5f + i) * scale_inv;
        size_t left   = (size_t)     floorf(center - width);
        for (size_t j = 0; j < window; ++j)
        {
            size_t index  =(i * window) +j;
            float  weight = image::sample_box(
                left  + j - center,
                scale,
                samples,
                filter_kernel,
                filter_args);
            weights[index] = weight;
            total         += weight;
        }
        for (size_t j = 0; j < window; ++j) weights[(i * window) + j] /= total;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::apply_polyphase_vertical_1d(
    image::polyphase_kernel_1d_t *kernel_weights,
    int32_t                       border_mode,
    size_t                        source_column,
    size_t                        source_width,
    size_t                        source_height,
    float                        *source_values,
    float                        *target_values)
{
    // kernel contains normalized weighting values.
    // the final value consists of the sum of pixels
    // from the source image that contribute to the
    // destination pixel multiplied by these weights.
    // @note: target size parameters are already encoded
    // in the values stored in kernel_weights.
    // @note: a different polyphase_kernel_1d_t is required
    // for each extent (horizontal and vertical).
    size_t  window     = kernel_weights->window_size;
    size_t  columns    = kernel_weights->column_count;
    float  *weights    = kernel_weights->filter_weights;
    float   width      = kernel_weights->filter_width;
    float   scale_inv  = kernel_weights->scale_inverse;
    for (size_t    i   = 0; i < columns; ++i)
    {
        // (0.5f + i)  = dst center * scale_inv => src coordinate space.
        // left is the top extent of the filter box.
        // the window slides from top to bottom.
        float   sum    =  0.0f;
        float   center = (0.5f + i) * scale_inv;
        int32_t left   = (int32_t)    floorf(center - width);
        for (size_t  j = 0; j < window; ++j)
        {
            size_t wid = (i * window) + j;
            size_t sid = image::sample_index(
                source_width,
                source_height,
                source_column,
                j  + left,
                border_mode);
            sum    += weights[wid]  * source_values[sid];
        }
        target_values[i] = sum;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::apply_polyphase_horizontal_1d(
    image::polyphase_kernel_1d_t *kernel_weights,
    int32_t                       border_mode,
    size_t                        source_row,
    size_t                        source_width,
    size_t                        source_height,
    float                        *source_values,
    float                        *target_values)
{
    // kernel contains normalized weighting values.
    // the final value consists of the sum of pixels
    // from the source image that contribute to the
    // destination pixel multiplied by these weights.
    // @note: target size parameters are already encoded
    // in the values stored in kernel_weights.
    // @note: a different polyphase_kernel_1d_t is required
    // for each extent (horizontal and vertical).
    size_t  window     = kernel_weights->window_size;
    size_t  columns    = kernel_weights->column_count;
    float  *weights    = kernel_weights->filter_weights;
    float   width      = kernel_weights->filter_width;
    float   scale_inv  = kernel_weights->scale_inverse;
    for (size_t    i   = 0; i < columns; ++i)
    {
        // (0.5f + i)  = dst center * scale_inv => src coordinate space.
        // left is the top extent of the filter box.
        // the window slides from left to right.
        float   sum    =  0.0f;
        float   center = (0.5f + i) * scale_inv;
        int32_t left   = (int32_t)    floorf(center - width);
        for (size_t  j = 0; j < window; ++j)
        {
            size_t wid = (i * window) + j;
            size_t sid = image::sample_index(
                source_width,
                source_height,
                left  + j,
                source_row,
                border_mode);
            sum    += weights[wid]  * source_values[sid];
        }
        target_values[i] = sum;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::lab_to_rgb(uint8_t l, uint8_t a, uint8_t b, uint8_t *out_rgb)
{
    // first convert LAB -> XYZ, and then XYZ -> RGB:
    double L    = ((double) l / 2.55);
    double A    = ((double) a / 1.00 - 127.5);
    double B    = ((double) b / 1.00 - 127.5);
    double v_Y  = (L   + 16.0) / 116.0;
    double v_X  =  A   / 500.0 + v_Y;
    double v_Z  =  v_Y - B     / 200.0;
    double v_X3 =  v_X * v_X   * v_X;
    double v_Y3 =  v_Y * v_Y   * v_Y;
    double v_Z3 =  v_Z * v_Z   * v_Z;
    if (v_Y3 > 0.008856) v_Y   = v_Y3;
    else                 v_Y   =(v_Y - 16 / 116) / 7.787;
    if (v_X3 > 0.008856) v_X   = v_X3;
    else                 v_X   =(v_X - 16 / 116) / 7.787;
    if (v_Z3 > 0.008856) v_Z   = v_Z3;
    else                 v_Z   =(v_Z - 16 / 116) / 7.787;
    image::xyz_to_rgb(95.047 * v_X, 100.000 * v_Y, 108.883 * v_Z, out_rgb);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::xyz_to_rgb(double x, double y, double z, uint8_t *out_rgb)
{
    double v_X =   x /  100.0;
    double v_Y =   y /  100.0;
    double v_Z =   z /  100.0;
    double v_R = v_X *  3.2406 + v_Y * -1.5372 + v_Z * -0.4986;
    double v_G = v_X * -0.9689 + v_Y *  1.8758 + v_Z *  0.0415;
    double v_B = v_X *  0.0557 + v_Y * -0.2040 + v_Z *  1.0570;

    if (v_R > 0.0031308)   v_R = 1.055 * pow(v_R, 1  / 2.4) - 0.055;
    else                   v_R = 12.92 * v_R;
    if (v_G > 0.0031308)   v_G = 1.055 * pow(v_G, 1  / 2.4) - 0.055;
    else                   v_G = 12.92 * v_G;
    if (v_B > 0.0031308)   v_B = 1.055 * pow(v_B, 1  / 2.4) - 0.055;
    else                   v_B = 12.92 * v_B;

    int64_t nr = (int64_t)(v_R * 256.0);
    int64_t ng = (int64_t)(v_G * 256.0);
    int64_t nb = (int64_t)(v_B * 256.0);
    out_rgb[0] = (uint8_t)((nr < 0) ? 0x00 : ((nr > 0xFF) ? 0xFF : nr));
    out_rgb[1] = (uint8_t)((ng < 0) ? 0x00 : ((ng > 0xFF) ? 0xFF : ng));
    out_rgb[2] = (uint8_t)((nb < 0) ? 0x00 : ((nb > 0xFF) ? 0xFF : nb));
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::cmyk_to_rgb(
    uint8_t  c,
    uint8_t  m,
    uint8_t  y,
    uint8_t  k,
    uint8_t *out_rgb)
{
    double  C  = (double)   (255 -  c) /  255.0;
    double  M  = (double)   (255 -  m) /  255.0;
    double  Y  = (double)   (255 -  y) /  255.0;
    double  K  = (double)   (255 -  k) /  255.0;
    int64_t nr = (int64_t) ((1.0 - (C  * (1 - K) + K))  * 255);
    int64_t ng = (int64_t) ((1.0 - (M  * (1 - K) + K))  * 255);
    int64_t nb = (int64_t) ((1.0 - (Y  * (1 - K) + K))  * 255);
    out_rgb[0] = (uint8_t) ((nr  <  0) ?  0x00   : ((nr > 0xFF) ? 0xFF : nr));
    out_rgb[1] = (uint8_t) ((ng  <  0) ?  0x00   : ((ng > 0xFF) ? 0xFF : ng));
    out_rgb[2] = (uint8_t) ((nb  <  0) ?  0x00   : ((nb > 0xFF) ? 0xFF : nb));
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::index_wrap(size_t width, size_t height, ptrdiff_t x, ptrdiff_t y)
{
    y = repeat_remainder(y, height);
    x = repeat_remainder(x, width);
    return (y * width) + x;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::index_clamp(
    size_t    width,
    size_t    height,
    ptrdiff_t x,
    ptrdiff_t y)
{
    if (x <  0) x = 0;
    if (y <  0) y = 0;
    if (size_t (x) >= width)  x = width  - 1;
    if (size_t (y) >= height) y = height - 1;
    return (y * width) + x;
}

/*/////////////////////////////////////////////////////////////////////////80*/

size_t image::index_mirror(
    size_t    width,
    size_t    height,
    ptrdiff_t x,
    ptrdiff_t y)
{
    while ((x < 0) || (size_t(x) > (width - 1)))
    {
        if (x < 0)
        {
            x = -x;
        }
        if (size_t(x) >= width)
        {
            x = width + width - x - 1;
        }
    }
    while ((y < 0) || (size_t(y) > (height - 1)))
    {
        if (y < 0)
        {
            y = -y;
        }
        if (size_t(y) >= height)
        {
            y = height + height - y - 1;
        }
    }
    return (y * width) + x;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::copy_channel(
    float       *dst_channel,
    float const *src_channel,
    size_t       channel_width,
    size_t       channel_height)
{
    size_t total_size = channel_width * channel_height * sizeof(float);
    memcpy(dst_channel, src_channel, total_size);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::fill_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height,
    float   fill_value)
{
    size_t  channel_els  = channel_width   * channel_height;
    float  *channel_end  = channel_values  + channel_els;
    while  (channel_end != channel_values) *channel_values++ = fill_value;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::flip_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height)
{
    float  t  = 0.0f;
    size_t w  = channel_width;
    size_t h  = channel_height;
    size_t hh = channel_height >> 1;
    for (size_t y = 0; y < hh; ++y)
    {
        float *src = &channel_values[(y * w)];
        float *dst = &channel_values[(h - 1 - y) * w];
        for (size_t x = 0; x < w; ++x)
        {
            t      = *src;
            *src++ = *dst;
            *dst++ =  t;
        }
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::clamp_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height,
    float   channel_min,
    float   channel_max)
{
    size_t  channel_els  = channel_width  * channel_height;
    float  *channel_end  = channel_values + channel_els;
    while  (channel_end != channel_values)
    {
        float v = *channel_values;
        if (v < channel_min) *channel_values = channel_min;
        if (v > channel_max) *channel_values = channel_max;
        ++channel_values;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::exponentiate_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height,
    float   power)
{
    size_t  channel_els   = channel_width  * channel_height;
    float  *channel_end   = channel_values + channel_els;
    while  (channel_end  != channel_values)
    {
        float v = powf(*channel_values, power);
        *channel_values++ = v;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::scale_bias_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height,
    float   scale,
    float   bias)
{
    size_t  channel_els   = channel_width   * channel_height;
    float  *channel_end   = channel_values  + channel_els;
    while  (channel_end  != channel_values)
    {
        float v = (*channel_values * scale) + bias;
        *channel_values++ = v;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::channel_minimum(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height)
{
    float   channel_min   = std::numeric_limits<float>::max();
    size_t  channel_els   = channel_width   * channel_height;
    float  *channel_end   = channel_values  + channel_els;
    while  (channel_end  != channel_values)
    {
        float v = *channel_values++;
        if   (v <  channel_min) channel_min = v;
    }
    return channel_min;
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::channel_maximum(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height)
{
    float   channel_max   = std::numeric_limits<float>::min();
    size_t  channel_els   = channel_width   * channel_height;
    float  *channel_end   = channel_values  + channel_els;
    while  (channel_end  != channel_values)
    {
        float v = *channel_values++;
        if   (v <  channel_max) channel_max = v;
    }
    return channel_max;
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::channel_average(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height)
{
    float   channel_sum  = 0.0f;
    size_t  channel_els  = channel_width   * channel_height;
    float  *channel_end  = channel_values  + channel_els;
    while  (channel_end != channel_values)
    {
        channel_sum += *channel_values++;
    }
    return (channel_els > 0) ? channel_sum / channel_els : 0.0f;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::monochrome(
    float                 *monochrome_values,
    image::buffer_t const *color_buffer,
    float                  scale_r,
    float                  scale_g,
    float                  scale_b,
    float                  scale_a)
{
    size_t  channel_count  = color_buffer->channel_count;
    size_t  channel_width  = color_buffer->channel_width;
    size_t  channel_height = color_buffer->channel_height;
    size_t  pixel_count    = channel_width*channel_height;
    switch (channel_count)
    {
        case 0:
            {
                float const *src_a    = color_buffer->channels[0];
                for (size_t  i = 0; i < pixel_count; ++i)
                {
                    monochrome_values[i] = src_a[i]  * scale_a;
                }
            }
            break;

        case 1:
            {
                float const *src_r    = color_buffer->channels[0];
                float const *src_g    = color_buffer->channels[1];
                for (size_t  i = 0; i < pixel_count; ++i)
                {
                    monochrome_values[i]   =
                        src_r[i] * scale_r +
                        src_g[i] * scale_g;
                }
            }
            break;

        case 2:
            {
                float const *src_r    = color_buffer->channels[0];
                float const *src_g    = color_buffer->channels[1];
                float const *src_b    = color_buffer->channels[2];
                for (size_t  i = 0; i < pixel_count; ++i)
                {
                    monochrome_values[i]   =
                        src_r[i] * scale_r +
                        src_g[i] * scale_g +
                        src_b[i] * scale_b;
                }
            }
            break;

        case 3:
            {
                float const *src_r    = color_buffer->channels[0];
                float const *src_g    = color_buffer->channels[1];
                float const *src_b    = color_buffer->channels[2];
                float const *src_a    = color_buffer->channels[3];
                for (size_t  i = 0; i < pixel_count; ++i)
                {
                    monochrome_values[i]   =
                        src_r[i] * scale_r +
                        src_g[i] * scale_g +
                        src_b[i] * scale_b +
                        src_a[i] * scale_a;
                }
            }
            break;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::grayscale(
    float                 *grayscale_values,
    image::buffer_t const *color_buffer)
{
    image::monochrome(grayscale_values, color_buffer, 0.39f, 0.50f, 0.11f, 0);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::clamp(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            channel_min,
    float            channel_max)
{
    for (size_t i = 0; i < channel_count; ++i)
    {
        image::clamp_channel(
            buffer->channels[channel_base + i],
            buffer->channel_width,
            buffer->channel_height,
            channel_min,
            channel_max);
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::exponentiate(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            power)
{
    for (size_t i = 0; i < channel_count; ++i)
    {
        image::exponentiate_channel(
            buffer->channels[channel_base + i],
            buffer->channel_width,
            buffer->channel_height,
            power);
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::scale_bias(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            scale,
    float            bias)
{
    for (size_t i = 0; i < channel_count; ++i)
    {
        image::scale_bias_channel(
            buffer->channels[channel_base + i],
            buffer->channel_width,
            buffer->channel_height,
            scale, bias);
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::linear(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            gamma_power /* = 2.2f */)
{
    float p = 1.0f / gamma_power;
    image::exponentiate(buffer, channel_base, channel_count, p);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::gamma(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            gamma_power /* = 2.2f */)
{
    float p = gamma_power;
    image::exponentiate(buffer, channel_base, channel_count, p);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::flip(image::buffer_t *buffer)
{
    for (size_t i = 0; i < buffer->channel_count; ++i)
    {
        image::flip_channel(
            buffer->channels[i],
            buffer->channel_width,
            buffer->channel_height);
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::swizzle(
    image::buffer_t * CMN_RESTRICT input,
    image::buffer_t * CMN_RESTRICT output,
    size_t                         index_c0,
    size_t                         index_c1,
    size_t                         index_c2,
    size_t                         index_c3)
{
    size_t indices[MAX_IMAGE_CHANNELS] =
    {
        index_c0,
        index_c1,
        index_c2,
        index_c3
    };

    // copy the buffer attributes:
    output->channel_data   = input->channel_data;
    output->channel_count  = input->channel_count;
    output->channel_width  = input->channel_width;
    output->channel_height = input->channel_height;

    // swizzle the channels:
    for (size_t i = 0; i < input->channel_count; ++i)
    {
        output->channels[i] = input->channels[indices[i]];
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

float image::alpha_test_coverage(
    float const *alpha_channel,
    size_t       channel_width,
    size_t       channel_height,
    float        alpha_reference)
{
    float  coverage = 0.0f;
    size_t w        = channel_width;
    size_t h        = channel_height;
    for (size_t y = 0; y < h; ++y)
    {
        size_t  s = y * w;
        for (size_t x = 0; x < w; ++x)
        {
            if (alpha_channel[s + x] > alpha_reference)
            {
                coverage += 1.0f;
            }
        }
    }
    return coverage / (w * h);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::scale_alpha_to_coverage(
    float  *alpha_channel,
    size_t  channel_width,
    size_t  channel_height,
    float   desired_coverage,
    float   alpha_reference)
{
    float min_ref = 0.0f;
    float max_ref = 1.0f;
    float mid_ref = 0.5f;
    float scale   = 1.0f;

    // determine desired scale using a binary search.
    // the search is hard-coded to a maximum of 8 steps.
    for (size_t i = 0; i < 10; i++)
    {
        float curr_coverage = image::alpha_test_coverage(
            alpha_channel,
            channel_width,
            channel_height,
            mid_ref);
        if      (curr_coverage > desired_coverage) min_ref = mid_ref;
        else if (curr_coverage < desired_coverage) max_ref = mid_ref;
        else break;
        mid_ref = 0.5f * (min_ref + max_ref);
    }

    // scale the alpha values so that the desired coverage is achieved.
    scale = alpha_reference / mid_ref;
    image::scale_bias_channel(
        alpha_channel,
        channel_width,
        channel_height,
        scale, 0.0f);

    // clamp all alpha values into the [0, 1] range.
    image::clamp_channel(
        alpha_channel,
        channel_width,
        channel_height,
        0.0f, 1.0f);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void image::premultiply_alpha(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float const     *alpha_channel)
{
    size_t w = buffer->channel_width;
    size_t h = buffer->channel_height;
    size_t c = buffer->channel_count;
    for (size_t i = 0; i < c; ++i)
    {
        float const *a = alpha_channel;
        float       *p = buffer->channels[channel_base + i];
        for (size_t  j = 0; j < w * h; ++j)
            *p++ *= *a++;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

/*/////////////////////////////////////////////////////////////////////////////
//    $Id$
///////////////////////////////////////////////////////////////////////////80*/
