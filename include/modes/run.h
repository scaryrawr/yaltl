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

            std::wstring Name() const override
            {
                return L"run";
            }

            const Entries &Results() override;

            bool FirstWordOnly() const override
            {
                return true;
            }

            PostExec Execute(const Entry &result, const std::wstring &text) override;

        private:
            std::future<Entries> m_loader;
            Entries m_binaries;
        };
    } // namespace modes

} // namespace tofi
