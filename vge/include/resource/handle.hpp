#pragma once

#include "defines.hpp"

namespace Resource {

// A 32-bit handle will contain 8 bits for generation counters,
// meaning 2^32 resources could be created, with at most 2^(32-8)
// = 2^24 alive at the same time.
struct Handle {
    U32 index : 24;
    U32 generation : 8;
};

} // namespace Resource