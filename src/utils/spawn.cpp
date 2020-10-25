#include "utils/spawn.h"

#include "utils/command.h"
#include "utils/string.h"

#include <vector>
#include <sys/wait.h>
#include <unistd.h>

namespace tofi
{
    bool spawn(const std::string &full_command)
    {
        pid_t pid{fork()};
        if (0 == pid)
        {
            if (pid < 0)
            {
                exit(errno);
            }

            pid = setsid();
            if (pid < 0)
            {
                exit(errno);
            }

            pid = fork();
            if (0 == pid)
            {
                Command command{commands::parse(full_command)};

                std::vector<char *> argv{command.argv.size() + 1, nullptr};
                std::transform(std::begin(command.argv), std::end(command.argv), std::begin(argv), [](std::string &str) {
                    return str.data();
                });

                execvp(argv[0], argv.data());
            }

            exit(pid > 0 ? 0 : errno);
        }

        if (pid < 0)
        {
            return false;
        }

        int stat{};
        waitpid(pid, &stat, 0);

        return WIFEXITED(stat) && 0 == WEXITSTATUS(stat);
    }
} // namespace tofi
