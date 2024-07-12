#pragma once

#include "mem/arena_allocator.h"
#include "util/helpers.h"

namespace ark::es2panda::ir {

class AstBuilder {
public:
    AstBuilder(ark::ArenaAllocator *allocator) : allocator_(allocator) {}

    ark::ArenaAllocator *Allocator()
    {
        return allocator_;
    }

    // FIXME: copied from checker
    template <typename T, typename... Args>
    T *AllocNode(Args &&...args)
    {
        return util::NodeAllocator::ForceSetParent<T>(allocator_, std::forward<Args>(args)...);
    }

private:
    ark::ArenaAllocator *allocator_;
};

}  // namespace ark::es2panda::ir
