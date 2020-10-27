#pragma once

#include <algorithm>
#include <codecvt>
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

        /**
         * @brief Does a case insensitive "fuzzy" search.
         * 
         * @tparam CharT Character type for both input strings and output fuzzy string.
         * @param outer The string that may contain the other.
         * @param inner The inner text being searched for.
         * @return std::optional<std::basic_string<CharT>> The fuzz found.
         */
        template <class CharT>
        std::optional<std::basic_string<CharT>> fuzzy_find(std::basic_string_view<CharT> outer, std::basic_string_view<CharT> inner)
        {
            using RegexItr = std::regex_iterator<typename std::basic_string_view<CharT>::const_iterator>;
            using Regex = std::basic_regex<CharT>;

            std::basic_ostringstream<CharT> builder;
            std::copy(std::begin(inner), std::end(inner), std::ostream_iterator<CharT, CharT>(builder, regex_delim<CharT>()));
            Regex regex{builder.str(), std::regex_constants::icase};
            RegexItr itr{std::begin(outer), std::end(outer), regex};
            RegexItr end{};

            std::vector<std::basic_string<CharT>> results;
            std::transform(itr, end, std::back_inserter(results), [](const std::match_results<const CharT *> &result) {
                return result.str();
            });

            if (results.empty())
            {
                return std::nullopt;
            }

            std::sort(std::begin(results), std::end(results), [](const std::basic_string<CharT> &lhs, const std::basic_string<CharT> &rhs) {
                return lhs.length() < rhs.length();
            });

            return results[0];
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

            /**
             * @brief Checks if one string contains another
             * 
             * @tparam CharT The string character type
             * @param outer The outer string to search inside of
             * @param inner The inner string to search for
             * @param locale The locale
             * @return true The inner string was found in the outer
             * @return false The inner string was not found in the other
             */
            template <class CharT>
            bool contains(std::basic_string_view<CharT> outer, std::basic_string_view<CharT> inner, const std::locale &locale = std::locale())
            {
                return find<CharT>(outer, inner, 0, locale) != std::basic_string_view<CharT>::npos;
            }

            /**
             * @brief Checks if one string is a permutation of the other
             * 
             * @tparam CharT The character type
             * @param outer The first string
             * @param inner The second string
             * @param locale The locale
             * @return true - They are permutations of each other
             * @return false - They are not permutations of each other
             */
            template <class CharT>
            bool permutation(std::basic_string_view<CharT> outer, std::basic_string_view<CharT> inner, const std::locale &locale = std::locale())
            {
                return std::is_permutation(std::begin(outer), std::end(outer), std::begin(inner), std::end(inner), equals<CharT>{locale}) ||
                       (outer.size() > inner.size() && std::is_permutation(std::begin(outer), std::begin(outer) + inner.size(), std::begin(inner), std::end(inner), equals<CharT>{locale}));
            }
        } // namespace insensitive
    }     // namespace string
} // namespace tofi
