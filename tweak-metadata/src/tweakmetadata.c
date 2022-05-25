/**
 * @file tweakmetadata.c
 * @ingroup tweak-api
 *
 * @brief implementation of metadata instance factory.
 *
 * @copyright 2020-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.
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

#include <tweak2/metadata.h>
#include <tweak2/json.h>
#include <float.h>
#include <math.h>

#include <stdlib.h>
#include <utarray.h>

static void metadata_option_init(void *elt);

static void metadata_option_copy(void *dst, const void *src);

static void metadata_option_destroy(void *elt);

static const UT_icd ut_size_t_icd = {sizeof(size_t), NULL, NULL, NULL};

struct option {
    tweak_variant_string text;
    tweak_variant value;
};

static UT_icd metadata_option_icd = {
    .sz = sizeof(struct option),
    .init = metadata_option_init,
    .copy = metadata_option_copy,
    .dtor = metadata_option_destroy
};

struct tweak_metadata_options_base {
    UT_array *options;
};

struct tweak_metadata_layout_base {
    tweak_metadata_layout_order order;
    UT_array *dimensions;
};

struct tweak_metadata_base {
    tweak_metadata_control_type control_type;
    tweak_variant min;
    tweak_variant max;
    bool readonly;
    uint32_t decimals;
    tweak_variant step;
    tweak_variant_string caption;
    tweak_variant_string unit;
    bool options_present;
    struct tweak_metadata_options_base options;
    bool layout_present;
    struct tweak_metadata_layout_base layout;
};

static void fill_with_defaults(struct tweak_metadata_base* blank_metadata,
    tweak_variant_type item_type, size_t item_count);

static void update_with_custom_user_settings(tweak_variant_type item_type, size_t item_count,
    struct tweak_metadata_base* default_metadata, struct tweak_json_node* document);

tweak_metadata tweak_metadata_create(tweak_variant_type item_type, size_t item_count, const char* json_snippet) {
    (void)item_count;
    tweak_metadata result = calloc(1, sizeof(*result));
    if (!result) {
        return NULL;
    }
    fill_with_defaults(result, item_type, item_count);
    struct tweak_json_node* document = tweak_json_parse(json_snippet);
    if (document) {
        update_with_custom_user_settings(item_type, item_count, result, document);
        tweak_json_destroy(document);
    }
    return result;
}

struct tweak_metadata_options_base clone_options(struct tweak_metadata_options_base* options) {
    struct tweak_metadata_options_base result = { 0 };
    if (!options->options) {
        return result;
    }

    size_t num_options = utarray_len(options->options);
    UT_array *options_copy = NULL;
    utarray_new(options_copy, &metadata_option_icd);
    for (size_t ix = 0; ix < num_options; ++ix) {
        struct option* option = (struct option*)utarray_eltptr(options->options, ix);
        struct option option_copy;
        option_copy.text = tweak_variant_string_copy(&option->text);
        option_copy.value = tweak_variant_copy(&option->value);
        utarray_push_back(options_copy, &option_copy);
    }
    result.options = options_copy;
    return result;
}

struct tweak_metadata_layout_base clone_layout(struct tweak_metadata_layout_base* layout) {
    struct tweak_metadata_layout_base result = { (tweak_metadata_layout_order)0 };
    if (!layout->dimensions) {
        return result;
    }

    UT_array *dimensions_copy = NULL;
    size_t num_dimensions = utarray_len(layout->dimensions);
    utarray_new(dimensions_copy, &ut_size_t_icd);
    for (size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        size_t dimension_size = *((size_t*)utarray_eltptr(layout->dimensions, dimension));
        utarray_push_back(dimensions_copy, &dimension_size);
    }
    result.order = layout->order;
    result.dimensions = dimensions_copy;
    return result;
}

tweak_metadata tweak_metadata_copy(tweak_metadata metadata) {
    tweak_metadata result = malloc(sizeof(*result));
    if (!result) {
        TWEAK_FATAL("malloc() returned NULL");
    }
    result->control_type = metadata->control_type;
    result->min = tweak_variant_copy(&metadata->min);
    result->max = tweak_variant_copy(&metadata->max);
    result->readonly = metadata->readonly;
    result->decimals = result->decimals;
    result->step = tweak_variant_copy(&metadata->step);
    result->caption = tweak_variant_string_copy(&metadata->caption);
    result->unit = tweak_variant_string_copy(&metadata->unit);
    result->options_present = metadata->options_present;
    if (metadata->options_present) {
        result->options = clone_options(&metadata->options);
    } else {
        memset(&result->options, 0, sizeof(result->options));
    }
    result->layout_present = metadata->layout_present;
    if (metadata->layout_present) {
        result->layout = clone_layout(&metadata->layout);
    } else {
        memset(&result->layout, 0, sizeof(result->layout));
    }
    return result;
}

tweak_metadata_control_type tweak_metadata_get_control_type(tweak_metadata metadata) {
    return metadata->control_type;
}

const tweak_variant* tweak_metadata_get_min(tweak_metadata metadata) {
    return &metadata->min;
}

const tweak_variant* tweak_metadata_get_max(tweak_metadata metadata) {
    return &metadata->max;
}

bool tweak_metadata_get_readonly(tweak_metadata metadata) {
    return metadata->readonly;
}

uint32_t tweak_metadata_get_decimals(tweak_metadata metadata) {
    return metadata->decimals;
}

const tweak_variant* tweak_metadata_get_step(tweak_metadata metadata) {
    return &metadata->step;
}

const tweak_variant_string* tweak_metadata_get_caption(tweak_metadata metadata) {
    return &metadata->caption;
}

const tweak_variant_string* tweak_metadata_get_unit(tweak_metadata metadata) {
    return &metadata->unit;
}

tweak_metadata_options tweak_metadata_get_options(tweak_metadata metadata) {
    return metadata->options_present ? &metadata->options : NULL;
}

size_t tweak_metadata_get_enum_size(tweak_metadata_options arg) {
    return utarray_len(arg->options);
}

const tweak_variant_string* tweak_metadata_get_enum_text(tweak_metadata_options arg, size_t index) {
    struct option* option = (struct option*)utarray_eltptr(arg->options, index);
    return option ? &option->text : NULL;
}

const tweak_variant* tweak_metadata_get_enum_value(tweak_metadata_options arg, size_t index) {
    struct option* option = (struct option*)utarray_eltptr(arg->options, index);
    return option ? &option->value : NULL;
}

void tweak_metadata_destroy(tweak_metadata metadata) {
    if (!metadata) {
        return;
    }
    tweak_variant_destroy(&metadata->min);
    tweak_variant_destroy(&metadata->max);
    tweak_variant_destroy(&metadata->step);
    if (metadata->options_present) {
        utarray_free(metadata->options.options);
    }
    if (metadata->layout_present) {
        utarray_free(metadata->layout.dimensions);
    }
    free(metadata);
}

enum { DEFAULT_FLOAT_DECIMALS = 4 };

enum { DEFAULT_DOUBLE_DECIMALS = 4 };

#define IMPLEMENT_DEFAULT_METADATA_INITIALIZER(SUFFIX, CONTROL, VARIANT_TYPE, MIN_VALUE, MAX_VALUE, DECIMALS, STEP) \
    static void fill_with_defaults_##SUFFIX(struct tweak_metadata_base* blank_metadata) {                           \
        memset(blank_metadata, 0, sizeof(*blank_metadata));                                                         \
        blank_metadata->control_type = CONTROL;                                                                     \
        tweak_variant_assign_##SUFFIX(&blank_metadata->min, (MIN_VALUE));                                           \
        tweak_variant_assign_##SUFFIX(&blank_metadata->max, (MAX_VALUE));                                           \
        blank_metadata->decimals = (DECIMALS);                                                                      \
        tweak_variant_assign_##SUFFIX(&blank_metadata->step, (STEP));                                               \
    }

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    bool, TWEAK_METADATA_CONTROL_CHECKBOX, TWEAK_VARIANT_TYPE_BOOL, false, true, 0, 0
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    sint8, TWEAK_METADATA_CONTROL_SPINBOX, TWEAK_VARIANT_TYPE_SINT8, INT8_MIN, INT8_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    sint16, TWEAK_METADATA_CONTROL_SPINBOX, TWEAK_VARIANT_TYPE_SINT16, INT16_MIN, INT16_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    sint32, TWEAK_METADATA_CONTROL_SPINBOX, TWEAK_VARIANT_TYPE_SINT32, INT32_MIN, INT32_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    sint64, TWEAK_METADATA_CONTROL_SPINBOX, TWEAK_VARIANT_TYPE_SINT64, INT64_MIN, INT64_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    uint8, TWEAK_METADATA_CONTROL_SPINBOX,  TWEAK_VARIANT_TYPE_UINT8, 0, UINT8_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    uint16, TWEAK_METADATA_CONTROL_SPINBOX, TWEAK_VARIANT_TYPE_UINT16, 0, UINT16_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    uint32, TWEAK_METADATA_CONTROL_SPINBOX, TWEAK_VARIANT_TYPE_UINT32, 0, UINT32_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    uint64, TWEAK_METADATA_CONTROL_SPINBOX, TWEAK_VARIANT_TYPE_UINT64, 0, UINT64_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    float, TWEAK_METADATA_CONTROL_SLIDER, TWEAK_VARIANT_TYPE_FLOAT, -FLT_MAX, FLT_MAX,
    DEFAULT_FLOAT_DECIMALS, powf(10.f, -DEFAULT_FLOAT_DECIMALS)
)

IMPLEMENT_DEFAULT_METADATA_INITIALIZER(
    double, TWEAK_METADATA_CONTROL_SLIDER, TWEAK_VARIANT_TYPE_DOUBLE, -DBL_MAX, DBL_MAX,
    DEFAULT_DOUBLE_DECIMALS, pow(10., -DEFAULT_DOUBLE_DECIMALS)
)

static void fill_with_defaults_string(struct tweak_metadata_base *blank_metadata) {
    memset(blank_metadata, 0, sizeof(*blank_metadata));
    blank_metadata->control_type = TWEAK_METADATA_CONTROL_EDIT;
    tweak_variant_assign_string(&blank_metadata->min, "");
    tweak_variant_assign_string(&blank_metadata->max, "");
    blank_metadata->decimals = 0;
    tweak_variant_assign_string(&blank_metadata->step, "");
}

#define IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(SUFFIX, CONTROL, VARIANT_TYPE, MIN_VALUE, MAX_VALUE, DECIMALS, STEP) \
static void fill_with_defaults_vector_##SUFFIX(struct tweak_metadata_base *blank_metadata, size_t item_count) {            \
    memset(blank_metadata, 0, sizeof(*blank_metadata));                                                                    \
    blank_metadata->control_type = CONTROL;                                                                                \
    tweak_variant_assign_##SUFFIX(&blank_metadata->min, MIN_VALUE);                                                        \
    tweak_variant_assign_##SUFFIX(&blank_metadata->max, MAX_VALUE);                                                        \
    blank_metadata->decimals = DECIMALS;                                                                                   \
    tweak_variant_assign_##SUFFIX(&blank_metadata->step, STEP);                                                            \
    blank_metadata->layout_present = true;                                                                                 \
    UT_array* dimensions = NULL;                                                                                           \
    utarray_new(dimensions, &ut_size_t_icd);                                                                               \
    utarray_push_back(dimensions, &item_count);                                                                            \
    blank_metadata->layout.dimensions = dimensions;                                                                        \
    blank_metadata->layout.order = TWEAK_METADATA_LAYOUT_ORDER_ROW_MAJOR;                                                  \
}

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    sint8, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_SINT8, INT8_MIN, INT8_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    sint16, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_SINT16, INT16_MIN, INT16_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    sint32, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_SINT32, INT32_MIN, INT32_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    sint64, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_SINT64, INT64_MIN, INT64_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    uint8, TWEAK_METADATA_CONTROL_TABLE,  TWEAK_VARIANT_TYPE_UINT8, 0, UINT8_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    uint16, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_UINT16, 0, UINT16_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    uint32, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_UINT32, 0, UINT32_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    uint64, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_UINT64, 0, UINT64_MAX, 0, 1
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    float, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_FLOAT, -FLT_MAX, FLT_MAX,
    DEFAULT_FLOAT_DECIMALS, powf(10.f, -DEFAULT_FLOAT_DECIMALS)
)

IMPLEMENT_DEFAULT_METADATA_VECTOR_INITIALIZER(
    double, TWEAK_METADATA_CONTROL_TABLE, TWEAK_VARIANT_TYPE_DOUBLE, -DBL_MAX, DBL_MAX,
    DEFAULT_DOUBLE_DECIMALS, pow(10., -DEFAULT_DOUBLE_DECIMALS)
)

static void fill_with_defaults(struct tweak_metadata_base* blank_metadata, tweak_variant_type item_type, size_t item_count) {
    switch (item_type) {
    case TWEAK_VARIANT_TYPE_NULL:
        TWEAK_FATAL("No metadata for null type");
        return;
    case TWEAK_VARIANT_TYPE_BOOL:
        fill_with_defaults_bool(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_SINT8:
        fill_with_defaults_sint8(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_SINT16:
        fill_with_defaults_sint16(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_SINT32:
        fill_with_defaults_sint32(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_SINT64:
        fill_with_defaults_sint64(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_UINT8:
        fill_with_defaults_uint8(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_UINT16:
        fill_with_defaults_uint16(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_UINT32:
        fill_with_defaults_uint32(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_UINT64:
        fill_with_defaults_uint64(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_FLOAT:
        fill_with_defaults_float(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_DOUBLE:
        fill_with_defaults_double(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_STRING:
        fill_with_defaults_string(blank_metadata);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
        fill_with_defaults_vector_sint8(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
        fill_with_defaults_vector_sint16(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
        fill_with_defaults_vector_sint32(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
        fill_with_defaults_vector_sint64(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
        fill_with_defaults_vector_uint8(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
        fill_with_defaults_vector_uint16(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
        fill_with_defaults_vector_uint32(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
        fill_with_defaults_vector_uint64(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
        fill_with_defaults_vector_float(blank_metadata, item_count);
        return;
    case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
        fill_with_defaults_vector_double(blank_metadata, item_count);
        return;
    }
    TWEAK_FATAL("Unknown type: %d", item_type);
}

static void parse_control_type(tweak_metadata_control_type* dst, const struct tweak_json_node* node, tweak_metadata_control_type def_val);

static void parse_variant(tweak_variant* dst, const struct tweak_json_node* node, const tweak_variant* def_val);

static void parse_bool(bool* dst, const struct tweak_json_node* node, bool def_val);

static void parse_uint32(uint32_t* dst, const struct tweak_json_node* node, uint32_t def_val);

static void parse_string(tweak_variant_string* dst, const struct tweak_json_node* node,
    const tweak_variant_string* def_val);

static bool validate_metadata(tweak_variant_type item_type, size_t item_count, struct tweak_metadata_base* metadata);

static bool parse_options_array(struct tweak_metadata_options_base* options,
    const struct tweak_json_node* options_node, tweak_variant_type option_type);

static void merge_metadata_common(struct tweak_metadata_base* user_metadata,
    struct tweak_metadata_base* default_metadata, struct tweak_json_node* document);

static void merge_metadata_default(struct tweak_metadata_base* user_metadata,
    struct tweak_metadata_base* default_metadata, struct tweak_json_node* document);

static void merge_metadata_options_map_present(struct tweak_metadata_base* user_metadata,
    struct tweak_metadata_base* default_metadata, struct tweak_json_node* document);

static bool parse_layout_object(struct tweak_metadata_layout_base *user_layout,
    const struct tweak_json_node* layout_node);

static void update_with_custom_user_settings(tweak_variant_type item_type,
    size_t item_count, struct tweak_metadata_base* metadata, struct tweak_json_node* document)
{
    struct tweak_metadata_base user_settings = { (tweak_metadata_control_type)0 };

    parse_bool(&user_settings.readonly,
        tweak_json_get_object_field(document, "readonly", TWEAK_JSON_NODE_TYPE_BOOL),
        metadata->readonly);

    parse_string(&user_settings.unit,
        tweak_json_get_object_field(document, "unit", TWEAK_JSON_NODE_TYPE_STRING),
        &metadata->unit);

    const struct tweak_json_node* options_node = tweak_json_get_object_field(document, "options", TWEAK_JSON_NODE_TYPE_ARRAY);
    struct tweak_metadata_options_base user_options = { 0 };

    user_settings.options_present =
        tweak_json_get_type(options_node) == TWEAK_JSON_NODE_TYPE_ARRAY
            && parse_options_array(&user_options, options_node, metadata->min.type);

    const struct tweak_json_node* layout_node = tweak_json_get_object_field(document, "layout", TWEAK_JSON_NODE_TYPE_OBJECT);
    user_settings.layout_present = layout_node != NULL && parse_layout_object(&user_settings.layout, layout_node);

    if (user_settings.options_present) {
        user_settings.options = user_options;
        merge_metadata_options_map_present(&user_settings, metadata, document);
    } else {
        merge_metadata_default(&user_settings, metadata, document);
    }

    if (validate_metadata(item_type, item_count, &user_settings)) {
        metadata->control_type = user_settings.control_type;
        if (user_settings.options_present && user_settings.control_type != TWEAK_METADATA_CONTROL_COMBOBOX) {
            TWEAK_LOG_WARN("Options node is present, but control_type isn't combobox");
        }
        tweak_variant_swap(&metadata->min, &user_settings.min);
        tweak_variant_destroy(&user_settings.min);
        tweak_variant_swap(&metadata->max, &user_settings.max);
        tweak_variant_destroy(&user_settings.max);
        metadata->readonly = user_settings.readonly;
        metadata->decimals = user_settings.decimals;
        tweak_variant_swap(&metadata->step, &user_settings.step);
        tweak_variant_destroy(&user_settings.step);
        tweak_variant_swap_string(&metadata->caption, &user_settings.caption);
        tweak_variant_swap_string(&metadata->unit, &user_settings.unit);
        tweak_variant_destroy_string(&user_settings.caption);
        metadata->options_present = user_settings.options_present;
        if (metadata->options_present) {
            metadata->options = user_settings.options;
        }
        metadata->layout_present = user_settings.layout_present;
        if (metadata->layout_present) {
            if (metadata->layout.dimensions != NULL) {
                utarray_free(metadata->layout.dimensions);
            }
            metadata->layout = user_settings.layout;
        }
    }
}

static void merge_metadata_default(struct tweak_metadata_base* user_metadata,
    struct tweak_metadata_base* default_metadata, struct tweak_json_node* document)
{
    parse_control_type(&user_metadata->control_type,
        tweak_json_get_object_field(document, "control", TWEAK_JSON_NODE_TYPE_STRING),
        default_metadata->control_type);
    parse_variant(&user_metadata->min,
        tweak_json_get_object_field(document, "min", TWEAK_JSON_NODE_TYPE_NUMBER),
        &default_metadata->min);
    parse_variant(&user_metadata->max,
        tweak_json_get_object_field(document, "max", TWEAK_JSON_NODE_TYPE_NUMBER),
        &default_metadata->max);
    parse_variant(&user_metadata->step,
        tweak_json_get_object_field(document, "step", TWEAK_JSON_NODE_TYPE_NUMBER),
        &default_metadata->step);
    parse_uint32(&user_metadata->decimals,
        tweak_json_get_object_field(document, "decimals", TWEAK_JSON_NODE_TYPE_NUMBER),
        default_metadata->decimals);
    merge_metadata_common(user_metadata, default_metadata, document);
}

static void merge_metadata_options_map_present(struct tweak_metadata_base* user_metadata,
    struct tweak_metadata_base* default_metadata, struct tweak_json_node* document)
{
    parse_control_type(&user_metadata->control_type,
        tweak_json_get_object_field(document, "control", TWEAK_JSON_NODE_TYPE_STRING),
        TWEAK_METADATA_CONTROL_COMBOBOX);
    tweak_variant null_default = TWEAK_VARIANT_INIT_EMPTY;
    user_metadata->min = tweak_variant_copy(&null_default);
    user_metadata->max = tweak_variant_copy(&null_default);
    user_metadata->step = tweak_variant_copy(&null_default);
    user_metadata->decimals = 0U;
    merge_metadata_common(user_metadata, default_metadata, document);
}

static void merge_metadata_common(struct tweak_metadata_base* user_metadata,
    struct tweak_metadata_base* default_metadata, struct tweak_json_node* document)
{
    parse_string(&user_metadata->caption,
        tweak_json_get_object_field(document, "caption", TWEAK_JSON_NODE_TYPE_STRING),
        &default_metadata->caption);
    parse_string(&user_metadata->unit,
        tweak_json_get_object_field(document, "unit", TWEAK_JSON_NODE_TYPE_STRING),
        &default_metadata->unit);
}

/*
| Tweak type                | Data flavour |
|---------------------------|--------------|
| TWEAK_VARIANT_TYPE_NULL   | -            |
| TWEAK_VARIANT_TYPE_BOOL   | Boolean      |
| TWEAK_VARIANT_TYPE_SINT8  | Integer      |
| TWEAK_VARIANT_TYPE_SINT16 | Integer      |
| TWEAK_VARIANT_TYPE_SINT32 | Integer      |
| TWEAK_VARIANT_TYPE_SINT64 | Integer      |
| TWEAK_VARIANT_TYPE_UINT8  | Integer      |
| TWEAK_VARIANT_TYPE_UINT16 | Integer      |
| TWEAK_VARIANT_TYPE_UINT32 | Integer      |
| TWEAK_VARIANT_TYPE_UINT64 | Integer      |
| TWEAK_VARIANT_TYPE_FLOAT  | Float        |
| TWEAK_VARIANT_TYPE_DOUBLE | Float        |
*/

