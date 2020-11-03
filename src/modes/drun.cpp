#include "modes/drun.h"
#include "utils/command.h"
#include "utils/spawn.h"

#include <giomm/appinfo.h>
#include <giomm/init.h>
#include <mtl/string.hpp>

#include <algorithm>
#include <codecvt>
#include <locale>

namespace tofi
{
    using AppInfo = Glib::RefPtr<Gio::AppInfo>;

    struct AppEntry : public Entry
    {
        using Entry::Entry;

        AppInfo app;
    };

    namespace modes
    {
        std::wstring get_app_display(AppInfo &appinfo)
        {
            std::string description{appinfo->get_description()};
            std::string name{appinfo->get_display_name()};
            name = name.empty() ? appinfo->get_name() : name;
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

            return converter.from_bytes(description.empty() ? name : name + ": " + description);
        }

        std::future<Entries> load_apps()
        {
            static std::once_flag GIO_INIT_FLAG;
            std::call_once(GIO_INIT_FLAG, Gio::init);

            return std::async(std::launch::async, [] {
                auto apps{Gio::AppInfo::get_all()};
                Entries results(apps.size());
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::transform(std::begin(apps), std::end(apps), std::begin(results), [&converter](AppInfo appinfo) {
                    auto appResult{std::make_shared<AppEntry>(get_app_display(appinfo))};
                    appResult->app = appinfo;
                    if (appinfo->should_show())
                    {
                        std::vector<std::wstring> criteria{{
                            converter.from_bytes(appinfo->get_name()),
                            converter.from_bytes(appinfo->get_display_name()),
                            converter.from_bytes(appinfo->get_executable()),
                            converter.from_bytes(commands::parse(appinfo->get_commandline()).path.filename()),
                        }};

                        std::sort(std::begin(criteria), std::end(criteria), mtl::string::iless<wchar_t>{});
                        criteria.erase(std::unique(std::begin(criteria), std::end(criteria), mtl::string::iequals<wchar_t>{}), std::end(criteria));

                        appResult->criteria.emplace(std::move(criteria));
                    }

                    return appResult;
                });

                results.erase(std::remove_if(std::begin(results), std::end(results), [](std::shared_ptr<Entry> res) {
                                  auto ptr{reinterpret_cast<AppEntry *>(res.get())};
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
                m_entries = m_loading.get();
            }

            return m_entries;
        }

        PostExec drun::execute(const Entry &result, const std::wstring &)
        {
            const AppEntry *appResult = reinterpret_cast<const AppEntry *>(&result);
            auto &info{appResult->app};
            std::string full_command{info->get_commandline()};

            // Remove special placeholders for AppInfo entry
            mtl::string::ierase_all(full_command, "%u");
            mtl::string::ierase_all(full_command, "%f");

            return spawn(full_command) ? PostExec::CloseSuccess : PostExec::CloseFailure;
        }
    } // namespace modes

} // namespace tofi
