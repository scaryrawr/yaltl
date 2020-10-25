#include "utils/command.h"
#include "utils/string.h"

namespace tofi
{
    namespace commands
    {
        Command parse(const std::string &commandline)
        {
            std::vector<std::string> argv;
            string::split<char, std::string>(commandline, " ", std::back_inserter(argv));
            argv.erase(std::remove_if(std::begin(argv), std::end(argv), [](const std::string &str) {
                           return str.empty();
                       }),
                       std::end(argv));

            tofi::Command retval;
            retval.path = argv[0];
            retval.argv = std::move(argv);

            return retval;
        }
    } // namespace commands
} // namespace tofi
