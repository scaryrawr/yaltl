#include "modes/run.h"

#include "utils/spawn.h"
#include "utils/string.h"

#include <cstdlib>
#include <numeric>

namespace tofi
{
    namespace modes
    {
        run::run() : m_path{std::getenv("PATH")}
        {
            std::vector<std::string> paths;
            string::split(m_path, std::string{":"}, std::back_inserter(paths));

            for (auto &path : paths)
            {
                std::filesystem::directory_iterator dir{path};
                std::transform(std::filesystem::begin(dir), std::filesystem::end(dir), std::back_inserter(m_binaries), [](const std::filesystem::directory_entry &entry) {
                    return string::converter.from_bytes(entry.path().filename().string());
                });
            }

            std::sort(std::begin(m_binaries), std::end(m_binaries));
            m_binaries.erase(std::unique(std::begin(m_binaries), std::end(m_binaries)), std::end(m_binaries));
            m_binaries.erase(std::remove_if(std::begin(m_binaries), std::end(m_binaries), [](const std::wstring &bin) {
                                 return bin.starts_with(L'.') || bin.starts_with(L'[') || bin.empty();
                             }),
                             std::end(m_binaries));
        }

        Results run::results(const std::wstring &search)
        {
            std::vector<const wchar_t *> binaries;
            binaries.reserve(m_binaries.size());
            std::transform(std::begin(m_binaries), std::end(m_binaries), std::back_inserter(binaries), [](const std::wstring &bin) {
                return bin.c_str();
            });

            std::vector<const wchar_t *> filtered;
            std::copy_if(std::begin(binaries), std::end(binaries), std::back_inserter(filtered), [&search](const wchar_t *bin) {
                if (search.empty())
                {
                    return true;
                }

                std::vector<std::wstring> parts;
                string::split(search, std::wstring(L" "), std::back_inserter(parts));

                std::wstring binstr{bin};
                return string::insensitive::fuzzy_contains(binstr, parts[0]);
            });

            Results results;
            results.reserve(filtered.size());
            std::transform(std::begin(filtered), std::end(filtered), std::back_inserter(results), [](const wchar_t *bin) {
                return Result{bin, bin};
            });

            return results;
        }

        bool run::execute(const Result &result)
        {
            // User might have typed args to pass to the command as well
            const wchar_t *bin{static_cast<const wchar_t *>(result.context)};
            const wchar_t *cmd{wcslen(bin) > result.display.length() ? bin : result.display.c_str()};
            std::string command = string::converter.to_bytes(cmd);
            std::vector<std::string> parts;
            string::split(command, std::string(" "), std::back_inserter(parts));

            // Incase there was a typo for the first word
            parts[0] = string::converter.to_bytes(bin);
            command = std::accumulate(std::begin(parts), std::end(parts), std::string(""), [](std::string &&init, std::string &part) {
                return init.empty() ? part : (init + " " + part);
            });

            return spawn(command);
        }
    } // namespace modes

} // namespace tofi
