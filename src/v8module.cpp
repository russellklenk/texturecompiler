/*/////////////////////////////////////////////////////////////////////////////
/// @summary Implements the V8 module exports and Node.js add-on init() routine
/// to expose the functionality of this add-on to V8/JavaScript and Node.js.
///////////////////////////////////////////////////////////////////////////80*/

/*////////////////
//   Includes   //
////////////////*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <node.h>
#include <v8.h>
#include "compiler.hpp"

/*//////////////////////////
//   Using Declarations   //
//////////////////////////*/

/*//////////////////////
//   Implementation   //
//////////////////////*/

/*/////////////////////////////////////////////////////////////////////////80*/

#undef  STRINGIFY
#define STRINGIFY(e)    #e

/*/////////////////////////////////////////////////////////////////////////80*/

#define SAFE_FREE(f)    \
    if ((f))            \
    {                   \
        free((f));      \
        (f) = NULL;     \
    }

/*/////////////////////////////////////////////////////////////////////////80*/

enum texture_format_e
{
    TEXTURE_FORMAT_UNKNOWN      = 0,
    TEXTURE_FORMAT_565_I        = 1,  /// 'RGB565'
    TEXTURE_FORMAT_5551_I       = 2,  /// 'RGBA5551'
    TEXTURE_FORMAT_4444_I       = 3,  /// 'RGBA4444'
    TEXTURE_FORMAT_8_I          = 4,  /// 'R8' (default 1-channel)
    TEXTURE_FORMAT_88_I         = 5,  /// 'RG8' (default 2-channel)
    TEXTURE_FORMAT_888_I        = 6,  /// 'RGB8' or 'RGB' (default 3-channel)
    TEXTURE_FORMAT_8888_I       = 7,  /// 'RGBA8' or 'RGBA' (default 4-channel)
    TEXTURE_FORMAT_16_F         = 8,  /// 'R16F'
    TEXTURE_FORMAT_1616_F       = 9,  /// 'RG16F'
    TEXTURE_FORMAT_161616_F     = 10, /// 'RGB16F'
    TEXTURE_FORMAT_16161616_F   = 11, /// 'RGBA16F'
    TEXTURE_FORMAT_32_F         = 12, /// 'R32F'
    TEXTURE_FORMAT_3232_F       = 13, /// 'RG32F'
    TEXTURE_FORMAT_323232_F     = 14, /// 'RGB32F'
    TEXTURE_FORMAT_32323232_F   = 15, /// 'RGBA32F'
    TEXTURE_FORMAT_COUNT        = 16,
    TEXTURE_FORMAT_FORCE_32BIT  = CMN_FORCE_32BIT
};

/*/////////////////////////////////////////////////////////////////////////80*/

enum texture_target_e
{
    TEXTURE_TARGET_UNKNOWN      = 0,
    TEXTURE_TARGET_2D           = 1,  /// 'TEXTURE_2D' (default)
    TEXTURE_TARGET_CUBE_POS_X   = 2,  /// 'TEXTURE_CUBE_MAP_POSITIVE_X'
    TEXTURE_TARGET_CUBE_NEG_X   = 3,  /// 'TEXTURE_CUBE_MAP_NEGATIVE_X'
    TEXTURE_TARGET_CUBE_POS_Y   = 4,  /// 'TEXTURE_CUBE_MAP_POSITIVE_Y'
    TEXTURE_TARGET_CUBE_NEG_Y   = 5,  /// 'TEXTURE_CUBE_MAP_NEGATIVE_Y'
    TEXTURE_TARGET_CUBE_POS_Z   = 6,  /// 'TEXTURE_CUBE_MAP_POSITIVE_Z'
    TEXTURE_TARGET_CUBE_NEG_Z   = 7,  /// 'TEXTURE_CUBE_MAP_NEGATIVE_Z'
    TEXTURE_TARGET_COUNT        = 8,
    TEXTURE_TARGET_FORCE_32BIT  = CMN_FORCE_32BIT
};

/*/////////////////////////////////////////////////////////////////////////80*/

enum texture_type_e
{
    TEXTURE_TYPE_UNKNOWN        = 0,
    TEXTURE_TYPE_COLOR          = 1,  /// 'COLOR' (default 3/4-channel)
    TEXTURE_TYPE_ALPHA          = 2,  /// 'ALPHA'
    TEXTURE_TYPE_LUMINANCE      = 3,  /// 'LUMINANCE' (default 1-channel)
    TEXTURE_TYPE_LUMINANCE_ALPHA= 4,  /// 'LUMINANCE_ALPHA' (default 2-channel)
    TEXTURE_TYPE_DISTANCE_FIELD = 5,  /// 'DISTANCE_FIELD'
    TEXTURE_TYPE_HEIGHT         = 6,  /// 'HEIGHT'
    TEXTURE_TYPE_NORMAL         = 7,  /// 'NORMAL'
    TEXTURE_TYPE_COUNT          = 8,
    TEXTURE_TYPE_FORCE_32BIT    = CMN_FORCE_32BIT
};

/*/////////////////////////////////////////////////////////////////////////80*/

