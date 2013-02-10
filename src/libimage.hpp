/*/////////////////////////////////////////////////////////////////////////////
/// @summary Defines storage structures and routines for manipulating image
/// data in a generic image container format. This format can store 1D, 2D, 3D
/// and cubemap images, with or without mipmaps, in either integer or floating-
/// point formats, and with or without compression.
///////////////////////////////////////////////////////////////////////////80*/
#ifndef LIBIMAGE_HPP_INCLUDED
#define LIBIMAGE_HPP_INCLUDED

/*////////////////
//   Includes   //
////////////////*/
#include <math.h>
#include <float.h>
#include "commondefs.hpp"

/*///////////////////////
//   Namespace Begin   //
///////////////////////*/
namespace image {

/*////////////////////////////
//   Forward Declarations   //
////////////////////////////*/

/*//////////////////////////////////
//   Public Types and Functions   //
//////////////////////////////////*/
/// Define the maximum number of data channels (R, G, B, A, etc.) we allow in
/// a single image or image buffer.
#ifndef MAX_IMAGE_CHANNELS
#define MAX_IMAGE_CHANNELS    4
#endif /* !defined(MAX_IMAGE_CHANNELS) */

/// Define the maximum number of image faces that can be specified in an image.
#ifndef MAX_IMAGE_FACES
#define MAX_IMAGE_FACES       6
#endif /* !defined(MAX_IMAGE_FACES) */

/// An enumeration defining the supported runtime image data formats. Not all
/// formats will be supported by all hardware. Likewise, some hardware may not
/// support some of the formats listed here.
enum format_e
{
    /// The image format is not known or has not been specified.
    FORMAT_UNKNOWN          = 0,
    /// The image contains a single channel of data, with elements stored as
    /// unsigned values in the range of [0, 255].
    FORMAT_R8               = 1,
    /// The image contains two channels of data, with elements stored as
    /// unsigned values in the range of [0, 255].
    FORMAT_RG8              = 2,
    /// The image contains three channels of data, with elements stored as
    /// unsigned values in the range of [0, 255].
    FORMAT_RGB8             = 3,
    /// The image contains four channels of data, with elements stored as
    /// unsigned values in the range of [0, 255].
    FORMAT_RGBA8            = 4,
    /// The image contains a single channel of data, with elements stored as
    /// unsigned values in the range of [0, 65535].
    FORMAT_R16              = 5,
    /// The image contains two channels of data, with elements stored as
    /// unsigned values in the range of [0, 65535].
    FORMAT_RG16             = 6,
    /// The image contains four channels of data, with elements stored as
    /// unsigned values in the range of [0, 65535].
    FORMAT_RGBA16           = 7,
    /// The image contains a single channel of data, with elements stored as
    /// a 16-bit half-precision floating-point value.
    FORMAT_R16F             = 8,
    /// The image contains two channels of data, with elements stored as 16-bit
    /// half-precision floating-point values.
    FORMAT_RG16F            = 9,
    /// The image contains four channels of data, with elements stored as
    /// 16-bit half-precision floating-point values.
    FORMAT_RGBA16F          = 10,
    /// The image contains a single channel of data, with elements stored as
    /// a 32-bit full-precision floating-point value.
    FORMAT_R32F             = 11,
    /// The image contains two channels of data, with elements stored as 32-bit
    /// full-precision floating point values.
    FORMAT_RG32F            = 12,
    /// The image contains four channels of data, with elements stored as
    /// 32-bit full-precision floating-point values.
    FORMAT_RGBA32F          = 13,
    /// The image contains four channels of data, tightly packed, with 30 bits
    /// of precision for color data and 2 bits of alpha channel. This format
    /// is typically used to store HDR images in LDR data formats.
    FORMAT_RGB10A2          = 14,
    /// Each element consists of a block of 4x4 pixels stored in a compressed
    /// format, consuming a total of 64 bits. Only RGB information can be
    /// encoded. Also known as DXT1.
    FORMAT_BC1              = 15,
    /// Each element consists of a block of 4x4 pixels stored in a compressed
    /// format, consuming a total of 128 bits. RGB plus alpha information can
    /// be encoded, but the alpha step size is 16, so this format is not
    /// recommended for storing data with smooth alpha gradients. Also known
    /// as DXT3.
    FORMAT_BC2              = 16,
    /// Each element consists of a block of 4x4 pixels stored in a compressed
    /// format, consuming a total of 128 bits. RGB plus alpha information can
    /// be encoded, and alpha information is interpolated, so this format may
    /// be used to store data with smooth alpha gradients. Also known as DXT5.
    FORMAT_BC3              = 17,
    /// A swizzled variant on BC3/DXT5.
    FORMAT_BC3_XGBR         = 18,
    /// A swizzled variant on BC3/DXT5.
    FORMAT_BC3_RXBG         = 19,
    /// A swizzled variant on BC3/DXT5.
    FORMAT_BC3_RBXG         = 20,
    /// A swizzled variant on BC3/DXT5.
    FORMAT_BC3_XRBG         = 21,
    /// A swizzled variant on BC3/DXT5.
    FORMAT_BC3_RGXB         = 22,
    /// A swizzled variant on BC3/DXT5.
    FORMAT_BC3_XGXR         = 23,
    /// A compressed format encoding a single element and designed for storing
    /// tangent-space normal map data.
    FORMAT_BC4              = 24,
    /// A compressed format encoding two elements and designed for storing
    /// tangent-space normal map data.
    FORMAT_BC5              = 25,
    /// A compressed format encoding two elements, interpreted as X- and Y-
    /// normal components, designed for storing tangent-space normal map data.
    FORMAT_BC5_XY           = 26,
    /// A compressed format encoding two elements, interpreted as X- and Y-
    /// normal components, designed for storing tangent-space normal map data
    /// in a BC3/DXT5 format.
    FORMAT_ATI2N_DXT5       = 27,
    /// A compressed PowerVR 2-bpp format containing four channels of data and
    /// suitable for encoding ARGB images.
    FORMAT_PVRTC1           = 28,
    /// A compressed PowerVR 4-bpp format containing four channels of data and
    /// suitable for encoding ARGB images.
    FORMAT_PVRTC2           = 29,
    /// Forces the storage size of enumeration values to 32-bits.
    FORMAT_FORCE_32BIT      = CMN_FORCE_32BIT
};

/// An enumeration defining the different face identifiers for cubemap images.
enum cube_face_e
{
    /// The face representing the positive direction along the X-axis.
    CUBE_FACE_POSITIVE_X    = 0,
    /// The face representing the negative direction along the X-axis.
    CUBE_FACE_NEGATIVE_X    = 1,
    /// The face representing the positive direction along the Y-axis.
    CUBE_FACE_POSITIVE_Y    = 2,
    /// The face representing the negative direction along the Y-axis.
    CUBE_FACE_NEGATIVE_Y    = 3,
    /// The face representing the positive direction along the Z-axis.
    CUBE_FACE_POSITIVE_Z    = 4,
    /// The face representing the negative direction along the Z-axis.
    CUBE_FACE_NEGATIVE_Z    = 5,
    /// Forces the storage size of enumeration values to 32-bits.
    CUBE_FACE_FORCE_32BIT   = CMN_FORCE_32BIT
};

/// An enumeration defining the various supported sampling behavior at image
/// borders.
enum border_mode_e
{
    /// The sample will wrap around to the opposite extent.
    BORDER_MODE_WRAP        = 0,
    /// The sample will be clamped to the maximum extent.
    BORDER_MODE_CLAMP       = 1,
    /// The sample will be taken from the mirrored edge location.
    BORDER_MODE_MIRROR      = 2,
    /// Forces the storage size of enumeration values to 32-bits.
    BORDER_MODE_FORCE_32BIT = CMN_FORCE_32BIT
};

/// Defines a series of bitflags which may be set for an image to indicate the
/// type of data present in the image and any special attributes such as
/// whether image data is stored in a linear (vs. gamma) color space.
enum attributes_e
{
    /// Indicates that no image attributes are specified. This value should
    /// not appear in any valid image header structure.
    ATTRIBUTES_NONE          =  0,
    /// Indicates that the image should be interpreted as a 1D image.
    ATTRIBUTES_1D            = (1 <<  0),
    /// Indicates that the image should be interpreted as a 2D image.
    ATTRIBUTES_2D            = (1 <<  1),
    /// Indicates that the image should be interpreted as a 3D volume image.
    ATTRIBUTES_3D            = (1 <<  2),
    /// Indicates that the image should be interpreted as a cubemap, that is,
    /// it has six square image faces.
    ATTRIBUTES_CUBEMAP       = (1 <<  3),
    /// Indicates that the image data should be interpreted as an image array,
    /// which is similar to a volume image but mipmaps are not computed along
    /// the depth axis.
    ATTRIBUTES_ARRAY         = (1 <<  4),
    /// Indicates that the image data represents an image atlas, where multiple
    /// small images are packed into a single larger image. This flag also
    /// indicates the presence of the atlas data section when stored within a
    /// image file.
    ATTRIBUTES_ATLAS         = (1 <<  5),
    /// Indicates that image data should be interpreted as color values.
    ATTRIBUTES_COLOR         = (1 <<  6),
    /// Indicates that image data should be interpreted as depth values.
    ATTRIBUTES_DEPTH         = (1 <<  7),
    /// Indicates that image data should be interpreted as height values.
    ATTRIBUTES_HEIGHT        = (1 <<  8),
    /// Indicates that image data should be interpreted as vector values.
    ATTRIBUTES_VECTOR        = (1 <<  9),
    /// Indicates that image data is stored in a linear color space instead of
    /// a gamma color space.
    ATTRIBUTES_LINEAR        = (1 << 10),
    /// Indicates that image color values are pre-multiplied by alpha values.
    ATTRIBUTES_PREMULTIPLIED = (1 << 11),
    /// Forces the storage size of enumeration values to 32-bits.
    ATTRIBUTES_FORCE_32BIT   = CMN_FORCE_32BIT
};

/// Represents a single rectangle in a texture atlas. Each entry defines the
/// bounding rectangle for a single sub-image on the atlas image. This
/// structure is meant to be read directly from and written directly to a file.
#pragma pack(push, 1)
struct atlas_entry_t /* 16 bytes */
{
    uint16_t       x;           /// Absolute x-coordinate of upper-left
    uint16_t       y;           /// Absolute y-coordinate of upper-left
    uint16_t       width;       /// Width, in pixels
    uint16_t       height;      /// Height, in pixels
};
#pragma pack(pop)

/// Represents the header that appears at the beginning of an image container
/// file. The fixed-length header is followed immediately by image data. If the
/// image encodes texture atlas data, the atlas data immediately follows the
/// image data. This structure is meant to be read directly from and written
/// directly to an image file.
#pragma pack(push, 1)
struct header_t /* 64 bytes */
{
    int32_t        format;      /// One of image_format_e.
    int32_t        flags;       /// Combination of image_attributes_e.
    uint32_t       items;       /// Number of items in the image array.
    uint32_t       levels;      /// Number of mipmap levels in each item.
    uint32_t       width;       /// Width in pixels of level 0 of each item.
    uint32_t       height;      /// Height in pixels of level 0 of each item.
    uint32_t       slices;      /// Number of slices in level 0 of each item.
    uint64_t       image_size;  /// Total size of all image data, in bytes.
    uint64_t       atlas_size;  /// Total size of all atlas data, in bytes.
    uint32_t       reserved[5]; /// Space reserved for future expansion.
};
#pragma pack(pop)

/// A simple structure representing image atlas data for a single slice of an
/// image container. Image containers support arrays of image atlas data. Data
/// is stored such that entry_names[i] corresponds to entry_rects[i]. The name
/// array is stored sorted for efficient lookup.
struct atlas_t
{
    size_t         total_size;  /// Total size of the atlas record, in bytes.
    size_t         entry_count; /// Dimension of entry_names and entry_rects.
    uint32_t      *entry_names; /// Unique integer identifiers for each entry.
    atlas_entry_t *entry_rects; /// Bounding rectangles for each entry.
};

/// A simple structure representing an image container object. The entire
/// image can be stored in a single contiguous block of memory.
struct container_t
{
    int32_t        format;      /// One of image::format_e.
    int32_t        flags;       /// Combination of image::attributes_e.
    size_t         items;       /// Number of items in the image array.
    size_t         levels;      /// Number of mipmap levels in each item.
    size_t         width;       /// Width in pixels of level 0 of each item.
    size_t         height;      /// Height in pixels of level 0 of each item.
    size_t         slices;      /// Number of slices in level 0 of each item.
    size_t         image_size;  /// Total size of all image data, in bytes.
    size_t         atlas_size;  /// Total size of all atlas data, in bytes.
    void          *alloc_base;  /// Pointer to start of allocated block.
    void          *image_data;  /// Pointer to start of image data.
    void          *atlas_data;  /// Pointer to start of atlas data.
};

/// The image buffer is used for manipulating image data without loss of
/// precision during image processing operations. A buffer consists of a number
/// of channels, each of whose elements is represented as a 32-bit floating
/// point value.
struct buffer_t
{
    float  *channels[MAX_IMAGE_CHANNELS]; /// Pointers to each channel.
    float  *channel_data;       /// The raw channel data block.
    size_t  channel_count;      /// Number of valid channels.
    size_t  channel_width;      /// Channel width, in elements.
    size_t  channel_height;     /// Channel height, in elements.
};

/// Arguments used to configure a box filter.
struct box_args_t
{
    float   filter_width;       /// The filter width (def. 0.5)
};

/// Arguments used to configure a Kaiser filter. For more information, see
/// http://en.wikipedia.org/wiki/Kaiser_window
struct kaiser_args_t
{
    float   filter_width;       /// The filter width.
    float   stretch;            /// The stretch parameter.
    float   alpha;              /// The alpha parameter
};

/// Arguments used to configure a Lanczos filter. For more information, see
/// http://en.wikipedia.org/wiki/Lanczos_resampling
struct lanczos_args_t
{
    float   filter_width;       /// The filter width (def. 3.0)
};

/// Arguments used to configure a Mitchell filter. For more information, see
/// http://number-none.com/product/Mipmapping,%20Part%202/index.html
struct mitchell_args_t
{
    float   filter_width;       /// The filter width (def. 2.0)
    float   p0;                 /// This value is set indirectly
    float   p2;                 /// This value is set indirectly
    float   p3;                 /// This value is set indirectly
    float   q0;                 /// This value is set indirectly
    float   q1;                 /// This value is set indirectly
    float   q2;                 /// This value is set indirectly
    float   q3;                 /// This value is set indirectly
};

/// Arguments used to configure a triangle filter.
struct triangle_args_t
{
    float   filter_width;       /// The filter width (def. 1.0)
};

/// A structure for storing a convolution kernel in a generic way.
struct convolution_kernel_t
{
    size_t  window_size;        /// The kernel dimension
    float  *kernel_matrix;      /// The kernel matrix values
};

/// A structure for storing computed filter weight values for a given filter.
/// Weights can be pre-computed for a given filter configuration and reused.
struct filter_kernel_1d_t
{
    size_t  window_size;        /// The filter window size
    size_t  sample_count;       /// The number of samples to take
    float   scale_value;        /// The scaling value
    float   filter_width;       /// The filter width
    float  *filter_weights;     /// Normalized window_size weights
};

/// A structure for storing computed filter weight values for a given filter.
/// Weights can be pre-computed for a given filter configuration and reused.
/// Specifically, this structure stores a polyphase matrix, see
/// http://en.wikipedia.org/wiki/Polyphase_matrix
struct polyphase_kernel_1d_t
{
    size_t  window_size;        /// The row count/window size
    size_t  column_count;       /// The number of columns
    size_t  sample_count;       /// The number of samples to take
    float   scale_value;        /// The scaling value
    float   scale_inverse;      /// The inverse scaling value
    float   filter_width;       /// The filter width
    float  *filter_weights;     /// [window_size * column_count]
};

/// A function pointer type that can be passed to the various sampling
/// functions. Various filter function implementations can be defined.
///
/// @param x The sample value to evaluate.
/// @param args Additional filter-specific arguments.
typedef float (CMN_CALL_C *filter_fn)(float x, void *args);

/// Computes the basic attributes flags for a given set of image properties,
/// while simultaneously sanitizing the input properties.
///
/// @param image_count The number of images in the array. This value should be
/// at least 1.
/// @param pixel_width The width of level 0 of the image pyramid, in pixels.
/// This value should be at least 1.
/// @param pixel_height The height of level 0 of the image pyramid, in pixels.
/// This value should be at least 1.
/// @param slice_count The number of slices in level 0 of the image pyramid.
/// This value should be at least 1.
/// @param faces_count The number of faces in the image. For standard images,
/// this value should be 1; for cubemap images, this value should be 6.
/// @return A combination of image::attributes_e values. The caller is
/// responsible for setting the type of data in the image as well as any other
/// attributes that are not a function of size.
CMN_PUBLIC int32_t basic_attributes(
    size_t image_count,
    size_t pixel_width,
    size_t pixel_height,
    size_t slice_count,
    size_t faces_count);

/// Determines whether a particular image::format_e value is a non-packed, non-
/// compressed data format.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// @return true if @a image_format specifies a non-packed, non-compressed
/// image data format.
CMN_PUBLIC bool is_plain_format(int32_t image_format);

/// Determines whether a particular image::format_e value would store data as
/// floating-point values instead of unsigned integer values.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// @return true if @a image_format specifies a floating-point format.
CMN_PUBLIC bool is_float_format(int32_t image_format);

/// Determines whether a particular image::format_e value is a packed format,
/// that is, whether it contains components with widths less than 8 bits.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// @return true if @a image_format specifies a packed image data format.
CMN_PUBLIC bool is_packed_format(int32_t image_format);

/// Determines whether a particular image::format_e value stores data in a
/// compressed format.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// @return true if @a image_format specifies a compressed image data format.
CMN_PUBLIC bool is_compressed_format(int32_t image_format);

/// Determines whether a particular image::format_e value stores data in a
/// block-compressed (DXT/S3TC) format.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// @return true if @a image_format specifies a block-compressed image data
/// format.
CMN_PUBLIC bool is_block_compressed_format(int32_t image_format);

/// Determines whether a particular image::format_e value stores data in a
/// PowerVR compressed, packed texture format.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// @return true if @a image_format specifies a PVRTC compressed image data
/// format.
CMN_PUBLIC bool is_pvrtc_compressed_format(int32_t image_format);

/// Examines image attributes to determine the number of faces that should be
/// present in an image.
///
/// @param image_attributes A combination of image::attributes_e.
/// @return The number of faces in the image; 1 for standard images and 6 for
/// cubemap images.
CMN_PUBLIC size_t face_count(int32_t image_attributes);

/// Determines the number of data channels present for a given image::format_e.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// @return The number of data channels encoded in @a image_format.
CMN_PUBLIC size_t channel_count(int32_t image_format);

/// Determines the number of bytes used to encode a single block of pixel data
/// in a compressed-format image.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// This should specify a compressed image format.
/// @return The number of bytes-per-block for @a image_format.
CMN_PUBLIC size_t bytes_per_block(int32_t image_format);

/// Determines the number of bytes used to encode a single pixel in a non-
/// compressed format image.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// This should specify a non-compressed image format.
/// @return The number of bytes-per-pixel for @a image_format.
CMN_PUBLIC size_t bytes_per_pixel(int32_t image_format);

/// Determines the number of bytes used to store a single channel value in a
/// non-compressed, non-packed format image.
///
/// @param image_format One of the values of the image::format_e enumeration.
/// This should specify a non-compressed and non-packed image format.
/// @return The number of bytes-per-channel for @a image_format.
CMN_PUBLIC size_t bytes_per_channel(int32_t image_format);

/// Computes the total number of levels in the mip-map pyramid down to a 1x1x1
/// pixel resolution.
///
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices in the highest-resolution
/// mip-level (level 0) of the image.
/// @return The number of levels in the image pyramid.
CMN_PUBLIC size_t miplevel_count(
    size_t level0_width,
    size_t level0_height,
    size_t level0_slices);

/// Computes the width of a particular mip-level, given the width of the
/// highest-resolution mip-level.
///
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level_index The zero-based index of the mip-level to compute the
/// width of, where 0 corresponds to the highest-resolution level.
/// @return The width, in pixels, of the specified mip-level.
CMN_PUBLIC size_t miplevel_width(size_t level0_width, size_t level_index);

/// Computes the height of a particular mip-level, given the height of the
/// highest-resolution mip-level.
///
/// @param level0_height The height of the highest-resolution mip-level
/// (level 0) of the image, specified in pixels.
/// @param level_index The zero-based index of the mip-level to compute the
/// height of, where 0 corresponds to the highest-resolution level.
/// @return The height, in pixels, of the specified mip-level.
CMN_PUBLIC size_t miplevel_height(size_t level0_height, size_t level_index);

/// Computes the number of slices (depth) of a particular mip-level, given
/// the slice count of the highest-resolution mip-level.
///
/// @param level0_slices The number of slices of the highest-resolution mip-
/// level (level 0) of the image.
/// @param level_index The zero-based index of the mip-level to compute the
/// slice count of, where 0 corresponds to the highest-resolution level.
/// @return The slice count of the specified mip-level.
CMN_PUBLIC size_t miplevel_slices(size_t level0_slices, size_t level_index);

/// Computes the total number of bytes required to store a single mip-level
/// of image data.
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices in the highest-resolution
/// mip-level (level 0) of the image.
/// @param level_index The zero-based index of the mip-level for which the size
/// is being computed.
/// @return The number of bytes required to store the specified mip-level.
CMN_PUBLIC size_t miplevel_size(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  level_index);

/// Computes the total number of bytes required to store a single 2D slice of
/// a particular mip-level of image data.
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level_index The zero-based index of the mip-level for which the size
/// is being computed.
/// @return The number of bytes required to store the specified mip-level.
CMN_PUBLIC size_t miplevel_slice_size(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level_index);

/// Computes the byte offset of a particular mip-level within a data block
/// containing tightly-packed image data for the image pyramid.
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices in the highest-resolution
/// mip-level (level 0) of the image.
/// @param level_index The zero-based index of the mip-level for which the byte
/// offset is being computed.
/// @return The byte offset of the specified mipmap level.
CMN_PUBLIC size_t miplevel_offset(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  level_index);

/// Computes the byte offset of the start of a particular slice in a given
/// mip-level from the start of the mip-level, that is, the value returned by
/// image::miplevel_offset().
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level_index The zero-based index of the mip-level for which the
/// slice offset is being computed.
/// @param slice_index The zero-based index of the image slice.
/// @return The byte offset, relative to the start of the data for the
/// mip-level, of the start of the given slice.
CMN_PUBLIC size_t miplevel_slice_offset(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level_index,
    size_t  slice_index);

/// Computes the total number of bytes required to store an image within an
/// image array for a given set of attributes.
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param attributes A combination of image::attributes_e values specifying
/// whether the image is 3D vs. array, etc.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices in the highest-resolution
/// mip-level (level 0) of the image.
/// @param mipmap_count The number of mip-levels in the image. Specify 0 to
/// store all levels down to the minimum resolution. Specify 1 to not store
/// any mip-levels (except for the highest-resolution image.)
/// @return The total number of bytes required to store an image with the
/// specified attributes.
CMN_PUBLIC size_t subimage_size(
    int32_t image_format,
    int32_t attributes,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count);

/// Computes the total number of bytes required to store a single face within a
/// sub-image of an image array for a given set of attributes.
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices in the highest-resolution
/// mip-level (level 0) of the image.
/// @param mipmap_count The number of mip-levels in the image. Specify 0 to
/// store all levels down to the minimum resolution. Specify 1 to not store
/// any mip-levels (except for the highest-resolution image.)
/// @return The total number of bytes required to store image data for the
/// image face within the sub-image.
CMN_PUBLIC size_t subimage_face_size(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count);

/// Computes the byte offset of a particular sub-image (image within an image
/// array) from the start of image data.
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param attributes A combination of image::attributes_e values specifying
/// whether the image is 3D vs. array, etc.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices in the highest-resolution
/// mip-level (level 0) of the image.
/// @param mipmap_count The number of mip-levels in the image. Specify 0 to
/// store all levels down to the minimum resolution. Specify 1 to not store
/// any mip-levels (except for the highest-resolution image.)
/// @param image_index The zero-based index of the sub-image to retrieve.
/// @return The byte offset of the specified sub-image from the start of the
/// image data.
CMN_PUBLIC size_t subimage_offset(
    int32_t image_format,
    int32_t attributes,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count,
    size_t  image_index);

/// Computes the byte offset of a particular sub-image (image within an image
/// array) face from the start of sub-image data.
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices in the highest-resolution
/// mip-level (level 0) of the image.
/// @param mipmap_count The number of mip-levels in the image. Specify 0 to
/// store all levels down to the minimum resolution. Specify 1 to not store
/// any mip-levels (except for the highest-resolution image.)
/// @param face_index The zero-based index of the face to retrieve.
/// @return The byte offset of the specified face within the sub-image,
/// relative to the start of the image data for the sub-image.
CMN_PUBLIC size_t subimage_face_offset(
    int32_t image_format,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count,
    size_t  face_index);

/// Computes the total number of pixels in an image pyramid (mipmap chain.)
///
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices of the highest-resolution mip-
/// level (level 0) of the image.
/// @param level_index The zero-based index of the mip-level indicating the
/// level in the pyramid at which to start the pixel count computation, where
/// 0 indicates the highest-resolution level.
/// @param mipmap_count The number of mip-levels in the image. Specify 0 to
/// store all levels down to the minimum resolution. Specify 1 to not store
/// any mip-levels (except for the highest-resolution image.)
/// @return The number of pixels in the image pyramid with the specified
/// attributes.
CMN_PUBLIC size_t pixel_count(
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  level_index,
    size_t  mipmap_count);

/// Computes the number of bytes that must be allocated to store image data for
/// an image array with the specified attributes.
///
/// @param image_format One of the values of the image::format_e enumeration,
/// indicating the format of the image data.
/// @param attributes A combination of image::attributes_e values specifying
/// whether the image is 3D vs. array, etc.
/// @param image_count The number of images in the image array.
/// @param level0_width The width of the highest-resolution mip-level (level 0)
/// of the image, specified in pixels.
/// @param level0_height The height of the highest-resolution mip-level (level
/// 0) of the image, specified in pixels.
/// @param level0_slices The number of slices of the highest-resolution mip-
/// level (level 0) of the image.
/// @param mipmap_count The number of mip-levels in the image. Specify 0 to
/// store all levels down to the minimum resolution. Specify 1 to not store
/// any mip-levels (except for the highest-resolution image.)
/// @return The number of bytes required to store image data for the pyramid
/// with the specified attributes.
CMN_PUBLIC size_t image_size(
    int32_t image_format,
    int32_t attributes,
    size_t  image_count,
    size_t  level0_width,
    size_t  level0_height,
    size_t  level0_slices,
    size_t  mipmap_count);

/// Computes the size of additional texture atlas data for a single texture
/// atlas image. A separate atlas dictionary is stored for each item in an
/// image array, as each atlas may have a different number of entries.
///
/// @param atlas_entry_count The number of entries in the atlas.
/// @return The total size, in bytes, required to store the atlas dictionary.
CMN_PUBLIC size_t atlas_size(size_t atlas_entry_count);

/// Extracts fields from an image object and writes them to a header instance.
///
/// @param image The image container object.
/// @param out_header Pointer to an image header structure. On return, the fields
/// of this structure are set to the corresponding properties read from the
/// image container object, @a image.
CMN_PUBLIC void get_header(
    image::container_t *image,
    image::header_t    *out_header);

/// Initializes the fields of an image container object using data stored in a
/// header structure. All pointer fields are initialized to NULL.
///
/// @param header The image header structure specifying information about the
/// image container.
/// @param out_image Pointer to an image container structure. On return, the
/// non-pointer fields of this structure are initialized to values extracted
/// from @a header. The pointer fields are initialized to NULL.
CMN_PUBLIC void container_from_header(
    image::header_t    *header,
    image::container_t *out_image);

/// Computes the number of bytes that must be allocated to store element data
/// for a single channel in an image of the given dimensions.
///
/// @param channel_width The width of the image, in pixels.
/// @param channel_height The height of the image, in pixels.
/// @return The number of bytes required to store image elements for a channel
/// of the specified dimensions.
CMN_PUBLIC size_t channel_size(size_t channel_width, size_t channel_height);

/// Computes the number of bytes that must be allocated to store image channel
/// data for an image buffer.
///
/// @param channel_width The width of the image, in pixels.
/// @param channel_height The height of the image, in pixels.
/// @param channel_count The total number of valid channels.
/// @return The number of bytes required to store channel data for an image
/// buffer with the specified attributes, or 0 if one or more parameters is
/// invalid.
CMN_PUBLIC size_t buffer_size(
    size_t channel_width,
    size_t channel_height,
    size_t channel_count);

/// Sets up the fields of an image::buffer_t instance after the application has
/// computed the buffer size and allocated the necessary memory.
///
/// @param channel_width The width of the image, in pixels.
/// @param channel_height The height of the image, in pixels.
/// @param channel_count The number of channels in the image.
/// @param channel_memory A pointer to the block of memory used to store data
/// for all channels. This should be at least as large as the value returned
/// by the image::buffer_size() function.
/// @param out_buffer The image::buffer_t instance to initialize.
CMN_PUBLIC void buffer_init_with_memory(
    size_t           channel_width,
    size_t           channel_height,
    size_t           channel_count,
    void            *channel_memory,
    image::buffer_t *out_buffer);

/// Initializes a convolution_kernel_t instance, setting the window size and
/// computing the number of bytes required for storage of kernel matrix values.
///
/// @param window_size The kernel window size (matrix dimension.)
/// @param out_kernel Pointer to the convolution kernel structure to populate
/// with information on return.
/// @return The number of bytes to allocate for out_kernel->kernel_matrix.
CMN_PUBLIC size_t convolution_kernel_init(
    size_t                       window_size,
    image::convolution_kernel_t *out_kernel);

/// Sets a 3x3 (window_size = 3) kernel coefficient array to the values for a
/// Laplacian filter, typically used for image sharpening.
///
/// @param kernel A convolution_kernel_t instance with a kernel_matrix array of
/// 9 floating point values that will be set to the Laplacian kernel
/// coefficients.
CMN_PUBLIC void convolution_kernel_laplacian_3x3(
    image::convolution_kernel_t *kernel);

/// Sets a 3x3 (window_size = 3) kernel coefficient array to the values for an
/// edge detection filter.
///
/// @param kernel A convolution_kernel_t instance with a kernel_matrix array of
/// 9 floating point values that will be set to the edge detection kernel
/// coefficients.
CMN_PUBLIC void convolution_kernel_edge_detect_3x3(
    image::convolution_kernel_t *kernel);

/// Sets a 3x3 (window_size = 3) kernel coefficient array to the values for a
/// Sobel filter, typically used for gradient vector computation.
///
/// @param kernel A convolution_kernel_t instance with a kernel_matrix array of
/// 9 floating point values that will be set to the Sobel kernel coefficients.
CMN_PUBLIC void convolution_kernel_sobel_3x3(
    image::convolution_kernel_t *kernel);

/// Sets a 5x5 (window_size = 5) kernel coefficient array to the values for a
/// Sobel filter, typically used for gradient vector computation.
///
/// @param kernel A convolution_kernel_t instance with a kernel_matrix array of
/// 25 floating point values that will be set to the Sobel kernel coefficients.
CMN_PUBLIC void convolution_kernel_sobel_5x5(
    image::convolution_kernel_t *kernel);

/// Sets a 7x7 (window_size = 7) kernel coefficient array to the values for a
/// Sobel filter, typically used for gradient vector computation.
///
/// @param kernel A convolution_kernel_t instance with a kernel_matrix array of
/// 49 floating point values that will be set to the Sobel kernel coefficients.
CMN_PUBLIC void convolution_kernel_sobel_7x7(
    image::convolution_kernel_t *kernel);

/// Sets a 9x9 (window_size = 9) kernel coefficient array to the values for a
/// Sobel filter, typically used for gradient vector computation.
///
/// @param kernel A convolution_kernel_t instance with a kernel_matrix array of
/// 81 floating point values that will be set to the Sobel kernel coefficients.
CMN_PUBLIC void convolution_kernel_sobel_9x9(
    image::convolution_kernel_t *kernel);

/// Copies the coefficients of a convolution kernel.
///
/// @param dst The destination kernel. The kernel_matrix array must already be
/// allocated by the application. This kernel must have the same window_size as
/// the kernel referenced by @a src.
/// @param src The source kernel. The kernel_matrix array must already be
/// initialized by the application.
CMN_PUBLIC void convolution_kernel_copy(
    image::convolution_kernel_t       *dst,
    image::convolution_kernel_t const *src);

/// Normalizes a convolution kernel coefficient array so that the coefficients
/// sum to a value of 1.0. The normalization is performed in-place.
///
/// @param kernel The initialized convolution kernel instance.
CMN_PUBLIC void convolution_kernel_normalize(
    image::convolution_kernel_t *kernel);

/// Transposes a convolution kernel coefficient array. The transposition is
/// performed in-place.
///
/// @param kernel The initialized convolution kernel instance.
CMN_PUBLIC void convolution_kernel_transpose(
    image::convolution_kernel_t *kernel);

/// Applies a convolution kernel to a single sample location of an image
/// channel.
///
/// @param kernel The convolution kernel to apply.
/// @param border_mode One of the border_mode_e values indicating how to handle
/// sampling at the image borders.
/// @param source_x The zero-based index of the column in the input channel to
/// which the kernel will be applied.
/// @param source_y The zero-based index of the row in the input channel to
/// which the kerel will be applied.
/// @param source_width The maximum horizontal extent of the input channel.
/// @param source_height The maximum vertical extent of the input channel.
/// @param source_values A pointer to the image elements for the input channel.
/// @return The filtered sample value at location (@a source_x, @a source_y).
CMN_PUBLIC float convolution_kernel_apply(
    image::convolution_kernel_t *kernel,
    int32_t                      border_mode,
    size_t                       source_x,
    size_t                       source_y,
    size_t                       source_width,
    size_t                       source_height,
    float                       *source_values);

/// Used to compute a sample weight for a filter kernel or polyphase matrix.
///
/// @param x The unfiltered sample value in [0, 1].
/// @param scale The scale value, usually the width or height of the input.
/// @param filter_kernel The filter kernel to apply to the sample.
/// @param filter_args Arguments used to configure the filter.
/// @return The filtered sample value.
CMN_PUBLIC float sample_delta(
    float             x,
    float             scale,
    image::filter_fn  filter_kernel,
    void             *filter_args);

/// Used to compute a sample weight for a filter kernel or polyphase matrix.
///
/// @param x The unfiltered sample value in [0, 1].
/// @param scale The scale value, usually the width or height of the input.
/// @param sample_count The number of samples being taken.
/// @param filter_kernel The filter kernel to apply to the sample.
/// @param filter_args Arguments used to configure the filter.
/// @return The filtered sample value.
CMN_PUBLIC float sample_box(
    float             x,
    float             scale,
    size_t            sample_count,
    image::filter_fn  filter_kernel,
    void             *filter_args);

/// Used to compute a sample weight for a filter kernel or polyphase matrix.
///
/// @param x The unfiltered sample value in [0, 1].
/// @param scale The scale value, usually the width or height of the input.
/// @param sample_count The number of samples being taken.
/// @param filter_kernel The filter kernel to apply to the sample.
/// @param filter_args Arguments used to configure the filter.
/// @return The filtered sample value.
CMN_PUBLIC float sample_triangle(
    float             x,
    float             scale,
    size_t            sample_count,
    image::filter_fn  filter_kernel,
    void             *filter_args);

/// Initializes an arguments structure for a box filter. The default window
/// width of 0.5 is used.
///
/// @param out_args Pointer to the structure that will be initialized.
CMN_PUBLIC void box_args_init(image::box_args_t *out_args);

/// Initializes an arguments structure for a box filter using the specified
/// window width.
///
/// @param filter_width The filter width.
/// @param out_args Pointer to the structure that will be initialized.
CMN_PUBLIC void box_args_init(
    float              filter_width,
    image::box_args_t *out_args);

/// Evaluates a box filter for a given sample value.
///
/// @param x The sample value.
/// @param args Pointer to a box_args_t structure specifying filter arguments.
/// @return The filtered sample value.
CMN_PUBLIC float box_filter(float x, void *args);

/// Initializes an arguments structure for a Kaiser filter using the specified
/// window width and the default stretch and alpha parameter values.
///
/// @param filter_width The filter width.
/// @param out_args Pointer to the structure that will be initialized.
CMN_PUBLIC void kaiser_args_init(
    float                 filter_width,
    image::kaiser_args_t *out_args);

/// Initializes an arguments structure for a Kaiser filter using the specified
/// window width and stretch and alpha parameter values.
///
/// @param filter_width The filter width.
/// @param alpha The value for the alpha parameter.
/// @param stretch The value for the stretch parameter.
/// @param out_args Pointer to the structure that will be initialized.
CMN_PUBLIC void kaiser_args_init(
    float                 filter_width,
    float                 alpha,
    float                 stretch,
    image::kaiser_args_t *out_args);

/// Evaluates a Kaiser filter for a given sample value.
///
/// @param x The sample value.
/// @param args Pointer to a kaiser_args_t structure specifying filter data.
/// @return The filtered sample value.
CMN_PUBLIC float kaiser_filter(float x, void *args);

/// Initializes an arguments structure for a Lanczos filter using the specified
/// window width.
///
/// @param filter_width The filter window width.
/// @param out_args Pointer to the structure that will be initialized.
CMN_PUBLIC void lanczos_args_init(
    float                  filter_width,
    image::lanczos_args_t *out_args);

/// Evaluates a Lanczos filter for a given sample value.
///
/// @param x The sample value.
/// @param args Pointer to a lanczos_args_t structure specifying filter data.
/// @return The filtered sample value.
CMN_PUBLIC float lanczos_filter(float x, void *args);

/// Initializes a filter_kernel_1d_t instance, computing the scale value,
/// window size, and filter width for a given set of filter parameters. This
/// function does not allocate memory for the cached filter weight values.
///
/// @param scale_value The scaled dimension of the input (width or height.)
/// This value must be greater than 1.
/// @param sample_count The number of samples that should be taken during
/// filtering and scaling of the input.
/// @param filter_width The width of the filter window for this filter type.
/// @param out_kernel_info Pointer to the filter kernel structure to populate
/// with information on return.
/// @return The number of bytes to allocate for out_kernel_info->filter_weights
CMN_PUBLIC size_t filter_1d_init(
    size_t                     scale_value,
    size_t                     sample_count,
    float                      filter_width,
    image::filter_kernel_1d_t *out_kernel_info);

/// Computes the matrix of filter kernel weight values for a given filter
/// kernel configuration.
///
/// @param filter_kernel The filter kernel to apply.
/// @param filter_args Arguments used to configure the filter.
/// @param kernel_weights Pointer to the object that will store the filter
/// kernel weight values. The members of this structure should already have
/// been initialized by calling filter_1d_init(). This function will not
/// allocate memory for the filter weight values.
CMN_PUBLIC void compute_filter_weights_1d(
    image::filter_fn           filter_kernel,
    void                      *filter_args,
    image::filter_kernel_1d_t *kernel_weights);

/// Applies a filter in the vertical direction to a single sample location of
/// an image channel.
///
/// @param kernel_weights The pre-computed filter kernel weights.
/// @param border_mode One of the border_mode_e values indicating how to handle
/// sampling at the image borders.
/// @param source_x The zero-based index of the column in the input channel to
/// which the filter will be applied.
/// @param source_y The zero-based index of the row in the input channel to
/// which the filter will be applied.
/// @param source_width The maximum horizontal extent of the input channel.
/// @param source_height The maximum vertical extent of the input channel.
/// @param source_values A pointer to the image elements for the input channel.
/// @return The filtered sample value at location (@a source_x, @a source_y).
CMN_PUBLIC float apply_filter_vertical_1d(
    image::filter_kernel_1d_t *kernel_weights,
    int32_t                    border_mode,
    size_t                     source_x,
    size_t                     source_y,
    size_t                     source_width,
    size_t                     source_height,
    float                     *source_values);

/// Applies a filter in the horizontal direction to a single sample location of
/// an image channel.
///
/// @param kernel_weights The pre-computed filter kernel weights.
/// @param border_mode One of the border_mode_e values indicating how to handle
/// sampling at the image borders.
/// @param source_x The zero-based index of the column in the input channel to
/// which the filter will be applied.
/// @param source_y The zero-based index of the row in the input channel to
/// which the filter will be applied.
/// @param source_width The maximum horizontal extent of the input channel.
/// @param source_height The maximum vertical extent of the input channel.
/// @param source_values A pointer to the image elements for the input channel.
/// @return The filtered sample value at location (@a source_x, @a source_y).
CMN_PUBLIC float apply_filter_horizontal_1d(
    image::filter_kernel_1d_t *kernel_weights,
    int32_t                    border_mode,
    size_t                     source_x,
    size_t                     source_y,
    size_t                     source_width,
    size_t                     source_height,
    float                     *source_values);

/// Initializes a polyphase_kernel_1d_t instance, computing the scale value,
/// window size, filter width, etc. for a given set of filter parameters. This
/// function does not allocate memory for the cached filter weight values.
///
/// @param source_dimension The source dimension value (width or height.)
/// @param target_dimension The destination dimension value (width or height.)
/// @param sample_count The number of samples that should be taken during
/// filtering and scaling of the input.
/// @param filter_width The width of the filter window for this filter type.
/// @param out_kernel_info Pointer to the filter kernel structure to populate
/// with information on return.
/// @return The number of bytes to allocate for out_kernel_info->filter_weights
CMN_PUBLIC size_t polyphase_1d_init(
    size_t                        source_dimension,
    size_t                        target_dimension,
    size_t                        sample_count,
    float                         filter_width,
    image::polyphase_kernel_1d_t *out_kernel_info);

/// Computes the polyphase matrix of filter kernel weight values for a given
/// filter kernel configuration.
///
/// @param filter_kernel The filter kernel to apply.
/// @param filter_args Arguments used to configure the filter.
/// @param kernel_weights Pointer to the object that will store the filter
/// kernel weight values. The members of this structure should already have
/// been initialized by calling polyphase_1d_init(). This function will not
/// allocate memory for the filter weight values.
CMN_PUBLIC void compute_polyphase_matrix_1d(
    image::filter_fn              filter_kernel,
    void                         *filter_args,
    image::polyphase_kernel_1d_t *kernel_weights);

/// Applies a polyphase filter in the vertical direction to a single column of
/// an image channel.
///
/// @param kernel_weights The pre-computed polyphase kernel matrix.
/// @param border_mode One of the border_mode_e values indicating how to handle
/// sampling at the image borders.
/// @param source_column The zero-based index of the column in the input
/// channel to which the filter will be applied.
/// @param source_width The maximum horizontal extent of the input channel.
/// @param source_height The maximum vertical extent of the input channel.
/// @param source_values A pointer to the image elements for the input channel.
/// @param target_values A pointer to the image elements for the output channel.
CMN_PUBLIC void apply_polyphase_vertical_1d(
    image::polyphase_kernel_1d_t *kernel_weights,
    int32_t                       border_mode,
    size_t                        source_column,
    size_t                        source_width,
    size_t                        source_height,
    float                        *source_values,
    float                        *target_values);

/// Applies a polyphase filter in the horizontal direction to a single row of
/// an image channel.
///
/// @param kernel_weights The pre-computed polyphase kernel matrix.
/// @param border_mode One of the border_mode_e values indicating how to handle
/// sampling at the image borders.
/// @param source_row The zero-based index of the row in the input channel to
/// which the filter will be applied.
/// @param source_width The maximum horizontal extent of the input channel.
/// @param source_height The maximum vertical extent of the input channel.
/// @param source_values A pointer to the image elements for the input channel.
/// @param target_values A pointer to the image elements for the output channel.
CMN_PUBLIC void apply_polyphase_horizontal_1d(
    image::polyphase_kernel_1d_t *kernel_weights,
    int32_t                       border_mode,
    size_t                        source_row,
    size_t                        source_width,
    size_t                        source_height,
    float                        *source_values,
    float                        *target_values);

/// Converts a color value stored in the LAB colorspace to a color value stored
/// in the RGB colorspace.
///
/// @param l The lightness component.
/// @param a A color-opponent channel value.
/// @param b A color-opponent channel value.
/// @param out_rgb A pointer to at least three bytes that will be used to store
/// the corresponding RGB color value.
CMN_PUBLIC void lab_to_rgb(
    uint8_t  l,
    uint8_t  a,
    uint8_t  b,
    uint8_t *out_rgb);

/// Converts a color value stored in the CIE XYZ colorspace to a color value
/// stored in the RGB colorspace.
///
/// @param x The x-channel value.
/// @param y The y-channel value.
/// @param z The z-channel value.
/// @param out_rgb A pointer to at least three bytes that will be used to store
/// the corresponding RGB color value.
CMN_PUBLIC void xyz_to_rgb(
    double   x,
    double   y,
    double   z,
    uint8_t *out_rgb);

/// Converts a color value stored in the CMYK colorspace to a color value
/// stored in the RGB colorspace.
///
/// @param c The cyan component.
/// @param m The magenta component.
/// @param y The yellow component.
/// @param k The black component.
/// @param out_rgb A pointer to at least three bytes that will be used to store
/// the corresponding RGB color value.
CMN_PUBLIC void cmyk_to_rgb(
    uint8_t  c,
    uint8_t  m,
    uint8_t  y,
    uint8_t  k,
    uint8_t *out_rgb);

/// Computes the index for a given integer (x, y) coordinate in a 2D image,
/// wrapping at boundaries.
///
/// @param width The maximum horizontal extent.
/// @param height The maximum vertical extent.
/// @param at_x The sample location in the horizontal direction.
/// @param at_y The sample location in the vertical direction.
/// @return The zero-based array index of the specified image element within a
/// channel or pixel array.
CMN_PUBLIC size_t index_wrap(
    size_t    width,
    size_t    height,
    ptrdiff_t at_x,
    ptrdiff_t at_y);

/// Computes the index for a given integer (x, y) coordinate in a 2D image,
/// clamping to boundaries.
///
/// @param width The maximum horizontal extent.
/// @param height The maximum vertical extent.
/// @param at_x The sample location in the horizontal direction.
/// @param at_y The sample location in the vertical direction.
/// @return The zero-based array index of the specified image element within a
/// channel or pixel array.
CMN_PUBLIC size_t index_clamp(
    size_t    width,
    size_t    height,
    ptrdiff_t at_x,
    ptrdiff_t at_y);

/// Computes the index for a given integer (x, y) coordinate in a 2D image,
/// mirroring at boundaries.
///
/// @param width The maximum horizontal extent.
/// @param height The maximum vertical extent.
/// @param at_x The sample location in the horizontal direction.
/// @param at_y The sample location in the vertical direction.
/// @return The zero-based array index of the specified image element within a
/// channel or pixel array.
CMN_PUBLIC size_t index_mirror(
    size_t    width,
    size_t    height,
    ptrdiff_t at_x,
    ptrdiff_t at_y);

/// Copies the contents of one channel buffer to another. The channels must
/// have the same dimensions.
///
/// @param dst_channel A pointer to the destination image channel buffer.
/// @param src_channel A pointer to the source image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
CMN_PUBLIC void copy_channel(
    float       *dst_channel,
    float const *src_channel,
    size_t       channel_width,
    size_t       channel_height);

/// Fills a channel buffer so that all elements have a particular value.
///
/// @param channel_values A pointer to the image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @param fill_value The value which will be written to every element in the
/// image channel buffer.
CMN_PUBLIC void fill_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height,
    float   fill_value);

