#include "bedrock/server/commands/command_registry.h"

#include "endstone/core/permissions/default_permissions.h"
#include "endstone/runtime/hook.h"

void CommandRegistry::registerCommand(const std::string &name, char const *description, CommandPermissionLevel level,
                                      CommandFlag flag1, CommandFlag flag2)
{
    ENDSTONE_HOOK_CALL_ORIGINAL(&CommandRegistry::registerCommand, this, name, description, level, flag1, flag2);
    using namespace endstone;
    const auto &server = core::EndstoneServer::getInstance();
    // Check if minecraft.command permission has been registered, if not, skip registration.
    auto *result = server.getPluginManager().getPermission("minecraft.command");
    if (!result) {
        return;
    }
    core::CommandPermissions::registerPermission(
        "minecraft.command." + name, name,
        fmt::format("Allows the user to use the /{} command provided by LeviLamina mods.", name), *result);
    result->recalculatePermissibles();
}
