#pragma once

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>

namespace tofi
{
    namespace string
    {
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

        template <class StringT, class OutItr>
        auto split(const StringT &str, const StringT &delim, OutItr out)
        {
            size_t position = 0;
            size_t delim_len = delim.length();
            for (auto pos{str.find(delim)}; pos != StringT::npos; pos = str.find(delim, position))
            {
                (*out++) = str.substr(position, pos - position);
                position = pos + delim_len;
            }

            if (position < str.length())
            {
                (*out++) = str.substr(position, str.length() - position);
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

            template <class StringT>
            auto find(const StringT &str, const StringT &other, size_t startPosition = 0, const std::locale &locale = std::locale())
            {
                auto itr = std::search(std::begin(str) + startPosition, std::end(str), std::begin(other), std::end(other), equals<typename StringT::value_type>{locale});
                return itr == std::end(str) ? StringT::npos : itr - std::begin(str);
            }

            template <class StringT>
            void erase_all(StringT &str, const StringT &other, const std::locale &locale = std::locale())
            {

                for (auto position{find(str, other, 0, locale)}; StringT::npos != position; position = find(str, other, position, locale))
                {
                    str.erase(position, other.length());
                }
            }

            template <class StringT>
            bool contains(const StringT &outter, const StringT &inner, const std::locale &locale = std::locale())
            {
                return find(outter, inner, 0, locale) != StringT::npos;
            }

            template <class StringT>
            bool permutation(const StringT &outter, const StringT &inner, const std::locale &locale = std::locale())
            {
                return std::is_permutation(std::begin(outter), std::end(outter), std::begin(inner), std::end(inner), equals<typename StringT::value_type>{locale}) ||
                       (outter.size() > inner.size() && std::is_permutation(std::begin(outter), std::begin(outter) + inner.size(), std::begin(inner), std::end(inner), equals<typename StringT::value_type>{locale}));
            }

            template <class StringT>
            bool fuzzy_contains(const StringT &outter, const StringT &inner, const std::locale &locale = std::locale())
            {
                // TODO: figure out how to write fuzzy search...

                std::function<bool(const StringT &, const StringT &, const std::locale &)> funcs[] = {
                    contains<StringT>,
                    permutation<StringT>,
                };

                return std::any_of(std::begin(funcs), std::end(funcs), [&outter, &inner, &locale](auto &func) {
                    return func(outter, inner, locale);
                });
            }
        } // namespace insensitive
    }     // namespace string
} // namespace tofi
