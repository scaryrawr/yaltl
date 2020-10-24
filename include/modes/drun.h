#pragma once

#include "../mode.h"
#include <giomm/appinfo.h>
#include <giomm/desktopappinfo.h>

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

            bool execute(const Result &result) override;

        private:
            Glib::ListHandle<Glib::RefPtr<Gio::AppInfo>> m_apps;
        };
    } //namespace modes
} // namespace tofi
