/**
 * @file tweak2.h
 * @ingroup tweak-api
 *
 * @brief tweak2 public API.
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
 * @defgroup tweak-api Tweak API
 * Part of library API. Can be used by user to develop applications
 */

#ifndef TWEAK2_H_INCLUDED
#define TWEAK2_H_INCLUDED

#include <tweak2/types.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Tweak Library.
 *
 * The simplest example is:
 * @code
 * int main(int argc, char *argv[])
 * {
 *   // Init tweak from application main() (no threads)
 *   tweak_initialize_library("nng", "role=server", "tcp://0.0.0.0:7777/"); // No error code is returned,
 *                                                                          // analyze tweak log for problems.
 *
 *   // Other code that uses tweak library. This code can be executed from multiple threads concurrently.
 *
 *   // Just before application terminates, cleanup internal resources used by the tweak library
 *   //  no need to call tweak_remove() before.
 *   tweak_finalize_library();
 *
 *   // All tweak ids became invalid and no subsequent calls to any other tweak_* functions are permitted.
 * }
 * @endcode
 *
 * @notes The function can cause application crash in case invalid arguments are supplied or memory allocation failed.
 *
 * @param context_type Network backed type. Valid options are: 'nng' and 'rpmsg'.
 * @param params Various options 'role=server' for example.
 * @param uri Connection URI for the given network backend. tcp://0.0.0.0:7777/ for
 * tcp protocol, IP=0.0.0.0 and port=7777.
 */
void tweak_initialize_library(const char *context_type, const char *params, const char *uri);

/**
 * @brief Listener for item change events.
 *
 * These callbacks are fired within context of an RPC call coming from tweak client to a tweak server.
 * Since this system uses singular point to point connection, it is capable to serve only one incoming
 * RPC request at a time. Thus, next change item event won't be handled unless this callback finishes its work.
 *
 * Because of that, duration of this callback times should be considerably shorter than average time gap
 * between two inbound item change events. Because of that, it's unadvised to do any IO operation
 * within this callback. If a user fails to follow this guideline, then the behaviour is backend specific.
 * Some implementations might drop some inbound packets, thus causing loss of synchronization
 * between client and server. Some implementations could have their IO buffers being clogged
 * with outdated packets. Either way, behaviour is undefined.
 *
 * User could do tweak_get*() call here.
 *
 * User could do tweak_set*() call here if it doesn't cause infinite exchange of packets
 * where each side enforces contradicting value of the same item.
 *
 * @param tweak_id id of an entry recently updated.
 * @param cookie user parameter passed as parameter to @see tweak_set_item_change_listener.
 */
typedef void (*tweak_item_change_listener)(tweak_id tweak_id, void* cookie);

/**
 * @param item_change_listener id of an entry recently updated.
 *
 * @param cookie user parameter passed as parameter to @see tweak_set_item_change_listener.
 */
void tweak_set_item_change_listener(tweak_item_change_listener item_change_listener, void* cookie);

/**
 * @brief Find id of a tweak given its @p uri.
 *
 * @return tweak_id of an existing item if there's one or @p TWEAK_INVALID_ID otherwise.
 */
tweak_id tweak_find_id(const char* uri);

/**
 * @brief Descriptor encapsulating most parameters for item creation except initial value.
 */
struct tweak_add_item_ex_desc {
  /**
   * @brief uri parameter.
   */
  const char* uri;
  /**
   * @brief description parameter.
   */
  const char* description;
  /**
   * @brief meta parameter.
   */
  const char* meta;
  /**
   * @brief procedure to invoke upon item change initiated by remote side.
   */
  tweak_item_change_listener item_change_listener;
  /**
   * @brief user parameter to pass into item_change_listener.
   */
  void* cookie;
};

/**
 * @{
 * @brief Add a new item with boolean type.
 *
 * @details It allocates memory for a newly created item,
 * updates underlying indices and makes this item visible
 * for client when it lists items upon connect / reconnect
 * or receives notification upon new item being created.
 *
 * @param uri Unique uri of an item within tweak collection.
 *
 * @param description textual description of an item.
 *
 * @param meta freeform representation hint that should be understood
 * by the gui.
 *
 * @param initial_value item's value for current_value and
 * default_value fields.
 *
 * @return id of a newly created item or TWEAK_INVALID_ID if there was an error.
 *
 * @see tweak_app_server_add_item for detailed descriptions of @p uri,
 * @p description, @p meta, and @p initial_value.
 */