/// Flips a channel vertically, in place.
///
/// @param channel_values A pointer to the image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
CMN_PUBLIC void flip_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height);

/// Clamps each element in a channel buffer so it falls within a defined range.
///
/// @param channel_values A pointer to the image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @param channel_min The lower bound value of the range of valid values.
/// @param channel_max The upper bound value of the range of valid values.
CMN_PUBLIC void clamp_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height,
    float   channel_min,
    float   channel_max);

/// Raises each element in a channel buffer to a power, such that each element
/// v' = v ** power.
///
/// @param channel_values A pointer to the image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @param power The power value.
CMN_PUBLIC void exponentiate_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height,
    float   power);

/// Scales (multiplies) and biases (adds) a value to each element in the
/// channel, such that each element v' = (v * scale) + bias.
///
/// @param channel_values A pointer to the image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @param scale The scale factor to be applied to each element.
/// @param bias The bias factor to be applied to each element.
CMN_PUBLIC void scale_bias_channel(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height,
    float   scale,
    float   bias);

/// Examines each element in a channel buffer to determine the minimum value.
///
/// @param channel_values A pointer to the image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @return The smallest value in @a channel_values.
CMN_PUBLIC float channel_minimum(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height);

/// Examines each element in a channel buffer to determine the maximum value.
///
/// @param channel_values A pointer to the image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @return The largest value in @a channel_values.
CMN_PUBLIC float channel_maximum(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height);

