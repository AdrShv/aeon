/*******************************************************************************
* Copyright 2016-2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <sstream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <iostream>
#include <iterator>

#include "block_loader_file.hpp"
#include "util.hpp"
#include "file_util.hpp"
#include "log.hpp"
#include "base64.hpp"

using namespace std;
using namespace nervana;

block_loader_file::block_loader_file(shared_ptr<manifest_file> manifest, size_t block_size)
    : async_manager<std::vector<std::string>, encoded_record_list>(manifest, "block_loader_file")
    , m_block_size(block_size)
    , m_record_count{manifest->record_count()}
    , m_manifest(manifest)
{
    m_block_count         = round((float)m_manifest->record_count() / (float)m_block_size);
    m_elements_per_record = manifest->elements_per_record();
}

nervana::encoded_record_list* block_loader_file::filler()
{
    m_state                 = async_state::wait_for_buffer;
    encoded_record_list* rc = get_pending_buffer();
    m_state                 = async_state::processing;
    rc->clear();

    for (int i = 0; i < m_block_size; i++)
    {
        auto& element_list = *m_source->next();

        const vector<manifest::element_t>& types = m_manifest->get_element_types();
        encoded_record                     record;
        for (int j = 0; j < m_elements_per_record; ++j)
        {
            const string& element = element_list[j];
            switch (types[j])
            {
                case manifest::element_t::FILE:
                {
                    auto buffer = file_util::read_file_contents(element);
                    record.add_element(std::move(buffer));
                    break;
                }
                case manifest::element_t::BINARY:
                {
                    vector<char> buffer  = string2vector(element);
                    vector<char> decoded = base64::decode(buffer);
                    record.add_element(std::move(decoded));
                    break;
                }
                case manifest::element_t::STRING:
                {
                    record.add_element(element.data(), element.size());
                    break;
                }
                case manifest::element_t::ASCII_INT:
                {
                    int32_t value = stod(element);
                    record.add_element(&value, sizeof(value));
                    break;
                }
                case manifest::element_t::ASCII_FLOAT:
                {
                    float value = stof(element);
                    record.add_element(&value, sizeof(value));
                    break;
                }
            }
        }
        rc->add_record(std::move(record));
    }

    if (rc && rc->size() == 0)
    {
        rc = nullptr;
    }

    return rc;
}



block_loader_cache::block_loader_cache(std::shared_ptr<CacheSource> cache_file, size_t block_size)
    : async_manager<encoded_record, encoded_record_list>(cache_file, "block_loader_cache")
    , m_block_size(block_size)
    , m_cache_file(cache_file)
{
    m_block_count         = round((float)cache_file->get_record_count() / (float)m_block_size);
    m_elements_per_record = m_cache_file->get_elements_per_record();
    m_record_count = m_cache_file->get_record_count();
}

nervana::encoded_record_list* block_loader_cache::filler()
{
    encoded_record_list* rc = get_pending_buffer();
    rc->clear();

    for (int i = 0; i < m_block_size; i++)
    {
        rc->add_record(std::move(m_cache_file->get_record()));
    }

    if (rc && rc->size() == 0)
    {
        rc = nullptr;
    }

    return rc;
}
