#include "../dirEntry.h"
#include "../ff_actions.h"
#include "mockFile.hpp"
#include <sstream>
#include <iostream>
#include <CppUTest/TestHarness.h>

TEST_GROUP(dirEntryTest)
{
    
};

TEST(dirEntryTest, canReadDirEntry)
{
    mockFile_init(32);
    ff_setFuncs(mockFile_mockRead, mockFile_mockWrite, mockFile_mockLseek);

    mockFile_close();
}
