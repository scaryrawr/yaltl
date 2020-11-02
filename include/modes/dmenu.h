#pragma once

#include "../mode.h"
#include "utils/popen.h"

#define TOFI_HAS_DMENU

namespace tofi
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
            name() const override
            {
                return L"dmenu";
            }

            const Entries &results() override
            {
                return m_entries;
            }

            PostExec execute(const Entry &result, const std::wstring &text) override;

        private:
            ///
            /// WARNING, order here matters, DO NOT RE-ORDER
            ///

            //! The lines read from stdin
            const Entries m_entries;

            //! The file descriptor to "stdout"
            std::optional<int> m_stdoutCopy{};

            //! The file descriptor to "stdin"
            std::optional<int> m_stdinCopy{};

            //! The file handle to tty-in
            unique_file m_ttyIn;

            //! The file handle to tty-out
            unique_file m_ttyOut;
        };
    } // namespace modes

} // namespace tofi
