#pragma once

#include "../mode.h"

#include <i3ipc++/ipc.hpp>

namespace tofi
{
    namespace modes
    {
        /**
         * @brief Uses i3ipc to switch between active windows in swaywm/i3wm.
         * 
         */
        class i3wm : public Mode
        {
            using con_t = std::shared_ptr<i3ipc::container_t>;

        public:
            i3wm(std::string &&self_id);

            std::wstring name() const override
            {
                return L"windows";
            }

            Results results(const std::wstring &search) override;

            void preview(const Result &selected) override;

            PostExec execute(const Result &result) override;

        private:
            i3ipc::connection m_conn;
            std::vector<con_t> m_active;
            std::string m_self_id;
        };
    } // namespace modes

} // namespace tofi
