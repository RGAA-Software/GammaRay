//
// Created by RGAA on 22/01/2025.
//

#include <thread>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

#define LOGGER 0

void saveLog(const std::string& log) {
#if LOGGER
    static std::ofstream file("shadow_deleter.txt", std::ios::out);
    file.write(log.c_str(), log.size());
    file.write("\n", 1);
    file.flush();
#endif
}

int main(int argc, char** argv) {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    fs::path exe_path = fs::canonical(fs::path(argv[0])).parent_path();
#if LOGGER
    file.write((char*)exe_path.string().c_str(), exe_path.string().size());
    file.write("\n", 1);
    file.flush();
#endif

    for (auto const& entry : std::filesystem::directory_iterator{exe_path}) {
        saveLog(std::format("path: {}", entry.path().string()));
        if (fs::is_directory(entry)) {
            if (fs::remove_all(entry.path())) {
                saveLog(std::format("OK path: {}", entry.path().string()));
            }
            else {
                saveLog(std::format("FAILED path: {}", entry.path().string()));
            }
        }
        else if (fs::is_regular_file(entry)) {
            if (entry.path().string().find("shadow_deleter") != std::string::npos) {
                continue;
            }
            if (fs::remove(entry.path())) {
                saveLog(std::format("OK path: {}", entry.path().string()));
            }
            else {
                saveLog(std::format("FAILED path: {}", entry.path().string()));
            }
        }
    }

    return 0;
}
