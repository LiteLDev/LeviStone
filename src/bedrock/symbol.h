#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "endstone/detail.h"

namespace endstone::runtime {
void *get_module_base();
std::string get_module_pathname();
void *get_executable_base();
std::string get_executable_pathname();
void *resolve_symbol(std::string_view symbol);

template <size_t N>
class FixedSymbol {
public:
    char sym[N + 1]{};

    [[nodiscard]] consteval FixedSymbol(std::string_view symbol)
    {
        std::copy_n(symbol.data(), symbol.size(), sym);
    }
    [[nodiscard]] consteval FixedSymbol(char const *symbol)
    {
        std::copy_n(symbol, N, sym);
    }

    [[nodiscard]] consteval size_t size() const
    {
        return N;
    }

    [[nodiscard]] constexpr std::string_view view() const
    {
        return std::string_view{sym};
    }
};
template <size_t N>
FixedSymbol(char const (&)[N]) -> FixedSymbol<N - 1>;

template <FixedSymbol symbol>
inline void *symbolAddressCache = resolve_symbol(symbol.view());

#ifdef _WIN32
template <FixedSymbol symbol, typename Class, typename... Args>
Class *(*get_ctor())(Class *, Args...)
{
    auto *addr = symbolAddressCache<symbol>;
    return reinterpret_cast<Class *(*)(Class *, Args...)>(addr);
}
#elif __linux__
template <FixedSymbol symbol, typename Class, typename... Args>
void (*get_ctor())(Class *, Args...)
{
    auto *addr = symbolAddressCache<symbol>;
    return reinterpret_cast<void (*)(Class *, Args...)>(addr);
}
#endif
}  // namespace endstone::runtime

#define BEDROCK_CALL(fp, ...) \
    std::invoke(endstone::detail::fp_cast(fp, endstone::runtime::symbolAddressCache<__FUNCDNAME__>), ##__VA_ARGS__)

#define BEDROCK_VAR(type, name) reinterpret_cast<type>(endstone::runtime::symbolAddressCache<name>)

#define BEDROCK_CTOR(type, ...) endstone::runtime::get_ctor<__FUNCDNAME__, type, ##__VA_ARGS__>()
