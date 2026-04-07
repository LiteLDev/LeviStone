#include "endstone/runtime/vtable_hook.h"

#include <stdexcept>
#include <unordered_map>

#include <ll/api/memory/Hook.h>
#include <ll/api/thread/GlobalThreadPauser.h>

namespace endstone::runtime::vhook {
namespace details {

struct HookData {
    void *original{};
    void *target{};
    void *detour{};

    void init(void *t, void *d)
    {
        if (target || detour) {
            throw std::runtime_error("vtable hook installed more than once");
        }
        target = t;
        detour = d;
        const int status = ll::memory::hook(target, detour, &original, ll::memory::HookPriority::Normal, false);
        if (status != 0) {
            throw std::runtime_error("Unable to install vtable hook");
        }
    }

    ~HookData()
    {
        if (target && detour) {
            ll::memory::unhook(target, detour, false);
        }
    }
};

using OriginalMap = std::unordered_map<void *, HookData>;

static OriginalMap &originals()
{
    static OriginalMap originals;
    return originals;
}

void create(void **vtable, int ordinal, void *detour)
{
    ll::thread::GlobalThreadPauser g;
    void *target = vtable[ordinal];
    originals().try_emplace(detour).first->second.init(target, detour);
}

}  // namespace details

void *get_original(void *detour)
{
    const auto it = details::originals().find(detour);
    if (it == details::originals().end()) {
        throw std::runtime_error("original function not found");
    }
    return it->second.original;
}

}  // namespace endstone::runtime::vhook