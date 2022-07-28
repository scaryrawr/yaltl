#include "utils/regex.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>

constexpr auto REGEX_DELIM = L".*";
constexpr auto SPECIAL_CHARS = L".()[\\+$^*|?";

namespace yaltl
{
    namespace regex
    {
        std::wstring build_pattern(std::wstring_view search)
        {
            std::wstring pattern{std::accumulate(std::begin(search), std::end(search), std::wstring{}, [](std::wstring sum, wchar_t ch) {
                if (std::isspace(ch))
                {
                    return sum;
                }

                return std::move(sum) +
                       (std::any_of(SPECIAL_CHARS, SPECIAL_CHARS + wcslen(SPECIAL_CHARS), std::bind(std::equal_to<wchar_t>(), std::placeholders::_1, ch)) ? L"\\" : L"") +
                       ch +
                       REGEX_DELIM;
            })};

            return pattern;
        }
    } // namespace regex

} // namespace yaltl
