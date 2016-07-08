/*
 Copyright 2015 Nervana Systems Inc.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#pragma once

#include <memory>

#include "buffer_in.hpp"
#include "batch_iterator.hpp"

// This can inherit from BatchIterator, but should it?
class MinibatchIterator : public BatchIterator {
public:
    MinibatchIterator(std::shared_ptr<BatchIterator> macroBatchIterator, int minibatch_size);

    void read(BufferArray& dest);
    void reset();
protected:
    void popItemFromMacrobatch(BufferArray& dest);
    void transferBufferItem(Buffer* dest, Buffer* src);

    std::shared_ptr<BatchIterator> _macroBatchIterator;
    int _minibatchSize;

    BufferArray _macrobatch;
    // the index into the _macrobatch to read next
    int _i;
};