/// Examines each element in a channel buffer to determine the average value.
///
/// @param channel_values A pointer to the image channel buffer.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @return The average of the values in @a channel_values.
CMN_PUBLIC float channel_average(
    float  *channel_values,
    size_t  channel_width,
    size_t  channel_height);

/// Converts an image buffer into a single-channel, monochrome image.
///
/// @param monochrome_values Pointer to the buffer to which monochrome values
/// will be written. This must have the same dimensions as @a color_buffer.
/// @param color_buffer Pointer to the input image buffer. This buffer may have
/// one, two, three or four channels.
/// @param scale_r The scale value to apply to any red components.
/// @param scale_g The scale value to apply to any green components.
/// @param scale_b The scale value to apply to any blue components.
/// @param scale_a The scale value to apply to any alpha components.
CMN_PUBLIC void monochrome(
    float                 *monochrome_values,
    image::buffer_t const *color_buffer,
    float                  scale_r,
    float                  scale_g,
    float                  scale_b,
    float                  scale_a);

/// Converts an image buffer into a single-channel, grayscale image by calling
/// image::monochrome() with scale_r = 0.39f, scale_g = 0.50f, scale_b = 0.11f
/// ans scale_a = 0.0f.
///
/// @param grayscale_values Pointer to the buffer to which grayscale values
/// will be written. This must have the same dimensions as @a color_buffer.
/// @param color_buffer Pointer to the input image buffer. This buffer may have
/// one, two, three or four channels.
CMN_PUBLIC void grayscale(
    float                 *grayscale_values,
    image::buffer_t const *color_buffer);

