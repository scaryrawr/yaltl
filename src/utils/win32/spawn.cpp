#include "utils/spawn.h"
#include "utils/command.h"
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <wil/resource.h>

namespace cofi
{
    bool spawn(Command command)
    {
        std::ostringstream argbuilder;
        argbuilder << command.path;
        if (!command.argv.empty())
        {
            argbuilder << " ";
            std::copy(std::begin(command.argv), std::end(command.argv), std::ostream_iterator<std::string>(argbuilder, " "));
        }

        std::string argv{argbuilder.str()};
        DWORD flags{};
        if (!(command.path.string().ends_with("bat") || command.path.string().ends_with("BAT")))
        {
            flags |= DETACHED_PROCESS;
        }

        STARTUPINFOA startupInfo{};
        startupInfo.cb = sizeof(startupInfo);
        wil::unique_process_information processInfo;
        if (!CreateProcessA(nullptr,
                            argv.data(),
                            nullptr,
                            nullptr,
                            FALSE,
                            flags,
                            nullptr,
                            nullptr,
                            &startupInfo,
                            &processInfo))
        {
            const HRESULT hr{HRESULT_FROM_WIN32(GetLastError())};
            return SUCCEEDED(hr);
        }

        if (command.path.string().ends_with("bat") || command.path.string().ends_with("BAT"))
        {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
        }

        return true;
    }
} // namespace cofi