#include "modes/drun.h"
#include "glibmm/listhandle.h"
#include "utils/command.h"
#include "utils/spawn.h"
#include "utils/string.h"

#include <mtl/string.hpp>

#include <algorithm>
#include <iostream>

namespace tofi
{
    using AppInfo = Glib::RefPtr<Gio::AppInfo>;

    struct AppResult : public Entry
    {
        AppResult(std::wstring &&name) : Entry(std::move(name))
        {
        }

        AppInfo app;
    };

    namespace modes
    {
        std::wstring get_app_display(AppInfo &appinfo)
        {
            std::string description{appinfo->get_description()};
            std::string name{appinfo->get_display_name()};
            name = name.empty() ? appinfo->get_name() : name;

            return string::converter.from_bytes(description.empty() ? name : name + ": " + description);
        }

        std::future<Entries> load_apps()
        {
            return std::async(std::launch::async, [] {
                auto apps{Gio::AppInfo::get_all()};
                Entries results(apps.size());
                std::transform(std::begin(apps), std::end(apps), std::begin(results), [](AppInfo appinfo) {
                    auto appResult{std::make_shared<AppResult>(get_app_display(appinfo))};
                    appResult->app = appinfo;
                    if (appinfo->should_show())
                    {
                        std::vector<std::wstring> criteria{{
                            string::converter.from_bytes(appinfo->get_name()),
                            string::converter.from_bytes(appinfo->get_display_name()),
                            string::converter.from_bytes(appinfo->get_executable()),
                            string::converter.from_bytes(commands::parse(appinfo->get_commandline()).path.filename()),
                        }};

                        std::sort(std::begin(criteria), std::end(criteria), mtl::string::iless<wchar_t>{});
                        criteria.erase(std::unique(std::begin(criteria), std::end(criteria), mtl::string::iequals<wchar_t>{}), std::end(criteria));

                        appResult->criteria.emplace(std::move(criteria));
                    }

                    return appResult;
                });

                results.erase(std::remove_if(std::begin(results), std::end(results), [](std::shared_ptr<Entry> res) {
                                  auto ptr{reinterpret_cast<AppResult *>(res.get())};
                                  return !ptr->app->should_show();
                              }),
                              std::end(results));

                return results;
            });
        }

        drun::drun() : m_loading{load_apps()}
        {
        }

        const Entries &drun::results()
        {
            // Get results from launch, be willing to wait
            if (m_entries.empty() && m_loading.valid())
            {
                m_entries = std::move(m_loading.get());
            }

            return m_entries;
        }

        PostExec drun::execute(const Entry &result, const std::wstring &)
        {
            const AppResult *appResult = reinterpret_cast<const AppResult *>(&result);
            auto &info{appResult->app};
            std::string full_command{info->get_commandline()};

            // Remove special placeholders for AppInfo entry
            mtl::string::ierase_all(full_command, "%u");
            mtl::string::ierase_all(full_command, "%f");

            return spawn(full_command) ? PostExec::CloseSuccess : PostExec::CloseFailure;
        }
    } // namespace modes

} // namespace tofi