/// Clamps the values in one or more channels of an image to a specific range.
///
/// @param buffer The image buffer to modify.
/// @param channel_base The zero-based index of the first channel to modify.
/// @param channel_count The number of channels, starting at @a channel_base,
/// to modify.
/// @param channel_min The lower bound value of the range of valid values.
/// @param channel_max The upper bound value of the range of valid values.
CMN_PUBLIC void clamp(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            channel_min,
    float            channel_max);

/// Raises the values in one or more channels of an image to a power, such that
/// v' = v ** power.
///
/// @param buffer The image buffer to modify.
/// @param channel_base The zero-based index of the first channel to modify.
/// @param channel_count The number of channels, starting at @a channel_base,
/// to modify.
/// @param power The power value.
CMN_PUBLIC void  exponentiate(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            power);

/// Applies a scale (multiplication) and bias (addition) to each element in
/// one or more channels. Each element v' = (v * scale) + bias.
///
/// @param buffer The image buffer to modify.
/// @param channel_base The zero-based index of the first channel to modify.
/// @param channel_count The number of channels, starting at @a channel_base,
/// to modify.
/// @param scale The scale value.
/// @param bias The bias value.
CMN_PUBLIC void scale_bias(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            scale,
    float            bias);

/// Converts an image buffer from gamma-corrected color space to a linear
/// color space.
///
/// @param buffer The image buffer to modify.
/// @param channel_base The zero-based index of the first channel to modify.
/// @param channel_count The number of channels, starting at @a channel_base,
/// to modify.
/// @param gamma_power The gamma power value, typically 2.2 on PCs and 1.8
/// on Mac OS.
CMN_PUBLIC void linear(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            gamma_power = 2.2f);

