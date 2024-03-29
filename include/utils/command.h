#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace yaltl
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
         * @return yaltl::Command 
         */
        yaltl::Command parse(std::string_view commandline);
    } // namespace commands

} // namespace yaltl
