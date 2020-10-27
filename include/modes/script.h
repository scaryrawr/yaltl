#pragma once

#include "../mode.h"

#include <future>
#include <string>
#include <string_view>
#include <vector>

namespace tofi
{
    namespace modes
    {
        /**
         * @brief Loads a script and will recall script with selected output until no output is returned.
         * 
         */
        class script : public Mode
        {
        public:
            script(std::string_view name, std::string_view script);

            std::wstring name() const override
            {
                return m_name;
            }

            Results results(const std::wstring &search) override;
            PostExec execute(const Result &result) override;

        private:
            std::wstring m_name;
            std::string m_script;
            std::future<std::vector<std::wstring>> m_loader;
            std::optional<std::vector<std::wstring>> m_results;
        };
    } // namespace modes

} // namespace tofi
