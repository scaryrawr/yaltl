#include "modes/dmenu.h"

#include "utils/popen.h"

#include <unistd.h>
#include <cstdio>
#include <iostream>

namespace tofi
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
            Entries lines;
            for (std::wstring tmp; std::getline(std::wcin, tmp);)
            {
                lines.emplace_back(std::make_shared<Entry>(std::move(tmp)));
            }

            return lines;
        }

        dmenu::dmenu() : m_entries(load_stdin()),                    // Load stdin before re-routing I/O
                         m_stdoutCopy{dup(STDOUT_FILENO)},           // Save off stdout, this is what gets piped to the next process
                         m_stdinCopy{dup(STDIN_FILENO)},             // Save off stdin, probably not important...
                         m_ttyOut{freopen("/dev/tty", "a", stdout)}, // Open up the tty for output (otherwise tofi won't render).
                         m_ttyIn{freopen("/dev/tty", "r", stdin)}    // Open up the tty for input (otherwise the user can't interact with tofi)

        {
        }

        dmenu::~dmenu()
        {
            // Restore original stdout and stdin
            if (m_stdoutCopy.has_value())
            {
                dup2(m_stdoutCopy.value(), STDOUT_FILENO);
            }

            if (m_stdinCopy.has_value())
            {
                dup2(m_stdinCopy.value(), STDIN_FILENO);
            }
        }

        PostExec dmenu::execute(const Entry &result, const std::wstring &)
        {
            // Restore the original stdout so we can write to the next process in the pipeline
            int tty{dup(STDOUT_FILENO)};
            dup2(m_stdoutCopy.value(), STDOUT_FILENO);

            std::wcout << result.display << std::endl;

            dup2(tty, STDOUT_FILENO);

            return PostExec::CloseSuccess;
        }
    } // namespace modes

} // namespace tofi
