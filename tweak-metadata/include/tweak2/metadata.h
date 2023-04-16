/**
 * @file metadata.h
 * @ingroup tweak-api
 *
 * @brief Metadata instance factory.
 *
 * @copyright 2020-2023 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK_METADATA_H_INCLUDED
#define TWEAK_METADATA_H_INCLUDED

#include <stdbool.h>
#include <tweak2/variant.h>
#include <tweak2/string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief forward declaration for metadata class.
 */
struct tweak_metadata_base;

/**
 * @brief handle for metadata instance.
 */
typedef struct tweak_metadata_base *tweak_metadata;

/**
 * @brief forward declaration for enum class.
 */
struct tweak_metadata_options_base;

/**
 * @brief handle for options enum class.
 */
typedef struct tweak_metadata_options_base *tweak_metadata_options;

/**
 * @brief forward declaration for enum class.
 */
struct tweak_metadata_layout_base;

/**
 * @brief handle for options enum class.
 */
typedef struct tweak_metadata_layout_base *tweak_metadata_layout;

/**
 * @brief Known control types.
 */
typedef enum {
    TWEAK_METADATA_CONTROL_UNSPECIFIED = 0,
    TWEAK_METADATA_CONTROL_CHECKBOX,
    TWEAK_METADATA_CONTROL_SPINBOX,
    TWEAK_METADATA_CONTROL_SLIDER,
    TWEAK_METADATA_CONTROL_COMBOBOX,
    TWEAK_METADATA_CONTROL_BUTTON,
    TWEAK_METADATA_CONTROL_EDITBOX,
    TWEAK_METADATA_CONTROL_TABLE
} tweak_metadata_control_type;

/**
 * @brief Known layouts.
 */
typedef enum {
    TWEAK_METADATA_LAYOUT_ORDER_ROW_MAJOR = 0,
    TWEAK_METADATA_LAYOUT_ORDER_COLUMN_MAJOR
} tweak_metadata_layout_order;

/**
 * @brief Constructor for metadata instance.
 *
 * @param item_type Affects default values of most settings.
 * For instance, when this parameter is equal to
 * TWEAK_VARIANT_TYPE_BOOL, then default value of metadata's
 * control field will be equal to TWEAK_METADATA_CONTROL_CHECKBOX,
 * with min=false and max=true. Refer to Metadata.md for detailed info.
 *
 * @param json_snippet Alters default values. For instance,
 * if it is equal to "{ "options\": ["Foo", "Bar", "Baz"] }"
 * then value of "control" shall be "combobox" min=0 and max=2
 * even though default control type for integer values is
 * TWEAK_METADATA_CONTROL_SPINBOX.
 *
 * @param item_count Number of elements in vector data type.
 * Ignored for scalar types and is expected to be 1.
 *
 * @note When @p json_snippet is NULL or a single value json an empty string,
 * it's assumed to be equal to "{}". That would mean that metadata constructor
 * shall compose metadata instance from default values implied by @p item_type
 * parameter.
 *
 * @return New metadata instance.
 */
tweak_metadata tweak_metadata_create(tweak_variant_type item_type, size_t item_count, const char* json_snippet);

/**
 * @brief Accessor method for control_type field. Control type describes
 * default control to edit values.
 *
 * @param metadata Instance returned by @see tweak_metadata_create.
 * @return Recommended editor control.
 */
tweak_metadata_control_type tweak_metadata_get_control_type(tweak_metadata metadata);

/**
 * @brief Accessor method for min field.
 *
 * Min is the lowest possible value that can be set in editor for given tweak.
 * Not applicable for non numeric types.
 *
 * @param metadata Instance returned by @see tweak_metadata_create.
 * @return Lower bound of value could be set by an editor.
 * User doesn't own this instance and is supposed to copy it prior
 * to @see tweak_metadata_destroy with @p metadata as an argument.
 */
const tweak_variant* tweak_metadata_get_min(tweak_metadata metadata);

/**
 * @brief Accessor method for max field.
 *
 * Max is the highest possible value that can be set in editor for given tweak.
 * Not applicable for non numeric types.
 *
 * @param metadata instance returned by @see tweak_metadata_create.
 * @return upper bound of value could be set by an editor.
 * User doesn't own this instance and is supposed to copy it prior
 * to @see tweak_metadata_destroy with @p metadata as an argument.
 */
const tweak_variant* tweak_metadata_get_max(tweak_metadata metadata);

/**
 * @brief Accessor method for readonly field.
 *
 * Readonly tweaks cannot be altered by user.
 *
 * @param metadata instance returned by @see tweak_metadata_create.
 * @return true if chosen editor shouldn't allow user to edit value.
 */
bool tweak_metadata_get_readonly(tweak_metadata metadata);

