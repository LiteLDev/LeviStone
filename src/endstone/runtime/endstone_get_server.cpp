#include "endstone/core/Server.h"

// Because of header conflict, I put endstone_get_server in separate file
extern "C" [[maybe_unused]] ENDSTONE_EXPORT void *endstone_get_server()
{
    return &endstone::core::EndstoneServer::getInstance();
}