tweak_id tweak_add_scalar_bool(const char* uri, const char* description, const char* meta, bool initial_value);

/**
 * @brief Add a new item with int8_t type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_int8(const char* uri, const char* description, const char* meta, int8_t initial_value);

/**
 * @brief Add a new item with int16_t type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_int16(const char* uri, const char* description, const char* meta, int16_t initial_value);

/**
 * @brief Add a new item with int32_t type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_int32(const char* uri, const char* description, const char* meta, int32_t initial_value);

/**
 * @brief Add a new item with int64_t type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_int64(const char* uri, const char* description, const char* meta, int64_t initial_value);

/**
 * @brief Add a new item with uint32_t type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_uint32(const char* uri, const char* description, const char* meta, uint32_t initial_value);

/**
 * @brief Add a new item with uint8_t type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_uint8(const char* uri, const char* description, const char* meta, uint8_t initial_value);

/**
 * @brief Add a new item with uint16_t type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_uint16(const char* uri, const char* description, const char* meta, uint16_t initial_value);

/**
 * @brief Add a new item with uint64_t type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_uint64(const char* uri, const char* description, const char* meta, uint64_t initial_value);

/**
 * @brief Add a new item with float type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_float(const char* uri, const char* description, const char* meta, float initial_value);

/**
 * @brief Add a new item with double type.
 *
 * @copydetails tweak_add_scalar_bool(const char*, const char*, const char*, bool)
 */
tweak_id tweak_add_scalar_double(const char* uri, const char* description, const char* meta, double initial_value);
/**
 * @}
 */

/**
 * @{
 * @brief Extended version of @see tweak_add_scalar_bool.
 *
 * @note Use this function to specify individual callback to track item's current_value.
 *
 * @param desc structure containing all parameters needed to create an item.
 * @see tweak_add_item_ex_desc.
 *
 * @param initial_value item's value for current_value and
 * default_value fields. Has specific type and thus cannot be included to @p desc.
 *
 * @return id of a newly created item or TWEAK_INVALID_ID if there was an error.
 */
tweak_id tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc* desc, bool initial_value);

/**
 * @brief Extended version of tweak_add_scalar_int8(const char*, const char*, const char*, int8_t)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_int8_ex(const struct tweak_add_item_ex_desc* desc, int8_t initial_value);

/**
 * @brief Extended version of tweak_add_scalar_int16(const char*, const char*, const char*, int16_t)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_int16_ex(const struct tweak_add_item_ex_desc* desc, int16_t initial_value);

/**
 * @brief Extended version of tweak_add_scalar_int32(const char*, const char*, const char*, int32_t)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_int32_ex(const struct tweak_add_item_ex_desc* desc, int32_t initial_value);

/**
 * @brief Extended version of @see tweak_add_scalar_int64.
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_int64_ex(const struct tweak_add_item_ex_desc* desc, int64_t initial_value);

/**
 * @brief Extended version of tweak_add_scalar_uint8(const char*, const char*, const char*, uint8_t)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_uint8_ex(const struct tweak_add_item_ex_desc* desc, uint8_t initial_value);

/**
 * @brief Extended version of tweak_add_scalar_uint16(const char*, const char*, const char*, uint16_t)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_uint16_ex(const struct tweak_add_item_ex_desc* desc, uint16_t initial_value);

/**
 * @brief Extended version of tweak_add_scalar_uint32(const char*, const char*, const char*, uint32_t)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_uint32_ex(const struct tweak_add_item_ex_desc* desc, uint32_t initial_value);

/**
 * @brief Extended version of tweak_add_scalar_uint64(const char*, const char*, const char*, uint64_t)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_uint64_ex(const struct tweak_add_item_ex_desc* desc, uint64_t initial_value);

/**
 * @brief Extended version of tweak_add_scalar_float(const char*, const char*, const char*, float)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_float_ex(const struct tweak_add_item_ex_desc* desc, float initial_value);

/**
 * @brief Extended version of tweak_add_scalar_double(const char*, const char*, const char*, double)
 *
 * @copydetails tweak_add_scalar_bool_ex(const struct tweak_add_item_ex_desc*, bool)
 */
