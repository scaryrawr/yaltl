#include "modes/run.h"

#include "utils/fuzzyresult.h"
#include "utils/spawn.h"
#include "utils/string.h"

#include <cstdlib>
#include <numeric>

namespace tofi
{
    using BinSearch = FuzzyResult<const wchar_t *, wchar_t>;

    namespace modes
    {
        run::run() : m_path{std::getenv("PATH")}
        {
            std::vector<std::string> paths;
            string::split<char>(m_path, ":", std::back_inserter(paths));

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
            std::vector<BinSearch> binaries;
            binaries.reserve(m_binaries.size());
            std::transform(std::begin(m_binaries), std::end(m_binaries), std::back_inserter(binaries), [&search](const std::wstring &bin) {
                std::vector<std::wstring> parts;
                string::split<wchar_t>(search, L" ", std::back_inserter(parts));

                std::wstring binstr{bin};
                return BinSearch{string::fuzzy_find<wchar_t>(binstr, parts.empty() ? L"" : parts[0]), bin.c_str()};
            });

            binaries.erase(std::remove_if(std::begin(binaries), std::end(binaries), [](const BinSearch &res) {
                               return !res.match.has_value();
                           }),
                           std::end(binaries));

            std::sort(std::begin(binaries), std::end(binaries));

            Results results;
            results.reserve(binaries.size());
            std::transform(std::begin(binaries), std::end(binaries), std::back_inserter(results), [](const BinSearch &bin) {
                return Result{bin.value, bin.value};
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
            string::split<char>(command, " ", std::back_inserter(parts));

            // Incase there was a typo for the first word
            parts[0] = string::converter.to_bytes(bin);
            command = std::accumulate(std::begin(parts), std::end(parts), std::string(""), [](std::string &&init, std::string &part) {
                return init.empty() ? part : (init + " " + part);
            });

            return spawn(command);
        }
    } // namespace modes

} // namespace tofi
