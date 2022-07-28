#pragma once

#include "utils/command.h"

namespace yaltl
{
    /**
     * @brief Spawn a process that will outlive the parent process.
     * 
     * @param command The command to run to start the process
     * @return true - Process most likely started
     * @return false - Process most likely not started
     */
    bool spawn(Command command);
} // namespace yaltl
