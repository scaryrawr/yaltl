#pragma once

#include <optional>
#include <string>
#include <compare>

namespace tofi
{
    template <class Value, class CharT>
    struct FuzzyResult
    {
        std::optional<std::basic_string<CharT>> match;
        Value value{};

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
