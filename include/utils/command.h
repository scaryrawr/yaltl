#pragma once

#include <filesystem>
#include <vector>

namespace tofi
{
    struct Command
    {
        /**
         * @brief The path to the binary
         * 
         */
        std::filesystem::path path;

        /**
         * @brief Arguments to pass to the binary
         * 
         */
        std::vector<std::string> argv;
    };

    namespace commands
    {
        /**
         * @brief Parses the "command line" into binary path and arguments
         * 
         * @param commandline The command line to parse. i.e: firefox -h
         * @return tofi::Command 
         */
        tofi::Command parse(const std::string &commandline);
    } // namespace commands

} // namespace tofi
