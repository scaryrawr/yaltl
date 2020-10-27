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
        // Fork first child to start a new session
        pid_t pid{fork()};
        if (0 == pid)
        {
            if (pid < 0)
            {
                exit(errno);
            }

            // Try getting a new session id.
            pid = setsid();
            if (pid < 0)
            {
                exit(errno);
            }

            // Fork second child who shall live a very long life
            pid = fork();
            if (0 == pid)
            {
                Command command{commands::parse(full_command)};

                std::vector<char *> argv{command.argv.size() + 1, nullptr};
                std::transform(std::begin(command.argv), std::end(command.argv), std::begin(argv), [](std::string &str) {
                    return str.data();
                });

                // command should be the first argument in argv, otherwise certain apps won't run correctly
                execvp(argv[0], argv.data());
            }

            // Exit success or failure for the first child.
            exit(pid > 0 ? 0 : errno);
        }

        if (pid < 0)
        {
            return false;
        }

        int stat{};
        waitpid(pid, &stat, 0);

        // Oddly, even if we're "successful" here, we don't actually know what happened to the child we care about
        return WIFEXITED(stat) && 0 == WEXITSTATUS(stat);
    }
} // namespace tofi
