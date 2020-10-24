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

            std::wstring name() const
            {
                return L"run";
            }

            Results results(const std::wstring &search);

            bool execute(const Result &result);

        private:
            std::string m_path;
            std::vector<std::wstring> m_binaries;
        };
    } // namespace modes

} // namespace tofi
