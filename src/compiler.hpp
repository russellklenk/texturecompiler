
/*/////////////////////////////////////////////////////////////////////////////
/// @summary Defines the interface to the texture compiler.
///////////////////////////////////////////////////////////////////////////80*/
#ifndef TEXTURE_COMPILER_COMPILER_HPP_INCLUDED
#define TEXTURE_COMPILER_COMPILER_HPP_INCLUDED

/*////////////////
//   Includes   //
////////////////*/
#include "libimage.hpp"

/*///////////////////////
//   Namespace Begin   //
///////////////////////*/

/*////////////////////////////
//   Forward Declarations   //
////////////////////////////*/

/*//////////////////////////////////
//   Public Types and Functions   //
//////////////////////////////////*/
/// Define the maximum number of levels supported.
#ifndef TEXTURE_COMPILER_MAX_LEVELS
#define TEXTURE_COMPILER_MAX_LEVELS    16U
#endif /* !defined(TEXTURE_COMPILER_MAX_LEVELS) */

/// A structure used for passing arguments to the texture compiler.
struct texture_compiler_inputs_t
{
    image::buffer_t *input_image;    /// Load from disk using file_to_buffer().
    int32_t          border_mode;    /// Border sample mode during resize.
    size_t           target_width;   /// Desired width, in pixels.
    size_t           target_height;  /// Desired height, in pixels.
    size_t           maximum_levels; /// Maximum number of mip-levels.
    bool             build_mipmaps;  /// Build mipmap chain?
    bool             force_pow2;     /// Force power-of-two dimensions?
    bool             premultiply_a;  /// Output premultiplied alpha?
    bool             flip_y;         /// Flip image for bottom-left origin?
};

/// A structure used for returning data from the texture compiler.
struct texture_compiler_outputs_t
{
    char const      *error_message;  /// Static error message string.
    size_t           channel_count;  /// Number of color channels.
    size_t           level_count;    /// Number of valid entries in level_data.
    image::buffer_t  level_data[TEXTURE_COMPILER_MAX_LEVELS];
};

/// Initializes a texture_compiler_inputs_t structure to default values.
/// @param inputs Pointer to the structure to initialize.
CMN_PUBLIC void  texture_compiler_inputs_init(
    texture_compiler_inputs_t *inputs);

/// Updates fields of a texture_compiler_inputs_t structure based on the input
/// image and processing restrictions imposed by the compiler. Upon return,
/// the number of mipmap levels will be set and so forth.
/// @param inputs Pointer to the structure to sanitize.
CMN_PUBLIC void  texture_compiler_inputs_sanitize(
    texture_compiler_inputs_t *inputs);

/// Initializes a texture_compiler_outputs_t structure to default values.
/// @param outputs Pointer to the structure to initialize.
CMN_PUBLIC void  texture_compiler_outputs_init(
    texture_compiler_outputs_t *outputs);

/// Releases the memory associated with a texture_compiler_outputs_t structure.
/// @param outputs Pointer to the structure to release.
CMN_PUBLIC void  texture_compiler_outputs_free(
    texture_compiler_outputs_t *outputs);

/// Loads a file into a buffer ready for processing.
/// @param file The path of the source file.
/// @param buffer Pointer to the buffer structure to populate.
/// @return true if the buffer was loaded successfully.
CMN_PUBLIC bool  file_to_buffer(char const *file, image::buffer_t *buffer);

/// Determines if the dimensions for an image buffer are not powers of two.
/// @param buffer The image buffer to inspect.
/// @return true if the image buffer dimensions are not powers of two.
CMN_PUBLIC bool  is_non_power_of_two(image::buffer_t *buffer);

/// Calculates width and height values that are the next power of two greater
/// than or equal to the dimensions of an existing image buffer.
/// @param buffer The image buffer to inspect.
/// @param out_pot_width On return, stores the power-of-two width, in pixels.
/// @param out_pot_height On return, stores the power-of-two height, in pixels.
/// @return true if the dimensions of @a buffer were power-of-two values to
/// begin with, or false otherwise.
CMN_PUBLIC bool  power_of_two_dimensions(
    image::buffer_t *buffer,
    size_t          *out_pot_width,
    size_t          *out_pot_height);

/// Copies the contents of one image buffer to another.
/// @param target The target image buffer.
/// @param source The source image buffer.
CMN_PUBLIC void  copy_buffer(
    image::buffer_t *target,
    image::buffer_t *source);

/// Copies the contents of a source image buffer to a specified location on a
/// target image buffer. The entire source image buffer is copied.
/// @param target The target image buffer.
/// @param source The source image buffer.
/// @param target_x The x-coordinate of the location where the upper-left
/// corner of the source image will be copied to the target image.
/// @param target_y The y-coordinate of the location where the upper-left
/// corner of the source image will be copied to the target image.
CMN_PUBLIC void  copy_buffer_to_region(
    image::buffer_t *target,
    image::buffer_t *source,
    size_t           target_x,
    size_t           target_y);

