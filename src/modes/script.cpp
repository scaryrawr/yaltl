#include "modes/script.h"

#include "utils/fuzzyresult.h"
#include "utils/popen.h"
#include "utils/string.h"

#include <cstdio>
#include <memory>
#include <sstream>
#include <vector>

namespace tofi
{
    using fuzzy_string = FuzzyResult<const wchar_t *, wchar_t>;

    namespace modes
    {
        std::future<std::vector<std::wstring>> async_popen(const std::string &command)
        {
            return std::async(std::launch::async, [command] {
                return popen<wchar_t>(command);
            });
        }

        script::script(std::string_view name, std::string_view script) : m_name(string::converter.from_bytes(std::string(name))),
                                                                         m_script(std::string(script)),
                                                                         m_loader(async_popen(m_script))
        {
        }

        Results script::results(const std::wstring &search)
        {
            // We run the script before the user may even come to script mode
            if (!m_results.has_value())
            {
                m_results.emplace(m_loader.get());
            }

            std::vector<fuzzy_string> fuzzy;
            fuzzy.reserve(m_results.value().size());
            std::transform(std::begin(m_results.value()), std::end(m_results.value()), std::back_inserter(fuzzy), [&search](const std::wstring &disp) {
                return fuzzy_string{string::fuzzy_find<wchar_t>(disp, search), disp.c_str()};
            });

            std::sort(std::begin(fuzzy), std::end(fuzzy));

            fuzzy.erase(std::remove_if(std::begin(fuzzy), std::end(fuzzy), [](const fuzzy_string fuz) {
                            return !fuz.match.has_value();
                        }),
                        std::end(fuzzy));

            Results results;
            results.reserve(fuzzy.size());
            std::transform(std::begin(fuzzy), std::end(fuzzy), std::back_inserter(results), [](const fuzzy_string &fuz) {
                return Result{fuz.value, fuz.value};
            });

            return results;
        }

        PostExec script::execute(const Result &result)
        {
            // Build the command to run to see if the script keeps going or has exited nicely
            std::ostringstream cmd;
            cmd << m_script << " " << string::converter.to_bytes(static_cast<const wchar_t *>(result.context));

            // Don't load async since user since user is interacting with us anyways
            m_results.emplace(popen<wchar_t>(cmd.str()));

            return m_results.value().empty() ? PostExec::CloseSuccess : PostExec::StayOpen;
        }
    } // namespace modes

} // namespace tofi
