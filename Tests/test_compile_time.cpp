#include <ltl/fast.h>

using namespace ltl::fast;

bool f(int);

int main() {
    using a = function_to_metafunction<decltype(f)>::apply<int>;

    static_assert(std::is_same_v<a, bool>);
}