enum texture_wrap_e
{
    TEXTURE_WRAP_UNKNOWN        = 0,
    TEXTURE_WRAP_REPEAT         = 1,  /// 'REPEAT'
    TEXTURE_WRAP_CLAMP_TO_EDGE  = 2,  /// 'CLAMP_TO_EDGE' (default)
    TEXTURE_WRAP_MIRRORED_REPEAT= 3,  /// 'MIRRORED_REPEAT'
    TEXTURE_WRAP_COUNT          = 4,
    TEXTURE_WRAP_FORCE_32BIT    = CMN_FORCE_32BIT
};

/*/////////////////////////////////////////////////////////////////////////80*/

enum texture_filter_e
{
    TEXTURE_FILTER_UNKNOWN      = 0,
    TEXTURE_FILTER_NEAREST      = 1,  /// 'NEAREST'
    TEXTURE_FILTER_LINEAR       = 2,  /// 'LINEAR' (default)
    TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST = 3,  /// 'NEAREST_MIPMAP_NEAREST'
    TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR  = 4,  /// 'NEAREST_MIPMAP_LINEAR'
    TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST  = 5,  /// 'LINEAR_MIPMAP_NEAREST'
    TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR   = 6,  /// 'LINEAR_MIPMAP_LINEAR'
    TEXTURE_FILTER_COUNT        = 7,
    TEXTURE_FILTER_FORCE_32BIT  = CMN_FORCE_32BIT
};

/*/////////////////////////////////////////////////////////////////////////80*/

struct texture_compiler_args_t
{
    char    *source_path;       /// The path of the input file.
    char    *target_path;       /// The path of the output file.
    char    *target_format;     /// One of the texture_format_e strings.
    char    *texture_type;      /// One of the texture_type_e strings.
    char    *texture_target;    /// One of the texture_target_e strings.
    char    *magnify_filter;    /// One of the texture_wrap_e strings.
    char    *minify_filter;     /// One of the texture_wrap_e strings.
    char    *wrap_mode_s;       /// One of the texture_filter_e strings.
    char    *wrap_mode_t;       /// One of the texture_filter_e strings.
    char    *border_mode;       /// One of the image::border_mode_e strings.
    bool     flip_y;            /// Flip the image before writing it out?
    bool     premultiplied;     /// Store with alpha premultiplied?
    bool     force_pow2;        /// Force to power-of-two dimensions?
    bool     build_mipmaps;     /// Do we build mipmaps for this texture?
    uint32_t level_count;       /// The number of mipmap levels (0 = all).
    size_t   target_width;      /// The specific target width to force.
    size_t   target_height;     /// The specific target height to force.
};

/*/////////////////////////////////////////////////////////////////////////80*/