tweak_id tweak_add_scalar_double_ex(const struct tweak_add_item_ex_desc* desc, double initial_value);

/**
 * @}
 */

/**
 * @{
 * @brief Alters value of an existing item given its tweak_id and a new value.
 *
 * @details This function perform a lookup to find an item instance given its tweak_id,
 * changes its current_value field and makes this change visible to connected client
 * when it lists items' values upon connect / reconnect or receives notification upon
 * item being altered.
 *
 * This method might fail for a number of reasons, however there's
 * no error code to track its successful completion.
 * Configure logger interface to get more detailed information on error
 * if there was one. This particular API designed like this intentionally
 * for simplicity when no error handling is needed.
 *
 * @param id the id of an item being altered.
 *
 * @param value new value.
 */
void tweak_set_scalar_bool(tweak_id id, bool value);

/**
 * @brief Alters value of an existing item having int8_t type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_int8(tweak_id id, int8_t value);

/**
 * @brief Alters value of an existing item having int16_t type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_int16(tweak_id id, int16_t value);

/**
 * @brief Alters value of an existing item having int32_t type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_int32(tweak_id id, int32_t value);

/**
 * @brief Alters value of an existing item having int64_t type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_int64(tweak_id id, int64_t value);

/**
 * @brief Fetch current_value of item having uint8_t type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
uint8_t tweak_get_scalar_uint8(tweak_id id);

/**
 * @brief Alters value of an existing item having uint8_t type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_uint8(tweak_id id, uint8_t value);

/**
 * @brief Alters value of an existing item having uint16_t type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_uint16(tweak_id id, uint16_t value);

/**
 * @brief Alters value of an existing item having uint32_t type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_uint32(tweak_id id, uint32_t value);

/**
 * @brief Alters value of an existing item having uint64_t type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_uint64(tweak_id id, uint64_t value);

/**
 * @brief Alters value of an existing item having float type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_float(tweak_id id, float value);

/**
 * @brief Alters value of an existing item having double type given its tweak_id and new value.
 *
 * @copydetails tweak_set_scalar_bool(tweak_id, value)
 */
void tweak_set_scalar_double(tweak_id id, double value);

/**
 * @}
 */

/**
 * @{
 * @brief Fetch current_value of an item given its id.
 *
 * @details In the desired scenario, this function perform a lookup
 * to find an item instance given its tweak_id, and then copies its
 * current_value. This function can be called concurrently
 * with other tweak_get_*. This function shall
 * wait for concurrent tweak_add_* and tweak_remove calls to complete.
 * Since tweak_add_* and tweak_remove calls could block for hundreds of
 * milliseconds or longer, this call could block as well.
 * Reference documentation on these methods on how to avoid this.
 *
 * This method might fail for a number of reasons, however there's
 * no error code to track its successful completion.
 * Configure logger interface to get more detailed information on error
 * if there was one. This particular API designed like this intentionally
 * for simplicity when no error handling is needed.
 *
 * @param id the id of an item.
 *
 * @return item's value.
 */
bool tweak_get_scalar_bool(tweak_id id);

/**
 * @brief Fetch current_value of item having int8_t type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
int8_t tweak_get_scalar_int8(tweak_id id);

/**
 * @brief Fetch current_value of item having int16_t type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
int16_t tweak_get_scalar_int16(tweak_id id);

/**
 * @brief Fetch current_value of item having int32_t type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
int32_t tweak_get_scalar_int32(tweak_id id);


/**
 * @brief Fetch current_value of item having int64_t type given its id.
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
int64_t tweak_get_scalar_int64(tweak_id id);

/**
 * @brief Fetch current_value of item having uint16_t type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
uint16_t tweak_get_scalar_uint16(tweak_id id);

/**
 * @brief Fetch current_value of item having uint32_t type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
uint32_t tweak_get_scalar_uint32(tweak_id id);

/**
 * @brief Fetch current_value of item having uint64_t type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
uint64_t tweak_get_scalar_uint64(tweak_id id);

/**
 * @brief Fetch current_value of item having float type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
float tweak_get_scalar_float(tweak_id id);

/**
 * @brief Fetch current_value of item having double type given its id.
 *
 * @copydetails tweak_get_scalar_bool(tweak_id)
 */
double tweak_get_scalar_double(tweak_id id);

/**
 * @}
 */

