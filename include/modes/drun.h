#pragma once

#include "../mode.h"
#include <giomm/appinfo.h>
#include <giomm/desktopappinfo.h>
#include <future>

namespace tofi
{
    namespace modes
    {
        class drun : public Mode
        {
        public:
            explicit drun();

            ~drun() = default;

            std::wstring name() const override
            {
                return L"drun";
            }

            Results results(const std::wstring &search) override;

            PostExec execute(const Result &result) override;

        private:
            std::future<Glib::ListHandle<Glib::RefPtr<Gio::AppInfo>>> m_load;
            std::optional<Glib::ListHandle<Glib::RefPtr<Gio::AppInfo>>> m_apps;
        };
    } //namespace modes
} // namespace tofi