/// Applies gamma correction to an image buffer in linear color space.
///
/// @param buffer The image buffer to modify.
/// @param channel_base The zero-based index of the first channel to modify.
/// @param channel_count The number of channels, starting at @a channel_base,
/// to modify.
/// @param gamma_power The gamma power value, typically 2.2 on PCs and 1.8
/// on Mac OS.
CMN_PUBLIC void gamma(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float            gamma_power = 2.2f);

/// Flips an image vertically in-place.
///
/// @param buffer The image buffer to modify.
CMN_PUBLIC void flip(image::buffer_t *buffer);

/// Swizzles channels of an image buffer. This function simply swaps the
/// channel pointers and does not copy any channel data. The swap is performed
/// into another instance so that the original ordering can be preserved.
///
/// @param input The image buffer to swizzle.
/// @param output The image buffer to receive the swizzled channel pointers.
/// @param index_c0 The zero-based index of the channel to place in channel 0.
/// @param index_c1 The zero-based index of the channel to place in channel 1.
/// @param index_c2 The zero-based index of the channel to place in channel 2.
/// @param index_c3 The zero-based index of the channel to place in channel 3.
CMN_PUBLIC void swizzle(
    image::buffer_t * CMN_RESTRICT input,
    image::buffer_t * CMN_RESTRICT output,
    size_t                         index_c0,
    size_t                         index_c1,
    size_t                         index_c2,
    size_t                         index_c3);