/// Resizes an image buffer using a 32-sample Kaiser filter.
/// @param source Pointer to the structure representing the source image.
/// @param new_width The desired width of the target image, in pixels.
/// @param new_height The desired height of the target image, in pixels.
/// @param border_mode One of the image::border_mode_e constants describing how
/// to perform sampling at the borders of the image.
/// @param target Pointer to the buffer structure that will be allocated and
/// initialized with the resized image.
/// @return true if the operation was successful, or false if the necessary
/// memory could not be allocated or one or more parameters are invalid.
CMN_PUBLIC bool  resize_buffer(
    image::buffer_t *source,
    size_t           new_width,
    size_t           new_height,
    int32_t          border_mode,
    image::buffer_t *target);

/// Builds a level 0 version of a source image. The image is resized if
/// necessary; otherwise, it is copied.
/// @param source Pointer to the structure representing the source image.
/// @param target_width The width of the level 0 image, in pixels.
/// @param target_height The height of the level 0 image, in pixels.
/// @param border_mode One of the image::border_mode_e constants describing how
/// to perform sampling at the borders of the image.
/// @param target Pointer to the buffer structure that will be allocated and
/// initialized with the level 0 image data.
/// @return true if the operation was successful, or false if the necessary
/// memory could not be allocated or one or more parameters are invalid.
CMN_PUBLIC bool  build_level0(
    image::buffer_t *source,
    size_t           target_width,
    size_t           target_height,
    int32_t          border_mode,
    image::buffer_t *target);

/// Builds the mipmap chain for a given source image. Each dimension of the
/// source image is reduced by 50% at each mip-level.
/// @param level_0 Pointer to the structure representing the level 0 image.
/// @param border_mode One of the image::border_mode_e constants describing how
/// to perform sampling at the borders of the image.
/// @param level_count The number of mip-levels to generate, including the
/// level 0 image. This value must be at least 1.
/// @param level_data Pointer to an array of image buffer objects that will be
/// populated with the data for each mip-level.
/// @return true if the operation was successful, or false if the necessary
/// memory could not be allocated or one or more parameters are invalid.
CMN_PUBLIC bool  build_mipmaps(
    image::buffer_t *level_0,
    int32_t          border_mode,
    size_t           level_count,
    image::buffer_t *level_data);

/// Performs a series of operations on an input image to prepare it for
/// runtime use as a texture.
/// @param inputs The texture compiler inputs describing the operations to be
/// performed on the input image.
/// @param outputs Pointer to a structure used to store the result data.
/// @return true if the operation completed successfully.
CMN_PUBLIC bool  compile_texture(
    texture_compiler_inputs_t  *inputs,
    texture_compiler_outputs_t *outputs);

/// Converts an RGB image buffer to a pixel array of 16 bits-per-pixel unsigned
/// integer data.
/// @param buffer The buffer to convert. The buffer must have three channels.
/// @return A pointer to the interleaved pixel data.
CMN_PUBLIC void* buffer_to_pixels_16i_565(image::buffer_t *buffer);

/// Converts an RGBA image buffer to a pixel array of 16 bits-per-pixel
/// unsigned integer data.
/// @param buffer The buffer to convert. The buffer must have four channels.
/// @return A pointer to the interleaved pixel data.
CMN_PUBLIC void* buffer_to_pixels_16i_4444(image::buffer_t *buffer);

/// Converts an RGBA image buffer to a pixel array of 16 bits-per-pixel
/// unsigned integer data.
/// @param buffer The buffer to convert. The buffer must have four channels.
/// @return A pointer to the interleaved pixel data.
CMN_PUBLIC void* buffer_to_pixels_16i_5551(image::buffer_t *buffer);

/// Converts an image buffer to a pixel array of 8 bits-per-channel unsigned
/// integer data.
/// @param buffer The buffer to convert.
/// @return A pointer to the interleaved pixel data.
CMN_PUBLIC void* buffer_to_pixels_32i(image::buffer_t *buffer);

/// Converts an image buffer to a pixel array of 16 bits-per-channel half-
/// precision floating point data.
/// @param buffer The buffer to convert.
/// @return A pointer to the interleaved pixel data.
CMN_PUBLIC void* buffer_to_pixels_64f(image::buffer_t *buffer);

/// Converts an image buffer to a pixel array of 32 bits-per-channel single-
/// precision floating point data.
/// @param buffer The buffer to convert.
/// @return A pointer to the interleaved pixel data.
CMN_PUBLIC void* buffer_to_pixels_128f(image::buffer_t *buffer);

/// Releases the memory allocated for a buffer.
/// @param buffer Pointer to the buffer to be freed.
CMN_PUBLIC void  free_buffer(image::buffer_t *buffer);

/// Releases the memory allocated for an image.
/// @param pixels Pointer to the pixel buffer to be freed.
CMN_PUBLIC void  free_pixels(void *pixels);

/*/////////////////////
//   Namespace End   //
/////////////////////*/

#endif /* TEXTURE_COMPILER_COMPILER_HPP_INCLUDED */

/*/////////////////////////////////////////////////////////////////////////////
//    $Id$
///////////////////////////////////////////////////////////////////////////80*/
