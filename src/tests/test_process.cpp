//
// Created by RGAA  on 2024/5/24.
//
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include <iostream>
#include "tc_common_new/process_util.h"

using namespace tc;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

TEST(TestProcess, StartProcess) {
    auto ret = ProcessUtil::StartProcess("C:/software/u3d_demo/u3d_demo/u3d_shared_window_demo.exe", {}, true, false);
    std::cout << "result: " << ret << std::endl;
}