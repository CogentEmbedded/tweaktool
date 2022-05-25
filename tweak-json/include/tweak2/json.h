/**
 * @file json.h
 * @ingroup tweak-internal
 *
 * @brief Simple json parser.
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

#ifndef TWEAK_JSON_H_INCLUDED
#define TWEAK_JSON_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief JSON node types.
 */
typedef enum {
    /**
     * @brief Invalid or uninitialized node, used as error indicator.
     */
    TWEAK_JSON_NODE_TYPE_INVALID = 0,
    /**
     * @brief null node.
     */
    TWEAK_JSON_NODE_TYPE_NULL = 1,
    /**
     * @brief array node.
     */
    TWEAK_JSON_NODE_TYPE_ARRAY = 1<<1,
    /**
     * @brief object node.
     */
    TWEAK_JSON_NODE_TYPE_OBJECT = 1<<2,
    /**
     * @brief string node.
     */
    TWEAK_JSON_NODE_TYPE_STRING = 1<<3,
    /**
     * @brief bool node.
     */
    TWEAK_JSON_NODE_TYPE_BOOL = 1<<4,
    /**
     * @brief numeric node.
     */
    TWEAK_JSON_NODE_TYPE_NUMBER = 1<<5,
    /**
     * @brief Shall match any compound node type
     * such as object or array.
     * @see tweak_json_get_object_field
     * and @see tweak_json_get_array_item.
     */
    TWEAK_JSON_NODE_TYPE_COMPOUND = TWEAK_JSON_NODE_TYPE_ARRAY
        | TWEAK_JSON_NODE_TYPE_OBJECT,
    /**
     * @brief Shall match any value node type
     * such as string, numeric or bool.
     * @see tweak_json_get_object_field
     * and @see tweak_json_get_array_item.
     */
    TWEAK_JSON_NODE_TYPE_VALUE = TWEAK_JSON_NODE_TYPE_STRING
        | TWEAK_JSON_NODE_TYPE_BOOL
        | TWEAK_JSON_NODE_TYPE_NUMBER,
    /**
     * @brief Shall match any node type in
     *  @see tweak_json_get_object_field
     * and @see tweak_json_get_array_item.
     */
    TWEAK_JSON_NODE_TYPE_ANY = TWEAK_JSON_NODE_TYPE_NULL
        | TWEAK_JSON_NODE_TYPE_ARRAY
        | TWEAK_JSON_NODE_TYPE_OBJECT
        | TWEAK_JSON_NODE_TYPE_STRING
        | TWEAK_JSON_NODE_TYPE_BOOL
        | TWEAK_JSON_NODE_TYPE_NUMBER
} tweak_json_node_type;

/**
 * @brief Base for all node types.
 */
struct tweak_json_node;

/**
 * @brief Build document tree given json snippet.
 *
 * @param json_snippet string to parse.
 * @return root node of document parsed, or NULL in case of error.
 */
struct tweak_json_node* tweak_json_parse(const char* json_snippet);

/**
 * @brief Get type of a node.
 *
 * @param node Node to probe.
 *
 * @return Type of given node. If @p node is NULL, returns @see TWEAK_JSON_NODE_TYPE_INVALID.
 */
tweak_json_node_type tweak_json_get_type(const struct tweak_json_node* node);

/**
 * @brief Possible return values of @see tweak_json_get_array_size.
 */
typedef enum {
    /**
     * @brief Success.
     */
    TWEAK_JSON_GET_SIZE_SUCCESS = 0,
    /**
     * @brief Invalid arguments.
     */
    TWEAK_JSON_GET_SIZE_INVALID_ARG
} tweak_json_get_array_size_status;

/**
 * @brief Get number of items in an array node.
 *
 * @param array_node node to analyze.
 * @param size out parameter receiving size.
 *
 * @return TWEAK_JSON_GET_SIZE_SUCCESS on success, or @see TWEAK_JSON_GET_SIZE_INVALID_ARG if @p array_node is NULL,
 * or if @p array_node has type other than @see TWEAK_JSON_NODE_TYPE_ARRAY, or if @p psize is NULL.
 */
