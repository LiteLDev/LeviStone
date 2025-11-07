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

#include "levistone/runtime/endstone_runtime.h"

#include <memory>

#include <ll/api/chrono/GameChrono.h>
#include <ll/api/coro/CoroTask.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/thread/ServerThreadExecutor.h>
#include <pybind11/embed.h>

#include "endstone/runtime/hook.h"

namespace endstone::hook {
void uninstall();
}  // namespace endstone::hook

namespace endstone::core {

void unload_endstone_server();
void disable_endstone_server();
void enable_endstone_server();

EndstoneRuntime &EndstoneRuntime::getInstance()
{
    static EndstoneRuntime instance;
    return instance;
}

bool EndstoneRuntime::load()
{
    // Create endstone.toml
    std::filesystem::path configPath = "endstone.toml";
    std::filesystem::path defaultConfigPath = getSelf().getModDir() / "endstone" / "config" / "endstone.toml";
    if (!std::filesystem::exists(configPath)) {
        if (std::filesystem::exists(defaultConfigPath)) {
            std::filesystem::copy_file(defaultConfigPath, configPath);
        }
    }
    auto &logger = getSelf().getLogger();
    try {
        logger.info("Initialising...");
        // Install hooks
        endstone::hook::install();
    }
    catch (const std::exception &e) {
        logger.error("An exception occurred while initialising Endstone runtime.");
        logger.error("{}", e.what());
        throw;
    }
    enabled = true;
    return true;
}

bool EndstoneRuntime::enable()
{
    if (!enabled) {
        getSelf().getLogger().debug("Enabling...");
        enable_endstone_server();
        enabled = true;
    }

    return true;
}

bool EndstoneRuntime::disable()
{
    if (enabled) {
        getSelf().getLogger().debug("Disabling...");
        ll::coro::keepThis([]() -> ll::coro::CoroTask<> {
            disable_endstone_server();
            co_return;
        }).syncLaunch(ll::thread::ServerThreadExecutor::getDefault());
        enabled = false;
    }
    return true;
}

// TODO:
// bool EndstoneRuntime::unload()
// {
//     getSelf().getLogger().debug("Unloading...");
//     unload_endstone_server();
//     getSelf().getLogger().debug("uninstall hooks...");
//     hook::uninstall();
//     return true;
// }

}  // namespace endstone::core

LL_REGISTER_MOD(endstone::core::EndstoneRuntime, endstone::core::EndstoneRuntime::getInstance());
