/**
 * @file tweakjson.c
 * @ingroup tweak-internal
 *
 * @brief Simple DOM tree builder for JSON files.
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

/**
 * @ingroup tweak-internal Tweak Internals
 * Internal details of Tweak Tool implementation. May be useful for debugging,
 * extending and hacking.
 */

#include "json_suckless.h"
#include <tweak2/json.h>

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <utarray.h>
#include <uthash.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

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
    char* data;
};

struct json_parse_context {
  struct tweak_json_node* document;
  bool was_error;
};

static void destroy_object(struct tweak_json_node* node);

static void destroy_array(struct tweak_json_node* node);

static void destroy_simple(struct tweak_json_node* node);

static UT_icd json_node_icd;

static void add_element_to_array(struct tweak_json_node_array* parent,
    struct tweak_json_node* child)
{
    UT_array *items = parent->items;
    utarray_push_back(items, &child);
    parent->items = items;
}

static bool add_element_to_object(struct tweak_json_node_object* parent,
    const char* name, struct tweak_json_node* child)
{
    assert(name);
    struct object_kv* pair = NULL;

    HASH_FIND_STR(parent->fields, name, pair);
    char* name0;
    if (pair == NULL) {
        name0 = strdup(name);
    } else {
        char invalid_name[64] = { 0 };
        snprintf(invalid_name, sizeof(invalid_name), "duplicate_name_%p", (void*)child);
        name0 = strdup(invalid_name);
    }

    if (name0 == NULL)
        goto strdup_error;

    pair = malloc(sizeof(*pair));
    if (pair == NULL)
        goto cant_create_pair;

    pair->value = child;
    pair->name = name0;
    HASH_ADD_KEYPTR(hh, parent->fields, pair->name, strlen(pair->name), pair);
    return true;

cant_create_pair:
    free(name0);

strdup_error:
    return false;
}

static struct tweak_json_node* create_new_simple_node(tweak_json_node_type json_type,
    const char* string);

static struct tweak_json_node* create_new_node(enum JSONType json_type,
    const char* string)
{
    struct tweak_json_node *result = NULL;
    switch (json_type)
    {
    case JSON_TYPE_NULL:
        result = calloc(1, sizeof(struct tweak_json_node));
        if (result) {
            result->json_node_type = TWEAK_JSON_NODE_TYPE_NULL;
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
        }
        break;
    case JSON_TYPE_OBJECT:
        result = calloc(1, sizeof(struct tweak_json_node_object));
        if (result) {
            result->json_node_type = TWEAK_JSON_NODE_TYPE_OBJECT;
            result->node_destroy = &destroy_object;
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
    }
    return result;
}

static struct tweak_json_node* create_new_simple_node(tweak_json_node_type json_type,
    const char* string)
{
    struct tweak_json_node *result = calloc(1, sizeof(struct tweak_json_node_simple));
    char *tmp = NULL;

    if (result == NULL)
        goto calloc_error;
    result->json_node_type = json_type;
    result->node_destroy = &destroy_simple;
    tmp = strdup(string);
    if (!tmp)
        goto strdup_error;

    ((struct tweak_json_node_simple*)result)->data = tmp;
    return result;

strdup_error:
    free(result);

calloc_error:
    return NULL;
}

static void enter_callback(struct json_node* suckless_json_node_stack,
    size_t stack_size, const char *str, void* cookie)
{
    assert(suckless_json_node_stack);
    assert(str);
    assert(stack_size >= 1);

    enum JSONType json_type = suckless_json_node_stack[stack_size - 1].type;
    struct json_parse_context* context = (struct json_parse_context*)cookie;
    assert(context);
    if (context->was_error)
        return;

    struct tweak_json_node* child = create_new_node(json_type, str);
    if (!child) {
        context->was_error = true;
        return;
    }

    struct tweak_json_node *parent = stack_size > 1
        ? (struct tweak_json_node *)suckless_json_node_stack[stack_size - 2].cookie
        : NULL;

    switch (tweak_json_get_type(parent)) {
    case TWEAK_JSON_NODE_TYPE_ARRAY:
        add_element_to_array((struct tweak_json_node_array *)parent, child);
        break;
    case TWEAK_JSON_NODE_TYPE_OBJECT:
        context->was_error |= !add_element_to_object((struct tweak_json_node_object *)parent,
            suckless_json_node_stack[stack_size - 1].name, child);
        break;
    default:
        if (parent == NULL) {
            context->document = child;
        } else {
            tweak_json_destroy(child);
            child = NULL;
        }
        break;
    }
    suckless_json_node_stack[stack_size - 1].cookie = child;
}

struct tweak_json_node* tweak_json_parse(const char* json_snippet) {
    if (!json_snippet)
        return NULL;

    struct json_parse_context parse_context = {
        .document = NULL,
        .was_error = false
    };

    int parse_status = parsejson(json_snippet, &enter_callback, &parse_context);
    if (parse_status == 0 && !parse_context.was_error) {
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
    const struct tweak_json_node_array* array_node;
    const struct tweak_json_node* item_node;
    void** pp;

    if (tweak_json_get_type(node) != TWEAK_JSON_NODE_TYPE_ARRAY)
        goto error;

    array_node = (const struct tweak_json_node_array*)node;
    pp = (void**)utarray_eltptr(array_node->items, index);
    if (!pp)
        goto error;

    item_node = *(const struct tweak_json_node**)pp;
    return (tweak_json_get_type(item_node) & expected_type_mask) != 0
        ? item_node
        : NULL;

error:
    return NULL;
}

const struct tweak_json_node* tweak_json_get_object_field(const struct tweak_json_node* node,
    const char* field, tweak_json_node_type expected_type_mask)
{
    const struct object_kv* pair = NULL;
    const struct tweak_json_node_object* object_node;
    const struct tweak_json_node* field_node;

    if (tweak_json_get_type(node) != TWEAK_JSON_NODE_TYPE_OBJECT)
        goto error;

    object_node = (struct tweak_json_node_object*)node;
    HASH_FIND_STR(object_node->fields, field, pair);
    if (pair == NULL)
        goto error;

    field_node = pair->value;
    return (tweak_json_get_type(field_node) & expected_type_mask) != 0
        ? field_node
        : NULL;

error:
    return NULL;
}

const char* tweak_json_node_as_c_str(const struct tweak_json_node* node) {
    return (tweak_json_get_type(node) & TWEAK_JSON_NODE_TYPE_VALUE) != 0
        ? ((const struct tweak_json_node_simple*)node)->data
        : NULL;
}

void tweak_json_destroy(struct tweak_json_node* node) {
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
    struct tweak_json_node_array* array_node = (struct tweak_json_node_array*)node;
    utarray_free(array_node->items);
}

static void destroy_simple(struct tweak_json_node* node) {
    assert(node != NULL); /* Invoked as destructor for existing node only */
    free(((struct tweak_json_node_simple*)node)->data);
}

static void json_node_icd_destroy(void *elt) {
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
