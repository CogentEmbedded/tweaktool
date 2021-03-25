/**
 * @file tweak.h
 * @ingroup tweak-api
 *
 * @brief compatibility library adapting new implementation to legacy API.
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
void tweak_on_update(const char* name) __attribute__((deprecated));

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
