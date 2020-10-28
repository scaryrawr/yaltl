#pragma once

#include "../mode.h"
#include <giomm/appinfo.h>
#include <giomm/desktopappinfo.h>
#include <future>

namespace tofi
{
    namespace modes
    {
        /**
         * @brief Loads applications installed on desktop and helps user search and launch
         * 
         */
        class drun : public Mode
        {
        public:
            explicit drun();

            ~drun() = default;

            std::wstring name() const override
            {
                return L"drun";
            }

            const Entries &results() override;

            PostExec execute(const Entry &result, const std::wstring &text) override;

        private:
            Entries m_entries;
            std::future<Entries> m_loading;
        };
    } //namespace modes
} // namespace tofi
