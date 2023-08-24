#include <dta_test_ta.h>

#include "func_handlers_storage.h"

TEE_Result func_write_object(uint32_t param_types, TEE_Param params[4]) {
    const uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    TEE_ObjectHandle object;
    TEE_Result res;
    char *obj_id;
    size_t obj_id_sz;
    char *data;
    size_t data_sz;
    uint32_t obj_data_flag;

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    obj_id_sz = params[0].memref.size;
    obj_id = TEE_Malloc(obj_id_sz, 0);
    if (!obj_id)
        return TEE_ERROR_OUT_OF_MEMORY;

    TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

    data_sz = params[1].memref.size;
    data = TEE_Malloc(data_sz, 0);
    if (!data)
        return TEE_ERROR_OUT_OF_MEMORY;
    TEE_MemMove(data, params[1].memref.buffer, data_sz);

    obj_data_flag =
        TEE_DATA_FLAG_ACCESS_READ |  /* we can later read the oject */
        TEE_DATA_FLAG_ACCESS_WRITE | /* we can later write into the object */
        TEE_DATA_FLAG_ACCESS_WRITE_META | /* we can later destroy or rename the
                                             object */
        TEE_DATA_FLAG_OVERWRITE; /* destroy existing object of same ID */

    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, obj_id, obj_id_sz,
                                     obj_data_flag, TEE_HANDLE_NULL, NULL,
                                     0, /* we may not fill it right now */
                                     &object);
    if (res != TEE_SUCCESS) {
        log_error("TEE_CreatePersistentObject() failed. (res=0x%x)\n", res);
        TEE_Free(obj_id);
        TEE_Free(data);
        return res;
    }

    res = TEE_WriteObjectData(object, data, data_sz);
    if (res != TEE_SUCCESS) {
        log_error("TEE_WriteObjectData() failed. (res=0x%x)\n", res);
        TEE_CloseAndDeletePersistentObject1(object);
    } else
        TEE_CloseObject(object);

    TEE_Free(obj_id);
    TEE_Free(data);
    return res;
}

TEE_Result func_read_object(uint32_t param_types, TEE_Param params[4]) {
    const uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    TEE_ObjectHandle object;
    TEE_ObjectInfo object_info;
    TEE_Result res;
    uint32_t read_bytes;
    char *obj_id;
    size_t obj_id_sz;
    char *data;
    size_t data_sz;

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    obj_id_sz = params[0].memref.size;
    obj_id = TEE_Malloc(obj_id_sz, 0);
    if (!obj_id)
        return TEE_ERROR_OUT_OF_MEMORY;

    TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

    data_sz = params[1].memref.size;
    data = TEE_Malloc(data_sz, 0);
    if (!data)
        return TEE_ERROR_OUT_OF_MEMORY;

    res = TEE_OpenPersistentObject(
        TEE_STORAGE_PRIVATE, obj_id, obj_id_sz,
        TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ, &object);
    if (res != TEE_SUCCESS) {
        log_error("TEE_OpenPersistentObject() failed. (res=0x%x)\n", res);
        TEE_Free(obj_id);
        TEE_Free(data);
        return res;
    }

    res = TEE_GetObjectInfo1(object, &object_info);
    if (res != TEE_SUCCESS) {
        log_error("TEE_GetObjectInfo1() failed. (res=0x%x)\n", res);
        goto exit;
    }

    if (object_info.dataSize > data_sz) {
        params[1].memref.size = object_info.dataSize;
        res = TEE_ERROR_SHORT_BUFFER;
        goto exit;
    }

    res = TEE_ReadObjectData(object, data, object_info.dataSize, &read_bytes);
    if (res == TEE_SUCCESS)
        TEE_MemMove(params[1].memref.buffer, data, read_bytes);
    if (res != TEE_SUCCESS || read_bytes != object_info.dataSize) {
        log_error("TEE_ReadObjectData() failed. (res=0x%x, read_bytes=%" PRIu32
                  ", dataSize=%u)\n",
                  res, read_bytes, object_info.dataSize);
        goto exit;
    }

    params[1].memref.size = read_bytes;
exit:
    TEE_CloseObject(object);
    TEE_Free(obj_id);
    TEE_Free(data);
    return res;
}

TEE_Result func_delete_object(uint32_t param_types, TEE_Param params[4]) {
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    TEE_ObjectHandle object;
    TEE_Result res;
    char *obj_id;
    size_t obj_id_sz;

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    obj_id_sz = params[0].memref.size;
    obj_id = TEE_Malloc(obj_id_sz, 0);
    if (!obj_id)
        return TEE_ERROR_OUT_OF_MEMORY;

    TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

    res = TEE_OpenPersistentObject(
        TEE_STORAGE_PRIVATE, obj_id, obj_id_sz,
        TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_ACCESS_WRITE_META, &object);
    if (res != TEE_SUCCESS) {
        log_error("TEE_OpenPersistentObject() failed. (res=0x%x)\n", res);
        TEE_Free(obj_id);
        return res;
    }

    TEE_CloseAndDeletePersistentObject1(object);
    TEE_Free(obj_id);

    return res;
}