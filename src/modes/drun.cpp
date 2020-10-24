#include "modes/drun.h"
#include "utils/command.h"
#include "utils/spawn.h"
#include "utils/string.h"

#include <algorithm>
#include <iostream>

namespace tofi
{
    using AppInfo = Glib::RefPtr<Gio::AppInfo>;

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
            string::insensitive::erase_all(full_command, std::string{"%u"});
            string::insensitive::erase_all(full_command, std::string{"%f"});

            return spawn(full_command);
        }

        Results drun::results(const std::wstring &search)
        {
            std::vector<AppInfo> results;
            std::copy_if(std::begin(m_apps), std::end(m_apps), std::back_inserter(results), [&search](AppInfo appinfo) {
                if (!appinfo->should_show())
                {
                    return false;
                }

                std::vector<std::wstring> fields;
                fields.emplace_back(string::converter.from_bytes(appinfo->get_name()));
                fields.emplace_back(string::converter.from_bytes(appinfo->get_display_name()));
                fields.emplace_back(string::converter.from_bytes(appinfo->get_executable()));

                Command command{commands::parse(appinfo->get_commandline())};
                fields.emplace_back(string::converter.from_bytes(command.path.filename()));

                return std::any_of(std::begin(fields), std::end(fields), [&search](const std::wstring &field) {
                    return string::insensitive::fuzzy_contains(field, search);
                });
            });

            Results retval;
            retval.reserve(results.size());
            std::transform(std::begin(results), std::end(results), std::back_inserter(retval), [](AppInfo appinfo) {
                const std::string description{appinfo->get_description()};
                std::string name{appinfo->get_display_name()};
                name = name.empty() ? appinfo->get_name() : name;

                Command command{commands::parse(appinfo->get_commandline())};

                Result result{
                    string::converter.from_bytes(description.empty() ? name : name + ": " + description),
                    appinfo.get()};

                return result;
            });

            std::sort(std::begin(retval), std::end(retval));

            return retval;
        }
    } // namespace modes

} // namespace tofi
