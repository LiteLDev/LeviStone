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

// LeviStone: a trimmed version of endstone's bedrock_hooks/network_system.cpp.
//
// Since Endstone 0.11.14 the onNewIncomingConnection splice is the only place PacketSendEvent and
// PacketReceiveEvent are wired in (EndstoneBatchedNetworkPeer replaced the old BatchedNetworkPeer hooks).
// Upstream also swaps in EndstoneCompressedNetworkPeer (libdeflate) at the same spot; that part is an
// optimization and is left to LeviOptimize, so here only the batched layer is replaced and whatever
// compressed peer BDS (or another mod) put below it is kept as is.

#include "bedrock/network/network_system.h"

#include <memory>
#include <optional>
#include <utility>

#include "bedrock/network/batched_network_peer.h"
#include "bedrock/network/network_connection.h"
#include "endstone/core/network/batched_network_peer.h"
#include "endstone/core/server.h"
#include "endstone/runtime/hook.h"

bool NetworkSystem::onNewIncomingConnection(const NetworkIdentifier &id, std::shared_ptr<NetworkPeer> &&peer,
                                            std::optional<WebKey> web_key)
{
    if (!ENDSTONE_HOOK_CALL_ORIGINAL(&NetworkSystem::onNewIncomingConnection, this, id, std::move(peer),
                                     std::move(web_key))) {
        return false;
    }

    // BDS appends the new connection, while a lookup by id can find a closing one with the same id
    auto &connection = *connections_.back();
    const auto batched = connection.batched_peer.lock();
    if (!batched) {
        return true;
    }

    // Find the slot holding the batched peer. Peers stacked above it (e.g. PacketTraceNetworkPeer, or one added
    // by another mod) are kept, unlike upstream, which drops PacketTraceNetworkPeer for performance.
    auto *slot = &connection.peer;
    while (*slot && *slot != batched) {
        slot = &(*slot)->peer_;
    }
    if (!*slot || !batched->peer_) {
        endstone::core::EndstoneServer::getInstance().getLogger().error(
            "Unable to splice the peer chain of {}, packet events will not fire for it.", id.getAddress());
        return true;
    }

    // Keep the existing compressed peer below; EndstoneBatchedNetworkPeer only talks to it through NetworkPeer.
    auto new_batched = std::make_shared<endstone::core::EndstoneBatchedNetworkPeer>(connection.id, batched->peer_, batched);
    connection.batched_peer = new_batched;
    *slot = std::move(new_batched);
    return true;
}
