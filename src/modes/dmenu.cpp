#include "modes/dmenu.h"

#include "utils/popen.h"

#include <mtl/string.hpp>

#include <algorithm>
#include <cstdio>
#include <codecvt>
#include <iostream>
#include <locale>

#ifdef WIN32
#include <io.h>
#include <Windows.h>
constexpr auto CONSOLE_INPUT = "CONIN$";
constexpr auto CONSOLE_OUTPUT = "CONOUT$";
#define STDOUT_FILENO _fileno(stdout)
#define STDIN_FILENO _fileno(stdin)
#else
constexpr auto CONSOLE_INPUT = "/dev/tty";
constexpr auto CONSOLE_OUTPUT = "/dev/tty";
#include <unistd.h>
#endif

namespace yaltl
{
    namespace modes
    {
        /**
         * @brief Loads all lines from stdin
         * 
         * @return std::vector<std::wstring> The lines read from stdin
         */
        Entries load_stdin()
        {
            std::vector<char> buffer;
            char buff[1024]{};
            for (size_t size{fread(buff, 1, sizeof(buff), stdin)}; size > 0; size = fread(buff, 1, sizeof(buff), stdin))
            {
                buffer.insert(std::end(buffer), buff, buff + size);
            }

            buffer.insert(std::end(buffer), '\0');

            std::vector<std::string_view> rawLines;
            rawLines.reserve(std::count(std::begin(buffer), std::end(buffer), '\n') + 1);
            mtl::string::split(buffer.data(), "\n", std::back_inserter(rawLines));

            Entries lines;
            lines.reserve(rawLines.size());
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::transform(std::begin(rawLines), std::end(rawLines), std::back_inserter(lines), [&converter](std::string_view line) {
                return std::make_shared<Entry>(converter.from_bytes(line.data(), line.data() + line.size()));
            });

            return lines;
        }

        dmenu::dmenu() : m_entries(load_stdin()),                       // Load stdin before re-routing I/O
                         m_stdoutCopy{dup(STDOUT_FILENO)},              // Save off stdout, this is what gets piped to the next process
                         m_stdinCopy{dup(STDIN_FILENO)},                // Save off stdin, probably not important...
                         m_ttyIn{freopen(CONSOLE_INPUT, "r", stdin)},   // Open up the tty for input (otherwise the user can't interact with yaltl)
                         m_ttyOut{freopen(CONSOLE_OUTPUT, "w", stdout)} // Open up the tty for output (otherwise yaltl won't render).
        {
#ifdef WIN32
            std::ios::sync_with_stdio(true);

            std::cout.clear();
            std::wcout.clear();
            std::cin.clear();
            std::wcin.clear();
            setvbuf(stdout, nullptr, _IONBF, 0);
            setvbuf(stdin, nullptr, _IONBF, 0);
#endif
        }

        dmenu::~dmenu()
        {
            // Restore original stdout and stdin
            dup2(m_stdoutCopy, STDOUT_FILENO);
            dup2(m_stdinCopy, STDIN_FILENO);
        }

        PostExec dmenu::Execute(const Entry &result, const std::wstring &)
        {
            // Restore the original stdout so we can write to the next process in the pipeline
            int tty{dup(STDOUT_FILENO)};
            dup2(m_stdoutCopy, STDOUT_FILENO);

            std::wcout << result.display << std::endl;

            dup2(tty, STDOUT_FILENO);

            return PostExec::CloseSuccess;
        }
    } // namespace modes

} // namespace yaltl
