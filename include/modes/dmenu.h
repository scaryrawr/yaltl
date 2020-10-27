#pragma once

#include "../mode.h"
#include "utils/popen.h"

namespace tofi
{
    namespace modes
    {
        class dmenu : public Mode
        {
        public:
            dmenu();
            ~dmenu();

            std::wstring
            name() const override
            {
                return L"dmenu";
            }

            Results results(const std::wstring &search) override;

            PostExec execute(const Result &result) override;

        private:
            std::vector<std::wstring> m_lines;
            std::optional<int> m_fdout{};
            std::optional<int> m_fdin{};
            unique_file m_ttyIn;
            unique_file m_ttyOut;
        };
    } // namespace modes

} // namespace tofi
