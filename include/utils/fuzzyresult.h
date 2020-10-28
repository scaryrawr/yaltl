#pragma once

#include "mode.h"

#include <memory>
#include <optional>
#include <string>
#include <compare>

namespace tofi
{
    struct FuzzyResult
    {
        std::shared_ptr<Entry> result;

        /**
         * @brief The match result (can be nullopt that no matches were found).
         * 
         */
        std::optional<std::wstring_view> match;

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
