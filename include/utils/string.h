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

        template <class T>
        inline const T *regex_delim()
        {
            return nullptr;
        }

        template <>
        inline const char *regex_delim<char>()
        {
            return ".*";
        }

        template <>
        inline const wchar_t *regex_delim<wchar_t>()
        {
            return L".*";
        }

        template <class CharT>
        auto build_regex(const std::basic_string_view<CharT> &search)
        {
            std::basic_ostringstream<CharT> builder;
            std::copy_if(std::begin(search), std::end(search), std::ostream_iterator<CharT, CharT>(builder, regex_delim<CharT>()), std::not_fn(std::bind(std::isspace<CharT>, std::placeholders::_1, std::locale())));

            return std::basic_regex<CharT>{builder.str(), std::regex_constants::icase};
        }

        /**
         * @brief Does a case insensitive "fuzzy" search.
         * 
         * @tparam CharT Character type for both input strings and output fuzzy string.
         * @param outer The string that may contain the other.
         * @param inner The inner text being searched for.
         * @return std::optional<std::basic_string<CharT>> The fuzz found.
         */
        template <class CharT, class RegexItr = std::regex_iterator<typename std::basic_string_view<CharT>::const_iterator>, class Regex = std::basic_regex<CharT>>
        std::optional<std::basic_string_view<CharT>> fuzzy_find(std::basic_string_view<CharT> outer, const Regex &search)
        {

            RegexItr itr{std::begin(outer), std::end(outer), search};
            RegexItr end{};

            std::optional<std::basic_string_view<CharT>> result;
            std::for_each(itr, end, [&result, &outer](const std::match_results<const CharT *> &res) {
                if (!result.has_value() || res.length() < result.value().length())
                {
                    result = outer.substr(res.position(), res.length());
                }
            });

            return result;
        }
    } // namespace string
} // namespace tofi
