#pragma once

#include <optional>
#include <string>
#include <compare>

namespace tofi
{
    /**
     * @brief Wrapper around some type that can has a fuzzy search result
     * 
     * @tparam Value The value type tied to the fuzzy search
     * @tparam CharT The char-type to support wide and regular characters
     */
    template <class Value, class CharT>
    struct FuzzyResult
    {
        /**
         * @brief The match result (can be nullopt that no matches were found).
         * 
         */
        std::optional<std::basic_string<CharT>> match;

        /**
         * @brief The object tied to the match results
         * 
         */
        Value value{};

        /**
         * @brief Compares fuzzy match results
         * 
         * - Both no value - equivalent
         * - One no value - the one missing value is greater
         * - Otherwise - one with shorter fuzz is less
         * 
         */
        auto operator<=>(const FuzzyResult &other)
        {
            if (!match.has_value() && !other.match.has_value())
            {
                return std::strong_ordering::equivalent;
            }

            if (!match.has_value())
            {
                return std::strong_ordering::greater;
            }

            if (!other.match.has_value())
            {
                return std::strong_ordering::less;
            }

            return match.value().length() <=> other.match.value().length();
        }
    };
} // namespace tofi