static void init_compiler_args(texture_compiler_args_t *args)
{
    if (args)
    {
        args->source_path    = NULL;
        args->target_path    = NULL;
        args->target_format  = NULL;
        args->texture_type   = NULL;
        args->texture_target = NULL;
        args->magnify_filter = NULL;
        args->minify_filter  = NULL;
        args->wrap_mode_s    = NULL;
        args->wrap_mode_t    = NULL;
        args->border_mode    = NULL;
        args->flip_y         = false;
        args->premultiplied  = false;
        args->build_mipmaps  = false;
        args->level_count    = 0;
        args->target_width   = 0;
        args->target_height  = 0;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

static void free_compiler_args(texture_compiler_args_t *args)
{
    if (args)
    {
        SAFE_FREE(args->source_path);
        SAFE_FREE(args->target_path);
        SAFE_FREE(args->target_format);
        SAFE_FREE(args->texture_type);
        SAFE_FREE(args->texture_target);
        SAFE_FREE(args->magnify_filter);
        SAFE_FREE(args->minify_filter);
        SAFE_FREE(args->wrap_mode_s);
        SAFE_FREE(args->wrap_mode_t);
        SAFE_FREE(args->border_mode);
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

static int32_t texture_format(char const *str, size_t channel_count)
{
    if (NULL == str || 0 == strlen(str))
    {
        switch (channel_count)
        {
            case 1:   return TEXTURE_FORMAT_8_I;
            case 2:   return TEXTURE_FORMAT_88_I;
            case 3:   return TEXTURE_FORMAT_888_I;
            case 4:   return TEXTURE_FORMAT_8888_I;
            default:  break;
        }
        return TEXTURE_FORMAT_UNKNOWN;
    }
    if (!strcmp(str, "RGB565"))   return TEXTURE_FORMAT_565_I;
    if (!strcmp(str, "RGBA4444")) return TEXTURE_FORMAT_4444_I;
    if (!strcmp(str, "RGBA5551")) return TEXTURE_FORMAT_5551_I;
    if (!strcmp(str, "R8"))       return TEXTURE_FORMAT_8_I;
    if (!strcmp(str, "RG8"))      return TEXTURE_FORMAT_88_I;
    if (!strcmp(str, "RGB"))      return TEXTURE_FORMAT_888_I;
    if (!strcmp(str, "RGB8"))     return TEXTURE_FORMAT_888_I;
    if (!strcmp(str, "RGBA"))     return TEXTURE_FORMAT_8888_I;
    if (!strcmp(str, "RGBA8"))    return TEXTURE_FORMAT_8888_I;
    if (!strcmp(str, "R16F"))     return TEXTURE_FORMAT_16_F;
    if (!strcmp(str, "RG16F"))    return TEXTURE_FORMAT_1616_F;
    if (!strcmp(str, "RGB16F"))   return TEXTURE_FORMAT_161616_F;
    if (!strcmp(str, "RGBA16F"))  return TEXTURE_FORMAT_16161616_F;
    if (!strcmp(str, "R32F"))     return TEXTURE_FORMAT_32_F;
    if (!strcmp(str, "RG32F"))    return TEXTURE_FORMAT_3232_F;
    if (!strcmp(str, "RGB32F"))   return TEXTURE_FORMAT_323232_F;
    if (!strcmp(str, "RGBA32F"))  return TEXTURE_FORMAT_32323232_F;
    return TEXTURE_FORMAT_UNKNOWN;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static int32_t texture_target(char const *str)
{
    if (NULL == str || 0 == strlen(str)) return TEXTURE_TARGET_2D;
    if (!strcmp(str, "TEXTURE_2D"))      return TEXTURE_TARGET_2D;
    if (!strcmp(str, "TEXTURE_CUBE_MAP_POSITIVE_X"))
        return TEXTURE_TARGET_CUBE_POS_X;
    if (!strcmp(str, "TEXTURE_CUBE_MAP_NEGATIVE_X"))
        return TEXTURE_TARGET_CUBE_NEG_X;
    if (!strcmp(str, "TEXTURE_CUBE_MAP_POSITIVE_Y"))
        return TEXTURE_TARGET_CUBE_POS_Y;
    if (!strcmp(str, "TEXTURE_CUBE_MAP_NEGATIVE_Y"))
        return TEXTURE_TARGET_CUBE_NEG_Y;
    if (!strcmp(str, "TEXTURE_CUBE_MAP_POSITIVE_Z"))
        return TEXTURE_TARGET_CUBE_POS_Z;
    if (!strcmp(str, "TEXTURE_CUBE_MAP_NEGATIVE_Z"))
        return TEXTURE_TARGET_CUBE_NEG_Z;
    return TEXTURE_TARGET_UNKNOWN;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static int32_t texture_type(char const *str, size_t channel_count)
{
    if (NULL == str || 0 == strlen(str))
    {
        switch (channel_count)
        {
            case 1:   return TEXTURE_TYPE_LUMINANCE;
            case 2:   return TEXTURE_TYPE_LUMINANCE_ALPHA;
            case 3:   return TEXTURE_TYPE_COLOR;
            case 4:   return TEXTURE_TYPE_COLOR;
            default:  break;
        }
        return TEXTURE_TYPE_UNKNOWN;
    }
    if (!strcmp(str, "COLOR"))           return TEXTURE_TYPE_COLOR;
    if (!strcmp(str, "ALPHA"))           return TEXTURE_TYPE_ALPHA;
    if (!strcmp(str, "LUMINANCE"))       return TEXTURE_TYPE_LUMINANCE;
    if (!strcmp(str, "LUMINANCE_ALPHA")) return TEXTURE_TYPE_LUMINANCE_ALPHA;
    if (!strcmp(str, "DISTANCE_FIELD"))  return TEXTURE_TYPE_DISTANCE_FIELD;
    if (!strcmp(str, "HEIGHT"))          return TEXTURE_TYPE_HEIGHT;
    if (!strcmp(str, "NORMAL"))          return TEXTURE_TYPE_NORMAL;
    return TEXTURE_TYPE_UNKNOWN;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static int32_t texture_wrap(char const *str)
{
    if (NULL == str || 0 == strlen(str)) return TEXTURE_WRAP_CLAMP_TO_EDGE;
    if (!strcmp(str, "REPEAT"))          return TEXTURE_WRAP_REPEAT;
    if (!strcmp(str, "CLAMP_TO_EDGE"))   return TEXTURE_WRAP_CLAMP_TO_EDGE;
    if (!strcmp(str, "MIRRORED_REPEAT")) return TEXTURE_WRAP_MIRRORED_REPEAT;
    return TEXTURE_WRAP_UNKNOWN;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static int32_t minify_filter(char const *str, bool mipmaps)
{
    if (mipmaps)
    {
        // textures with mipmaps support a wider range of filters.
        if (NULL == str || 0 == strlen(str))
            return TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
        if (!strcmp(str, "NEAREST"))
            return TEXTURE_FILTER_NEAREST;
        if (!strcmp(str, "LINEAR"))
            return TEXTURE_FILTER_LINEAR;
        if (!strcmp(str, "NEAREST_MIPMAP_NEAREST"))
            return TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;
        if (!strcmp(str, "NEAREST_MIPMAP_LINEAR"))
            return TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
        if (!strcmp(str, "LINEAR_MIPMAP_NEAREST"))
            return TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST;
        if (!strcmp(str, "LINEAR_MIPMAP_LINEAR"))
            return TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
        return TEXTURE_FILTER_UNKNOWN;
    }
    else
    {
        // only NEAREST and LINEAR are supported.
        if (NULL == str || 0 == strlen(str)) return TEXTURE_FILTER_LINEAR;
        if (!strcmp(str, "NEAREST"))         return TEXTURE_FILTER_NEAREST;
        if (!strcmp(str, "LINEAR"))          return TEXTURE_FILTER_LINEAR;
        return TEXTURE_FILTER_UNKNOWN;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

static int32_t magnify_filter(char const *str)
{
    if (NULL == str || 0 == strlen(str)) return TEXTURE_FILTER_LINEAR;
    if (!strcmp(str, "NEAREST"))         return TEXTURE_FILTER_NEAREST;
    if (!strcmp(str, "LINEAR"))          return TEXTURE_FILTER_LINEAR;
    return TEXTURE_FILTER_UNKNOWN;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static int32_t border_sample_mode(char const *str)
{
    if (NULL == str || 0 == strlen(str)) return image::BORDER_MODE_MIRROR;
    if (!strcmp(str, "WRAP"))            return image::BORDER_MODE_WRAP;
    if (!strcmp(str, "CLAMP"))           return image::BORDER_MODE_CLAMP;
    if (!strcmp(str, "MIRROR"))          return image::BORDER_MODE_MIRROR;
    return image::BORDER_MODE_MIRROR;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static void texture_format_bits_per_pixel(int32_t format, size_t *out_bpp)
{
    switch (format)
    {
        case TEXTURE_FORMAT_565_I:      *out_bpp =  16; break;
        case TEXTURE_FORMAT_5551_I:     *out_bpp =  16; break;
        case TEXTURE_FORMAT_4444_I:     *out_bpp =  16; break;
        case TEXTURE_FORMAT_8_I:        *out_bpp =   8; break;
        case TEXTURE_FORMAT_88_I:       *out_bpp =  16; break;
        case TEXTURE_FORMAT_888_I:      *out_bpp =  24; break;
        case TEXTURE_FORMAT_8888_I:     *out_bpp =  32; break;
        case TEXTURE_FORMAT_16_F:       *out_bpp =  16; break;
        case TEXTURE_FORMAT_1616_F:     *out_bpp =  32; break;
        case TEXTURE_FORMAT_161616_F:   *out_bpp =  48; break;
        case TEXTURE_FORMAT_16161616_F: *out_bpp =  64; break;
        case TEXTURE_FORMAT_32_F:       *out_bpp =  32; break;
        case TEXTURE_FORMAT_3232_F:     *out_bpp =  64; break;
        case TEXTURE_FORMAT_323232_F:   *out_bpp =  96; break;
        case TEXTURE_FORMAT_32323232_F: *out_bpp = 128; break;
        default:                        *out_bpp =   0; break;
    }
}

/*/////////////////////////////////////////////////////////////////////////80*/

static void level_byte_size(
    int32_t format,
    size_t  width,
    size_t  height,
    size_t *out_bpp,
    size_t *out_size)
{
    size_t bpp = 0;
    texture_format_bits_per_pixel(format, &bpp);
    *out_bpp   = bpp;
    *out_size  = width * height * (bpp / 8);
}

/*/////////////////////////////////////////////////////////////////////////80*/

static void* level_descriptor(
    image::buffer_t *level,
    int32_t          format,
    size_t          *out_bpp,
    size_t          *out_size)
{
    size_t width  = level->channel_width;
    size_t height = level->channel_height;
    level_byte_size(format, width, height, out_bpp, out_size);
    switch (format)
    {
        case TEXTURE_FORMAT_565_I:      return buffer_to_pixels_16i_565(level);
        case TEXTURE_FORMAT_5551_I:     return buffer_to_pixels_16i_5551(level);
        case TEXTURE_FORMAT_4444_I:     return buffer_to_pixels_16i_4444(level);
        case TEXTURE_FORMAT_8_I:        return buffer_to_pixels_32i(level);
        case TEXTURE_FORMAT_88_I:       return buffer_to_pixels_32i(level);
        case TEXTURE_FORMAT_888_I:      return buffer_to_pixels_32i(level);
        case TEXTURE_FORMAT_8888_I:     return buffer_to_pixels_32i(level);
        case TEXTURE_FORMAT_16_F:       return buffer_to_pixels_64f(level);
        case TEXTURE_FORMAT_1616_F:     return buffer_to_pixels_64f(level);
        case TEXTURE_FORMAT_161616_F:   return buffer_to_pixels_64f(level);
        case TEXTURE_FORMAT_16161616_F: return buffer_to_pixels_64f(level);
        case TEXTURE_FORMAT_32_F:       return buffer_to_pixels_128f(level);
        case TEXTURE_FORMAT_3232_F:     return buffer_to_pixels_128f(level);
        case TEXTURE_FORMAT_323232_F:   return buffer_to_pixels_128f(level);
        case TEXTURE_FORMAT_32323232_F: return buffer_to_pixels_128f(level);
        default:                        break;
    }
    return NULL;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static void  dump_data(char const *path, void const *data, size_t size)
{
    FILE  *fp = fopen(path, "wb");
    fwrite(data, size, 1, fp);
    fclose(fp);
}

/*/////////////////////////////////////////////////////////////////////////80*/

static v8::Handle<v8::Value> ex(char const *message)
{
    v8::HandleScope scope;
    if (NULL == message)  message = "";
    return scope.Close(v8::Exception::Error(v8::String::New(message)));
}

/*/////////////////////////////////////////////////////////////////////////80*/

/// Extracts a UTF8 character string from a V8 string object.
/// @param v The V8 string object.
/// @return A pointer to the newly allocated buffer. Use the standard C library
/// free() function to release the allocated memory.
static char* v8_string_to_utf8(v8::Local<v8::Value> v)
{
    v8::Local<v8::String> v_str = v->ToString();
    int                   b_len = v_str->Utf8Length() + 1;
    char                 *c_buf = (char*) malloc(b_len);
    int                   c_len = 0;
    v_str->WriteUtf8(c_buf, b_len, &c_len);
    return c_buf;
}

/*/////////////////////////////////////////////////////////////////////////80*/

static v8::Handle<v8::Value> gl_format_v8(
    char const *type,
    size_t      channel_count)
{
    v8::HandleScope scope;
    switch (channel_count)
    {
        case 1:
            if (!strcmp("ALPHA", type))
                return scope.Close(v8::String::New("ALPHA"));
            else
                return scope.Close(v8::String::New("LUMINANCE"));
        case 2:
            return scope.Close(v8::String::New("LUMINANCE_ALPHA"));
        case 3:
            return scope.Close(v8::String::New("RGB"));
        case 4:
            return scope.Close(v8::String::New("RGBA"));
        default:
            break;
    }
    return scope.Close(ex("Invalid channel count in gl_format_v8."));
}

/*/////////////////////////////////////////////////////////////////////////80*/

static v8::Handle<v8::Value> gl_target_v8(char const *target)
{
    v8::HandleScope     scope;
    int32_t target_id = texture_target(target);
    switch (target_id)
    {
        case TEXTURE_TARGET_2D:
            return scope.Close(v8::String::New("TEXTURE_2D"));
        case TEXTURE_TARGET_CUBE_POS_X:
            return scope.Close(v8::String::New("TEXTURE_CUBE_MAP_POSITIVE_X"));
        case TEXTURE_TARGET_CUBE_NEG_X:
            return scope.Close(v8::String::New("TEXTURE_CUBE_MAP_NEGATIVE_X"));
        case TEXTURE_TARGET_CUBE_POS_Y:
            return scope.Close(v8::String::New("TEXTURE_CUBE_MAP_POSITIVE_Y"));
        case TEXTURE_TARGET_CUBE_NEG_Y:
            return scope.Close(v8::String::New("TEXTURE_CUBE_MAP_NEGATIVE_Y"));
        case TEXTURE_TARGET_CUBE_POS_Z:
            return scope.Close(v8::String::New("TEXTURE_CUBE_MAP_POSITIVE_Z"));
        case TEXTURE_TARGET_CUBE_NEG_Z:
            return scope.Close(v8::String::New("TEXTURE_CUBE_MAP_NEGATIVE_Z"));
        default:
            break;
    }
    return scope.Close(ex("Invalid target in gl_target_v8."));
}

/*/////////////////////////////////////////////////////////////////////////80*/

static v8::Handle<v8::Value> gl_data_type_v8(
    char const *format,
    size_t      channel_count)
{
    v8::HandleScope     scope;
    int32_t format_id = texture_format(format, channel_count);
    switch (format_id)
    {
        case TEXTURE_FORMAT_565_I:
            return scope.Close(v8::String::New("UNSIGNED_SHORT_5_6_5"));
        case TEXTURE_FORMAT_5551_I:
            return scope.Close(v8::String::New("UNSIGNED_SHORT_5_5_5_1"));
        case TEXTURE_FORMAT_4444_I:
            return scope.Close(v8::String::New("UNSIGNED_SHORT_4_4_4_1"));
        case TEXTURE_FORMAT_8_I:
        case TEXTURE_FORMAT_88_I:
        case TEXTURE_FORMAT_888_I:
        case TEXTURE_FORMAT_8888_I:
            return scope.Close(v8::String::New("UNSIGNED_BYTE"));
        case TEXTURE_FORMAT_16_F:
        case TEXTURE_FORMAT_1616_F:
        case TEXTURE_FORMAT_161616_F:
        case TEXTURE_FORMAT_16161616_F:
            return scope.Close(v8::String::New("HALF_FLOAT_OES"));
        case TEXTURE_FORMAT_32_F:
        case TEXTURE_FORMAT_3232_F:
        case TEXTURE_FORMAT_323232_F:
        case TEXTURE_FORMAT_32323232_F:
            return scope.Close(v8::String::New("FLOAT"));
        default:
            break;
    }
    return scope.Close(ex("Invalid format in gl_data_type_v8."));
}

/*/////////////////////////////////////////////////////////////////////////80*/

/// Extracts texture compiler arguments from an object passed from JavaScript.
/// @param obj An object specifying the texture compiler arguments.
/// @param obj.sourcePath A string specifying the path of the source file.
/// @param obj.targetPath A string specifying the path of the target file.
/// @param args Pointer to the texture_compiler_args_t structure to populate.
/// @return undefined if the operation is successful; otherwise a V8 exception.
static v8::Handle<v8::Value> v8_object_to_compiler_args(
    v8::Local<v8::Object>    obj,
    texture_compiler_args_t *args)
{
    v8::HandleScope          scope;
    v8::Handle<v8::String>   type          = v8::String::New("type");
    v8::Handle<v8::String>   flipY         = v8::String::New("flipY");
    v8::Handle<v8::String>   format        = v8::String::New("format");
    v8::Handle<v8::String>   target        = v8::String::New("target");
    v8::Handle<v8::String>   wrapModeS     = v8::String::New("wrapModeS");
    v8::Handle<v8::String>   wrapModeT     = v8::String::New("wrapModeT");
    v8::Handle<v8::String>   borderMode    = v8::String::New("borderMode");
    v8::Handle<v8::String>   sourcePath    = v8::String::New("sourcePath");
    v8::Handle<v8::String>   targetPath    = v8::String::New("targetPath");
    v8::Handle<v8::String>   targetWidth   = v8::String::New("targetWidth");
    v8::Handle<v8::String>   targetHeight  = v8::String::New("targetHeight");
    v8::Handle<v8::String>   minifyFilter  = v8::String::New("minifyFilter");
    v8::Handle<v8::String>   magnifyFilter = v8::String::New("magnifyFilter");
    v8::Handle<v8::String>   premultiplied = v8::String::New("premultipliedAlpha");
    v8::Handle<v8::String>   forcePowerOf2 = v8::String::New("forcePowerOf2");
    v8::Handle<v8::String>   buildMipmaps  = v8::String::New("buildMipmaps");
    v8::Handle<v8::String>   levelCount    = v8::String::New("levelCount");

    // source file path. this field is required.
    init_compiler_args(args);
    if (obj->Has(sourcePath))
        args->source_path = v8_string_to_utf8(obj->Get(sourcePath));
    else
        return scope.Close(ex("Missing required field sourcePath."));

    // target file path. this field is required.
    if (obj->Has(targetPath))
        args->target_path = v8_string_to_utf8(obj->Get(targetPath));
    else
        return scope.Close(ex("Missing required field targetPath."));

    // texture type. this field must be validated later.
    if (obj->Has(type))
        args->texture_type = v8_string_to_utf8(obj->Get(type));

    // texture format. this field must be validated later.
    if (obj->Has(format))
        args->target_format = v8_string_to_utf8(obj->Get(format));

    // texture target. this field must be validated later.
    if (obj->Has(target))
        args->texture_target = v8_string_to_utf8(obj->Get(target));

    // border mode. this field is optional.
    if (obj->Has(borderMode))
        args->border_mode =v8_string_to_utf8(obj->Get(borderMode));

    // flip Y. this field is optional.
    if (obj->Has(flipY))
        args->flip_y = obj->Get(flipY)->IsTrue() ? true : false;
    else
        args->flip_y = true;

    // force power of two? this field is optional.
    if (obj->Has(forcePowerOf2))
        args->force_pow2 = obj->Get(forcePowerOf2)->IsTrue() ? true : false;
    else
        args->force_pow2 = false;

    // build mipmaps? this field is optional.
    if (obj->Has(buildMipmaps))
        args->build_mipmaps = obj->Get(buildMipmaps)->IsTrue() ? true : false;
    else
        args->build_mipmaps = false;

    // premultiply alpha? this field is optional.
    if (obj->Has(premultiplied))
        args->premultiplied = obj->Get(premultiplied)->IsTrue() ? true : false;
    else
        args->premultiplied = false;

    // maximum number of mip-levels? this field is optional.
    if (obj->Has(levelCount))
        args->level_count = obj->Get(levelCount)->Uint32Value();
    else
        args->level_count = args->build_mipmaps ? 0 : 1;

    // specific output width? this field is optional.
    if (obj->Has(targetWidth))
        args->target_width = obj->Get(targetWidth)->Uint32Value();
    else
        args->target_width = 0;

    // specific otput height? this field is optional.
    if (obj->Has(targetHeight))
        args->target_height = obj->Get(targetHeight)->Uint32Value();
    else
        args->target_height = 0;

    // wrap mode S? this field is optional.
    if (obj->Has(wrapModeS))
        args->wrap_mode_s = v8_string_to_utf8(obj->Get(wrapModeS));
    else
        args->wrap_mode_s = strdup("CLAMP_TO_EDGE");

    // wrap mode T? this field is optional.
    if (obj->Has(wrapModeT))
        args->wrap_mode_t = v8_string_to_utf8(obj->Get(wrapModeT));
    else
        args->wrap_mode_t = strdup("CLAMP_TO_EDGE");

    // minify filter? this field is optional.
    if (obj->Has(minifyFilter))
    {
        args->minify_filter = v8_string_to_utf8(obj->Get(minifyFilter));
    }
    else
    {
        if (args->build_mipmaps)
            args->minify_filter = strdup("LINEAR_MIPMAP_LINEAR");
        else
            args->minify_filter = strdup("LINEAR");
    }

    // magnify filter? this field is optional.
    if (obj->Has(magnifyFilter))
        args->magnify_filter = v8_string_to_utf8(obj->Get(magnifyFilter));
    else
        args->magnify_filter = strdup("LINEAR");
    return scope.Close(v8::Undefined());
}

/*/////////////////////////////////////////////////////////////////////////80*/

static v8::Handle<v8::Value> validate_arguments(
    texture_compiler_args_t *args,
    image::buffer_t         *source)
{
    v8::HandleScope   scope;
    bool   mipmaps  = args->build_mipmaps;
    size_t channels = source->channel_count;
    if (TEXTURE_TYPE_UNKNOWN == texture_type(args->texture_type, channels))
    {
        return scope.Close(ex("The type field has an invalid value."));
    }
    if (TEXTURE_FORMAT_UNKNOWN == texture_format(args->target_format, channels))
    {
        return scope.Close(ex("The format field has an invalid value."));
    }
    if (TEXTURE_TARGET_UNKNOWN == texture_target(args->texture_target))
    {
        return scope.Close(ex("The target field has an invalid value."));
    }
    if (TEXTURE_WRAP_UNKNOWN == texture_wrap(args->wrap_mode_s))
    {
        return scope.Close(ex("The wrapModeS field has an invalid value."));
    }
    if (TEXTURE_WRAP_UNKNOWN == texture_wrap(args->wrap_mode_t))
    {
        return scope.Close(ex("The wrapModeT field has an invalid value."));
    }
    if (TEXTURE_FILTER_UNKNOWN == magnify_filter(args->magnify_filter))
    {
        return scope.Close(ex("The magnifyFilter field has an invalid value."));
    }
    if (TEXTURE_FILTER_UNKNOWN == minify_filter(args->minify_filter, mipmaps))
    {
        return scope.Close(ex("The minifyFilter field has an invalid value."));
    }
    return scope.Close(v8::Undefined());
}

/*/////////////////////////////////////////////////////////////////////////80*/

/// Outputs texture data to a raw file containing the pixel data for each mip-
/// level of the texture, without any header information.
/// @param target_path A string specifying the path and filename of the file
/// to create and write with the raw pixel data.
/// @param target_format One of the values of the texture_format_e enumeration
/// specifying the target format for the texture pixel data.
/// @param levels A V8 array object to be populated with objects describing
/// each mip-level of the texture.
/// @param outputs An object specifying the outputs from the texture compiler.
/// @return undefined if the operation completes successfully; otherwise, an
/// exception object is returned.
static v8::Handle<v8::Value> v8_output_raw(
    char const                 *target_path,
    int32_t                     target_format,
    v8::Handle<v8::Array>       levels,
    texture_compiler_outputs_t *outputs)
{
    FILE  *file        = fopen(target_path, "wb");
    size_t level_count = outputs->level_count;
    size_t byte_offset = 0;
    size_t byte_size   = 0;
    v8::HandleScope  scope;

    // cache some property names so we don't create them repeatedly.
    v8::Handle<v8::String> prop_byteOffset = v8::String::New("byteOffset");
    v8::Handle<v8::String> prop_byteSize   = v8::String::New("byteSize");
    v8::Handle<v8::String> prop_height     = v8::String::New("height");
    v8::Handle<v8::String> prop_width      = v8::String::New("width");

    // open the target file to write the raw pixel data.
    if (NULL == file)
    {
        return scope.Close(ex("Cannot create file targetPath."));
    }
    // write the raw pixel data and build a descriptor for each level.
    for (size_t i = 0; i < level_count; ++i)
    {
        v8::Handle<v8::Object> desc   = v8::Object::New();
        image::buffer_t       *data   = &outputs->level_data[i];
        size_t                 width  = data->channel_width;
        size_t                 height = data->channel_height;
        size_t                 bpp    = 0;

        // get the raw pixel data and write it to the file.
        void *pixels = level_descriptor(data, target_format, &bpp, &byte_size);
        if   (pixels)
        {
            fwrite(pixels, byte_size, 1, file);
            free_pixels(pixels);
        }
        else
        {
            fclose(file); file = NULL;
            return scope.Close(ex("Cannot get pixel data for mip-level."));
        }

        // build an object describing the miplevel.
        desc->Set(prop_width,      v8::Integer::NewFromUnsigned(width));
        desc->Set(prop_height,     v8::Integer::NewFromUnsigned(height));
        desc->Set(prop_byteOffset, v8::Integer::NewFromUnsigned(byte_offset));
        desc->Set(prop_byteSize,   v8::Integer::NewFromUnsigned(byte_size));

        // add it to the back of the descriptor array.
        levels->Set(i, desc);

        // update the byte offset for the next level.
        byte_offset += byte_size;
    }
    fclose(file); file = NULL;
    return scope.Close(v8::Undefined());
}

/*/////////////////////////////////////////////////////////////////////////80*/

static v8::Handle<v8::Object> output_to_v8_object(
    texture_compiler_args_t    *args,
    texture_compiler_outputs_t *output,
    v8::Handle<v8::Array>       levels)
{
    v8::HandleScope        scope;
    v8::Handle<v8::Object> metadata        = v8::Object::New();
    v8::Handle<v8::String> prop_type       = v8::String::New("type");
    v8::Handle<v8::String> prop_wrapS      = v8::String::New("wrapS");
    v8::Handle<v8::String> prop_wrapT      = v8::String::New("wrapT");
    v8::Handle<v8::String> prop_levels     = v8::String::New("levels");
    v8::Handle<v8::String> prop_target     = v8::String::New("target");
    v8::Handle<v8::String> prop_format     = v8::String::New("format");
    v8::Handle<v8::String> prop_dataType   = v8::String::New("dataType");
    v8::Handle<v8::String> prop_magFilter  = v8::String::New("magFilter");
    v8::Handle<v8::String> prop_minFilter  = v8::String::New("minFilter");
    v8::Handle<v8::String> prop_hasMipmaps = v8::String::New("hasMipmaps");

    char const *type_string      = args->texture_type;
    char const *format_string    = args->target_format;
    char const *target_string    = args->texture_target;
    size_t      channels         = output->channel_count;
    bool        mipmaps          = output->level_count > 1;
    metadata->Set(prop_type,       v8::String::New(args->texture_type));
    metadata->Set(prop_target,     gl_target_v8(target_string));
    metadata->Set(prop_format,     gl_format_v8(type_string, channels));
    metadata->Set(prop_dataType,   gl_data_type_v8(format_string, channels));
    metadata->Set(prop_wrapS,      v8::String::New(args->wrap_mode_s));
    metadata->Set(prop_wrapT,      v8::String::New(args->wrap_mode_t));
    metadata->Set(prop_magFilter,  v8::String::New(args->magnify_filter));
    metadata->Set(prop_minFilter,  v8::String::New(args->minify_filter));
    metadata->Set(prop_hasMipmaps, mipmaps ? v8::True() : v8::False());
    metadata->Set(prop_levels,     levels);
    return scope.Close(metadata);
}

/*/////////////////////////////////////////////////////////////////////////80*/

v8::Handle<v8::Value> Compile(v8::Arguments const &args)
{
    texture_compiler_args_t    tcarg;
    texture_compiler_inputs_t  tcinp;
    texture_compiler_outputs_t tcout;
    image::buffer_t            image;
    v8::HandleScope            scope;
    v8::Local<v8::Object>      params = args[0]->ToObject();

    // extract the arguments into something we can work with
    // without V8; verify that required arguments are present.
    v8::Handle<v8::Value> r1 = v8_object_to_compiler_args(params, &tcarg);
    if (!r1->IsUndefined())
    {
        // an exception was thrown. return it.
        free_compiler_args(&tcarg);
        return scope.Close(v8::ThrowException(r1));
    }

    // load the image from the specified source file.
    if (!file_to_buffer(tcarg.source_path, &image))
    {
        free_compiler_args(&tcarg);
        return scope.Close(ex("Cannot load file specified by sourcePath."));
    }

    // validate the arguments against the image properties.
    v8::Handle<v8::Value> r2 = validate_arguments(&tcarg, &image);
    if (!r2->IsUndefined())
    {
        // an exception was thrown. return it.
        free_buffer(&image);
        free_compiler_args(&tcarg);
        return scope.Close(v8::ThrowException(r2));
    }

    // set up the inputs to the texture compiler.
    texture_compiler_inputs_init(&tcinp);
    texture_compiler_outputs_init(&tcout);
    tcinp.input_image    = &image;
    tcinp.border_mode    = border_sample_mode(tcarg.border_mode);
    tcinp.target_width   = tcarg.target_width;
    tcinp.target_height  = tcarg.target_height;
    tcinp.maximum_levels = tcarg.level_count;
    tcinp.build_mipmaps  = tcarg.build_mipmaps;
    tcinp.force_pow2     = tcarg.force_pow2;
    tcinp.premultiply_a  = tcarg.premultiplied;
    tcinp.flip_y         = tcarg.flip_y;

    // build the texture data.
    if (!compile_texture(&tcinp, &tcout))
    {
        free_compiler_args(&tcarg);
        return scope.Close(v8::ThrowException(ex(tcout.error_message)));
    }

    // write the raw texture data.
    size_t                 nlevels  = tcinp.maximum_levels;
    size_t                 channels = image.channel_count;
    int32_t                format   = texture_format(tcarg.target_format, channels);
    char const            *target   = tcarg.target_path;
    v8::Handle<v8::Array>  levels   = v8::Array::New(nlevels);
    v8::Handle<v8::Value>  r3       = v8_output_raw(target, format, levels, &tcout);
    if (!r3->IsUndefined())
    {
        texture_compiler_outputs_free(&tcout);
        free_buffer(&image);
        free_compiler_args(&tcarg);
        return scope.Close(v8::ThrowException(r3));
    }

    // build the object to return to JavaScript.
    v8::Handle<v8::Object> metadata = output_to_v8_object(&tcarg, &tcout, levels);

    // release resources that are no longer needed.
    texture_compiler_outputs_free(&tcout);
    free_buffer(&image);
    free_compiler_args(&tcarg);

    return scope.Close(metadata);
}

/*/////////////////////////////////////////////////////////////////////////80*/

void init(v8::Handle<v8::Object> target)
{
    // publish our global functions:
    target->Set(
        v8::String::NewSymbol("compile"),
        v8::FunctionTemplate::New(Compile)->GetFunction());
}
// @note: no semi-colon here intentionally.
NODE_MODULE(texture_compiler, init)

/*/////////////////////////////////////////////////////////////////////////80*/

/*/////////////////////////////////////////////////////////////////////////////
//    $Id$
///////////////////////////////////////////////////////////////////////////80*/
