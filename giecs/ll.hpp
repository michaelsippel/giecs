
#pragma once

#include <stack>

namespace giecs
{

#define LLFN(name) \
template <typename val_t, typename Container> \
static void name (std::stack<val_t, Container>& stack)

} // namespace giecs

