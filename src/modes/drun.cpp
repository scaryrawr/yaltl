#include "modes/drun.h"
#include "utils/command.h"
#include "utils/fuzzyresult.h"
#include "utils/spawn.h"
#include "utils/string.h"

#include <algorithm>
#include <iostream>

namespace tofi
{
    using AppInfo = Glib::RefPtr<Gio::AppInfo>;

    using AppSearch = FuzzyResult<AppInfo, wchar_t>;

    namespace modes
    {

        drun::drun() : m_apps{Gio::AppInfo::get_all()}
        {
        }

        bool drun::execute(const Result &result)
        {
            auto info{static_cast<const Gio::AppInfo *>(result.context)};
            std::string full_command{info->get_commandline()};

            // Remove special placeholders for AppInfo entry
            string::insensitive::erase_all<char>(full_command, "%u");
            string::insensitive::erase_all<char>(full_command, "%f");

            return spawn(full_command);
        }

        Results drun::results(const std::wstring &search)
        {
            std::vector<AppSearch> results;
            std::transform(std::begin(m_apps), std::end(m_apps), std::back_inserter(results), [&search](AppInfo appinfo) {
                if (!appinfo->should_show())
                {
                    return AppSearch{std::nullopt, appinfo};
                }

                std::vector<std::wstring> fields;
                fields.emplace_back(string::converter.from_bytes(appinfo->get_name()));
                fields.emplace_back(string::converter.from_bytes(appinfo->get_display_name()));
                fields.emplace_back(string::converter.from_bytes(appinfo->get_executable()));

                Command command{commands::parse(appinfo->get_commandline())};
                fields.emplace_back(string::converter.from_bytes(command.path.filename()));

                std::vector<std::optional<std::wstring>> matches;
                std::transform(std::begin(fields), std::end(fields), std::back_inserter(matches), [&search](const std::wstring &field) {
                    return string::fuzzy_find<wchar_t>(field, search);
                });

                matches.erase(std::remove_if(std::begin(matches), std::end(matches), [](auto &val) {
                                  return !val.has_value();
                              }),
                              std::end(matches));

                std::sort(std::begin(matches), std::end(matches), [](auto &lhs, auto &rhs) {
                    return lhs.value().length() < rhs.value().length();
                });

                if (matches.empty())
                {
                    return AppSearch{std::nullopt, appinfo};
                }

                return AppSearch{std::move(matches[0]), appinfo};
            });

            results.erase(std::remove_if(std::begin(results), std::end(results), [](const AppSearch &res) {
                              return !res.match.has_value();
                          }),
                          std::end(results));

            std::sort(std::begin(results), std::end(results));

            Results retval;
            retval.reserve(results.size());
            std::transform(std::begin(results), std::end(results), std::back_inserter(retval), [](const AppSearch &res) {
                AppInfo appinfo = res.value;
                const std::string description{appinfo->get_description()};
                std::string name{appinfo->get_display_name()};
                name = name.empty() ? appinfo->get_name() : name;

                Command command{commands::parse(appinfo->get_commandline())};

                Result result{
                    string::converter.from_bytes(description.empty() ? name : name + ": " + description),
                    appinfo.get()};

                return result;
            });

            return retval;
        }
    } // namespace modes

} // namespace tofi
