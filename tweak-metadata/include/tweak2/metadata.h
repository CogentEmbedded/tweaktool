/**
 * @file metadata.h
 * @ingroup tweak-api
 *
 * @brief Metadata instance factory.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
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
 * @brief Known control types.
 */
typedef enum {
    TWEAK_METADATA_CONTROL_UNSPECIFIED = 0,
    TWEAK_METADATA_CONTROL_CHECKBOX,
    TWEAK_METADATA_CONTROL_SPINBOX,
    TWEAK_METADATA_CONTROL_SLIDER,
    TWEAK_METADATA_CONTROL_COMBOBOX,
    TWEAK_METADATA_CONTROL_BUTTON
} tweak_metadata_control_type;

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
 * @note When @p json_snippet is NULL or a single value json an empty string,
 * it's assumed to be equal to "{}". That would mean that metadata constructor
 * shall compose metadata instance from default values implied by @p item_type
 * parameter.
 *
 * @return New metadata instance.
 */
tweak_metadata tweak_metadata_create(tweak_variant_type item_type, const char* json_snippet);

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
 * @brief Destroy metadata instance.
 *
 * @param metadata value returned by @see tweak_metadata_create.
 */
void tweak_metadata_destroy(tweak_metadata metadata);

#ifdef __cplusplus
}
#endif

#endif /* TWEAK_METADATA_H_INCLUDED */
