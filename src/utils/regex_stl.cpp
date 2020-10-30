#include "utils/regex/regex_stl.h"

#include <functional>
#include <sstream>
#include <string>

namespace tofi
{
    namespace regex
    {
        const wchar_t *regex_delim()
        {
            return L".*";
        }

        regex_t build_regex(std::wstring_view search)
        {
            std::wostringstream builder;
            std::copy_if(std::begin(search), std::end(search), std::ostream_iterator<wchar_t, wchar_t>(builder, regex_delim()), std::not_fn(std::bind(std::isspace<wchar_t>, std::placeholders::_1, std::locale())));
            return std::wregex{builder.str(), std::regex_constants::icase};
        }

        std::optional<std::wstring_view> fuzzy_find(std::wstring_view outer, const regex_t &search)
        {

            std::wcregex_iterator itr{std::begin(outer), std::end(outer), search};
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
