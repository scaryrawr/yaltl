#include "modes/dmenu.h"

#include "utils/fuzzyresult.h"
#include "utils/popen.h"

#include <unistd.h>
#include <cstdio>
#include <iostream>

namespace tofi
{
    using fuzzy_string = FuzzyResult<const wchar_t *, wchar_t>;

    namespace modes
    {
        std::vector<std::wstring> load_stdin()
        {
            std::vector<std::wstring> lines;
            for (std::wstring tmp; std::getline(std::wcin, tmp);)
            {
                lines.emplace_back(tmp);
            }

            return lines;
        }

        dmenu::dmenu() : m_lines(load_stdin()),
                         m_fdout{dup(STDOUT_FILENO)},
                         m_fdin{dup(STDIN_FILENO)},
                         m_ttyOut{freopen("/dev/tty", "a", stdout)},
                         m_ttyIn{freopen("/dev/tty", "r", stdin)}

        {
        }

        dmenu::~dmenu()
        {
            if (m_fdout.has_value())
            {
                dup2(m_fdout.value(), STDOUT_FILENO);
                close(m_fdout.value());
            }

            if (m_fdin.has_value())
            {
                dup2(m_fdin.value(), STDIN_FILENO);
                close(m_fdin.value());
            }
        }

        Results dmenu::results(const std::wstring &search)
        {
            std::vector<fuzzy_string> fuzzy;
            fuzzy.reserve(m_lines.size());
            std::transform(std::begin(m_lines), std::end(m_lines), std::back_inserter(fuzzy), [&search](const std::wstring &disp) {
                return fuzzy_string{string::fuzzy_find<wchar_t>(disp, search), disp.c_str()};
            });

            fuzzy.erase(std::remove_if(std::begin(fuzzy), std::end(fuzzy), [](const fuzzy_string fuz) {
                            return !fuz.match.has_value();
                        }),
                        std::end(fuzzy));

            std::sort(std::begin(fuzzy), std::end(fuzzy));

            Results results;
            results.reserve(fuzzy.size());
            std::transform(std::begin(fuzzy), std::end(fuzzy), std::back_inserter(results), [](const fuzzy_string &fuz) {
                return Result{fuz.value, fuz.value};
            });

            return results;
        }

        PostExec dmenu::execute(const Result &result)
        {
            dup2(m_fdout.value(), STDOUT_FILENO);
            close(m_fdout.value());
            m_fdout.reset();
            std::wcout << static_cast<const wchar_t *>(result.context) << std::endl;

            close(STDOUT_FILENO);

            return PostExec::CloseSuccess;
        }
    } // namespace modes

} // namespace tofi
