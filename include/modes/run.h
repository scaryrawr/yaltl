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

            const Entries &results() override;

            bool first_word_only() const override
            {
                return true;
            }

            PostExec execute(const Entry &result, const std::wstring &text) override;

        private:
            std::future<Entries> m_loader;
            Entries m_binaries;
        };
    } // namespace modes

} // namespace tofi