/// Computes the alpha coverage value (normalized percentage of the image that
/// would pass the alpha test) for an image and given alpha test reference
/// value.
///
/// @param alpha_channel The alpha channel values.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @param alpha_reference The alpha test reference value.
/// @return The computed coverage value, in [0, 1].
CMN_PUBLIC float alpha_test_coverage(
    float const *alpha_channel,
    size_t       channel_width,
    size_t       channel_height,
    float        alpha_reference);

/// Determines the scaling value to apply to an alpha channel so that a desired
/// percentage of fragments will pass an alpha test.
///
/// @param alpha_channel The alpha channel values.
/// @param channel_width The number of columns in the image.
/// @param channel_height The number of rows in the image.
/// @param desired_coverage A normalized percentage value in [0, 1] specifying
/// the desired percentage of fragments that should pass the alpha test.
/// @param alpha_reference The alpha test reference value.
CMN_PUBLIC void scale_alpha_to_coverage(
    float  *alpha_channel,
    size_t  channel_width,
    size_t  channel_height,
    float   desired_coverage,
    float   alpha_reference);

/// Pre-multiplies the RGB color values by the alpha channel value.
///
/// @param buffer The image buffer to modify.
/// @param channel_base The zero-based index of the first channel to modify.
/// @param channel_count The number of channels to modify, starting at the
/// channel @a channel_base.
/// @param alpha_channel The alpha channel values.
CMN_PUBLIC void premultiply_alpha(
    image::buffer_t *buffer,
    size_t           channel_base,
    size_t           channel_count,
    float const     *alpha_channel);

