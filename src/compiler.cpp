/*/////////////////////////////////////////////////////////////////////////////
/// @summary Implements the texture compiler interface.
///////////////////////////////////////////////////////////////////////////80*/

/*////////////////
//   Includes   //
////////////////*/
#include <stdlib.h>
#include "compiler.hpp"
#include "stb_image.c"

/*//////////////////////////
//   Using Declarations   //
//////////////////////////*/

/*//////////////////////
//   Implementation   //
//////////////////////*/

/*/////////////////////////////////////////////////////////////////////////80*/

#define NO_ERROR       ""
#define OUT_OF_MEMORY  "Could not allocate the required amount of memory."

/*/////////////////////////////////////////////////////////////////////////80*/

static void dump_data(char const *path, void const *data, size_t size)
{
    FILE  *fp = fopen(path, "wb");
    fwrite(data, size, 1, fp);
    fclose(fp);
}

/*/////////////////////////////////////////////////////////////////////////80*/

static bool is_pow2(size_t value)
{
    return ((value & (value - 1)) == 0);
}

/*/////////////////////////////////////////////////////////////////////////80*/

static bool create_buffer(
    size_t           width,
    size_t           height,
    size_t           channels,
    image::buffer_t *buffer)
{
    void  *data  = malloc(image::buffer_size(width, height, channels));
    if    (data != NULL)
    {
        image::buffer_init_with_memory(width, height, channels, data, buffer);
        return true;
    }
    return false;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static bool create_buffer(
    int              width,
    int              height,
    int              channels,
    image::buffer_t *buffer)
{
    size_t wd = (size_t) width;
    size_t ht = (size_t) height;
    size_t nc = (size_t) channels;
    return create_buffer(wd, ht, nc, buffer);
}

/*/////////////////////////////////////////////////////////////////////////80*/

static void init_buffer_from_float(image::buffer_t *buffer, float *pixels)
{
    size_t  count  = buffer->channel_count;
    size_t  width  = buffer->channel_width;
    size_t  height = buffer->channel_height;
    float  *source = pixels;
    switch (count)
    {
        case 1:
            {
                float *dest0  = buffer->channels[0];
                for (size_t i = 0; i < width * height; ++i)
                {
                    *dest0++  = *source++;
                }
            }
            break;

        case 2:
            {
                float *dest0  = buffer->channels[0];
                float *dest1  = buffer->channels[1];
                for (size_t i = 0; i < width * height; ++i)
                {
                    *dest0++  = *source++;
                    *dest1++  = *source++;
                }
            }
            break;

        case 3:
            {
                float *dest0  = buffer->channels[0];
                float *dest1  = buffer->channels[1];
                float *dest2  = buffer->channels[2];
                for (size_t i = 0; i < width * height; ++i)
                {
                    *dest0++  = *source++;
                    *dest1++  = *source++;
                    *dest2++  = *source++;
                }
            }
            break;

        case 4:
            {
                float *dest0  = buffer->channels[0];
                float *dest1  = buffer->channels[1];
                float *dest2  = buffer->channels[2];
                float *dest3  = buffer->channels[3];
                for (size_t i = 0; i < width * height; ++i)
                {
                    *dest0++  = *source++;
                    *dest1++  = *source++;
                    *dest2++  = *source++;
                    *dest3++  = *source++;
                }
            }
            break;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

static void init_buffer_from_uint8(image::buffer_t *buffer, uint8_t *pixels)
{
    float    scale  = 1.0f  / 255.0f;
    size_t   count  = buffer->channel_count;
    size_t   width  = buffer->channel_width;
    size_t   height = buffer->channel_height;
    uint8_t *source = pixels;
    switch  (count)
    {
        case 1:
            {
                float *dest0  = buffer->channels[0];
                for (size_t i = 0; i < width * height; ++i)
                {
                    uint8_t r = *source++;
                    *dest0++  =  scale * r;
                }
            }
            break;

        case 2:
            {
                float *dest0  = buffer->channels[0];
                float *dest1  = buffer->channels[1];
                for (size_t i = 0; i < width * height; ++i)
                {
                    uint8_t r = *source++;
                    uint8_t g = *source++;
                    *dest0++  =  scale * r;
                    *dest1++  =  scale * g;
                }
            }
            break;

        case 3:
            {
                float *dest0  = buffer->channels[0];
                float *dest1  = buffer->channels[1];
                float *dest2  = buffer->channels[2];
                for (size_t i = 0; i < width * height; ++i)
                {
                    uint8_t r = *source++;
                    uint8_t g = *source++;
                    uint8_t b = *source++;
                    *dest0++  =  scale * r;
                    *dest1++  =  scale * g;
                    *dest2++  =  scale * b;
                }
            }
            break;

        case 4:
            {
                float *dest0  = buffer->channels[0];
                float *dest1  = buffer->channels[1];
                float *dest2  = buffer->channels[2];
                float *dest3  = buffer->channels[3];
                for (size_t i = 0; i < width * height; ++i)
                {
                    uint8_t r = *source++;
                    uint8_t g = *source++;
                    uint8_t b = *source++;
                    uint8_t a = *source++;
                    *dest0++  =  scale * r;
                    *dest1++  =  scale * g;
                    *dest2++  =  scale * b;
                    *dest3++  =  scale * a;
                }
            }
            break;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_1_32(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(uint8_t) *1;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    uint8_t *pixels  = (uint8_t*) malloc(width * height * bpp);
    uint8_t *dest    =    pixels;
    for (size_t i    = 0; pixels &&  i < width * height;  ++i)
    {
        *dest++      = (uint8_t) (*source0++ * 255.0);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_2_32(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(uint8_t) *2;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    uint8_t *pixels  = (uint8_t*) malloc(width * height * bpp);
    uint8_t *dest    =    pixels;
    for (size_t i    = 0; pixels &&  i < width * height;  ++i)
    {
        *dest++      = (uint8_t) (*source0++ * 255.0f);
        *dest++      = (uint8_t) (*source1++ * 255.0f);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_3_32(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(uint8_t) *3;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    uint8_t *pixels  = (uint8_t*) malloc(width * height * bpp);
    uint8_t *dest    =    pixels;
    for (size_t i    = 0; pixels &&  i < width * height;  ++i)
    {
        *dest++      = (uint8_t) ((*source0++) * 255.0f);
        *dest++      = (uint8_t) ((*source1++) * 255.0f);
        *dest++      = (uint8_t) ((*source2++) * 255.0f);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_4_32(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(uint8_t) *4;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    float   *source3 = buffer->channels[3];
    uint8_t *pixels  = (uint8_t*) malloc(width * height * bpp);
    uint8_t *dest    =    pixels;
    for (size_t i    = 0; pixels &&  i < width * height;  ++i)
    {
        *dest++      = (uint8_t) (*source0++ * 255.0f);
        *dest++      = (uint8_t) (*source1++ * 255.0f);
        *dest++      = (uint8_t) (*source2++ * 255.0f);
        *dest++      = (uint8_t) (*source3++ * 255.0f);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

#define  HALF_MIN_BIASED_EXP_AS_SINGLE_EXP 0x38000000UL
#define  HALF_MAX_BIASED_EXP_AS_SINGLE_EXP 0x47800000UL
#define  HALF_MAX_BIASED_EXP              (0x1FUL << 10)
#define  SINGLE_MAX_BIASED_EXP            (0xFFUL << 23)

/// Convert an IEEE-754 single-precision (32-bit) floating point value to a
/// half-precision 16-bit value. From OpenGL ES 2.0 Programming Guide.
uint16_t float_to_half(float *f)
{
    uint32_t x        = *(uint32_t*) f;
    uint32_t sign     =  (uint16_t) (x >> 31);
    uint32_t mantissa =  x & ((1 << 23) -  1);
    uint32_t exponent =  x & SINGLE_MAX_BIASED_EXP;
    if (exponent >= HALF_MAX_BIASED_EXP_AS_SINGLE_EXP) // NaN or Infinity
    {
        if (mantissa && (exponent == SINGLE_MAX_BIASED_EXP))
        {
            mantissa   = (1 << 23) - 1;
        }
        else mantissa  = 0;
        return (((uint16_t) sign) << 15)         |
                ((uint16_t) HALF_MAX_BIASED_EXP) |
                ((uint16_t) mantissa >> 16);
    }
    else if (exponent <= HALF_MIN_BIASED_EXP_AS_SINGLE_EXP) // denormal or zero
    {
        exponent   = (HALF_MIN_BIASED_EXP_AS_SINGLE_EXP - exponent) >> 23;
        mantissa >>= (exponent + 14);
        return (((uint16_t) sign) << 15) | ((uint16_t) mantissa);
    }
    return (((uint16_t) sign) << 15)                                            |
            ((uint16_t)((exponent -  HALF_MIN_BIASED_EXP_AS_SINGLE_EXP) >> 13)) |
            ((uint16_t) (mantissa >> 13));
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_1_16f(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(uint16_t) * 1;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    uint16_t *pixels = (uint16_t*) malloc(width * height * bpp);
    uint16_t *dest   =    pixels;
    for (size_t i    = 0; pixels &&   i < width * height;  ++i)
    {
        *dest++      = float_to_half(source0++);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_2_16f(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(uint16_t) * 2;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    uint16_t *pixels = (uint16_t*) malloc(width * height * bpp);
    uint16_t *dest   =    pixels;
    for (size_t i    = 0; pixels &&   i < width * height;  ++i)
    {
        *dest++      = float_to_half(source0++);
        *dest++      = float_to_half(source1++);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_3_16f(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(uint16_t) * 3;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    uint16_t *pixels = (uint16_t*) malloc(width * height * bpp);
    uint16_t *dest   =    pixels;
    for (size_t i    = 0; pixels &&   i < width * height;  ++i)
    {
        *dest++      = float_to_half(source0++);
        *dest++      = float_to_half(source1++);
        *dest++      = float_to_half(source2++);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_4_16f(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(uint16_t) * 4;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    float   *source3 = buffer->channels[3];
    uint16_t *pixels = (uint16_t*) malloc(width * height * bpp);
    uint16_t *dest   =    pixels;
    for (size_t i    = 0; pixels &&   i < width * height;  ++i)
    {
        *dest++      = float_to_half(source0++);
        *dest++      = float_to_half(source1++);
        *dest++      = float_to_half(source2++);
        *dest++      = float_to_half(source3++);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_1_32f(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(float)  * 1;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *pixels  = (float*)  malloc(width * height * bpp);
    float   *dest    =    pixels;
    for (size_t i    = 0; pixels && i < width * height;  ++i)
    {
        *dest++      = *source0++;
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_2_32f(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(float)  * 2;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *pixels  = (float*)  malloc(width * height * bpp);
    float   *dest    =    pixels;
    for (size_t i    = 0; pixels && i < width * height;  ++i)
    {
        *dest++      = *source0++;
        *dest++      = *source1++;
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_3_32f(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(float)  * 3;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    float   *pixels  = (float*)  malloc(width * height * bpp);
    float   *dest    =    pixels;
    for (size_t i    = 0; pixels && i < width * height;  ++i)
    {
        *dest++      = *source0++;
        *dest++      = *source1++;
        *dest++      = *source2++;
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_4_32f(image::buffer_t *buffer)
{
    size_t   bpp     = sizeof(float)  * 4;
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    float   *source3 = buffer->channels[3];
    float   *pixels  = (float*)  malloc(width * height * bpp);
    float   *dest    =    pixels;
    for (size_t i    = 0; pixels && i < width * height;  ++i)
    {
        *dest++      = *source0++;
        *dest++      = *source1++;
        *dest++      = *source2++;
        *dest++      = *source3++;
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void texture_compiler_inputs_init(texture_compiler_inputs_t *inputs)
{
    if (inputs)
    {
        inputs->input_image     = NULL;
        inputs->border_mode     = image::BORDER_MODE_MIRROR;
        inputs->target_width    = 0;
        inputs->target_height   = 0;
        inputs->maximum_levels  = 0;
        inputs->build_mipmaps   = false;
        inputs->force_pow2      = false;
        inputs->premultiply_a   = false;
        inputs->flip_y          = false;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void texture_compiler_inputs_sanitize(texture_compiler_inputs_t *inputs)
{
    if (inputs)
    {
        image::buffer_t *source  = inputs->input_image;
        size_t source_width      = source->channel_width;
        size_t source_height     = source->channel_height;
        size_t target_width      = inputs->target_width;
        size_t target_height     = inputs->target_height;
        size_t max_levels        = 0;

        if (0 == target_width)     target_width  = source_width;
        if (0 == target_height)    target_height = source_height;
        if (inputs->build_mipmaps) inputs->force_pow2 = true;
        if (inputs->force_pow2)
        {
            // adjust dimensions to be the next highest power-of-two
            // only if the target dimensions are not already pow2.
            if (!is_pow2(target_width))
            {
                target_width = 1;
                while (target_width < source_width)
                    target_width  <<= 1;
            }
            if (!is_pow2(target_height))
            {
                target_height = 1;
                while (target_height < source_height)
                    target_height  <<= 1;
            }
        }
        if (inputs->build_mipmaps)
        {
            // calculate the maximum number of mip-levels down to 1x1.
            max_levels = image::miplevel_count(
                target_width,
                target_height,
                1);

            // if the inputs do not specify a maximum level count, then
            // the level count is set to the maximum value (down to 1x1).
            // if the inputs specify a valid maximum level, use that.
            // otherwise, clamp the value to the maximum number of levels.
            if (inputs->maximum_levels == 0)
                inputs->maximum_levels  = max_levels;
            if (inputs->maximum_levels <= max_levels)
                max_levels  = inputs->maximum_levels;
        }
        else    max_levels  = 1; // no mipmaps, only one level.

        // update the inputs with possibly adjusted values.
        inputs->target_width   = target_width;
        inputs->target_height  = target_height;
        inputs->maximum_levels = max_levels;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void texture_compiler_outputs_init(texture_compiler_outputs_t *outputs)
{
    if (outputs)
    {
        outputs->error_message  = NO_ERROR;
        outputs->channel_count  = 0;
        outputs->level_count    = 0;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void texture_compiler_outputs_free(texture_compiler_outputs_t *outputs)
{
    if (outputs)
    {
        for (size_t i = 0; i < outputs->level_count; ++i)
        {
            free_buffer(&outputs->level_data[i]);
        }
        outputs->channel_count  = 0;
        outputs->level_count    = 0;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool file_to_buffer(char const *file, image::buffer_t *buffer)
{
    int width    = 0;
    int height   = 0;
    int channels = 0;

    if (stbi_is_hdr(file))
    {
        stbi_set_unpremultiply_on_load(1);
        float *pixels  = stbi_loadf(file,  &width, &height, &channels, 0);
        if (pixels && create_buffer(width, height, channels, buffer))
        {
            init_buffer_from_float(buffer, pixels);
            stbi_image_free(pixels);
            return true;
        }
        if (pixels) stbi_image_free(pixels);
        return false;
    }
    else
    {
        stbi_set_unpremultiply_on_load(1);
        uint8_t *pixels = stbi_load(file, &width, &height, &channels, 0);
        if (pixels && create_buffer(width, height, channels, buffer))
        {
            init_buffer_from_uint8(buffer, pixels);
            stbi_image_free(pixels);
            return true;
        }
        if (pixels) stbi_image_free(pixels);
        return false;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool is_non_power_of_two(image::buffer_t *buffer)
{
    size_t w  = buffer->channel_width;
    size_t h  = buffer->channel_height;
    if ((w & (w - 1)) != 0) return true;
    if ((h & (h - 1)) != 0) return true;
    return false;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool power_of_two_dimensions(
    image::buffer_t *buffer,
    size_t          *out_pot_width,
    size_t          *out_pot_height)
{
    // @note: if you want to go to the next smallest power-of-two,
    // just take  the returned dimensions and shift right by one.
    size_t w  = buffer->channel_width;
    size_t h  = buffer->channel_height;
    bool   r  = true;
    if ((w & (w - 1)) != 0) { w = 1;  r = false; }
    if ((h & (h - 1)) != 0) { h = 1;  r = false; }
    while (w  < buffer->channel_width)  w <<= 1;
    while (h  < buffer->channel_height) h <<= 1;
    if (out_pot_width)  *out_pot_width  = w;
    if (out_pot_height) *out_pot_height = h;
    return r;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void copy_buffer(image::buffer_t *target, image::buffer_t *source)
{
    for (size_t i = 0; i < source->channel_count; ++i)
    {
        image::copy_channel(
            target->channels[i],
            source->channels[i],
            source->channel_width,
            source->channel_height);
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void copy_buffer_to_region(
    image::buffer_t *target,
    image::buffer_t *source,
    size_t           target_x,
    size_t           target_y)
{
    size_t num_rows      = source->channel_height;
    size_t num_cols      = source->channel_width;
    size_t source_width  = source->channel_width;
    size_t target_height = target->channel_height;
    size_t target_width  = target->channel_width;
    size_t target_offset = target_y * target_width + target_x;
    size_t source_offset = 0;
    size_t copy_size     = 0;

    // clip to the bottom edge of the target image.
    if (target_y  + num_rows > target_height)
        num_rows -=(target_y + num_rows) - target_height;

    // clip to the right edge of the target image.
    if (target_x  + num_cols > target_width)
        num_cols -=(target_x + num_cols) - target_width;

    // figure out the number of bytes to copy per-row.
    copy_size = num_cols * sizeof(float);

    // copy scanline by scanline, channel by channel.
    for (size_t src_y = 0; src_y < num_rows; ++src_y)
    {
        for (size_t c = 0; c < source->channel_count; ++c)
        {
            float *source_ptr = source->channels[c] + source_offset;
            float *target_ptr = target->channels[c] + target_offset;
            memcpy(target_ptr, source_ptr, copy_size);
        }
        source_offset += source_width;
        target_offset += target_width;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool resize_buffer(
    image::buffer_t *source,
    size_t           new_width,
    size_t           new_height,
    int32_t          border_mode,
    image::buffer_t *target)
{
    image::buffer_t              tb;
    image::kaiser_args_t         fa;
    image::polyphase_kernel_1d_t fx;
    image::polyphase_kernel_1d_t fy;
    float  width   = 1.0f; // filter width
    size_t samples = 32;   // sample count
    size_t src_w   = source->channel_width;
    size_t src_h   = source->channel_height;
    size_t dst_w   = new_width;
    size_t dst_h   = new_height;
    size_t fx_size = image::polyphase_1d_init(src_w, dst_w, samples, width, &fx);
    size_t fy_size = image::polyphase_1d_init(src_h, dst_h, samples, width, &fy);

    // allocate a temporary buffer (dst_w, src_h) to hold the results scaled
    // in the horizontal dimension, and our final buffer (dst_w, dst_h).
    if (!create_buffer(dst_w, dst_h, source->channel_count, target))
    {
        // failed to allocate final result buffer.
        return false;
    }
    if (!create_buffer(dst_w, src_h, source->channel_count, &tb))
    {
        // failed to allocate temporary buffer.
        free_buffer(target);
        return false;
    }

    // allocate memory for the kernel filter weights (polyphase matrices).
    fx.filter_weights = (float*) malloc(fx_size);
    if (NULL == fx.filter_weights)
    {
        free_buffer(&tb);
        free_buffer(target);
        return false;
    }
    fy.filter_weights = (float*) malloc(fy_size);
    if (NULL == fy.filter_weights)
    {
        free(fx.filter_weights);
        free_buffer(&tb);
        free_buffer(target);
        return false;
    }

    // initialize the Lanczos filter and filter weights.
    image::kaiser_args_init(width, &fa);
    image::compute_polyphase_matrix_1d(image::kaiser_filter, &fa, &fx);
    image::compute_polyphase_matrix_1d(image::kaiser_filter, &fa, &fy);

    // allocate a small buffer to store a single image column.
    float *tmp_cd = (float*) malloc(sizeof(float) * dst_h);

    // resample the image in each direction independently.
    for (size_t c = 0; c < source->channel_count; ++c)
    {
        // resize along the horizontal direction from source into tb.
        for (size_t y = 0; y < src_h; ++y)
        {
            size_t od = y  * dst_w;
            image::apply_polyphase_horizontal_1d(
                &fx, border_mode, y,
                src_w, src_h,
                source->channels[c],
                tb.channels[c] + od);
        }
        // resize along the vertical direction from tb into target.
        for (size_t x = 0; x < dst_w; ++x)
        {
            float *sc = tb.channels[c];
            float *tc = target->channels[c];
            image::apply_polyphase_vertical_1d(
                &fy, border_mode, x,
                dst_w, src_h,
                sc,    tmp_cd);
            for (size_t y = 0; y  < dst_h; ++y)
                tc[y * dst_w + x] = tmp_cd[y];
        }
    }

    // clean up temporary resources.
    free(tmp_cd);
    free(fy.filter_weights);
    free(fx.filter_weights);
    free_buffer(&tb);
    return true;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool build_level0(
    image::buffer_t *source,
    size_t           target_width,
    size_t           target_height,
    int32_t          border_mode,
    image::buffer_t *target)
{
    size_t width    = target_width;
    size_t height   = target_height;
    size_t channels = source->channel_count;

    if (!create_buffer(width, height, channels, target))
    {
        return false;
    }
    if (source->channel_width  != target_width  ||
        source->channel_height != target_height)
    {
        resize_buffer(source, width, height, border_mode, target);
    }
    else
    {
        copy_buffer(target, source);
    }
    return true;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool build_mipmaps(
    image::buffer_t *level_0,
    int32_t          border_mode,
    size_t           level_count,
    image::buffer_t *level_data)
{
    size_t   color_count = level_0->channel_count;
    if (4 == color_count)
    {
        // don't include the alpha channel when converting color spaces.
        color_count = 3;
    }

    // store the data for the first mip-level.
    level_data[0] = *level_0;

    // build any additional levels from level_0.
    if (level_count > 1)
    {
        size_t   l0_w = level_0->channel_width;
        size_t   l0_h = level_0->channel_height;
        size_t   l0_c = level_0->channel_count;
        int32_t  mode = border_mode;

        // convert level_0 to linear-light space before downsampling.
        // our mipmaps will be in this linear-light space after filtering.
        // http://number-none.com/product/Mipmapping,%20Part%202/index.html
        image::linear(level_0, 0, color_count);
        // generate the mipmaps, each using the level_0 image as the
        // source to avoid propagation of artifacts.
        for (size_t i = 1; i < level_count; ++i)
        {
            size_t level_w = image::miplevel_width (l0_w, i);
            size_t level_h = image::miplevel_height(l0_h, i);
            create_buffer(level_w, level_h, l0_c, &level_data[i]);
            resize_buffer(level_0, level_w, level_h, mode, &level_data[i]);
            image::gamma(&level_data[i], 0, color_count);
        }
        // convert level_0 back to gamma-ramped space for storage and display.
        // http://number-none.com/product/Mipmapping,%20Part%202/index.html
        image::gamma(level_0, 0, color_count);
    }
    return true;
}

/*/////////////////////////////////////////////////////////////////////////80*/

bool compile_texture(
    texture_compiler_inputs_t  *inputs,
    texture_compiler_outputs_t *outputs)
{
    texture_compiler_outputs_init(outputs);
    texture_compiler_inputs_sanitize(inputs);

    // create our highest-resolution working buffer.
    image::buffer_t  level_0;
    size_t           level_0_w   = inputs->target_width;
    size_t           level_0_h   = inputs->target_height;
    int32_t          mode        = inputs->border_mode;
    build_level0(inputs->input_image, level_0_w, level_0_h, mode, &level_0);
    if (inputs->flip_y)  image::flip(&level_0);

    // generate mipmaps (or not, if level_count is 1).
    size_t           level_count = inputs->maximum_levels;
    image::buffer_t *level_data  = outputs->level_data;
    build_mipmaps(&level_0, mode,  level_count, level_data);

    // pre-multiply RGB color values by alpha, if desired  and
    // if the image has four channels (one assumed to be alpha).
    if (inputs->premultiply_a && 4 == level_0.channel_count)
    {
        for (size_t i = 0; i < level_count; ++i)
        {
            image::premultiply_alpha(
                &level_data[i], 0, 3,
                 level_data[i].channels[3]);
        }
    }

    // we're done; set the result structure.
    outputs->error_message  = NO_ERROR;
    outputs->channel_count  = level_0.channel_count;
    outputs->level_count    = inputs->maximum_levels;
    return true;
}

/*/////////////////////////////////////////////////////////////////////////80*/

#define MAKE_RGB565(r, g, b) \
    ((uint16_t) ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3))

void* buffer_to_pixels_16i_565(image::buffer_t *buffer)
{
    if (buffer->channel_count < 3)
        return NULL;

    size_t   bpp     = sizeof(uint16_t);
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    uint16_t *pixels = (uint16_t*) malloc(width * height * bpp);
    uint16_t *dest   =    pixels;
    for (size_t i    = 0; pixels  &&  i < width * height;  ++i)
    {
        uint8_t r    = (uint8_t) (*source0++ * 255.0f);
        uint8_t g    = (uint8_t) (*source1++ * 255.0f);
        uint8_t b    = (uint8_t) (*source2++ * 255.0f);
        *dest++      = MAKE_RGB565(r, g, b);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

#define MAKE_RGB4444(r, g, b, a) \
    ((uint16_t) ((r >> 4) << 12) | ((g >> 4) << 8) | ((b >> 4) << 4) | ((a >> 4)))

void* buffer_to_pixels_16i_4444(image::buffer_t *buffer)
{
    if (buffer->channel_count < 4)
        return NULL;

    size_t   bpp     = sizeof(uint16_t);
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    float   *source3 = buffer->channels[3];
    uint16_t *pixels = (uint16_t*) malloc(width * height * bpp);
    uint16_t *dest   =    pixels;
    for (size_t i    = 0; pixels  &&  i < width * height;  ++i)
    {
        uint8_t r    = (uint8_t) (*source0++ * 255.0f);
        uint8_t g    = (uint8_t) (*source1++ * 255.0f);
        uint8_t b    = (uint8_t) (*source2++ * 255.0f);
        uint8_t a    = (uint8_t) (*source3++ * 255.0f);
        *dest++      = MAKE_RGB4444(r, g, b, a);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

#define MAKE_RGB5551(r, g, b, a) \
    ((uint16_t) ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1) | ((a >> 7)))

void* buffer_to_pixels_16i_5551(image::buffer_t *buffer)
{
    if (buffer->channel_count < 4)
        return NULL;

    size_t   bpp     = sizeof(uint16_t);
    size_t   width   = buffer->channel_width;
    size_t   height  = buffer->channel_height;
    float   *source0 = buffer->channels[0];
    float   *source1 = buffer->channels[1];
    float   *source2 = buffer->channels[2];
    float   *source3 = buffer->channels[3];
    uint16_t *pixels = (uint16_t*) malloc(width * height * bpp);
    uint16_t *dest   =    pixels;
    for (size_t i    = 0; pixels  &&  i < width * height;  ++i)
    {
        uint8_t r    = (uint8_t) (*source0++ * 255.0f);
        uint8_t g    = (uint8_t) (*source1++ * 255.0f);
        uint8_t b    = (uint8_t) (*source2++ * 255.0f);
        uint8_t a    = (uint8_t) (*source3++ * 255.0f);
        *dest++      = MAKE_RGB5551(r, g, b, a);
    }
    return pixels;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_32i(image::buffer_t *buffer)
{
    switch (buffer->channel_count)
    {
        case 1:  return buffer_to_pixels_1_32(buffer);
        case 2:  return buffer_to_pixels_2_32(buffer);
        case 3:  return buffer_to_pixels_3_32(buffer);
        case 4:  return buffer_to_pixels_4_32(buffer);
        default: break;
    }
    return NULL;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_64f(image::buffer_t *buffer)
{
    switch (buffer->channel_count)
    {
        case 1:  return buffer_to_pixels_1_16f(buffer);
        case 2:  return buffer_to_pixels_2_16f(buffer);
        case 3:  return buffer_to_pixels_3_16f(buffer);
        case 4:  return buffer_to_pixels_4_16f(buffer);
        default: break;
    }
    return NULL;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void* buffer_to_pixels_128f(image::buffer_t *buffer)
{
    switch (buffer->channel_count)
    {
        case 1:  return buffer_to_pixels_1_32f(buffer);
        case 2:  return buffer_to_pixels_2_32f(buffer);
        case 3:  return buffer_to_pixels_3_32f(buffer);
        case 4:  return buffer_to_pixels_4_32f(buffer);
        default: break;
    }
    return NULL;
}

/*/////////////////////////////////////////////////////////////////////////80*/

void free_buffer(image::buffer_t *buffer)
{
    if (buffer->channel_data != NULL)
    {
        free(buffer->channel_data);
        buffer->channel_data  = NULL;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

void free_pixels(void *pixels)
{
    if (pixels) free(pixels);
}

/*/////////////////////////////////////////////////////////////////////////80*/

/*/////////////////////////////////////////////////////////////////////////////
//    $Id$
///////////////////////////////////////////////////////////////////////////80*/
