/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.5 */

#ifndef PB_TWEAK_PB_TWEAK_PB_H_INCLUDED
#define PB_TWEAK_PB_TWEAK_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
/* Body of "subscribe" request. */
typedef struct _tweak_pb_subscribe { 
    pb_callback_t uri_patterns; 
} tweak_pb_subscribe;

/* Body of "remove item" request. */
typedef struct _tweak_pb_remove_item { 
    /* Tweak id */
    uint64_t tweak_id; 
} tweak_pb_remove_item;

/* Variant value for transmission. */
typedef struct _tweak_pb_value { 
    pb_size_t which_values;
    union {
        bool is_null;
        bool scalar_bool;
        int32_t scalar_sint8;
        int32_t scalar_sint16;
        int32_t scalar_sint32;
        int64_t scalar_sint64;
        uint32_t scalar_uint8;
        uint32_t scalar_uint16;
        uint32_t scalar_uint32;
        uint64_t scalar_uint64;
        float scalar_float;
        double scalar_double;
    } values; 
} tweak_pb_value;

/* A model is a flat list of these items. */
typedef struct _tweak_pb_add_item { 
    /* Tweak id */
    uint64_t tweak_id; 
    /* API references tweaks by this name */
    pb_callback_t uri; 
    /* Tweak current value */
    bool has_current_value;
    tweak_pb_value current_value; 
    /* Detailed description */
    pb_callback_t description; 
    /* Representation details
recognizable by GUI. */
    pb_callback_t meta; 
    /* Tweak default value */
    bool has_default_value;
    tweak_pb_value default_value; 
} tweak_pb_add_item;

/* Body of singular change of tweak's current value.
Can go from server endpoint to client or vice versa. */
typedef struct _tweak_pb_change_item { 
    /* id provided by model. */
    uint64_t tweak_id; 
    /* new value. */
    bool has_value;
    tweak_pb_value value; 
} tweak_pb_change_item;

/* Envelope for messages being sent by client endpoint
to server endpoint. */
typedef struct _tweak_pb_client_node_message { 
    pb_callback_t cb_request;
    pb_size_t which_request;
    union {
        tweak_pb_subscribe subscribe;
        tweak_pb_change_item change_item;
    } request; 
} tweak_pb_client_node_message;

/* Envelope for messages being sent by server endpoint
to client endpoint. */
typedef struct _tweak_pb_server_node_message { 
    pb_callback_t cb_request;
    pb_size_t which_request;
    union {
        tweak_pb_add_item add_item;
        tweak_pb_change_item change_item;
        tweak_pb_remove_item remove_item;
    } request; 
} tweak_pb_server_node_message;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define tweak_pb_value_init_default              {0, {0}}
#define tweak_pb_add_item_init_default           {0, {{NULL}, NULL}, false, tweak_pb_value_init_default, {{NULL}, NULL}, {{NULL}, NULL}, false, tweak_pb_value_init_default}
#define tweak_pb_subscribe_init_default          {{{NULL}, NULL}}
#define tweak_pb_change_item_init_default        {0, false, tweak_pb_value_init_default}
#define tweak_pb_remove_item_init_default        {0}
#define tweak_pb_client_node_message_init_default {{{NULL}, NULL}, 0, {tweak_pb_subscribe_init_default}}
#define tweak_pb_server_node_message_init_default {{{NULL}, NULL}, 0, {tweak_pb_add_item_init_default}}
#define tweak_pb_value_init_zero                 {0, {0}}
#define tweak_pb_add_item_init_zero              {0, {{NULL}, NULL}, false, tweak_pb_value_init_zero, {{NULL}, NULL}, {{NULL}, NULL}, false, tweak_pb_value_init_zero}
#define tweak_pb_subscribe_init_zero             {{{NULL}, NULL}}
#define tweak_pb_change_item_init_zero           {0, false, tweak_pb_value_init_zero}
#define tweak_pb_remove_item_init_zero           {0}
#define tweak_pb_client_node_message_init_zero   {{{NULL}, NULL}, 0, {tweak_pb_subscribe_init_zero}}
#define tweak_pb_server_node_message_init_zero   {{{NULL}, NULL}, 0, {tweak_pb_add_item_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define tweak_pb_subscribe_uri_patterns_tag      1
#define tweak_pb_remove_item_tweak_id_tag        1
#define tweak_pb_value_is_null_tag               1
#define tweak_pb_value_scalar_bool_tag           2
#define tweak_pb_value_scalar_sint8_tag          3
#define tweak_pb_value_scalar_sint16_tag         4
#define tweak_pb_value_scalar_sint32_tag         5
#define tweak_pb_value_scalar_sint64_tag         6
#define tweak_pb_value_scalar_uint8_tag          7
#define tweak_pb_value_scalar_uint16_tag         8
#define tweak_pb_value_scalar_uint32_tag         9
#define tweak_pb_value_scalar_uint64_tag         10
#define tweak_pb_value_scalar_float_tag          11
#define tweak_pb_value_scalar_double_tag         12
#define tweak_pb_add_item_tweak_id_tag           1
#define tweak_pb_add_item_uri_tag                2
#define tweak_pb_add_item_current_value_tag      3
#define tweak_pb_add_item_description_tag        4
#define tweak_pb_add_item_meta_tag               5
#define tweak_pb_add_item_default_value_tag      6
#define tweak_pb_change_item_tweak_id_tag        1
#define tweak_pb_change_item_value_tag           2
#define tweak_pb_client_node_message_subscribe_tag 1
#define tweak_pb_client_node_message_change_item_tag 2
#define tweak_pb_server_node_message_add_item_tag 1
#define tweak_pb_server_node_message_change_item_tag 2
#define tweak_pb_server_node_message_remove_item_tag 3