static bool validate_metadata_bool(struct tweak_metadata_base* user_settings);

static bool validate_metadata_integer(struct tweak_metadata_base* user_settings);

static bool validate_metadata_float(struct tweak_metadata_base* user_settings);

static bool validate_metadata_string(struct tweak_metadata_base* user_settings);

static bool validate_metadata_vector(struct tweak_metadata_base* user_settings, size_t item_count);

static bool validate_metadata(tweak_variant_type item_type,
    size_t item_count, struct tweak_metadata_base* user_settings)
{
    switch (item_type) {
    case TWEAK_VARIANT_TYPE_NULL:
        TWEAK_FATAL("Null type");
        return false;
    case TWEAK_VARIANT_TYPE_BOOL:
        return validate_metadata_bool(user_settings);
    case TWEAK_VARIANT_TYPE_SINT8:
    case TWEAK_VARIANT_TYPE_SINT16:
    case TWEAK_VARIANT_TYPE_SINT32:
    case TWEAK_VARIANT_TYPE_SINT64:
    case TWEAK_VARIANT_TYPE_UINT8:
    case TWEAK_VARIANT_TYPE_UINT16:
    case TWEAK_VARIANT_TYPE_UINT32:
    case TWEAK_VARIANT_TYPE_UINT64:
        return validate_metadata_integer(user_settings);
    case TWEAK_VARIANT_TYPE_FLOAT:
    case TWEAK_VARIANT_TYPE_DOUBLE:
        return validate_metadata_float(user_settings);
    case TWEAK_VARIANT_TYPE_STRING:
        return validate_metadata_string(user_settings);
    case TWEAK_VARIANT_TYPE_VECTOR_SINT8:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT16:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT32:
    case TWEAK_VARIANT_TYPE_VECTOR_SINT64:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT8:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT16:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT32:
    case TWEAK_VARIANT_TYPE_VECTOR_UINT64:
    case TWEAK_VARIANT_TYPE_VECTOR_FLOAT:
    case TWEAK_VARIANT_TYPE_VECTOR_DOUBLE:
        return validate_metadata_vector(user_settings, item_count);
    }
    TWEAK_FATAL("Unknown type: %d", item_type);
    return false;
}