/**
 * @{
 * @brief Create vector of int8_t items
 *
 * @param desc structure containing all parameters needed to create an item.
 * @see tweak_add_item_ex_desc.
 *
 * @param buffer initial value. Could be NULL, item shall be initialized
 * with zeros in this case.
 *
 * @param count number of items in vector.
 *
 * @return id of a newly created item or TWEAK_INVALID_ID if there was an error.
 */
tweak_id tweak_create_vector_sint8(const struct tweak_add_item_ex_desc* desc,
  const int8_t* initial_value, size_t count);

/**
 * @brief Create vector of int16_t items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_sint16(const struct tweak_add_item_ex_desc* desc,
  const int16_t* initial_value, size_t count);

/**
 * @brief Create vector of int32_t items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_sint32(const struct tweak_add_item_ex_desc* desc,
  const int32_t* initial_value, size_t count);

/**
 * @brief Create vector of int64_t items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_sint64(const struct tweak_add_item_ex_desc* desc,
  const int64_t* initial_value, size_t count);

/**
 * @brief Create vector of uint8_t items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_uint8(const struct tweak_add_item_ex_desc* desc,
  const uint8_t* initial_value, size_t count);

/**
 * @brief Create vector of uint16_t items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_uint16(const struct tweak_add_item_ex_desc* desc,
  const uint16_t* initial_value, size_t count);

/**
 * @brief Create vector of uint32_t items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_uint32(const struct tweak_add_item_ex_desc* desc,
  const uint32_t* initial_value, size_t count);

/**
 * @brief Create vector of uint64_t items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_uint64(const struct tweak_add_item_ex_desc* desc,
  const uint64_t* initial_value, size_t count);

/**
 * @brief Create vector of float items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_float(const struct tweak_add_item_ex_desc* desc,
  const float* initial_value, size_t count);

/**
 * @brief Create vector of double items
 * @copydetails tweak_create_vector_sint8(const struct tweak_add_item_ex_desc*, const int8_t*, size_t)
 */
tweak_id tweak_create_vector_double(const struct tweak_add_item_ex_desc* desc,
  const double* initial_value, size_t count);
/**@}*/

/**@{*/
/**
 * @brief Updates a vector with int8_t items.
 *
 * @param id value returned by respective tweak_create_vector_* function.
 *
 * @param buffer new data to store in given vector of the same size that
 * was passed to create.
 */
void tweak_set_vector_sint8(tweak_id id, const int8_t* buffer);

/**
 * @brief Updates a vector with int16_t items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_sint16(tweak_id id, const int16_t* buffer);

/**
 * @brief Updates a vector with int32_t items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_sint32(tweak_id id, const int32_t* buffer);

/**
 * @brief Updates a vector with int64_t items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_sint64(tweak_id id, const int64_t* buffer);

/**
 * @brief Updates a vector with uint8_t items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_uint8(tweak_id id, const uint8_t* buffer);

/**
 * @brief Updates a vector with uint16_t items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_uint16(tweak_id id, const uint16_t* buffer);

/**
 * @brief Updates a vector with uint32_t items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_uint32(tweak_id id, const uint32_t* buffer);

/**
 * @brief Updates a vector with uint64_t items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_uint64(tweak_id id, const uint64_t* buffer);

/**
 * @brief Updates a vector with float items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_float(tweak_id id, const float* buffer);

/**
 * @brief Updates a vector with double items.
 * @copydetails tweak_set_vector_sint8(tweak_id, const int8_t*)
 */
void tweak_set_vector_double(tweak_id id, const double* buffer);
/**@}*/

/**@{*/
/**
 * @brief Extracts data from a vector of int8_t elements
 * to an external array.
 *
 * @param id value returned by respective tweak_create_vector_* function.
 *
 * @param buffer buffer to place data of the same size that
 * was passed to create.
 */
void tweak_get_vector_sint8(tweak_id id, int8_t* buffer);

