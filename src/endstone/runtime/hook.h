#pragma once

#include <string_view>
#include <system_error>
#include <unordered_map>

#include <entt/entt.hpp>

#include "bedrock/symbol.h"
#include "endstone/detail.h"

namespace endstone::runtime::hook {
namespace details {
const std::error_category &error_category();
void *&get_original(std::string_view symbol);
std::unordered_map<std::string, void *> &get_targets();
const std::unordered_map<std::string, void *> &get_detours();
}

void install();
void uninstall();

template <endstone::runtime::FixedSymbol symbol>
void *get_original()
{
    static void **original = &details::get_original(symbol.view());
    return *original;
}

}  // namespace endstone::runtime::hook

#define ENDSTONE_HOOK_CALL_ORIGINAL(fp, ...) ENDSTONE_HOOK_CALL_ORIGINAL_NAME(fp, __FUNCDNAME__, ##__VA_ARGS__)
#define ENDSTONE_HOOK_CALL_ORIGINAL_NAME(fp, name, ...) \
    std::invoke(endstone::detail::fp_cast(fp, endstone::runtime::hook::get_original<name>()), ##__VA_ARGS__)
