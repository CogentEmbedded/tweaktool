/**
 * @file tweakjson.c
 * @ingroup tweak-internal
 *
 * @brief Simple DOM tree builder for JSON files.
 *
 * @copyright 2018-2021 Cogent Embedded Inc. ALL RIGHTS RESERVED.
 *
 * This file is a part of Cogent Tweak Tool feature.
 *
 * It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution or by request via www.cogentembedded.com
 */

/**
 * @ingroup tweak-internal Tweak Internals
 * Internal details of Tweak Tool implementation. May be useful for debugging,
 * extending and hacking.
 */

#include "json_suckless.h"

#include <tweak2/string.h>
#include <tweak2/json.h>
#include <tweak2/log.h>

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <utarray.h>
#include <uthash.h>
#include <inttypes.h>

typedef void (*tweak_json_node_destroy)(struct tweak_json_node* node);

struct tweak_json_node {
    tweak_json_node_type json_node_type;
    tweak_json_node_destroy node_destroy;
};

struct tweak_json_node_array {
    struct tweak_json_node parent;
    UT_array *items;
};

struct object_kv {
    char* name;
    struct tweak_json_node* value;
    UT_hash_handle hh;
};

struct tweak_json_node_object {
    struct tweak_json_node parent;
    struct object_kv* fields;
};

struct tweak_json_node_simple {
    struct tweak_json_node parent;
    tweak_variant_string data;
};

struct json_parse_context {
  struct tweak_json_node* document;
};

static void destroy_object(struct tweak_json_node* node);

static void destroy_array(struct tweak_json_node* node);

static void destroy_simple(struct tweak_json_node* node);

static UT_icd json_node_icd;

static const char* node_type_str(const struct tweak_json_node* node) {
    switch (tweak_json_get_type(node)) {
    case TWEAK_JSON_NODE_TYPE_NULL:
        return "TWEAK_JSON_NODE_TYPE_NULL";
    case TWEAK_JSON_NODE_TYPE_ARRAY:
        return "TWEAK_JSON_NODE_TYPE_ARRAY";
    case TWEAK_JSON_NODE_TYPE_OBJECT:
        return "TWEAK_JSON_NODE_TYPE_OBJECT";
    case TWEAK_JSON_NODE_TYPE_STRING:
        return "TWEAK_JSON_NODE_TYPE_STRING";
    case TWEAK_JSON_NODE_TYPE_BOOL:
        return "TWEAK_JSON_NODE_TYPE_BOOL";
    case TWEAK_JSON_NODE_TYPE_NUMBER:
        return "TWEAK_JSON_NODE_TYPE_NUMBER";
    default:
        return "<Invalid enum value>";
    }
}

static void add_element_to_array(struct tweak_json_node_array* parent,
    struct tweak_json_node* child)
{
    TWEAK_LOG_TRACE_ENTRY("Add %p of type %s to array node %p", child, node_type_str(child), parent);
    UT_array *items = parent->items;
    utarray_push_back(items, &child);
    parent->items = items;
}

static void add_element_to_object(struct tweak_json_node_object* parent,
    const char* name, struct tweak_json_node* child)
{
    TWEAK_LOG_TRACE_ENTRY("Add %p of type %s to object node %p with field name %s", child, node_type_str(child), parent, name);
    char* name0 = strdup(name);
    if (name0 == NULL) {
        TWEAK_FATAL("strdup() returned NULL");
    }
    struct object_kv* pair = NULL;
    HASH_FIND_STR(parent->fields, name, pair);
    if (pair != NULL) {
        char invalid_name[64] = { 0 };
        snprintf(invalid_name, sizeof(invalid_name), "duplicate_name_%p", (void*)child);
        free(name0);
        name0 = strdup(invalid_name);
        TWEAK_LOG_WARN("Duplicate field %s in array node %p, inserting as field %s", name, parent, invalid_name);
        if (name0 == NULL) {
            TWEAK_FATAL("strdup() returned NULL");
        }
    }
    pair = calloc(1, sizeof(*pair));
    if (pair == NULL) {
        free(name0);
        TWEAK_FATAL("calloc() returned NULL");
    }
    pair->value = child;
    size_t length = strlen(name);
    pair->name = name0;
    HASH_ADD_KEYPTR(hh, parent->fields, pair->name, length, pair);
}

static struct tweak_json_node* create_new_simple_node(tweak_json_node_type json_type,
    const char* string);

