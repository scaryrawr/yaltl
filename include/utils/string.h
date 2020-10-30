#pragma once

#include <algorithm>
#include <codecvt>
#include <functional>
#include <locale>
#include <optional>
#include <string>
#include <sstream>
#include <regex>

namespace tofi
{
    namespace string
    {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    } // namespace string
} // namespace tofi
