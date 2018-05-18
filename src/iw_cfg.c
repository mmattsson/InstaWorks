// --------------------------------------------------------------------------
///
/// @file iw_cfg.c
///
/// Copyright (c) 2014-2018 Mattias Mattsson. All rights reserved.
/// This source is distributed under the license in LICENSE.txt in the top
/// InstaWorks directory.
///
// --------------------------------------------------------------------------

#include "iw_cfg.h"

#include "iw_log.h"
#include "iw_util.h"

#include <parson.h>

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------

/// Add a number to the configuration settings.
/// @param n The name of the setting.
/// @param p True if the value should be persisted.
/// @param m The error message for the setting.
/// @param r The range of the number.
#define ADD_NUM(n,p,m,r) iw_cfg_add_number(IW_CFG_##n,p,m,r,IW_DEF_##n)

/// Add a port number to the configuration settings. The allowed range for the
/// port number is 0-65535.
/// @param n The name of the setting.
/// @param p True if the value should be persisted.
#define ADD_PORT(n,p)  ADD_NUM(n,p,"Must be between 0 and 65535",IW_VAL_CRIT_PORT)

/// Add a boolean to the configuration settings. The allowed range for the
/// number is 0 or 1.
/// @param n The name of the setting.
/// @param p True if the value should be persisted.
#define ADD_BOOL(n,p)  ADD_NUM(n,p,"Must be 0 or 1",IW_VAL_CRIT_BOOL)

/// Add a string to the configuration settings.
/// @param n The name of the setting.
/// @param p True if the value should be persisted.
/// @param m The error message for the setting.
/// @param r The regular expression for the string.
#define ADD_STR(n,p,m,r) iw_cfg_add_string(IW_CFG_##n,p,m,r,IW_DEF_##n)

/// Add a character to the configuration settings. The string can only be
/// one character long.
/// @param n The name of the setting.
/// @param p True if the value should be persisted.
#define ADD_CHAR(n,p)  ADD_STR(n,p,"Must be a single character",IW_VAL_CRIT_CHAR)

// --------------------------------------------------------------------------

/// The root JSON config object name.
#define ROOT_CFG_OBJ    "cfg"

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
        NULL, NULL
};

// --------------------------------------------------------------------------
//
// Function API
//
// --------------------------------------------------------------------------

void iw_cfg_add_number(
    const char *name,
    bool persist,
    const char *msg,
    const char *regexp,
    int def)
{
    char buff[256];

    if(regexp != NULL) {
        iw_val_store_add_name_regexp(&iw_cfg, name, msg,
                                     IW_VAL_TYPE_NUMBER, regexp, persist);
    } else {
        iw_val_store_add_name(&iw_cfg, name, msg, IW_VAL_TYPE_NUMBER, persist);
    }
    if(iw_val_store_set_number(&iw_cfg, name, def, buff, sizeof(buff)) != IW_VAL_RET_OK) {
        LOG(IW_LOG_IW, "Failed to set default configuration setting for '%s' (%s)",
            name, buff);
    }
}

// --------------------------------------------------------------------------

