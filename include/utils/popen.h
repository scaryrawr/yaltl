#pragma once

#include "utils/string.h"

#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace tofi
{
    struct pclose
    {
        void operator()(FILE *stream)
        {
            ::pclose(stream);
        }
    };

    struct fclose
    {
        void operator()(FILE *file)
        {
            ::fclose(file);
        }
    };

    using unique_file = std::unique_ptr<FILE, fclose>;

    using unique_pfile = std::unique_ptr<FILE, pclose>;

    /**
     * @brief Reads all output from a command
     * 
     * @tparam CharT wide or regular char to return results as
     * @param command The command to run with popen
     * @return std::vector<std::basic_string<CharT>> Collection of lines that were outputted by the command
     */
    template <class CharT>
    std::vector<std::basic_string<CharT>> popen(const std::string &command)
    {
        unique_pfile file{::popen(command.c_str(), "r")};
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

            if constexpr (sizeof(CharT) == sizeof(char))
            {
                string::split<CharT, std::basic_string<CharT>>(contents.str(), "\n", std::back_inserter(lines));
            }
            else
            {
                string::split<CharT, std::basic_string<CharT>>(string::converter.from_bytes(contents.str()), L"\n", std::back_inserter(lines));
            }

            lines.erase(std::remove_if(std::begin(lines), std::end(lines), [](const std::basic_string<CharT> &line) {
                            return line.empty();
                        }),
                        std::end(lines));
        }

        return lines;
    }
} // namespace tofi