static struct tweak_json_node* create_new_node(enum JSONType json_type,
    const char* string)
{
    TWEAK_LOG_TRACE_ENTRY("node type = %c string = %s", json_type, string);
    struct tweak_json_node *result;
    switch (json_type)
    {
    case JSON_TYPE_NULL:
        result = calloc(1, sizeof(struct tweak_json_node));
        if (result) {
            result->json_node_type = TWEAK_JSON_NODE_TYPE_NULL;
        } else {
            TWEAK_FATAL("calloc() returned NULL");
        }
        break;
    case JSON_TYPE_ARRAY:
        result = calloc(1, sizeof(struct tweak_json_node_array));
        if (result) {
            result->json_node_type = TWEAK_JSON_NODE_TYPE_ARRAY;
            result->node_destroy = &destroy_array;
            UT_array *items = NULL;
            utarray_new(items, &json_node_icd);
            ((struct tweak_json_node_array*)result)->items = items;
        } else {
            TWEAK_FATAL("calloc() returned NULL");
        }
        break;
    case JSON_TYPE_OBJECT:
        result = calloc(1, sizeof(struct tweak_json_node_object));
        if (result) {
            result->json_node_type = TWEAK_JSON_NODE_TYPE_OBJECT;
            result->node_destroy = &destroy_object;
        } else {
            TWEAK_FATAL("calloc() returned NULL");
        }
        break;
    case JSON_TYPE_STRING:
        result = create_new_simple_node(TWEAK_JSON_NODE_TYPE_STRING, string);
        break;
    case JSON_TYPE_BOOL:
        result = create_new_simple_node(TWEAK_JSON_NODE_TYPE_BOOL, string);
        break;
    case JSON_TYPE_NUMBER:
        result = create_new_simple_node(TWEAK_JSON_NODE_TYPE_NUMBER, string);
        break;
    default:
        TWEAK_LOG_ERROR("Unknown type");
        result = NULL;
        break;
    }
    return result;
}

static struct tweak_json_node* create_new_simple_node(tweak_json_node_type json_type,
    const char* string)
{
    struct tweak_json_node *result = calloc(1, sizeof(struct tweak_json_node_simple));
    if (result != NULL) {
        result->json_node_type = json_type;
        result->node_destroy = &destroy_simple;
        tweak_variant_assign_string(&((struct tweak_json_node_simple*)result)->data, string);
    } else {
        TWEAK_FATAL("calloc() returned NULL");
    }
    return result;
}

static void enter_callback(struct json_node* suckless_json_node_stack,
    size_t stack_size, const char *str, void* cookie)
{
    assert(suckless_json_node_stack);
    assert(str);
    assert(stack_size >= 1);

    enum JSONType json_type = suckless_json_node_stack[stack_size - 1].type;
    TWEAK_LOG_TRACE_ENTRY("node %c stack size %zu str %s cookie %p",
        json_type, stack_size, str, cookie);
    struct json_parse_context* context = (struct json_parse_context*)cookie;
    assert(context);
    struct tweak_json_node* child = create_new_node(json_type, str);
    assert(child);
    struct tweak_json_node *parent = stack_size > 1
        ? (struct tweak_json_node *)suckless_json_node_stack[stack_size - 2].cookie
        : NULL;
    if (parent == NULL) {
        context->document = child;
    } else if ((tweak_json_get_type(parent) & TWEAK_JSON_NODE_TYPE_COMPOUND) != 0) {
        const char* field_name;
        switch (tweak_json_get_type(parent)) {
        case TWEAK_JSON_NODE_TYPE_ARRAY:
            add_element_to_array((struct tweak_json_node_array*)parent, child);
            break;
        case TWEAK_JSON_NODE_TYPE_OBJECT:
            field_name = suckless_json_node_stack[stack_size - 1].name;
            assert(field_name);
            add_element_to_object((struct tweak_json_node_object*)parent, field_name, child);
            break;
        default:
            TWEAK_FATAL("Unexpected");
            break;
        }
    } else {
        TWEAK_LOG_ERROR("Node being a child to a non-compound node");
        tweak_json_destroy(child);
        child = NULL;
    }
    suckless_json_node_stack[stack_size - 1].cookie = child;
}

struct tweak_json_node* tweak_json_parse(const char* json_snippet) {
    TWEAK_LOG_TRACE_ENTRY("json_snippet = <<<%s>>>", json_snippet);
    if (!json_snippet)
        return NULL;

    struct json_parse_context parse_context = { 0 };
    int parse_status = parsejson(json_snippet, &enter_callback, &parse_context);
    if (parse_status == 0) {
        return parse_context.document;
    } else {
        tweak_json_destroy(parse_context.document);
        return NULL;
    }
}

tweak_json_node_type tweak_json_get_type(const struct tweak_json_node* node) {
    return node ? node->json_node_type : TWEAK_JSON_NODE_TYPE_INVALID;
}

tweak_json_get_array_size_status tweak_json_get_array_size(const struct tweak_json_node* node,
    size_t *size)
{
    TWEAK_LOG_TRACE_ENTRY("node = %p, out param size = %uz", node, size);
    if (node == NULL || size == NULL) {
        return TWEAK_JSON_GET_SIZE_INVALID_ARG;
    }
    if (tweak_json_get_type(node) != TWEAK_JSON_NODE_TYPE_ARRAY) {
        return TWEAK_JSON_GET_SIZE_INVALID_ARG;
    }
    const struct tweak_json_node_array* array_node = (struct tweak_json_node_array*)node;
    *size = utarray_len(array_node->items);
    return TWEAK_JSON_GET_SIZE_SUCCESS;
}

