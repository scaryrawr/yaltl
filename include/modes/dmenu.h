#pragma once

#include "../mode.h"
#include "utils/popen.h"

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

            Results results(const std::wstring &search) override;

            PostExec execute(const Result &result) override;

        private:
            ///
            /// WARNING, order here matters, DO NOT RE-ORDER
            ///

            //! The lines read from stdin
            const std::vector<std::wstring> m_lines;

            //! The file descriptor to "stdout"
            std::optional<int> m_fdout{};

            //! The file descriptor to "stdin"
            std::optional<int> m_fdin{};

            //! The file handle to tty-in
            unique_file m_ttyIn;

            //! The file handle to tty-out
            unique_file m_ttyOut;
        };
    } // namespace modes

} // namespace tofi