/*
 | Data flavour       | checkbox | spinbox | slider | combobox |
 |--------------------|----------|---------|--------|----------|
 | Boolean            | Y        | Y       | N      | Y        |

 | Data flavour       | readonly | min | max | decimals | step | options | default editor |
 |--------------------|----------|-----|-----|----------|------|---------|----------------|
 | Boolean            | Y        | N   | N   | N        | N    | N       | checkbox       |
 */
static bool validate_metadata_bool(struct tweak_metadata_base* user_settings) {
    switch (user_settings->control_type) {
    case TWEAK_METADATA_CONTROL_BUTTON:
    case TWEAK_METADATA_CONTROL_CHECKBOX:
    case TWEAK_METADATA_CONTROL_SPINBOX:
    case TWEAK_METADATA_CONTROL_COMBOBOX:
        break;
    default:
        return false;
    }
    return true;
}

/*
 | Data flavour       | checkbox | spinbox | slider | combobox |
 |--------------------|----------|---------|--------|----------|
 | Integer            | N        | Y       | Y      | Y        |

 | Data flavour       | readonly | min | max | decimals | step | options | default editor |
 |--------------------|----------|-----|-----|----------|------|---------|----------------|
 | Integer            | Y        | Y   | Y   | N        | Y    | Y       | spinbox        |
 */
