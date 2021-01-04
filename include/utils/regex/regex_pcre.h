#pragma once

#include <pcre2.h>
#include <mtl/memory.hpp>

namespace tofi
{
    namespace regex
    {
        using unique_regex_t = mtl::unique_ptr<decltype(pcre2_code_free), &pcre2_code_free>;
        using regex_t = std::shared_ptr<unique_regex_t::element_type>;
    } // namespace regex

} // namespace tofi
