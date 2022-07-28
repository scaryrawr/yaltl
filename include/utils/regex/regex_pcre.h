#pragma once

#include <pcre2.h>
#include <mtl/memory.hpp>

namespace yaltl
{
    namespace regex
    {
        using regex_t = mtl::unique_ptr<decltype(pcre2_code_free), &pcre2_code_free>;
    } // namespace regex

} // namespace yaltl