/// Retrieves a pointer to the data for a specified item within an image array.
///
/// @param image Pointer to the descriptor for the image array.
/// @param image_index The zero-based index of the sub-image to access.
/// @param cface_index The zero-based index of the face within the sub-image.
/// @param level_index The zero-based index of the mip-level within the face.
/// @param slice_index The zero-based index of the slice within the mip-level.
/// @return A pointer to the data at the specified location within the image.
template <typename T>
inline T* data_at(
    image::container_t *image,
    size_t              image_index,
    size_t              cface_index,
    size_t              level_index,
    size_t              slice_index)
{
    uint8_t *bp = (uint8_t*) image->image_data;
    int32_t  f  = image->format;
    int32_t  a  = image->flags;
    size_t   w  = image->width;
    size_t   h  = image->height;
    size_t   d  = image->slices;
    size_t   m  = image->levels;
    size_t   ii = image_index;
    size_t   fi = cface_index;
    size_t   li = level_index;
    size_t   si = slice_index;
    // compute the offset of the sub-image, relative to base_ptr.
    size_t   io = image::subimage_offset(f, a, w, h, d, m, ii);
    // compute the offset of the face, relative to subi_ofs.
    size_t   fo = image::subimage_face_offset(f, w, h, d, m, fi);
    // compute the offset of the mip-level, relative to face_ofs.
    // (face_ofs points to the start of mip-level 0 of the face.)
    size_t   mo = image::miplevel_offset(f, w, h, d, li);
    // compute the offset of the slice, relative to mipl_ofs.
    size_t   so = image::miplevel_slice_offset(f, w, h, li, si);
    // finally, compute the data address.
    return (T*) (bp + io + fo + mo + so);
}

