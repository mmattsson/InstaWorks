// --------------------------------------------------------------------------
///
/// @file iw_cfg.c
///
/// Copyright (c) 2014-2015 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_cfg.h"

#include "iw_log.h"

#include <parson.h>

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------

/// Add a number to the configuration settings.
/// @param n The name of the setting.
/// @param m The error message for the setting.
/// @param r The range of the number.
#define ADD_NUM(n,m,r) iw_cfg_add_number(IW_CFG_##n,m,r,IW_DEF_##n)

/// Add a port number to the configuration settings. The allowed range for the
/// port number is 0-65535.
/// @param n The name of the setting.
#define ADD_PORT(n)  ADD_NUM(n,"Must be between 0 and 65535",IW_VAL_CRIT_PORT)

/// Add a boolean to the configuration settings. The allowed range for the
/// number is 0 or 1.
/// @param n The name of the setting.
#define ADD_BOOL(n)  ADD_NUM(n,"Must be 0 or 1",IW_VAL_CRIT_BOOL)

/// Add a string to the configuration settings.
/// @param n The name of the setting.
/// @param m The error message for the setting.
/// @param r The regular expression for the string.
#define ADD_STR(n,m,r) iw_cfg_add_string(IW_CFG_##n,m,r,IW_DEF_##n)

/// Add a character to the configuration settings. The string can only be
/// one character long.
/// @param n The name of the setting.
#define ADD_CHAR(n)  ADD_STR(n,"Must be a single character",IW_VAL_CRIT_CHAR)

// --------------------------------------------------------------------------

/// The configuration.
iw_val_store iw_cfg;

/// The configuration file.
char *iw_cfg_file = NULL;

// --------------------------------------------------------------------------
//
// Data structures
//
// --------------------------------------------------------------------------

iw_callbacks iw_cb = {
        NULL
};

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_cfg_add_number(
    const char *name,
    const char *msg,
    const char *regexp,
    int def)
{
    char buff[256];

    if(regexp != NULL) {
        iw_val_store_add_name_regexp(&iw_cfg, name, msg,
                                     IW_VAL_TYPE_NUMBER, regexp);
    } else {
        iw_val_store_add_name(&iw_cfg, name, msg, IW_VAL_TYPE_NUMBER);
    }
    if(iw_val_store_set_number(&iw_cfg, name, def, buff, sizeof(buff)) != IW_VAL_RET_OK) {
        LOG(IW_LOG_IW, "Failed to set default configuration setting for '%s' (%s)",
            name, buff);
    }
}

// --------------------------------------------------------------------------

void iw_cfg_add_string(
    const char *name,
    const char *msg,
    const char *regexp,
    const char *def)
{
    char buff[256];

    if(regexp != NULL) {
        iw_val_store_add_name_regexp(&iw_cfg, name, msg,
                                     IW_VAL_TYPE_STRING, regexp);
    } else {
        iw_val_store_add_name(&iw_cfg, name, msg, IW_VAL_TYPE_STRING);
    }
    if(def != NULL) {
        if(iw_val_store_set_string(&iw_cfg, name, def, buff, sizeof(buff)) != IW_VAL_RET_OK) {
            LOG(IW_LOG_IW, "Failed to set default configuration setting for '%s' (%s)",
                name, buff);
        }
    }
}

// --------------------------------------------------------------------------

void iw_cfg_init() {
    static bool initialized = false;
    if(initialized) {
        return;
    }
    initialized = true;

    iw_val_store_initialize(&iw_cfg, true);

    ADD_PORT(CMD_PORT);
    ADD_BOOL(FOREGROUND);
    ADD_CHAR(FOREGROUND_OPT);
    ADD_BOOL(DAEMONIZE);
    ADD_CHAR(DAEMONIZE_OPT);
    ADD_NUM(LOGLEVEL, NULL, NULL);
    ADD_CHAR(LOGLEVEL_OPT);
    ADD_BOOL(ALLOW_QUIT);
    ADD_BOOL(CRASHHANDLER_ENABLE);
    ADD_STR(CRASHHANDLER_FILE, NULL, NULL);
    ADD_BOOL(MEMTRACK_ENABLE);
    ADD_NUM(MEMTRACK_SIZE, NULL, NULL);
    ADD_BOOL(HEALTHCHECK_ENABLE);
    ADD_BOOL(WEBGUI_ENABLE);
    ADD_STR(WEBGUI_CSS_FILE, NULL, NULL);
    ADD_NUM(SYSLOG_SIZE, NULL, NULL);
    ADD_STR(PRG_NAME, NULL, NULL);
    ADD_STR(PRG_ABOUT, NULL, NULL);
}

