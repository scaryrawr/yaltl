#include "utils/regex.h"

#include <functional>
#include <limits>
#include <sstream>
#include <string>

namespace yaltl
{
    namespace regex
    {
        regex_t build_regex(std::wstring_view search)
        {
            std::wstring pattern{build_pattern(search)};
            int32_t error{};
            PCRE2_SIZE errorOffset{};

            return regex_t{pcre2_compile(reinterpret_cast<PCRE2_SPTR32>(pattern.c_str()), PCRE2_ZERO_TERMINATED, PCRE2_CASELESS, &error, &errorOffset, nullptr)};
        }

        /**
         * @brief Does a case insensitive "fuzzy" search.
         * 
         * @tparam CharT Character type for both input strings and output fuzzy string.
         * @param outer The string that may contain the other.
         * @param inner The inner text being searched for.
         * @return std::optional<std::basic_string<CharT>> The fuzz found.
         */
        std::optional<std::wstring_view> fuzzy_find(std::wstring_view outer, const regex_t &search)
        {
            using match_data = mtl::unique_ptr<decltype(pcre2_match_data_free), &pcre2_match_data_free>;
            match_data data{pcre2_match_data_create_from_pattern(search.get(), nullptr)};
            int32_t error{pcre2_match(search.get(), reinterpret_cast<PCRE2_SPTR32>(outer.data()), outer.size(), 0, 0, data.get(), nullptr)};
            if (error < 0)
            {
                return std::nullopt;
            }

            uint32_t count{pcre2_get_ovector_count(data.get())};
            PCRE2_SIZE *ovector{pcre2_get_ovector_pointer(data.get())};
            PCRE2_SIZE start{};
            PCRE2_SIZE end{std::numeric_limits<PCRE2_SIZE>::max()};
            for (uint32_t x{}; x < count; ++x)
            {
                if (ovector[2 * x + 1] - ovector[2 * x] < end - start)
                {
                    end = ovector[2 * x + 1];
                    start = ovector[2 * x];
                }
            }

            return outer.substr(start, end - start);
        }
    } // namespace regex

} // namespace yaltl