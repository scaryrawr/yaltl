#include "modes/run.h"

#include "utils/spawn.h"

#include <mtl/string.hpp>

#include <cstdlib>
#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <locale>
#include <numeric>

namespace tofi
{
    namespace modes
    {
        /**
         * @brief Gets all the binaries from $PATH
         * 
         * @return std::future<std::vector<std::wstring>> 
         */
        std::future<Entries> load()
        {
            return std::async(std::launch::async, []() -> Entries {
                const char *path{std::getenv("PATH")};

                // There is no path environment
                if (!path)
                {
                    return {};
                }

                std::vector<std::string_view> paths;
                mtl::string::split(path, ":", std::back_inserter(paths));

                std::vector<std::wstring> binaries;

                for (auto &path : paths)
                {
                    if (!std::filesystem::exists(path))
                    {
                        continue;
                    }

                    std::filesystem::directory_iterator dir{path};
                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                    std::transform(std::filesystem::begin(dir), std::filesystem::end(dir), std::back_inserter(binaries), [&converter](const std::filesystem::directory_entry &entry) {
                        return converter.from_bytes(entry.path().filename().string());
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

                Entries res(binaries.size());
                std::transform(std::begin(binaries), std::end(binaries), std::begin(res), [](std::wstring &bin) {
                    return std::make_shared<Entry>(std::move(bin));
                });

                return res;
            });
        }

        run::run() : m_loader{load()}
        {
        }

        const Entries &run::results()
        {
            // Wait to finish loaded if needed
            if (m_binaries.empty() && m_loader.valid())
            {
                m_binaries = m_loader.get();
            }

            return m_binaries;
        }

        PostExec run::execute(const Entry &result, const std::wstring &text)
        {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::ostringstream spawnargs;

            // We don't trust the command that the user is typed since we have fuzzy find.
            spawnargs << converter.to_bytes(result.display) << " ";

            // User might have typed args to pass to the command as well
            std::string command = converter.to_bytes(text.c_str());
            std::vector<std::string_view> parts;
            mtl::string::split(command, " ", std::back_inserter(parts));

            // Skip the first part since it's the "incorrect" binary name
            std::copy(std::begin(parts) + 1, std::end(parts), std::ostream_iterator<std::string_view>(spawnargs, " "));

            return spawn(spawnargs.str()) ? PostExec::CloseSuccess : PostExec::CloseFailure;
        }
    } // namespace modes

} // namespace tofi