// --------------------------------------------------------------------------

static void iw_cfg_get_json_obj(JSON_Object *obj, size_t idx) {
    const char *name = json_object_get_name(obj, idx);
    JSON_Value *val = json_object_get_value(obj, name);
    JSON_Value_Type type = json_value_get_type(val);
    const char *str;
    double num;
    int boolean;
    switch(type) {
    case JSONString :
        str = json_value_get_string(val);
        break;
    case JSONNumber :
        num = json_value_get_number(val);
        break;
    case JSONBoolean :
        boolean = json_value_get_boolean(val);
        break;
    case JSONObject : {
        JSON_Object *sub_obj = json_value_get_object(val);
        size_t max = json_object_get_count(sub_obj);
        size_t sub_idx;
        for(sub_idx=0;sub_idx < max;sub_idx++) {
            iw_cfg_get_json_obj(sub_obj, sub_idx);
        }
        } break;
    }
}

// --------------------------------------------------------------------------

bool iw_cfg_load(const char *file) {
    JSON_Value *root_value, *val;
    JSON_Array *array;
    JSON_Object *obj, *cfg;
    size_t idx;

    // Update the file name if needed.
    if(file != NULL) {
        if(iw_cfg_file != NULL) {
            free(iw_cfg_file);
        }
        iw_cfg_file = strdup(file);
    }

    // Load the configuration settings (if any).
    root_value = json_parse_file(file);
    if(root_value == NULL || json_value_get_type(root_value) != JSONObject) {
        LOG(IW_LOG_IW, "Failed to read configuration file.");
        return false;
    }

    // Access the json variables
    obj = json_object(root_value);
    cfg = json_object_get_object(obj, "cfg");
    size_t max = json_object_get_count(cfg);
    for(idx=0;idx < max;idx++) {
        // Iterate through each setting and set the internal configuration
        iw_cfg_get_json_obj(cfg, idx);
    }

    json_value_free(root_value);

    return true;
}

// --------------------------------------------------------------------------

bool iw_cfg_save(const char *file) {
    // Update the file name if needed.
    if(file != NULL) {
        if(iw_cfg_file != NULL) {
            free(iw_cfg_file);
        }
        iw_cfg_file = strdup(file);
    }

    // Create a JSON object, set the variables and write the JSON
    // object to file.
    JSON_Value *val = json_value_init_object();
    if(val == NULL) {
        LOG(IW_LOG_IW, "Failed to create JSON value for saving configuration.");
        return false;
    }
    JSON_Object *obj = json_value_get_object(val);
    if(obj == NULL) {
        LOG(IW_LOG_IW, "Failed to create JSON object for saving configuration.");
        return false;
    }

    unsigned long token;
    iw_val *value = iw_val_store_get_first(&iw_cfg, &token);
    while(value != NULL) {
        switch(value->type) {
        case IW_VAL_TYPE_NONE :
            break;
        case IW_VAL_TYPE_NUMBER :
            json_object_dotset_number(obj, value->name, value->v.number);
            break;
        case IW_VAL_TYPE_STRING :
            json_object_dotset_string(obj, value->name, value->v.string);
            break;
        case IW_VAL_TYPE_ADDRESS : {
            char value_buff[128];
            iw_val_to_str(value, value_buff, sizeof(value_buff));
            json_object_dotset_string(obj, value->name, value_buff);
            } break;
        }

        value = iw_val_store_get_next(&iw_cfg, &token);
    }

    JSON_Status status = json_serialize_to_file_pretty(val, iw_cfg_file);
    if(status != JSONSuccess) {
        LOG(IW_LOG_IW, "Failed to create file for saving configuration.");
        return false;
    }

    return false;
}

// --------------------------------------------------------------------------

void iw_cfg_exit() {
    iw_val_store_destroy(&iw_cfg);

    free(iw_cfg_file);
}

// --------------------------------------------------------------------------
