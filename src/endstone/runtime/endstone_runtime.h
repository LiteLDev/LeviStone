#pragma once

#include <ll/api/mod/NativeMod.h>

namespace endstone::core {
class EndstoneRuntime {

public:
    static EndstoneRuntime &getInstance();
    EndstoneRuntime() : mSelf(*ll::mod::NativeMod::current()) {}
    [[nodiscard]] ll::mod::NativeMod &getSelf() const
    {
        return mSelf;
    }
    bool load();
    bool enable();
    bool disable();
    // bool unload();

private:
    ll::mod::NativeMod &mSelf;
    bool enabled{};
};
}  // namespace endstone::core
