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
            std::vector<std::string_view> paths;
            string::split<char, std::string_view>(m_path, ":", std::back_inserter(paths));

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
                std::vector<std::wstring_view> parts;
                string::split<wchar_t, std::wstring_view>(search, L" ", std::back_inserter(parts));

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

        PostExec run::execute(const Result &result)
        {
            // User might have typed args to pass to the command as well
            const wchar_t *bin{static_cast<const wchar_t *>(result.context)};
            std::string command = string::converter.to_bytes(result.display.c_str());
            std::vector<std::string_view> parts;
            string::split<char, std::string_view>(command, " ", std::back_inserter(parts));

            std::ostringstream spawnargs;

            // We may have showed early results, so use what was from the list
            spawnargs << string::converter.to_bytes(bin) << " ";

            std::copy(std::begin(parts) + 1, std::end(parts), std::ostream_iterator<std::string_view>(spawnargs, " "));

            return spawn(spawnargs.str()) ? PostExec::CloseSuccess : PostExec::CloseFailure;
        }
    } // namespace modes

} // namespace tofi
