#pragma once

#ifndef TOFI_USE_PCRE
#include "./regex/regex_stl.h"
#else
#include "./regex/regex_pcre.h"
#endif

namespace tofi
{
    namespace regex
    {
        std::wstring build_pattern(std::wstring_view search);

        regex_t build_regex(std::wstring_view search);

        /**
         * @brief Does a case insensitive "fuzzy" search.
         * 
         * @tparam CharT Character type for both input strings and output fuzzy string.
         * @param outer The string that may contain the other.
         * @param inner The inner text being searched for.
         * @return std::optional<std::basic_string<CharT>> The fuzz found.
         */
        std::optional<std::wstring_view> fuzzy_find(std::wstring_view outer, const regex_t &search);
    } // namespace regex

} // namespace tofi
