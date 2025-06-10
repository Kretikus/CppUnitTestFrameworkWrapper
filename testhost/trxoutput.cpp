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

#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <filesystem>
#include <fstream>
#include <unistd.h>

#include "guid.h"
#include "timehelpers.h"
#include "test.h"
#include "trxoutput.h"

// --------------------------------------------------------------------------
static std::string escapeXML(const std::string& input) {

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string output;
    output.reserve(input.size() * 1.3);

    for (char c : input) {
        switch (c) {
        case '&': output += "&amp;"; break;
        case '<': output += "&lt;"; break;
        case '>': output += "&gt;"; break;
        case '"': output += "&quot;"; break;
        case '\'': output += "&apos;"; break;
        default: output += c;
        }
    }
    return output;
}

// --------------------------------------------------------------------------
static std::string GetHostname()
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
        return hostname;

    return "NoHostanme";
}

// --------------------------------------------------------------------------
void TrxOutput::OutputToFile( const std::string& rTrxPath
                            , const std::string& rTestSo
                            , const SomeTime testEntry
                            , const SomeTime testCompletion
                            , const std::vector<Test>& allTests
                          )
{
    if (rTrxPath.empty())
        return;

    std::ofstream trx(rTrxPath);
    if (!trx.is_open()) {
        std::cerr << "Failed to open trx output file: " << rTrxPath << std::endl;
        return;
    }

    int nbExecuted = 0;
    int nbPasssed = 0;
    int nbFailed = 0;
    int nbError = 0;
    int nbIgnored = 0;
    for (const auto& test : allTests)
    {
        if (test.ignored)
        {
            ++nbIgnored;
            continue;
        }

        if (test.error)
        {
            ++nbError;
            continue;
        }

        if (test.executed)
        {
            ++nbExecuted;
            if (test.pFailure)            
                ++nbFailed;            
            else
                ++nbPasssed;
        }
    }        

    auto strHostName = GetHostname();

    trx << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    trx << "<TestRun id=\"" << Guid::New().ToString() << "\" name=\"LinuxTestRun" << Iso8601WithMs(Now()) << "\" xmlns=\"http://microsoft.com/schemas/VisualStudio/TeamTest/2010\">\n";    
    trx << "  <Times creation=\"" << Iso8601WithMs(testEntry) << "\" start=\"" << Iso8601WithMs(testEntry) << "\" finish=\"" << Iso8601WithMs(testCompletion) << "\" />\n";
    trx << "  <TestSettings name=\"default\" id=\"1a61beb3-bd7a-4cb9-9d96-a9999be895f1\" />\n";     

    trx << "  <Results>\n";
    for (const auto& test : allTests) {
        trx << "    <UnitTestResult outcome=\"" << test.GetTrxOutcome() << "\" testName=\"" << escapeXML(test.functionName) << "\"" 
            << " executionId=\"" << test.executionId.ToString() << "\" testId=\"" << test.testId.ToString() << "\""
            << " startTime=\"" << Iso8601WithMs(test.entryTime) << "\" endTime=\"" << Iso8601WithMs(test.endTime) << "\""
            << " duration=\"" << FormatDuration(test.endTime - test.entryTime) << "\""
            << " computerName=\"" << escapeXML(strHostName) << "\""
            << " testType=\"13cdc9d9-ddb5-4fa4-a97d-d965ccfc6d4b\""
            << " testListId=\"8c84fa94-04c1-424b-9868-57a2d4851a1d\"";

        if (!test.pFailure)
        {
            trx  << " />\n"; 
        }
        else
        {
            trx << " >\n";
            trx << "      <Output>\n";
            trx << "        <ErrorInfo>"
                << "          <Message>" << escapeXML(test.pFailure->what()) << "</Message>\n"
                << "        </ErrorInfo>\n"
                << "      </Output>\n"
                << "    </UnitTestResult>\n";
        }
    }
    trx << "  </Results>\n";    

    trx << "  <TestDefinitions>\n";
    for (const auto& test : allTests) {
        trx << "    <UnitTest name=\"" << escapeXML(test.functionName) << "\"" 
            << " storage=\"" << escapeXML(rTestSo) << "\""
            << " id=\"" << test.testId.ToString() << "\""      
            << " >\n";      
        trx << "      <Execution id=\"" << test.executionId.ToString() << "\""     
            << " />\n";
        trx << "      <TestMethod codeBase=\"" << escapeXML(rTestSo) << "\""    
            << " className=\"" << escapeXML(test.GetClassName()) << "\""         
            << " name=\"" << escapeXML(test.GetShortName()) << "\""         
            << " />\n";             

        trx << "    </UnitTest>\n";
    }
    trx << "  </TestDefinitions>\n";    

    trx << "  <TestEntries>\n";
    for (const auto& test : allTests) {
        trx << "    <TestEntry testId=\"" << test.testId.ToString() << "\"" 
            << " executionId=\"" << test.executionId.ToString() << "\""
            << " testListId=\"8c84fa94-04c1-424b-9868-57a2d4851a1d\""            
            << " />\n";
    }
    trx << "  </TestEntries>\n";

    trx << "  <TestLists>\n"
        << "    <TestList name=\"Results Not in a List\" id=\"8c84fa94-04c1-424b-9868-57a2d4851a1d\" />\n"
        << "    <TestList name=\"All Loaded Results\" id=\"19431567-8539-422a-85d7-44ee4e166bda\" />\n"
        << "  </TestLists>\n";    

    trx << "  <ResultSummary outcome=\"Completed\">\n";
    trx << "    <Counters total=\"" << allTests.size()
        << "\" executed=\"" << nbExecuted
        << "\" passed=\"" << nbPasssed
        << "\" failed=\"" << nbFailed
        << "\" error=\"" << nbError
        << "\" notExecuted=\"" << nbIgnored
        << "\" timeout=\"0\" aborted=\"0\" inconclusive=\"0\" passedButRunAborted=\"0\" notRunnable=\"0\" "
        << "   />\n";
    trx << "  </ResultSummary>\n";

    trx << "</TestRun>\n";
    trx.close();
}
