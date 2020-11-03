#pragma once

#include "utils/command.h"

namespace tofi
{
    /**
     * @brief Spawn a process that will outlive the parent process.
     * 
     * @param command The command to run to start the process
     * @return true - Process most likely started
     * @return false - Process most likely not started
     */
    bool spawn(const Command &command);
} // namespace tofi
