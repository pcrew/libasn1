#pragma once

#include <libasn/per/per_reader.h>

namespace libasn {

struct aper_reader : per_reader {
    using per_reader::per_reader;
};

} // namespace libasn