static bool validate_metadata_integer(struct tweak_metadata_base* user_settings){
    switch (user_settings->control_type) {
    case TWEAK_METADATA_CONTROL_SPINBOX:
    case TWEAK_METADATA_CONTROL_SLIDER:
    case TWEAK_METADATA_CONTROL_COMBOBOX:
        break;
    default:
        return false;
    }
    return true;
}

/*
 | Data flavour       | checkbox | spinbox | slider | combobox |
 |--------------------|----------|---------|--------|----------|
 | Float              | N        | Y       | Y      | N        |

 | Data flavour       | readonly | min | max | decimals | step | options | default editor |
 |--------------------|----------|-----|-----|----------|------|---------|----------------|
 | Float              | Y        | Y   | Y   | Y        | Y    | N       | slider         |
 */
static bool validate_metadata_float(struct tweak_metadata_base* user_settings) {
    switch (user_settings->control_type) {
    case TWEAK_METADATA_CONTROL_SLIDER:
    case TWEAK_METADATA_CONTROL_SPINBOX:
        break;
    default:
        return false;
    }
    return true;
}

static bool validate_metadata_string(struct tweak_metadata_base* user_settings) {
    assert(user_settings);
    switch (user_settings->control_type) {
    case TWEAK_METADATA_CONTROL_EDIT:
        break;
    default:
        return false;
    }
    return true;
}

