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
#include <string>

#include "guid.h"
#include "timehelpers.h"
#include "../include/mstest.h"

// --------------------------------------------------------------------------
class Test
{
public:    
    std::string functionName;
    Guid testId;
    Guid executionId;
    SomeTime entryTime; 
    SomeTime endTime; 
    std::shared_ptr<AssertX::AssertFailed> pFailure;
    bool ignored = false;
    bool executed = false;
    bool error = false;

    std::string GetTrxOutcome() const
    {
        if (error)
            return "Error";
        if (pFailure)
            return "Failed";
        if (ignored)
            return "NotExecuted";
        if (executed)
            return "Passed";
        return "Error";
    }

    std::string GetClassName() const
    {
        size_t pos = functionName.rfind("::");
        if (pos != std::string::npos)
            return functionName.substr(0, pos);
        return functionName;
    }

    std::string GetShortName() const
    {
        size_t pos = functionName.rfind("::");
        if (pos != std::string::npos)
            return functionName.substr(pos + 2);
        return "";
    }    
};