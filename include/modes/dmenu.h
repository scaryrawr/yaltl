#pragma once

#include "../mode.h"
#include "utils/popen.h"

#define YALTL_HAS_DMENU

namespace yaltl
{
    namespace modes
    {
        /**
         * @brief Takes input from stdin, enables user to search and select for selection to be printed on stdout
         * 
         */
        class dmenu : public Mode
        {
        public:
            dmenu();
            ~dmenu();

            std::wstring
            Name() const override
            {
                return L"dmenu";
            }

            const Entries &Results() override
            {
                return m_entries;
            }

            PostExec Execute(const Entry &result, const std::wstring &text) override;

        private:
            ///
            /// WARNING, order here matters, DO NOT RE-ORDER
            ///

            //! The lines read from stdin
            const Entries m_entries;

            //! The file descriptor to "stdout"
            int m_stdoutCopy{};

            //! The file descriptor to "stdin"
            int m_stdinCopy{};

            //! The file handle to tty-in
            unique_file m_ttyIn;

            //! The file handle to tty-out
            unique_file m_ttyOut;
        };
    } // namespace modes

} // namespace yaltl
