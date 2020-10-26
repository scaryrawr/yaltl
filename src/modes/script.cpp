#include "modes/script.h"

#include "utils/fuzzyresult.h"
#include "utils/string.h"

#include <cstdio>
#include <memory>
#include <sstream>
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

    using unique_file = std::unique_ptr<FILE, pclose>;
    using fuzzy_string = FuzzyResult<const wchar_t *, wchar_t>;

    namespace modes
    {
        std::vector<std::wstring> popen(const std::string &command)
        {
            unique_file file{::popen(command.c_str(), "r")};
            if (!file)
            {
                // failed to open script
                return {};
            }

            std::vector<std::wstring> lines;
            {
                std::ostringstream contents;
                char buffer[1024];
                while (fgets(buffer, sizeof(buffer), file.get()))
                {
                    contents << buffer;
                }

                string::split<wchar_t, std::wstring>(string::converter.from_bytes(contents.str()), L"\n", std::back_inserter(lines));
            }

            return lines;
        }

        script::script(std::string_view name, std::string_view script) : m_name(string::converter.from_bytes(std::string(name))),
                                                                         m_script(std::string(script)),
                                                                         m_results(popen(m_script.c_str()))
        {
        }

        Results script::results(const std::wstring &search)
        {
            std::vector<fuzzy_string> fuzzy;
            fuzzy.reserve(m_results.size());
            std::transform(std::begin(m_results), std::end(m_results), std::back_inserter(fuzzy), [&search](const std::wstring &disp) {
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

        PostExec script::execute(const Result &result)
        {
            std::ostringstream cmd;
            cmd << m_script << " " << string::converter.to_bytes(static_cast<const wchar_t *>(result.context));
            m_results = popen(cmd.str());
            return m_results.empty() ? PostExec::CloseSuccess : PostExec::StayOpen;
        }
    } // namespace modes

} // namespace tofi
