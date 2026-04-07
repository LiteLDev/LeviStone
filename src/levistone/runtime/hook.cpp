// Copyright (c) 2024, The Endstone Project. (https://endstone.dev) All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "endstone/runtime/hook.h"

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#include <fmt/format.h>
#include <ll/api/memory/Hook.h>
#include <ll/api/memory/Symbol.h>
#include <ll/api/thread/GlobalThreadPauser.h>

#include "endstone/core/platform.h"

namespace endstone::runtime::hook {

namespace details {

struct HookData {
    void *original{};
    void *target{};
    void *detour{};
    void init(std::string_view name, void *t, void *d)
    {
        if (target || detour) {
            throw std::runtime_error(fmt::format("{} hooked more than once", name));
        }
        target = t;
        detour = d;
        int status = ll::memory::hook(target, detour, &original, ll::memory::HookPriority::Normal, false);
        if (status != 0) {
            throw std::runtime_error(fmt::format("Unable to hook {}", name));
        }
    }
    ~HookData()
    {
        if (target && detour) {
            ll::memory::unhook(target, detour, false);
        }
    }
};

using OriginalMap = std::unordered_map<std::string_view, HookData>;

static OriginalMap &originals()
{
    static OriginalMap originals;
    return originals;
}

void *&get_original(std::string_view symbol)
{
    return originals()[symbol].original;
}

std::unordered_map<std::string, void *> &get_targets()
{
    static std::unordered_map<std::string, void *> targets;
    return targets;
}

const std::error_category &error_category()
{
    return std::system_category();
}

}  // namespace details

void install()
{
    const auto &detours = details::get_detours();
    auto &target_map = details::get_targets();

    ll::thread::GlobalThreadPauser g;

    for (const auto &[name, detour] : detours) {
        if (name.starts_with("ll_") || name == "EntryPoint" ||  // Workaround to fix LeviStone on Wine
            name.starts_with("?executeWorldInteraction")        // lambda
        ) {
            continue;
        }
        void *target{};
        if (auto it = target_map.find(name); it != target_map.end()) {
            target = it->second;
        }
        else {
            target = resolve_symbol(name);
            target_map.emplace(name, target);
        }
        details::originals().try_emplace(name).first->second.init(name, target, detour);
    }
}

void uninstall()
{
    ll::thread::GlobalThreadPauser g;
    details::originals().clear();
}
}  // namespace endstone::runtime::hook
