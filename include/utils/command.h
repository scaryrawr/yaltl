#pragma once

#include <filesystem>
#include <vector>

namespace tofi
{
    struct Command
    {
        std::filesystem::path path;
        std::vector<std::string> argv;
    };

    namespace commands
    {
        tofi::Command parse(const std::string &commandline);
    } // namespace commands

} // namespace tofi
