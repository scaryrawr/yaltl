#include "utils/regex.h"

#include <functional>
#include <sstream>
#include <string>

namespace yaltl
{
    namespace regex
    {
        regex_t build_regex(std::wstring_view search)
        {
            return std::wregex{build_pattern(search), std::regex_constants::icase};
        }

        std::optional<std::wstring_view> fuzzy_find(std::wstring_view outer, const regex_t &search)
        {

            std::wcregex_iterator itr{outer.data(), outer.data() + outer.size(), search};
            std::wcregex_iterator end{};
            auto resItr{std::min_element(itr, end, [](const auto &lhs, const auto &rhs) {
                return lhs.length() < rhs.length();
            })};

            if (resItr != end)
            {
                return outer.substr(resItr->position(), resItr->length());
            }

            return std::nullopt;
        }
    } // namespace regex

} // namespace yaltl
