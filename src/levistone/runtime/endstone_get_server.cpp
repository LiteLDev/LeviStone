#include "endstone/core/Server.h"

// Because of header conflict, I put endstone_get_server in separate file
extern "C" [[maybe_unused]] __declspec(dllexport) void *endstone_get_server()
{
    return &entt::locator<endstone::core::EndstoneServer>::value();
}
