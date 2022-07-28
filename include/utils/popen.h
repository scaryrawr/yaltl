#pragma once

#include <stdio.h>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

#include <mtl/memory.hpp>
#include <mtl/string.hpp>

namespace cofi
{
    using unique_file = mtl::unique_ptr<decltype(::fclose), &::fclose>;

#ifdef WIN32
    using unique_pfile = mtl::unique_ptr<decltype(::_pclose), &::_pclose>;
#else
    using unique_pfile = mtl::unique_ptr<decltype(::pclose), &::pclose>;
#endif

    /**
     * @brief Reads all output from a command
     *
     * @tparam CharT wide or regular char to return results as
     * @param command The command to run with popen
     * @return std::vector<std::basic_string<CharT>> Collection of lines that were outputted by the command
     */
    template <class CharT = wchar_t>
    std::vector<std::basic_string<CharT>> popen(const std::string &command)
    {
#ifdef WIN32
        unique_pfile file{::_popen(command.c_str(), "r")};
#else
        unique_pfile file{::popen(command.c_str(), "r")};
#endif
        if (!file)
        {
            // failed to open script
            return {};
        }

        std::vector<std::basic_string<CharT>> lines;
        {
            std::ostringstream contents;
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), file.get()))
            {
                contents << buffer;
            }

            const CharT *delim{};
            if constexpr (sizeof(CharT) == sizeof(char))
            {
                delim = "\n";
                mtl::string::split<CharT, std::basic_string<CharT>>(contents.str(), delim, std::back_inserter(lines));
            }
            else
            {
                delim = L"\n";
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                mtl::string::split<CharT, std::basic_string<CharT>>(converter.from_bytes(contents.str()), delim, std::back_inserter(lines));
            }

            lines.erase(std::remove_if(std::begin(lines), std::end(lines), [](const std::basic_string<CharT> &line)
                                       { return line.empty(); }),
                        std::end(lines));
        }

        return lines;
    }
} // namespace cofi
