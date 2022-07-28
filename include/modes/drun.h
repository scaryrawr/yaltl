#pragma once

#include "../mode.h"
#include <future>

namespace cofi
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

            std::wstring Name() const override
            {
                return L"drun";
            }

            const Entries &Results() override;

            PostExec Execute(const Entry &result, const std::wstring &text) override;

        private:
            Entries m_entries;
            std::future<Entries> m_loading;
        };
    } //namespace modes
} // namespace cofi