/// Retrieves the coefficient stored at a particular (row, column) location
/// within a convolution kernel.
///
/// @param kernel The kernel coefficient array.
/// @param window_size The size of the kernel filter window.
/// @param row The zero-based row index.
/// @param col The zero-based column index.
/// @return The kernel coefficient stored at location @a row, @a col.
inline float kernel_at(
    float  *kernel,
    size_t  window_size,
    size_t  row,
    size_t  col)
{
    return kernel[(row * window_size) + col];
}

/// Computes the index for a given scanline in a 2D image.
///
/// @param width The maximum horizontal extent.
/// @param scanline The zero-based scanline index.
/// @return The zero-based array index of the start of the specified scanline
/// within a channel or pixel array.
inline size_t scanline_index(size_t width, size_t scanline)
{
    return scanline * width;
}

/// Computes the index for a given integer (x, y) coordinate in a 2D image,
/// wrapping at boundaries using the specified function.
///
/// @param width The maximum horizontal extent.
/// @param height The maximum vertical extent.
/// @param at_x The sample location in the horizontal direction.
/// @param at_y The sample location in the vertical direction.
/// @param border_mode One of the border_mode_e values.
/// @return The zero-based array index of the specified image element within a
/// channel or pixel array.
inline size_t sample_index(
    size_t  width,
    size_t  height,
    size_t  at_x,
    size_t  at_y,
    int32_t border_mode)
{
    switch (border_mode)
    {
        case image::BORDER_MODE_WRAP:
            return image::index_wrap(width, height, at_x, at_y);
        case image::BORDER_MODE_CLAMP:
            return image::index_clamp(width, height, at_x, at_y);
        case image::BORDER_MODE_MIRROR:
            return image::index_mirror(width, height, at_x, at_y);
        default:
            break;
    }
    return 0;
}

/// Implements the sinc function: http://en.wikipedia.org/wiki/Sinc_function
///
/// @param x The input parameter.
/// @return The sinc(x) output, sin(x) / x.
inline float sinc(float x)
{
    if (fabsf(x) < FLT_EPSILON)
    {
        return 1.0f + x * x * (-1.0f / 6.0f + x * x * 1.0f / 120.0f);
    }
    return sinf(x) / x;
}

/// Implements a Bessel function of the first kind, as outlined in Jon Blow's
/// mipmapping article series: http://number-none.com/product/.
///
/// @param x The input value.
/// @return See http://mathworld.wolfram.com/BesselFunctionoftheFirstKind.html
inline float bessel0(float x)
{
    float xh    = 0.5f * x;
    float sum   = 1.0f;
    float power = 1.0f;
    float ds    = 1.0f;
    int   k     = 0;
    while (ds   > sum * 1e-6f)
    {
        ++k;
        power   = power * (xh / k);
        ds      = power * power;
        sum     = sum + ds;
    }
    return sum;
}

/*/////////////////////
//   Namespace End   //
/////////////////////*/
}; /* end namespace image */

#endif /* LIBIMAGE_HPP_INCLUDED */

/*/////////////////////////////////////////////////////////////////////////////
//    $Id$
///////////////////////////////////////////////////////////////////////////80*/
