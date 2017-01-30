/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "include/m2mtlvserializer.h"
#include "include/nsdllinker.h"
#include "mbed-client/m2mconstants.h"
#include <stdlib.h>

#define TRACE_GROUP "mClt"

#define MAX_TLV_LENGTH_SIZE 3
#define MAX_TLV_ID_SIZE 2
#define TLV_TYPE_SIZE 1

M2MTLVSerializer::M2MTLVSerializer()
{
}

M2MTLVSerializer::~M2MTLVSerializer()
{
}

uint8_t* M2MTLVSerializer::serialize(M2MObjectInstanceList object_instance_list, uint32_t &size)
{
    return serialize_object_instances(object_instance_list, size);
}

uint8_t* M2MTLVSerializer::serialize(M2MResourceList resource_list, uint32_t &size)
{
    bool valid = true;
    return serialize_resources(resource_list, size,valid);
}

uint8_t* M2MTLVSerializer::serialize(M2MResource *resource, uint32_t &size)
{
    uint8_t* data = NULL;
    serialize(resource, data, size);
    return data;
}

uint8_t* M2MTLVSerializer::serialize_object_instances(M2MObjectInstanceList object_instance_list, uint32_t &size)
{
    uint8_t *data = NULL;

    if(!object_instance_list.empty()) {
        M2MObjectInstanceList::const_iterator it;
        it = object_instance_list.begin();
        for (; it!=object_instance_list.end(); it++) {
            uint16_t id = (*it)->instance_id();
            serialize(id, *it, data, size);
        }
    }
    return data;
}

uint8_t* M2MTLVSerializer::serialize_resources(M2MResourceList resource_list, uint32_t &size, bool &valid)
{
    uint8_t *data = NULL;

    if(!resource_list.empty()) {
        M2MResourceList::const_iterator it;
        it = resource_list.begin();
        for (; it!=resource_list.end(); it++) {
            if((*it)->name_id() == -1) {
                valid = false;
                break;
            }
        }
        if(valid) {
            it = resource_list.begin();
            for (; it!=resource_list.end(); it++) {
                serialize(*it, data, size);
            }
        }
    }
    return data;
}

void M2MTLVSerializer::serialize(uint16_t id, M2MObjectInstance *object_instance, uint8_t *&data, uint32_t &size)
{
    uint8_t *resource_data = NULL;
    uint32_t resource_size = 0;

    bool valid = true;
    resource_data = serialize_resources(object_instance->resources(),resource_size,valid);
    if(valid) {
        serialize_TILV(TYPE_OBJECT_INSTANCE, id, resource_data, resource_size, data, size);
    }
    free(resource_data);
}

bool M2MTLVSerializer::serialize(M2MResource *resource, uint8_t *&data, uint32_t &size)
{
    bool success = false;
    if(resource->name_id() != -1) {
        success = resource->supports_multiple_instances() ?
                serialize_multiple_resource(resource, data, size) :
                serialize_resource(resource, data, size);
    }
    return success;
}

bool M2MTLVSerializer::serialize_resource(M2MResource *resource, uint8_t *&data, uint32_t &size)
{
    bool success = false;
    if(resource->name_id() != -1) {
        success = true;
        serialize_TILV(TYPE_RESOURCE, resource->name_id(), resource->value(), resource->value_length(), data, size);
    }
    return success;
}

bool M2MTLVSerializer::serialize_multiple_resource(M2MResource *resource, uint8_t *&data, uint32_t &size)
{
    bool success = false;
    uint8_t *nested_data = NULL;
    uint32_t nested_data_size = 0;

    M2MResourceInstanceList instance_list = resource->resource_instances();
    if(!instance_list.empty()) {
        M2MResourceInstanceList::const_iterator it;
        it = instance_list.begin();
        for (; it!=instance_list.end(); it++) {
            uint16_t id = (*it)->instance_id();
            serialize_resource_instance(id, (*it), nested_data, nested_data_size);            
        }
    }
    if(resource->name_id() != -1) {
        success = true;
        serialize_TILV(TYPE_MULTIPLE_RESOURCE, resource->name_id(), nested_data, nested_data_size, data, size);
    }

    free(nested_data);
    nested_data = NULL;
    return success;
}

void M2MTLVSerializer::serialize_resource_instance(uint16_t id, M2MResourceInstance *resource, uint8_t *&data, uint32_t &size)
{
    serialize_TILV(TYPE_RESOURCE_INSTANCE, id, resource->value(), resource->value_length(), data, size);
}

void M2MTLVSerializer::serialize_TILV(uint8_t type, uint16_t id, uint8_t *value, uint32_t value_length, uint8_t *&data, uint32_t &size)
{
    uint8_t *tlv = 0;
    const uint32_t type_length = TLV_TYPE_SIZE;
    type += id < 256 ? 0 : ID16;
    type += value_length < 8 ? value_length :
            value_length < 256 ? LENGTH8 :
            value_length < 65536 ? LENGTH16 : LENGTH24;
    uint8_t tlv_type;
    tlv_type = type & 0xFF;

    uint32_t id_size;
    uint8_t id_array[MAX_TLV_ID_SIZE];
    serialize_id(id, id_size, id_array);

    uint32_t length_size;
    uint8_t length_array[MAX_TLV_LENGTH_SIZE];
    serialize_length(value_length, length_size, length_array);

    tlv = (uint8_t*)malloc(size + type_length + id_size + length_size + value_length);
    if(data) {
        memcpy(tlv, data, size);
        free(data);
    }

    uint8_t *out_ptr = tlv + size;
    *out_ptr++=tlv_type;

    int i=0;
    while(i<id_size) {
        *out_ptr++=id_array[i++];
    }

    i=0;
    while(i<length_size) {
        *out_ptr++=length_array[i++];
    }

    memcpy(tlv+size+type_length+id_size+length_size, value, value_length);

    data = tlv;
    size += type_length + id_size + length_size + value_length;
}

void M2MTLVSerializer::serialize_id(uint16_t id, uint32_t &size, uint8_t *id_ptr)
{
    if(id > 255) {
        size=2;
        id_ptr[0] = (id & 0xFF00) >> 8;
        id_ptr[1] = id & 0xFF;
    } else {
        size=1;
        id_ptr[0] = id & 0xFF;
    }
}

void M2MTLVSerializer::serialize_length(uint32_t length, uint32_t &size, uint8_t *length_ptr)
{
    if (length > 65535) {
        size = 3;
        length_ptr[0] = (length & 0xFF0000) >> 16;
        length_ptr[1] = (length & 0xFF00) >> 8;
        length_ptr[2] = length & 0xFF;
    } else if (length > 255) {
        size = 2;
        length_ptr[0] = (length & 0xFF00) >> 8;
        length_ptr[1] = length & 0xFF;
    } else if (length > 7) {
        size = 1;
        length_ptr[0] = length & 0xFF;
    } else {
        size=0;
    }
}

