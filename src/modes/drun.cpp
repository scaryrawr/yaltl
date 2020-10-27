#include "modes/drun.h"
#include "glibmm/listhandle.h"
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
        std::future<Glib::ListHandle<AppInfo>> load_apps()
        {
            return std::async(std::launch::async, [] {
                return Gio::AppInfo::get_all();
            });
        }

        drun::drun() : m_load{load_apps()}
        {
        }

        Results drun::results(const std::wstring &search)
        {
            // Get results from launch, be willing to wait
            if (!m_apps.has_value())
            {
                m_apps.emplace(std::move(m_apps.value_or(m_load.get())));
            }

            // Get fuzzy factors
            std::vector<AppSearch> results;
            results.reserve(m_apps.value().size());
            std::transform(std::begin(m_apps.value()), std::end(m_apps.value()), std::back_inserter(results), [&search](AppInfo appinfo) {
                if (!appinfo->should_show())
                {
                    return AppSearch{std::nullopt, appinfo};
                }

                // Collections of possible strings from the app information to use for fuzzy factors
                std::array<std::wstring, 4> fields{{
                    string::converter.from_bytes(appinfo->get_name()),
                    string::converter.from_bytes(appinfo->get_display_name()),
                    string::converter.from_bytes(appinfo->get_executable()),
                    string::converter.from_bytes(commands::parse(appinfo->get_commandline()).path.filename()),
                }};

                // Get all the fuzzy factors
                std::vector<std::optional<std::wstring>> matches;
                matches.reserve(fields.size());
                std::transform(std::begin(fields), std::end(fields), std::back_inserter(matches), [&search](const std::wstring &field) {
                    return string::fuzzy_find<wchar_t>(field, search);
                });

                std::sort(std::begin(matches), std::end(matches));

                // Only use the best fuzzy factor
                return AppSearch{std::move(matches[0]), appinfo};
            });

            std::sort(std::begin(results), std::end(results));

            // Get rid of anything that didn't have a fuzzy factor
            results.erase(std::remove_if(std::begin(results), std::end(results), [](const AppSearch &res) {
                              return !res.match.has_value();
                          }),
                          std::end(results));

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

        PostExec drun::execute(const Result &result)
        {
            auto info{static_cast<const Gio::AppInfo *>(result.context)};
            std::string full_command{info->get_commandline()};

            // Remove special placeholders for AppInfo entry
            string::insensitive::erase_all<char>(full_command, "%u");
            string::insensitive::erase_all<char>(full_command, "%f");

            return spawn(full_command) ? PostExec::CloseSuccess : PostExec::CloseFailure;
        }
    } // namespace modes

} // namespace tofi