static bool validate_metadata_vector(struct tweak_metadata_base *user_settings,
                                     size_t item_count)
{
  if (user_settings->control_type == TWEAK_METADATA_CONTROL_TABLE) {
    if (user_settings->layout_present) {
      size_t product = 1;
      size_t num_dimensions = utarray_len(user_settings->layout.dimensions);
      for (size_t dimension = 0; dimension < num_dimensions; ++dimension) {
        size_t dimension_size = *((size_t *)utarray_eltptr(
            user_settings->layout.dimensions, dimension));
        product *= dimension_size;
      }
      if (item_count == product) {
        return true;
      } else {
        TWEAK_LOG_WARN(
            "Dimensions disagree: product of all dimensions is equal to %zu,"
            " number of elements is %zu",
            product, item_count);
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

static bool variant_increment(tweak_variant* arg);

static void parse_options_array_string_node(UT_array *array,
    const struct tweak_json_node* option_string_node, tweak_variant* default_value);

static bool parse_options_array_option_node(UT_array *array,
    const struct tweak_json_node* option_object_node, tweak_variant* default_value);

static bool parse_options_array(struct tweak_metadata_options_base* options,
    const struct tweak_json_node* options_node, tweak_variant_type option_type)
{
    size_t size;
    UT_array *options_array = NULL;
    tweak_json_get_array_size_status array_size_status = tweak_json_get_array_size(options_node, &size);
    if (array_size_status == TWEAK_JSON_GET_SIZE_SUCCESS) {
        tweak_variant default_value = { .type = option_type };
        utarray_new(options_array, &metadata_option_icd);
        for (size_t index = 0; index < size; index++) {
            if (index != 0 && !variant_increment(&default_value))
                goto error;

            const struct tweak_json_node* array_item_node =
                tweak_json_get_array_item(options_node, index,
                    (tweak_json_node_type)(TWEAK_JSON_NODE_TYPE_STRING | TWEAK_JSON_NODE_TYPE_OBJECT));

            switch (tweak_json_get_type(array_item_node)) {
            case TWEAK_JSON_NODE_TYPE_STRING:
                parse_options_array_string_node(options_array, array_item_node, &default_value);
                break;
            case TWEAK_JSON_NODE_TYPE_OBJECT:
                if (!parse_options_array_option_node(options_array, array_item_node, &default_value))
                    goto error;
                break;
            default:
                goto error;
            }
        }
    }
    options->options = options_array;
    return true;
error:
    utarray_free(options_array);
    return false;
}

static void parse_options_array_string_node(UT_array *array,
    const struct tweak_json_node* option_string_node, tweak_variant* default_value)
{
    struct option option = {
        .text = TWEAK_VARIANT_STRING_EMPTY,
        .value = TWEAK_VARIANT_INIT_EMPTY
    };
    tweak_assign_string(&option.text, tweak_json_node_as_c_str(option_string_node));
    option.value = tweak_variant_copy(default_value);
    utarray_push_back(array, &option);
    tweak_variant_destroy(&option.value);
    tweak_variant_destroy_string(&option.text);
}

static bool parse_options_array_option_node(UT_array *array,
    const struct tweak_json_node* option_object_node, tweak_variant* default_value)
{
    const struct tweak_json_node* text_node = tweak_json_get_object_field(option_object_node,
        "text", TWEAK_JSON_NODE_TYPE_STRING);

    const struct tweak_json_node* value_node = tweak_json_get_object_field(option_object_node,
        "value", (tweak_json_node_type)(TWEAK_JSON_NODE_TYPE_NUMBER | TWEAK_JSON_NODE_TYPE_BOOL));

    if (text_node != NULL && value_node != NULL)
    {
        tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;
        tweak_variant_type_conversion_result conversion_result =
            tweak_variant_from_string(tweak_json_node_as_c_str(value_node),
                default_value->type, &tmp);
        switch (conversion_result) {
        case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED:
            {
                tweak_variant_string truncated_str = tweak_variant_to_string(&tmp);
                TWEAK_LOG_WARN("Value %s has been truncated to %s",
                    tweak_json_node_as_c_str(value_node),
                    tweak_variant_string_c_str(&truncated_str));
                tweak_variant_destroy_string(&truncated_str);
            } /* Fallthrough */
        case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS:
            {
                tweak_variant_swap(default_value, &tmp);
                tweak_variant_destroy(&tmp);
                struct option option = {
                    .text = TWEAK_VARIANT_STRING_EMPTY,
                    .value = TWEAK_VARIANT_INIT_EMPTY};
                tweak_assign_string(&option.text, tweak_json_node_as_c_str(text_node));
                option.value = tweak_variant_copy(default_value);
                utarray_push_back(array, &option);
                tweak_variant_destroy(&option.value);
                tweak_variant_destroy_string(&option.text);
                return true;
            }
        default:
            break;
        }
    }
    return false;
}

static void parse_control_type(tweak_metadata_control_type* dst, const struct tweak_json_node* node,
    tweak_metadata_control_type def_val)
{
    if (tweak_json_get_type(node) == TWEAK_JSON_NODE_TYPE_STRING) {
        const char* str = tweak_json_node_as_c_str(node);
        if (strcmp("checkbox", str) == 0) {
            *dst = TWEAK_METADATA_CONTROL_CHECKBOX;
            return;
        } else if (strcmp("spinbox", str) == 0) {
            *dst = TWEAK_METADATA_CONTROL_SPINBOX;
            return;
        } else if (strcmp("slider", str) == 0) {
            *dst = TWEAK_METADATA_CONTROL_SLIDER;
            return;
        } else if (strcmp("combobox", str) == 0) {
            *dst = TWEAK_METADATA_CONTROL_COMBOBOX;
            return;
        } else if (strcmp("button", str) == 0) {
            *dst = TWEAK_METADATA_CONTROL_BUTTON;
            return;
        }
    }
    *dst = def_val;
}

static void parse_variant(tweak_variant* dst, const struct tweak_json_node* node,
    const tweak_variant* def_val)
{
    if ((tweak_json_get_type(node) & TWEAK_JSON_NODE_TYPE_VALUE) != 0) {
        tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;
        tweak_variant_type_conversion_result conversion_result =
            tweak_variant_from_string(tweak_json_node_as_c_str(node), def_val->type, &tmp);
        switch (conversion_result) {
        case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED:
            {
                tweak_variant_string truncated_str = tweak_variant_to_string(&tmp);
                TWEAK_LOG_WARN("Value %s has been truncated to %s",
                    tweak_json_node_as_c_str(node),
                    tweak_variant_string_c_str(&truncated_str));
                tweak_variant_destroy_string(&truncated_str);
            } /* Fallthrough */
        case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS:
            tweak_variant_swap(dst, &tmp);
            tweak_variant_destroy(&tmp);
            return;
        default:
            break;
        }
    }
    *dst = tweak_variant_copy(def_val);
}

static void parse_bool(bool* dst, const struct tweak_json_node* node,
    bool def_val)
{
    if ((tweak_json_get_type(node) & TWEAK_JSON_NODE_TYPE_VALUE) != 0) {
        tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;
        tweak_variant_type_conversion_result conversion_result =
            tweak_variant_from_string(tweak_json_node_as_c_str(node), TWEAK_VARIANT_TYPE_BOOL, &tmp);

        *dst = conversion_result == TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS
            ? tmp.value.b
            : def_val;

        tweak_variant_destroy(&tmp);
        return;
    }
    *dst = def_val;
}

static void parse_uint32(uint32_t* dst, const struct tweak_json_node* node,
    uint32_t def_val)
{
    if ((tweak_json_get_type(node) & TWEAK_JSON_NODE_TYPE_NUMBER) != 0) {
        tweak_variant tmp = TWEAK_VARIANT_INIT_EMPTY;
        tweak_variant_type_conversion_result conversion_result =
            tweak_variant_from_string(tweak_json_node_as_c_str(node), TWEAK_VARIANT_TYPE_UINT32, &tmp);

        switch (conversion_result) {
        case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_ERROR_TRUNCATED:
            {
                tweak_variant_string truncated_str = tweak_variant_to_string(&tmp);
                TWEAK_LOG_WARN("Value %s has been truncated to %s",
                    tweak_json_node_as_c_str(node),
                    tweak_variant_string_c_str(&truncated_str));
                tweak_variant_destroy_string(&truncated_str);
            } /* Fallthrough */
        case TWEAK_VARIANT_TYPE_CONVERSION_RESULT_SUCCESS:
            *dst = tmp.value.uint32;
            tweak_variant_destroy(&tmp);
            break;
        default:
            *dst = def_val;
            break;
        }
    }
}

static void parse_string(tweak_variant_string* dst, const struct tweak_json_node* node,
    const tweak_variant_string* def_val)
{
    if ((tweak_json_get_type(node) & TWEAK_JSON_NODE_TYPE_STRING) != 0) {
        tweak_assign_string(dst, tweak_json_node_as_c_str(node));
        return;
    }
    *dst = tweak_variant_string_copy(def_val);
}

static void metadata_option_init(void *elt) {
    struct option* option = (struct option*) elt;
    memset(option, 0, sizeof(*option));
}

static void metadata_option_copy(void *dst, const void *src) {
    const struct option* option_src = (const struct option*) src;
    struct option* option_dst = (struct option*) dst;
    option_dst->text = tweak_variant_string_copy(&option_src->text);
    option_dst->value = tweak_variant_copy(&option_src->value);
}

static void metadata_option_destroy(void *elt) {
    struct option* option = (struct option*) elt;
    tweak_variant_destroy_string(&option->text);
    tweak_variant_destroy(&option->value);
}

static bool variant_increment(tweak_variant* arg) {
    switch (arg->type) {
    case TWEAK_VARIANT_TYPE_BOOL:
        if (arg->value.b == false) {
            arg->value.b = true;
            return true;
        }
        break;
    case TWEAK_VARIANT_TYPE_SINT8:
        if (arg->value.sint8 < INT8_MAX) {
            ++arg->value.sint8;
            return true;
        }
        break;
    case TWEAK_VARIANT_TYPE_SINT16:
        if (arg->value.sint16 < INT16_MAX) {
            ++arg->value.sint16;
            return true;
        }
        break;
    case TWEAK_VARIANT_TYPE_SINT32:
        if (arg->value.sint32 < INT32_MAX) {
            ++arg->value.sint32;
            return true;
        }
        break;
    case TWEAK_VARIANT_TYPE_SINT64:
        if (arg->value.sint64 < INT64_MAX) {
            ++arg->value.sint64;
            return true;
        }
        break;
    case TWEAK_VARIANT_TYPE_UINT8:
        if (arg->value.uint8 < UINT8_MAX) {
            ++arg->value.uint8;
            return true;
        }
        break;
    case TWEAK_VARIANT_TYPE_UINT16:
        if (arg->value.uint16 < UINT16_MAX) {
            ++arg->value.uint16;
            return true;
        }
        break;
    case TWEAK_VARIANT_TYPE_UINT32:
        if (arg->value.uint32 < UINT32_MAX) {
            ++arg->value.uint32;
            return true;
        }
        break;
    case TWEAK_VARIANT_TYPE_UINT64:
        if (arg->value.uint64 < UINT64_MAX) {
            ++arg->value.uint64;
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

tweak_metadata_layout tweak_metadata_get_layout(tweak_metadata metadata) {
    return metadata->layout_present
        ? &metadata->layout
        : NULL;
}

size_t tweak_metadata_layout_get_number_of_dimensions(tweak_metadata_layout layout) {
    return utarray_len(layout->dimensions);
}

size_t tweak_metadata_layout_get_dimension(tweak_metadata_layout layout, size_t dimension) {
    size_t *dimension_ptr = (size_t*)utarray_eltptr(layout->dimensions, dimension);
    return dimension_ptr
        ? *dimension_ptr
        : 0;
}

tweak_metadata_layout_order tweak_metadata_layout_get_order(tweak_metadata_layout layout) {
    return layout->order;
}

static bool parse_layout_object(struct tweak_metadata_layout_base *user_layout,
    const struct tweak_json_node *layout_node)
{
    const struct tweak_json_node *order_node;
    const struct tweak_json_node *dimensions_node;
    tweak_metadata_layout_order order = TWEAK_METADATA_LAYOUT_ORDER_ROW_MAJOR;

    UT_array *dimensions = NULL;
    size_t dim_count = 1;

    dimensions_node = tweak_json_get_object_field(layout_node, "dimensions", TWEAK_JSON_NODE_TYPE_ARRAY);
    if (dimensions_node == NULL)
        goto exit;

    if (tweak_json_get_array_size(dimensions_node, &dim_count) != TWEAK_JSON_GET_SIZE_SUCCESS)
        goto exit;

    utarray_new(dimensions, &ut_size_t_icd);
    for (size_t ix = 0; ix < dim_count; ix++) {
        const struct tweak_json_node *item =
            tweak_json_get_array_item(dimensions_node, ix, TWEAK_JSON_NODE_TYPE_NUMBER);
        char *endp = NULL;
        size_t dimension = strtoul(tweak_json_node_as_c_str(item), &endp, 10);
        if (*endp != '\0')
            goto exit;

        utarray_push_back(dimensions, &dimension);
    }

    order_node = tweak_json_get_object_field(layout_node, "order", TWEAK_JSON_NODE_TYPE_STRING);
    if (order_node != NULL) {
        if (strcmp(tweak_json_node_as_c_str(order_node), "row-major") == 0) {
            order = TWEAK_METADATA_LAYOUT_ORDER_ROW_MAJOR;
        } else if (strcmp(tweak_json_node_as_c_str(order_node), "column-major") == 0) {
            order = TWEAK_METADATA_LAYOUT_ORDER_COLUMN_MAJOR;
        } else {
            goto exit;
        }
    }

    user_layout->dimensions = dimensions;
    user_layout->order = order;
    return true;

exit:
    if (dimensions != NULL) {
        utarray_free(dimensions);
    }
    return false;
}
