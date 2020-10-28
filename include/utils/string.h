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

        /**
         * @brief Splits a string
         * 
         * @tparam CharT character type of strings
         * @tparam OutType To return newly allocated strings, or views into the "split" string
         * @tparam OutItr Output iterator type.
         * @param str The string to split
         * @param delim The delimiter to split on
         * @param out Place to store output
         * @return auto The output iterator position that's the end
         */
        template <class CharT, class OutType, class OutItr>
        auto split(std::basic_string_view<CharT> str, std::basic_string_view<CharT> delim, OutItr out)
        {
            size_t position = 0;
            size_t delim_len = delim.length();
            for (auto pos{str.find(delim)}; pos != std::basic_string_view<CharT>::npos; pos = str.find(delim, position))
            {
                (*out++) = OutType(str.substr(position, pos - position));
                position = pos + delim_len;
            }

            if (position < str.length())
            {
                (*out++) = OutType(str.substr(position, str.length() - position));
            }

            return out;
        }

        namespace insensitive
        {
            template <class CharT>
            struct equals
            {
                equals(const std::locale &locale) : m_locale(locale)
                {
                }

                bool operator()(CharT left, CharT right)
                {
                    return std::toupper(left, m_locale) == std::toupper(right, m_locale);
                }

            private:
                const std::locale &m_locale;
            };

            template <class CharT>
            struct less
            {
                less(const std::locale &locale) : m_locale(locale)
                {
                }

                auto operator()(CharT left, CharT right)
                {
                    return std::toupper(left, m_locale) < std::toupper(right, m_locale);
                }

            private:
                const std::locale &m_locale;
            };

            /**
             * @brief Does a case insensitive "exact" match.
             * 
             * @tparam CharT The string character types
             * @param str The outer string to search in
             * @param other The string to search for
             * @param startPosition The position to start the search at
             * @param locale The string locale
             * @return auto The position of the string or npos
             */
            template <class CharT>
            auto find(std::basic_string_view<CharT> str, std::basic_string_view<CharT> other, size_t startPosition = 0, const std::locale &locale = std::locale())
            {
                auto itr = std::search(std::begin(str) + startPosition, std::end(str), std::begin(other), std::end(other), equals<CharT>{locale});
                return itr == std::end(str) ? std::basic_string_view<CharT>::npos : itr - std::begin(str);
            }

            /**
             * @brief Removes all occurrences of a specific string from another
             * 
             * @tparam CharT The string character type
             * @param str The string to remove occurrences from
             * @param other The occurrences to remove
             * @param locale The string locale
             */
            template <class CharT>
            void erase_all(std::basic_string<CharT> &str, std::basic_string_view<CharT> other, const std::locale &locale = std::locale())
            {
                for (auto position{find<CharT>(str, other, 0, locale)}; std::basic_string_view<CharT>::npos != position; position = find<CharT>(str, other, position, locale))
                {
                    str.erase(position, other.length());
                }
            }
        } // namespace insensitive
    }     // namespace string
} // namespace tofi