/**
 * @brief Accessor method for unit field.
 *
 * Measurement unit of a value.
 *
 * @param metadata instance returned by @see tweak_metadata_create.
 * @return GUI control's "unit" hint.
 */
const tweak_variant_string* tweak_metadata_get_unit(tweak_metadata metadata);

/**
 * @brief Accessor method for decimals field.
 *
 * Decimals field is only applicable for floating point value where
 * it's reasonable to restrict precision to few decimal positions
 * after dot.
 *
 * @param metadata instance returned by @see tweak_metadata_create.
 * @return number of decimal positions after dot.
 */
uint32_t tweak_metadata_get_decimals(tweak_metadata metadata);

/**
 * @brief Accessor method for step field.
 *
 * Step field is applicable for numeric editors prividing increment
 * and decrement features.
 *
 * @param metadata instance returned by @see tweak_metadata_create.
 * @return value being added of subtracted in editor when user press + or -.
 * User doesn't own this instance and is supposed to copy it prior
 * to @see tweak_metadata_destroy with @p metadata as an argument.
 */
const tweak_variant* tweak_metadata_get_step(tweak_metadata metadata);

/**
 * @brief Accessor method for caption field.
 *
 * Caption field for controls like button.
 *
 * @param metadata instance returned by @see tweak_metadata_create.
 * @return Caption field that shall be printed on control if
 * it supports this feature.
 */
const tweak_variant_string* tweak_metadata_get_caption(tweak_metadata metadata);

/**
 * @brief Accessor method for options field.
 *
 * Options is a collection of "text" -> "value" items for combobox editor.
 * Text is shown to user, value is being assigned to tweak.
 * If options is specified as an array in the metadata, then value is being
 * assigned to index of a string in the array.
 *
 * @param metadata instance returned by @see tweak_metadata_create.
 * @return options for combobox control or NULL .
 */
tweak_metadata_options tweak_metadata_get_options(tweak_metadata metadata);

/**
 * @brief Number of items in options map.
 *
 * @param options Instance returned by @see tweak_metadata_get_options.
 * @return Number of options in combobox map.
 */
size_t tweak_metadata_get_enum_size(tweak_metadata_options options);

/**
 * @brief Access "text" field of an items in options map given its @p index.
 *
 * @param options Instance returned by @see tweak_metadata_get_options.
 * @param index Index of an item.
 * @return Option's text. User doesn't own this instance and
 * is supposed to copy it prior to @see tweak_metadata_enum_destroy
 * with @p options as an argument.
 */
const tweak_variant_string* tweak_metadata_get_enum_text(tweak_metadata_options arg, size_t index);

/**
 * @brief Access "value" field of an items in options map given its @p index.
 *
 * @param options Instance returned by @see tweak_metadata_get_options.
 * @param index Index of an item.
 * @return Option's value. User doesn't own this instance and
 * is supposed to copy it prior to @see tweak_metadata_enum_destroy
 * with @p options as an argument.
 */
const tweak_variant* tweak_metadata_get_enum_value(tweak_metadata_options options, size_t index);

/**
 * @brief Accessor method for layout field.
 *
 * @param metadata instance returned by @see tweak_metadata_create.
 * @return layout instance.
 *
 * - Always NULL for scalar types
 * - Always non-NULL for vector types
 */
tweak_metadata_layout tweak_metadata_get_layout(tweak_metadata metadata);

/**
 * @brief Access number of dimensions in an array.
 * @param layout Instance returned by @see tweak_metadata_get_layout.
 *
 * @return number of dimensions in an array.
 */
size_t tweak_metadata_layout_get_number_of_dimensions(tweak_metadata_layout layout);

/**
 * @brief Access size of specific dimension.
 * @param layout Instance returned by @see tweak_metadata_get_layout.
 * @param dimension dimension index.
 *
 * @return number of dimensions in a matrix.
 */
size_t tweak_metadata_layout_get_dimension(tweak_metadata_layout layout, size_t dimension);

/**
 * @brief Access memory order of a matrix.
 *
 * @param layout Instance returned by @see tweak_metadata_get_layout.
 *
 * @return enumeration describing whether matrix is row major or column major.
 */
tweak_metadata_layout_order tweak_metadata_layout_get_order(tweak_metadata_layout layout);

/**
 * @brief Copy metadata instance.
 *
 * @param metadata value returned by @see tweak_metadata_create.
 */
tweak_metadata tweak_metadata_copy(tweak_metadata metadata);

/**
 * @brief Destroy metadata instance.
 *
 * @param metadata value returned by @see tweak_metadata_create.
 */
void tweak_metadata_destroy(tweak_metadata metadata);

#ifdef __cplusplus
}
#endif

#endif /* TWEAK_METADATA_H_INCLUDED */
