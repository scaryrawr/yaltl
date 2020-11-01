#include "utils/regex.h"

#include <functional>
#include <sstream>
#include <string>

#define REGEX_DELIM L".*"

namespace tofi
{
    namespace regex
    {
        std::wstring build_pattern(std::wstring_view search)
        {
            std::wostringstream builder;
            std::copy_if(std::begin(search), std::end(search), std::ostream_iterator<wchar_t, wchar_t>(builder, REGEX_DELIM), std::not_fn(std::bind(std::isspace<wchar_t>, std::placeholders::_1, std::locale())));
            std::wstring pattern = builder.str();

            return pattern.substr(0, pattern.size() - wcslen(REGEX_DELIM));
        }
    } // namespace regex

} // namespace tofi
