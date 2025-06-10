// ==========================================================================
// Copyright (C) 2025 by Genetec, Inc.
// 
// This file is under the MIT License (MIT).
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ==========================================================================

#pragma once

#include <cstring>
#include <string>
#include <uuid/uuid.h>

// --------------------------------------------------------------------------
class Guid
{
public:
    Guid() 
    {
        memset(_uuid, 0x00, sizeof(uuid_t));
    }

    Guid(const Guid &rGuid) 
    {
        memcpy(_uuid, rGuid._uuid, sizeof(uuid_t));
    }

    Guid &operator=(const Guid &rGuid) 
    {
        memcpy(_uuid, rGuid._uuid, sizeof(uuid_t));
        return *this;
    }

    bool operator==(const Guid &rB) 
    {
        return 0 == memcmp(_uuid, rB._uuid, sizeof(uuid_t));
    }        

    bool operator!=(const Guid &rB) 
    {
        return !operator==(rB);
    }     

    std::string ToString() const
    {
        char str[UUID_STR_LEN];
        uuid_unparse(_uuid, str);
        return std::string(str);
    }

    static Guid New()
    {       
        Guid oGuid; 
        uuid_generate(oGuid._uuid);
        return oGuid;
    }

private:
    uuid_t _uuid;
};