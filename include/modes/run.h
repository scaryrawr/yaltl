#pragma once

#include "../mode.h"

#include <vector>
#include <filesystem>

namespace tofi
{
    namespace modes
    {
        class run : public Mode
        {
        public:
            explicit run();

            std::wstring name() const override
            {
                return L"run";
            }

            Results results(const std::wstring &search) override;

            PostExec execute(const Result &result) override;

        private:
            std::string m_path;
            std::vector<std::wstring> m_binaries;
        };
    } // namespace modes

} // namespace tofi