const struct tweak_json_node* tweak_json_get_array_item(const struct tweak_json_node* node, size_t index,
    tweak_json_node_type expected_type_mask)
{
    TWEAK_LOG_TRACE_ENTRY("node = %p, index = %zu, expected_type_mask = %#x",
        node, index, expected_type_mask);
    if (tweak_json_get_type(node) == TWEAK_JSON_NODE_TYPE_ARRAY) {
        const struct tweak_json_node_array* array_node = (const struct tweak_json_node_array*)node;
        void** pp = (void**)utarray_eltptr(array_node->items, index);
        if (pp) {
            const struct tweak_json_node* item_node = *(const struct tweak_json_node**)pp;
            if ((tweak_json_get_type(item_node) & expected_type_mask) != 0) {
                return item_node;
            } else {
                TWEAK_LOG_WARN("Item at index %zu has unexpected type: %s, defaulting to NULL",
                    index, node_type_str(node));
                return NULL;
            }
        } else {
            size_t array_size = utarray_len(array_node->items);
            TWEAK_LOG_WARN("Array index %zu out of bounds, array size is %zu", index, array_size);
            return NULL;
        }
    } else {
        TWEAK_LOG_WARN("Getting array item by index %zu from node having type %s, returning NULL",
            index, node_type_str(node));
        return 0;
    }
}

const struct tweak_json_node* tweak_json_get_object_field(const struct tweak_json_node* node,
    const char* field, tweak_json_node_type expected_type_mask)
{
    TWEAK_LOG_TRACE_ENTRY("node = %p, field = %s, expected_type_mask = %#x", node,
        field, expected_type_mask);
    if (tweak_json_get_type(node) == TWEAK_JSON_NODE_TYPE_OBJECT) {
        const struct tweak_json_node_object* object_node = (struct tweak_json_node_object*)node;
        const struct object_kv* pair = NULL;
        HASH_FIND_STR(object_node->fields, field, pair);
        if (pair != NULL) {
            const struct tweak_json_node* field_node = pair->value;
            if ((tweak_json_get_type(field_node) & expected_type_mask) != 0) {
                return field_node;
            } else {
                TWEAK_LOG_WARN("Field \"%s\" has unexpected type: %s, defaulting to NULL",
                    field, node_type_str(node));
                return NULL;
            }
        } else {
            TWEAK_LOG_TRACE("Object lacks \"%s\" field, returning NULL", field);
            return NULL;
        }
    } else {
        TWEAK_LOG_WARN("Requesting object field from node having type %s, returning false",
            node_type_str(node));
        return false;
    }
}

const char* tweak_json_node_as_c_str(const struct tweak_json_node* node) {
    TWEAK_LOG_TRACE_ENTRY("node = %p", node);
    if ((tweak_json_get_type(node) & TWEAK_JSON_NODE_TYPE_VALUE) != 0)
    {
        return tweak_variant_string_c_str(&((const struct tweak_json_node_simple*)node)->data);
    } else {
        TWEAK_LOG_WARN("Requesting value from node having type %s, returning NULL",
            node_type_str(node));
        return NULL;
    }
}

void tweak_json_destroy(struct tweak_json_node* node) {
    TWEAK_LOG_TRACE_ENTRY("node = %p", node);
    if (!node)
        return;

    tweak_json_node_destroy destructor = node->node_destroy;
    if (destructor) {
        destructor(node);
    }
    free(node);
}

static void destroy_object(struct tweak_json_node* node) {
    assert(node != NULL); /* Invoked as destructor for existing node only */
    TWEAK_LOG_TRACE_ENTRY("node = %p", node);
    struct tweak_json_node_object* object = (struct tweak_json_node_object*)node;
    struct object_kv *pair = NULL;
    struct object_kv *tmp = NULL;
    HASH_ITER(hh, object->fields, pair, tmp) {
        HASH_DEL(object->fields, pair);
        tweak_json_destroy(pair->value);
        free(pair->name);
        free(pair);
    }
}

static void destroy_array(struct tweak_json_node* node) {
    assert(node != NULL); /* Invoked as destructor for existing node only */
    TWEAK_LOG_TRACE_ENTRY("node = %p", node);
    struct tweak_json_node_array* array_node = (struct tweak_json_node_array*)node;
    utarray_free(array_node->items);
}

static void destroy_simple(struct tweak_json_node* node) {
    assert(node != NULL); /* Invoked as destructor for existing node only */
    TWEAK_LOG_TRACE_ENTRY("node = %p", node);
    tweak_variant_destroy_string(&((struct tweak_json_node_simple*)node)->data);
}

static void json_node_icd_destroy(void *elt) {
    TWEAK_LOG_TRACE_ENTRY("pointer = %p", elt);
    if (!elt)
        return;

    struct tweak_json_node** node = (struct tweak_json_node**)elt;
    tweak_json_destroy(*node);
}

static UT_icd json_node_icd = {
    .sz = sizeof(struct tweak_json_node*),
    .init = NULL,
    .copy = NULL,
    .dtor = json_node_icd_destroy
};