/* Struct field encoding specification for nanopb */
#define tweak_pb_value_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    BOOL,     (values,is_null,values.is_null),   1) \
X(a, STATIC,   ONEOF,    BOOL,     (values,scalar_bool,values.scalar_bool),   2) \
X(a, STATIC,   ONEOF,    SINT32,   (values,scalar_sint8,values.scalar_sint8),   3) \
X(a, STATIC,   ONEOF,    SINT32,   (values,scalar_sint16,values.scalar_sint16),   4) \
X(a, STATIC,   ONEOF,    SINT32,   (values,scalar_sint32,values.scalar_sint32),   5) \
X(a, STATIC,   ONEOF,    SINT64,   (values,scalar_sint64,values.scalar_sint64),   6) \
X(a, STATIC,   ONEOF,    UINT32,   (values,scalar_uint8,values.scalar_uint8),   7) \
X(a, STATIC,   ONEOF,    UINT32,   (values,scalar_uint16,values.scalar_uint16),   8) \
X(a, STATIC,   ONEOF,    UINT32,   (values,scalar_uint32,values.scalar_uint32),   9) \
X(a, STATIC,   ONEOF,    UINT64,   (values,scalar_uint64,values.scalar_uint64),  10) \
X(a, STATIC,   ONEOF,    FLOAT,    (values,scalar_float,values.scalar_float),  11) \
X(a, STATIC,   ONEOF,    DOUBLE,   (values,scalar_double,values.scalar_double),  12)
#define tweak_pb_value_CALLBACK NULL
#define tweak_pb_value_DEFAULT NULL

#define tweak_pb_add_item_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT64,   tweak_id,          1) \
X(a, CALLBACK, SINGULAR, STRING,   uri,               2) \
X(a, STATIC,   OPTIONAL, MESSAGE,  current_value,     3) \
X(a, CALLBACK, SINGULAR, STRING,   description,       4) \
X(a, CALLBACK, SINGULAR, STRING,   meta,              5) \
X(a, STATIC,   OPTIONAL, MESSAGE,  default_value,     6)
#define tweak_pb_add_item_CALLBACK pb_default_field_callback
#define tweak_pb_add_item_DEFAULT NULL
#define tweak_pb_add_item_current_value_MSGTYPE tweak_pb_value
#define tweak_pb_add_item_default_value_MSGTYPE tweak_pb_value

#define tweak_pb_subscribe_FIELDLIST(X, a) \
X(a, CALLBACK, SINGULAR, STRING,   uri_patterns,      1)
#define tweak_pb_subscribe_CALLBACK pb_default_field_callback
#define tweak_pb_subscribe_DEFAULT NULL

#define tweak_pb_change_item_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT64,   tweak_id,          1) \
X(a, STATIC,   OPTIONAL, MESSAGE,  value,             2)
#define tweak_pb_change_item_CALLBACK NULL
#define tweak_pb_change_item_DEFAULT NULL
#define tweak_pb_change_item_value_MSGTYPE tweak_pb_value

#define tweak_pb_remove_item_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT64,   tweak_id,          1)
#define tweak_pb_remove_item_CALLBACK NULL
#define tweak_pb_remove_item_DEFAULT NULL

#define tweak_pb_client_node_message_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (request,subscribe,request.subscribe),   1) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (request,change_item,request.change_item),   2)
#define tweak_pb_client_node_message_CALLBACK NULL
#define tweak_pb_client_node_message_DEFAULT NULL
#define tweak_pb_client_node_message_request_subscribe_MSGTYPE tweak_pb_subscribe
#define tweak_pb_client_node_message_request_change_item_MSGTYPE tweak_pb_change_item

#define tweak_pb_server_node_message_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (request,add_item,request.add_item),   1) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (request,change_item,request.change_item),   2) \
X(a, STATIC,   ONEOF,    MSG_W_CB, (request,remove_item,request.remove_item),   3)
#define tweak_pb_server_node_message_CALLBACK NULL
#define tweak_pb_server_node_message_DEFAULT NULL
#define tweak_pb_server_node_message_request_add_item_MSGTYPE tweak_pb_add_item
#define tweak_pb_server_node_message_request_change_item_MSGTYPE tweak_pb_change_item
#define tweak_pb_server_node_message_request_remove_item_MSGTYPE tweak_pb_remove_item

extern const pb_msgdesc_t tweak_pb_value_msg;
extern const pb_msgdesc_t tweak_pb_add_item_msg;
extern const pb_msgdesc_t tweak_pb_subscribe_msg;
extern const pb_msgdesc_t tweak_pb_change_item_msg;
extern const pb_msgdesc_t tweak_pb_remove_item_msg;
extern const pb_msgdesc_t tweak_pb_client_node_message_msg;
extern const pb_msgdesc_t tweak_pb_server_node_message_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define tweak_pb_value_fields &tweak_pb_value_msg
#define tweak_pb_add_item_fields &tweak_pb_add_item_msg
#define tweak_pb_subscribe_fields &tweak_pb_subscribe_msg
#define tweak_pb_change_item_fields &tweak_pb_change_item_msg
#define tweak_pb_remove_item_fields &tweak_pb_remove_item_msg
#define tweak_pb_client_node_message_fields &tweak_pb_client_node_message_msg
#define tweak_pb_server_node_message_fields &tweak_pb_server_node_message_msg

/* Maximum encoded size of messages (where known) */
/* tweak_pb_add_item_size depends on runtime parameters */
/* tweak_pb_subscribe_size depends on runtime parameters */
/* tweak_pb_client_node_message_size depends on runtime parameters */
/* tweak_pb_server_node_message_size depends on runtime parameters */
#define tweak_pb_change_item_size                24
#define tweak_pb_remove_item_size                11
#define tweak_pb_value_size                      11

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
