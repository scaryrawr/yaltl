#include "modes/run.h"

#include "utils/spawn.h"

#include <mtl/string.hpp>

#include <cstdlib>
#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <locale>
#include <numeric>
#include <sstream>

#ifdef WIN32
#define PATH_DELIM ";"
constexpr bool INCLUDE_COMMAND{false};
#else
#define PATH_DELIM ":"
constexpr bool INCLUDE_COMMAND{true};
#endif

namespace cofi
{
    namespace modes
    {
        struct RunEntry : public Entry
        {
            using Entry::Entry;

            // Windows CreateProcess requires full path to binary since it doesn't perform path look up
            std::filesystem::path path;
        };

        /**
         * @brief Gets all the binaries from $PATH
         * 
         * @return std::future<std::vector<std::wstring>> 
         */
        std::future<Entries> load()
        {
            return std::async(std::launch::async, []() -> Entries {
                const char *environmentPath{std::getenv("PATH")};

                // There is no path environment
                if (!environmentPath)
                {
                    return {};
                }

                std::vector<std::string_view> paths;
                mtl::string::split(environmentPath, PATH_DELIM, std::back_inserter(paths));

                std::vector<std::filesystem::path> binpaths;
                for (auto &path : paths)
                {
                    if (!std::filesystem::exists(path))
                    {
                        continue;
                    }

                    std::filesystem::directory_iterator dir{path};

                    std::transform(std::filesystem::begin(dir), std::filesystem::end(dir), std::back_inserter(binpaths), [](const std::filesystem::directory_entry &entry) {
                        return entry.path();
                    });
                }

                binpaths.erase(std::remove_if(std::begin(binpaths), std::end(binpaths), [](const std::filesystem::path &path) {
#ifdef WIN32
                                   return !(path.string().ends_with("exe") ||
                                            path.string().ends_with("bat") ||
                                            path.string().ends_with("EXE") ||
                                            path.string().ends_with("BAT"));
#else
                                    const std::string bin{path.filename().string()};
                                    return bin.starts_with('.') || bin.starts_with('[') || bin.empty();
#endif
                               }),
                               std::end(binpaths));

                Entries entries;
                entries.reserve(binpaths.size());
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::transform(std::begin(binpaths), std::end(binpaths), std::back_inserter(entries), [&converter](std::filesystem::path path) {
                    auto entry{std::make_shared<RunEntry>(converter.from_bytes(path.filename().string()))};
                    entry->path = std::move(path);

                    return entry;
                });

                std::sort(std::begin(entries), std::end(entries), [](const std::shared_ptr<Entry> &lhs, const std::shared_ptr<Entry> &rhs) {
                    return lhs->display < rhs->display;
                });

                entries.erase(std::unique(std::begin(entries), std::end(entries), [](const std::shared_ptr<Entry> &lhs, const std::shared_ptr<Entry> &rhs) {
                                  return lhs->display == rhs->display;
                              }),
                              std::end(entries));

                return entries;
            });
        }

        run::run() : m_loader{load()}
        {
        }

        const Entries &run::Results()
        {
            // Wait to finish loaded if needed
            if (m_binaries.empty() && m_loader.valid())
            {
                m_binaries = m_loader.get();
            }

            return m_binaries;
        }

        PostExec run::Execute(const Entry &result, const std::wstring &text)
        {
            const RunEntry *entry{reinterpret_cast<const RunEntry *>(&result)};
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            Command command;
            command.path = entry->path;

            // User might have typed args to pass to the command as well
            std::string input{converter.to_bytes(text.c_str())};
            std::vector<std::string_view> parts;
            mtl::string::split(input, " ", std::back_inserter(parts));

            command.argv.reserve(parts.size());
            if (INCLUDE_COMMAND)
            {
                command.argv.push_back(command.path.string());
            }

            std::transform(std::begin(parts) + 1, std::end(parts), std::back_inserter(command.argv), [](std::string_view part) {
                return std::string{part};
            });

            return spawn(command) ? PostExec::CloseSuccess : PostExec::CloseFailure;
        }
    } // namespace modes

} // namespace cofi
