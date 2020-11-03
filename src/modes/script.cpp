#include "modes/script.h"

#include "utils/popen.h"

#include <cstdio>
#include <memory>
#include <sstream>
#include <vector>

namespace tofi
{
    namespace modes
    {
        std::future<Entries> async_popen(const std::string &command)
        {
            return std::async(std::launch::async, [command] {
                std::vector<std::wstring> contents{popen(command)};
                Entries res(contents.size());
                std::transform(std::begin(contents), std::end(contents), std::begin(res), [](std::wstring &line) {
                    return std::make_shared<Entry>(std::move(line));
                });

                return res;
            });
        }

        script::script(std::string_view name, std::string_view script) : m_name(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(std::string(name))),
                                                                         m_script(std::string(script)),
                                                                         m_loader(async_popen(m_script))
        {
        }

        const Entries &script::results()
        {
            // We run the script before the user may even come to script mode
            if (m_results.empty() && m_loader.valid())
            {
                m_results = m_loader.get();
            }

            return m_results;
        }

        PostExec script::execute(const Entry &result, const std::wstring &)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

            // Build the command to run to see if the script keeps going or has exited nicely
            std::ostringstream cmd;
            cmd << m_script << " " << converter.to_bytes(result.display);

            // Don't load async since user since user is interacting with us anyways
            std::vector<std::wstring> output{popen(cmd.str())};
            m_results.resize(output.size());
            std::transform(std::begin(output), std::end(output), std::begin(m_results), [](std::wstring &line) {
                return std::make_shared<Entry>(std::move(line));
            });

            return m_results.empty() ? PostExec::CloseSuccess : PostExec::StayOpen;
        }
    } // namespace modes

} // namespace tofi
