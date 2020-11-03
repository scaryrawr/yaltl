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

            std::wstring Name() const override
            {
                return m_name;
            }

            const Entries &Results() override;

            PostExec Execute(const Entry &result, const std::wstring &) override;

        private:
            std::wstring m_name;
            std::string m_script;
            std::future<Entries> m_loader;
            Entries m_results;
        };
    } // namespace modes
} // namespace tofi