void iw_cfg_add_string(
    const char *name,
    bool persist,
    const char *msg,
    const char *regexp,
    const char *def)
{
    char buff[256];

    if(regexp != NULL) {
        iw_val_store_add_name_regexp(&iw_cfg, name, msg,
                                     IW_VAL_TYPE_STRING, regexp, persist);
    } else {
        iw_val_store_add_name(&iw_cfg, name, msg, IW_VAL_TYPE_STRING, persist);
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

    ADD_PORT(CMD_PORT, true);
    ADD_BOOL(FOREGROUND, false);
    ADD_CHAR(FOREGROUND_OPT, true);
    ADD_BOOL(DAEMONIZE, false);
    ADD_CHAR(DAEMONIZE_OPT, true);
    ADD_NUM(LOGLEVEL, true, NULL, NULL);
    ADD_CHAR(LOGLEVEL_OPT, true);
    ADD_BOOL(ALLOW_QUIT, true);
    ADD_BOOL(CRASHHANDLER_ENABLE, true);
    ADD_STR(CRASHHANDLER_FILE, true, NULL, NULL);
    ADD_BOOL(MEMTRACK_ENABLE, true);
    ADD_NUM(MEMTRACK_SIZE, true, NULL, NULL);
    ADD_BOOL(HEALTHCHECK_ENABLE, true);
    ADD_BOOL(WEBGUI_ENABLE, true);
    ADD_STR(WEBGUI_CSS_FILE, true, NULL, NULL);
    ADD_NUM(SYSLOG_SIZE, true, NULL, NULL);
    ADD_STR(PRG_NAME, true, NULL, NULL);
    ADD_STR(PRG_ABOUT, false, NULL, NULL);
}

// --------------------------------------------------------------------------

static void iw_cfg_get_json_obj(JSON_Object *obj, size_t idx, const char *path) {
    const char *str;
    double num;
    int boolean;
    char buff[256];
    IW_VAL_RET ret = IW_VAL_RET_NO_SUCH_VALUE;

    // Get the value
    const char *name = json_object_get_name(obj, idx);
    JSON_Value *val = json_object_get_value(obj, name);
    JSON_Value_Type type = json_value_get_type(val);

    char *full_name = iw_util_concat(3, path, ".", name);
    if(full_name == NULL) {
        // Failed to allocate memory for the path.
        return;
    }

    // For leaf-values, we check whether we should persist them or not.
    // If we should not persist them, we return here. If it is not a
    // leaf-value, we should not check for persistance since non-leaf
    // values are not saved in the value store.
    switch(type) {
    case JSONString :
    case JSONNumber :
    case JSONBoolean : {
        bool persist = iw_val_store_get_persist(&iw_cfg, full_name);
        if(!persist) {
            // Should not persist this value
            free(full_name);
            return;
        }
        } break;
    }

    // Depending on type of value, set the configuration variable
    switch(type) {
    case JSONString :
        str = json_value_get_string(val);
        if(str != NULL) {
            ret = iw_val_store_set_string(&iw_cfg,
                                          full_name, str,
                                          buff, sizeof(buff));
        }
        break;
    case JSONNumber :
        num = json_value_get_number(val);
        ret = iw_val_store_set_number(&iw_cfg,
                                      full_name, num,
                                      buff, sizeof(buff));
        break;
    case JSONBoolean :
        boolean = json_value_get_boolean(val);
        ret = iw_val_store_set_number(&iw_cfg,
                                      full_name, boolean,
                                      buff, sizeof(buff));
        break;
    case JSONObject : {
        JSON_Object *sub_obj = json_value_get_object(val);
        size_t max = json_object_get_count(sub_obj);
        size_t sub_idx;
        for(sub_idx=0;sub_idx < max;sub_idx++) {
            iw_cfg_get_json_obj(sub_obj, sub_idx, full_name);
        }
        } break;
    }
    free(full_name);
}

// --------------------------------------------------------------------------

bool iw_cfg_load(const char *file) {
    JSON_Value *root_value, *val;
    JSON_Array *array;
    JSON_Object *obj, *cfg;
    size_t idx;

    if(file == NULL) {
        return false;
    }

    // Update the file name.
    if(iw_cfg_file != NULL) {
        free(iw_cfg_file);
    }
    iw_cfg_file = strdup(file);

    // Load the configuration settings (if any).
    root_value = json_parse_file(file);
    if(root_value == NULL || json_value_get_type(root_value) != JSONObject) {
        LOG(IW_LOG_IW, "Failed to read configuration file.");
        return false;
    }

    // Access the json variables
    obj = json_object(root_value);
    cfg = json_object_get_object(obj, ROOT_CFG_OBJ);
    size_t max = json_object_get_count(cfg);
    for(idx=0;idx < max;idx++) {
        // Iterate through each setting and set the internal configuration
        iw_cfg_get_json_obj(cfg, idx, ROOT_CFG_OBJ);
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
        // If we should not persist the value, then continue to the next.
        bool persist = iw_val_store_get_persist(&iw_cfg, value->name);
        if(persist) {
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
        }

        value = iw_val_store_get_next(&iw_cfg, &token);
    }

    JSON_Status status = json_serialize_to_file_pretty(val, iw_cfg_file);
    if(status != JSONSuccess) {
        LOG(IW_LOG_IW, "Failed to create file for saving configuration.");
        return false;
    }

    json_value_free(val);

    return false;
}

// --------------------------------------------------------------------------

void iw_cfg_exit() {
    iw_val_store_destroy(&iw_cfg);

    free(iw_cfg_file);
    iw_cfg_file = NULL;
}

// --------------------------------------------------------------------------
