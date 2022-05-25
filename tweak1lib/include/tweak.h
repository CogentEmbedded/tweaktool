/**
 * @file tweak.h
 * @ingroup tweak-api
 *
 * @brief compatibility library adapting new implementation to legacy API.
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

#ifndef TWEAK_H_INCLUDED
#define TWEAK_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tweakconfig.h>

#include <stdint.h>

#define TWEAK_VERSION_MAJOR 2
#define TWEAK_VERSION_MINOR 0


/**
 * @note This is a compatibility library to adapt legacy Tweak API to Tweak 2 infrastructure.
 */

/**
 * @note In Tweak 2 GUI, items are arranged by hierarchy:
 *
 * /connection_name/level_1/level_2/item_name where connection_name
 * is being associated in GUI with connection whilst /level_1/level_2/item_name
 * are taken from uri field.
 *
 * However, tweak1 had concept of layout and GUI was stateful.
 * Because of that, it is decided to map "name" to URI using following algorithm:
 *
 * 1) If current layout name isn't prefixed by slash, this adaptor library does prefix it by slash.
 * Thus, tweak_add_layout(... "foo") is equivalent to tweak_add_layout(... "/foo")
 *
 * 2) If item isn't prefixed by slash, this adaptor library does prefix it by slash.
 * Thus, void tweak_add_slider("bar" ...) is equivalent to tweak_add_slider("/bar" ...)
 *
 * 3) Adaptor library concatenates latest layout name (prefixed) and item name (prefixed)
 * and uses it as uri on an item. Thus, item uri becomes /foo/bar. Having that connection name
 * in Tweak 2 GUI URI is equal to "conn1", the item shall be accessible as /conn1/foo/bar.
 *
 * All the info related to item editor is brought together into meta field of an item.
 * Tweak 2 GUI uses json representation described in docs/METADATA.md
 * For instance, tweak_add_slider creates entry { ... "control": "slider" ...} within meta.
 *
 * Unlike tweak 2, tweak 1 has layouts. This adaptor adds most recent layout info to item
 * meta as well. Because of that,
 *
 * ```
 * tweak_add_layout(300, 0, "/foo");
 * tweak_add_slider("bar", -1., 1., 0., 4);
 * ```
 * Produces item with uri /foo/bar having meta field such as this:
 *
 * ```
 * {
 *      "control": "slider",
 *      "min": -1,
 *      "max": 1,
 *      "readonly": false,
 *      "decimals": 4,
 *      "layout": {
 *          "layout_id": 1,
 *          "width": 300,
 *          "horizontal_vertical": 0,
 *          "layout_name: "foo"
 *      }
 * }
 * ```
 * @note layout_id in this json is unique and auto generated.
 */

/**
 * @note Constructor for library. parameter are taken from tweakconfig.h
 */
int tweak_connect(void);

/**
 * @note Tweak 2 isn't bound to particular GUI implementation. This call defines default meta for all tweaks
 * created after this call.
 */
void tweak_add_layout(unsigned int width, unsigned int horizontal_vertical, const char* name);

/**
 * @note Destructor for library.
 */
void tweak_close();

/**
 * @note Supported. minv, maxv, def, and precision are transferred to meta field.
 */
void tweak_add_slider(const char* name, double minv, double maxv, double def, unsigned int precision);

/**
 * @note Supported. minv, maxv, def, and precision are transferred to meta field.
 */
void tweak_add_spinbox(const char* name, double minv, double maxv, double def, unsigned int precision);

/**
 * @note Supported. def_val is transferred to meta field.
 */
void tweak_add_checkbox(const char* name, int def_val);

/**
 * @note Limited support. Creates boolean item.
 */
void tweak_add_button(const char* name);

/**
 * @note Supported. desc and def are transferred to meta field.
 */
void tweak_add_groupbox(const char* name, const char* desc, unsigned int def);

/**
 * @note Limited support. Creates boolean item.
 */
void tweak_add_widget(const char* name);

/**
 * @note Supported, shall work as expected.
 */
double tweak_get(const char* name, double defval);

/**
 * @note Supported, shall work as expected.
 */
void tweak_set(const char* name, double val);

/**
 * @note Not Supported.
 */
double tweak_get_string(const char* name, double defval);

/**
 * @note Not Supported.
 */
void tweak_on_update(const char* name);

/**
 * @note Late binding mechanism. Callback is passed as a pointer.
 */
typedef void(*tweak_update_handler)(const char* name, void* cookie);

void tweak_set_update_handler(tweak_update_handler handler, void* cookie);

/**
 * @note Not Supported.
 */
uint64_t tweak_fopen(const char* name, const char* mode);

/**
 * @note Not Supported.
 */
uint64_t tweak_fclose(uint64_t fd);

/**
 * @note Not Supported.
 */
uint64_t tweak_ftell(uint64_t fd);

/**
 * @note Not Supported.
 */
uint64_t tweak_fseek(uint64_t fd, int32_t offset, int32_t where);

/**
 * @note Not Supported.
 */
uint64_t tweak_fwrite(uint64_t fd, uint32_t sz, void* p_data);

/**
 * @note Not Supported.
 */
uint64_t tweak_fread(uint64_t fd, uint32_t sz, void* p_data);

/**
 * @note Not Supported.
 */
uint64_t tweak_config_fopen(const char* name, const char* mode);

/**
 * @note Not Supported.
 */
uint64_t tweak_config_fclose(uint64_t fd);

/**
 * @note Not Supported.
 */
uint64_t tweak_config_add(uint32_t sz, void* p_data);

/**
 * @note Not Supported.
 */
void tweak_get_file_path(char* dst, const char* mask, uint32_t read);

/**
 * @note Not Supported.
 */
uint32_t tweak_json_config_read(void* p_data, uint32_t max_sz, int cfg_enum, const char* cfg_name_ver, const char* path);

/**
 * @note Not Supported.
 */
uint32_t tweak_json_config_write(void* p_data, uint32_t max_sz, int cfg_enum, const char* cfg_name_ver, const char* path);

#ifdef __cplusplus
}
#endif

#endif // TWEAK_H_INCLUDED
