#include "modes/run.h"

#include "utils/fuzzyresult.h"
#include "utils/spawn.h"
#include "utils/string.h"

#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <numeric>

namespace tofi
{
    using BinSearch = FuzzyResult<const wchar_t *, wchar_t>;

    namespace modes
    {
        /**
         * @brief Gets all the binaries from $PATH
         * 
         * @return std::future<std::vector<std::wstring>> 
         */
        std::future<std::vector<std::wstring>> load()
        {
            return std::async(std::launch::async, []() -> std::vector<std::wstring> {
                const char *path{std::getenv("PATH")};

                // There is no path environment
                if (!path)
                {
                    return {};
                }

                std::vector<std::string_view> paths;
                string::split<char, std::string_view>(path, ":", std::back_inserter(paths));

                std::vector<std::wstring> binaries;

                for (auto &path : paths)
                {
                    if (!std::filesystem::exists(path))
                    {
                        continue;
                    }

                    std::filesystem::directory_iterator dir{path};
                    std::transform(std::filesystem::begin(dir), std::filesystem::end(dir), std::back_inserter(binaries), [](const std::filesystem::directory_entry &entry) {
                        return string::converter.from_bytes(entry.path().filename().string());
                    });
                }

                // It's possible for a binary to be in things like /usr/bin and /usr/local/bin, and we're not in the business
                // of handling that, since it'll be handled for us when we go to spawn the command
                std::sort(std::begin(binaries), std::end(binaries));
                binaries.erase(std::unique(std::begin(binaries), std::end(binaries)), std::end(binaries));
                binaries.erase(std::remove_if(std::begin(binaries), std::end(binaries), [](const std::wstring &bin) {
                                   return bin.starts_with(L'.') || bin.starts_with(L'[') || bin.empty();
                               }),
                               std::end(binaries));
                return binaries;
            });
        }

        run::run() : m_loader{load()}
        {
        }

        Results run::results(const std::wstring &search)
        {
            // Wait to finish loaded if needed
            if (!m_binaries.has_value())
            {
                m_binaries.emplace(m_loader.get());
            }

            std::vector<BinSearch> binaries;
            binaries.reserve(m_binaries.value().size());
            std::transform(std::begin(m_binaries.value()), std::end(m_binaries.value()), std::back_inserter(binaries), [&search](const std::wstring &bin) {
                std::vector<std::wstring_view> parts;

                // This is to support the user typing arguments to pass to the binary being ran
                // such as code-insiders ~/Path/To/Repository
                string::split<wchar_t, std::wstring_view>(search, L" ", std::back_inserter(parts));

                std::wstring binstr{bin};
                return BinSearch{string::fuzzy_find<wchar_t>(binstr, parts.empty() ? L"" : parts[0]), bin.c_str()};
            });

            std::sort(std::begin(binaries), std::end(binaries));

            binaries.erase(std::remove_if(std::begin(binaries), std::end(binaries), [](const BinSearch &res) {
                               return !res.match.has_value();
                           }),
                           std::end(binaries));

            Results results;
            results.reserve(binaries.size());
            std::transform(std::begin(binaries), std::end(binaries), std::back_inserter(results), [](const BinSearch &bin) {
                return Result{bin.value, bin.value};
            });

            return results;
        }

        PostExec run::execute(const Result &result)
        {
            std::ostringstream spawnargs;

            // We don't trust the command that the user is typed since we have fuzzy find.
            const wchar_t *bin{static_cast<const wchar_t *>(result.context)};
            spawnargs << string::converter.to_bytes(bin) << " ";

            // User might have typed args to pass to the command as well
            std::string command = string::converter.to_bytes(result.display.c_str());
            std::vector<std::string_view> parts;
            string::split<char, std::string_view>(command, " ", std::back_inserter(parts));

            // Skip the first part since it's the "incorrect" binary name
            std::copy(std::begin(parts) + 1, std::end(parts), std::ostream_iterator<std::string_view>(spawnargs, " "));

            return spawn(spawnargs.str()) ? PostExec::CloseSuccess : PostExec::CloseFailure;
        }
    } // namespace modes

} // namespace tofi
