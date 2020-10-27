#pragma once

#include <sys/types.h>
#include <string>

namespace tofi
{
    /**
     * @brief Spawn a process that will outlive the parent process.
     * 
     * @param command The command to run to start the process
     * @return true - Process most likely started
     * @return false - Process most likely not started
     */
    bool spawn(const std::string &command);
} // namespace tofi