/**
 * @brief Extracts data from a vector of int16_t elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_sint16(tweak_id id, int16_t* buffer);

/**
 * @brief Extracts data from a vector of int32_t elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_sint32(tweak_id id, int32_t* buffer);

/**
 * @brief Extracts data from a vector of int64_t elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_sint64(tweak_id id, int64_t* buffer);

/**
 * @brief Extracts data from a vector of uint8_t elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_uint8(tweak_id id, uint8_t* buffer);

/**
 * @brief Extracts data from a vector of uint16_t elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_uint16(tweak_id id, uint16_t* buffer);

/**
 * @brief Extracts data from a vector of uint32_t elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_uint32(tweak_id id, uint32_t* buffer);

/**
 * @brief Extracts data from a vector of uint64_t elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_uint64(tweak_id id, uint64_t* buffer);

/**
 * @brief Extracts data from a vector of float elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_float(tweak_id id, float* buffer);

/**
 * @brief Extracts data from a vector of double elements
 * to an external array.
 *
 * @copydetails tweak_get_vector_sint8(tweak_id, int8_t*)
 */
void tweak_get_vector_double(tweak_id id, double* buffer);
/**@}*/

/**
 * @brief Returns number of elements in a vector passed
 * to respective tweak_create_vector_ function.
 *
 * @param id the id of an item.
 * @return size of a vector.
 */
size_t tweak_get_vector_item_count(tweak_id id);

/**
 * @brief Creates a string item. Strings have open length and are delimited by '\0'
 * terminator.
 *
 * @param desc structure containing all parameters needed to create an item.
 * @see tweak_add_item_ex_desc.
 *
 * @param initial_value string's initial value.
 *
 * @return id of a newly created item or TWEAK_INVALID_ID if there was an error.
 */
tweak_id tweak_create_string(const struct tweak_add_item_ex_desc* desc, const char* initial_value);

/**
 * @brief Updates value of a string item. Strings have open length and
 * are delimited by '\0' terminator.
 *
 * @param string new value.
 */
void tweak_set_string(tweak_id id, const char* string);

/**
 * @brief Retrieves value of a string item.
 * Strings have open length and are delimited by '\0' terminator.
 *
 * @param buffer an output buffer.
 *
 * @param size an output buffer's size. It the string doesn's fit,
 * it is getting truncated by "...\0" sequence.
 *
 * @return actual length of string, including '\0' terminator.
 * If it's greater than @p size, then string was truncated.
 */
size_t tweak_get_string(tweak_id id, char* buffer, size_t size);

/**
 * @brief remove an item from internal collection given its @p id.
 *
 * @details This method won't block unless there's an IO request overflow.
 * The client, if it's present, shall be notified about item removal
 * asynchronously.
 *
 * @note if there are active client's subscriptions, item shall
 * disappear after sending remove_item request to client.
 * Otherwise, item shall disappear immediately.
 *
 * @param id id of an item being removed.
 */
void tweak_remove(tweak_id id);

/**
 * @brief free library and deallocate all resources.
 */
void tweak_finalize_library();

/**
 * @cond PRIVATE
 * @brief Opaque type needed to access underlying @see tweak_app_server_context
 * in lower level interfaces.
 */
struct tweak_app_context_base;

typedef struct tweak_app_context_base *tweak_app_context_handle_t;

/**
 * @brief Access tweak_app_context_handle_t instance shared by all
 * components withing single application. It could be used as
 * application context parameter in tweak_app library.
 *
 * @return Unwrapped @see tweak_app_server_context instance.
 */
tweak_app_context_handle_t tweak_get_default_server_instance();

/**
 * @brief libraries using @see tweak_get_default_server_instance _must_ provide this
 * structure as item_cookie parameter to @see tweak_app_server_add_item call.
 * This library assumes ownership of this instance, so user isn't supposed to deallocate
 * instances of these structures.
 */
struct tweak_default_client_item_changed_listener;

typedef struct tweak_default_client_item_changed_listener *tweak_default_client_item_changed_listener_t;

/**
 * @brief Create instance of default @see tweak_default_client_item_changed_listener_t.
 *
 * @param client_callback procedure to invoke upon item change initiated by remote side.
 * @param cookie custom parameter to pass to client callback.
 *
 * @return new listener to pass as item_cookie to tweak_app_server_add_item call when
 * server instance is default or NULL if internal malloc() returned NULL.
 * Server assumes ownership of this instance, no deallocation is needed.
 */
tweak_default_client_item_changed_listener_t tweak_create_default_client_item_changed_listener(
  tweak_item_change_listener client_callback,
  void* client_cookie);

#ifdef __cplusplus
}
#endif

#endif /* TWEAK2_H_INCLUDED */
