#pragma once

#include "../mode.h"

#include <future>
#include <optional>
#include <vector>
#include <filesystem>

namespace tofi
{
    namespace modes
    {
        /**
         * @brief Finds binaries on path for user to search through and launch possibly with parameters.
         * 
         */
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
            std::future<std::vector<std::wstring>> m_loader;
            std::optional<std::vector<std::wstring>> m_binaries;
        };
    } // namespace modes

} // namespace tofi
