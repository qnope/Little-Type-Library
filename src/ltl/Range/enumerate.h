#pragma once

#include "Value.h"
#include "Zip.h"

namespace ltl {
template <typename Container>
auto enumerate(Container &&container) {
    using std::size;
    return zip(valueRange<std::size_t>(0, size(FWD(container))), FWD(container));
}
} // namespace ltl