tweak_json_get_array_size_status tweak_json_get_array_size(const struct tweak_json_node* node, size_t *size);

/**
 * @brief Get an item in array node given item index.
 *
 * @param array_node node to access.
 * @param index index of an item.
 * @param expected_types_mask Accepted type mask such as TWEAK_JSON_NODE_TYPE_BOOL | TWEAK_JSON_NODE_TYPE_NUMBER
 * or TWEAK_JSON_NODE_TYPE_ANY if user wants to analyze the item in some custom way.
 *
 * @return item at given index, unless:
 * - @p array_node is NULL
 * - @p array_node has type other than @see TWEAK_JSON_NODE_TYPE_ARRAY
 * - @p index is out of bounds
 * - item at given @p index has type that doesn't match @p expected_types_mask
 * In all these cases this method returns NULL.
 *
 * @note Node retrieved by this method is bound to root document node produced by @see tweak_json_parse.
 * Therefore, user mustn't pass it to @see tweak_json_destroy or @see free methods.
 */
const struct tweak_json_node* tweak_json_get_array_item(const struct tweak_json_node* array_node,
    size_t index, tweak_json_node_type expected_type_mask);

/**
 * @brief Get an item in object node given field name.
 *
 * If node node exists and its type matches @p expected_types_mask, this method always returns non-NULL value.
 *
 * @param object_node node to access.
 * @param field field name.
 * @param expected_types_mask Accepted type mask such as TWEAK_JSON_NODE_TYPE_BOOL | TWEAK_JSON_NODE_TYPE_NUMBER
 * or TWEAK_JSON_NODE_TYPE_ANY if user wants to analyze the field in some custom way.
 *
 * @return field of an object, unless:
 * - @p object_node is NULL
 * - @p object_node has type other than TWEAK_JSON_NODE_TYPE_OBJECT
 * - Given @p field doesn't exist within @p object_node
 * - Given @p field has type that doesn't match @p expected_types_mask
 * In all these cases this method returns NULL.
 *
 * @note Node retrieved by this method is bound to root document node produced by @see tweak_json_parse.
 * Therefore, user mustn't pass it to @see tweak_json_destroy or @see free methods.
 */
const struct tweak_json_node* tweak_json_get_object_field(const struct tweak_json_node* object_node,
    const char* field, tweak_json_node_type expected_type_mask);

/**
 * @brief Access C-style string stored in given simple json value node.
 *
 * @note type of an simple value could be queried with @see tweak_json_get_type.
 * For instance, if type is equal to TWEAK_JSON_NODE_TYPE_BOOL, then string returned
 * by this method would be equal to "true" or "false".
 *
 * @param node node to access.
 *
 * @return String representing simple json value.
 * "true" / "false" for a boolean, "12.3" or "42" for numeric values, or string contents.
 * Parsing is up to user. Can return NULL if:
 *  - @p node is NULL
 *  - (tweak_json_get_type(node) & TWEAK_JSON_NODE_TYPE_VALUE) == TWEAK_JSON_NODE_TYPE_INVALID
 * Second case is a particular instance of the first one since
 * tweak_json_get_type(NULL) == TWEAK_JSON_NODE_TYPE_INVALID.
 *
 * @note Pointer to memory buffer retrieved by this method is bound to root document
 * node produced by @see tweak_json_parse. Therefore, user mustn't pass it to
 * @see tweak_json_destroy or @see free methods.
 */
const char* tweak_json_node_as_c_str(const struct tweak_json_node* node);

/**
 * @brief Release memory allocated by document.
 *
 * @param node root node returned by @see tweak_json_parse call.
 */
void tweak_json_destroy(struct tweak_json_node* node);


#ifdef __cplusplus
}
#endif

#endif /* TWEAK_JSON_H_INCLUDED */
