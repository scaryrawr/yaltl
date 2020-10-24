#pragma once

#include "../mode.h"

#include <i3ipc++/ipc.hpp>

namespace tofi
{
    namespace modes
    {
        class i3wm : public Mode
        {
            using con_t = std::shared_ptr<i3ipc::container_t>;

        public:
            i3wm(std::string &&self_id);

            std::wstring name() const
            {
                return L"windows";
            }

            Results results(const std::wstring &search);

            void preview(const Result &selected);

            bool execute(const Result &result);

        private:
            i3ipc::connection m_conn;
            std::vector<con_t> m_active;
            std::string m_self_id;
        };
    } // namespace modes

} // namespace tofi
