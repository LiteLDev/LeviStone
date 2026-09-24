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

#pragma once

#include <entt/locator/locator.hpp>

#include "bedrock/bedrock.h"
#include "bedrock/common_types.h"
#include "bedrock/core/utility/unique_owner_pointer.h"
#include "bedrock/forward.h"
#include "bedrock/minecraft_app_interface.h"
#include "bedrock/server/console_input_reader.h"
#include "bedrock/server/server_exit_code.h"
#include "bedrock/world/events/server_instance_event_coordinator.h"

namespace endstone::core {
class EndstoneServer;
}

class DedicatedServer : public IMinecraftApp {
public:
    DedicatedServer();
    ~DedicatedServer() override;
    ENDSTONE_HOOK ServerExitCode start(const std::string &session_id, const Bedrock::ActivationArguments &args);
    [[nodiscard]] Bedrock::NotNullNonOwnerPtr<FileArchiver> getFileArchiver() const override;

private:
    friend class endstone::core::EndstoneServer;
    [[nodiscard]] Bedrock::NotNullNonOwnerPtr<Minecraft> getPrimaryMinecraft() override;
    [[nodiscard]] Bedrock::NotNullNonOwnerPtr<Automation::AutomationClient> getAutomationClient() const override;
    [[nodiscard]] bool isEduMode() const override;
    [[nodiscard]] bool isDedicatedServer() const override;
    void onNetworkMaxPlayersChanged(uint32_t newMaxPlayerCount) override;
    [[nodiscard]] IGameModuleShared &getGameModuleShared() override;
    void teardownHttp();
    void requestServerShutdown() override;
    bool requestInGamePause(const SubClientId &, bool) override;

    Bedrock::NonOwnerPointer<Minecraft> minecraft_;
    Bedrock::NonOwnerPointer<Core::FileSystem> file_system_;
    Bedrock::UniqueOwnerPointer<ServerInstanceEventCoordinator> server_instance_event_coordinator_;
    std::atomic<bool> wants_to_quit_;
    std::atomic<ServerExitCode> result_;
    std::unique_ptr<ConsoleInputReader> console_input_reader_;
    //...
};

/**
 * @brief Service locator holding the (non-owning) pointer to the dedicated server instance.
 *
 * entt 4 constrains the type used to initialise a locator with `stl::derived_from<Service>`, which
 * no type can ever satisfy when `Service` is a pointer. As the instance is owned by the application
 * rather than by the locator, this specialisation stores it as a plain pointer instead.
 *
 * Only the members actually used by the runtime hooks are provided on purpose, so that any other
 * use of this locator fails to compile rather than silently diverging from entt's implementation.
 */
template <>
class entt::locator<DedicatedServer *> final {
public:
    using type = DedicatedServer *;  // NOLINT(*-identifier-naming)

    locator() = delete;
    locator(const locator &) = delete;
    ~locator() = delete;
    locator &operator=(const locator &) = delete;

    [[nodiscard]] static bool has_value() noexcept
    {
        return (service != nullptr);
    }

    [[nodiscard]] static DedicatedServer *&value() noexcept
    {
        ENTT_ASSERT(has_value(), "Service not available");
        return service;
    }

    static DedicatedServer *&emplace(DedicatedServer *elem) noexcept
    {
        return (service = elem);
    }

    static void reset() noexcept
    {
        service = nullptr;
    }

private:
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    inline static DedicatedServer *service{};
};
