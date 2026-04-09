#include "bedrock/world/item/item.h"

#include "endstone/runtime/hook.h"

void Item::readUserData(ItemStackBase &stack, IDataInput &input, ReadOnlyBinaryStream &underlying_stream) const
{
    ENDSTONE_HOOK_CALL_ORIGINAL(&Item::readUserData, this, stack, input, underlying_stream);
}
