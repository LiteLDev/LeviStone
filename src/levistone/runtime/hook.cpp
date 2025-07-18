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

#include <string>
#include <system_error>
#include <unordered_map>

#include <entt/core/hashed_string.hpp>
#include <ll/api/memory/Hook.h>

#include "bedrock/symbol.h"
#include "endstone/detail/platform.h"

namespace endstone::hook {

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

using OriginalMap = std::unordered_map<void *, HookData>;

static OriginalMap &originals()
{
    static OriginalMap originals;
    return originals;
}

void *&get_original(void *target)
{
    const auto it = originals().find(target);
    if (it == originals().end()) {
        throw std::runtime_error("original function not found");
    }
    return it->second.original;
}

const std::unordered_map<std::string, void *> &get_targets()
{
    static std::unordered_map<std::string, void *> targets;
    if (!targets.empty()) {
        return targets;
    }
    auto *executable_base = detail::get_executable_base();
    detail::foreach_symbol([executable_base](const auto &key, auto offset) {
        auto *target = static_cast<char *>(executable_base) + offset;
        targets.emplace(key, target);
    });
    return targets;
}
}

void install()
{
    const auto &detours = details::get_detours();
    const auto &targets = details::get_targets();

    ll::thread::GlobalThreadPauser g;

    for (const auto &[name, detour] : detours) {
        if (name.starts_with("ll_")) {
            continue;
        }
        if (auto it = targets.find(name); it != targets.end()) {
            void *target = it->second;
            void *original = target;
            details::originals().try_emplace(target).first->second.init(name, target, detour);
        }
        else {
            throw std::runtime_error(fmt::format("Unable to find target function for detour: {}.", name));
        }
    }
}

void uninstall()
{
    ll::thread::GlobalThreadPauser g;
    details::originals().clear();
}
}  // namespace endstone::hook