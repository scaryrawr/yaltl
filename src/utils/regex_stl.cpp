#include "utils/regex.h"

#include <functional>
#include <map>
#include <string>

namespace tofi
{
    namespace regex
    {
        regex_t build_regex(std::wstring_view search)
        {
            static std::map<std::wstring, std::wregex> cache;
            const std::wstring key{search};
            auto itr{cache.find(key)};
            if (itr == std::end(cache))
            {
                std::tie(itr, std::ignore) = cache.emplace(key, std::wregex{build_pattern(search), std::regex_constants::icase});
            }

            return &itr->second;
        }

        std::optional<std::wstring_view> fuzzy_find(std::wstring_view outer, const regex_t &search)
        {

            std::wcregex_iterator itr{outer.data(), outer.data() + outer.size(), *search};
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

} // namespace tofi
