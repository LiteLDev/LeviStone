#include "bedrock/symbol.h"

#include <ll/api/memory/Symbol.h>

#include "demangler/MicrosoftDemangle.h"

namespace endstone::runtime {

std::unordered_map<std::string_view, std::string_view> const &get_vars()
{
    static std::unordered_map<std::string_view, std::string_view> vars{
        {"BlockState::StateListNode::mHead", "?mHead@StateListNode@BlockState@@SAAEAPEAU12@XZ"},
        {"Enchant::mEnchants",
         "?mEnchants@Enchant@@SAAEAV?$vector@V?$unique_ptr@VEnchant@@U?$default_delete@VEnchant@@@std@@@std@@V?$"
         "allocator@V?$unique_ptr@VEnchant@@U?$default_delete@VEnchant@@@std@@@std@@@2@@std@@XZ"},
    };
    return vars;
}

std::unordered_map<std::string_view, std::string_view> const &get_maps()
{
    static std::unordered_map<std::string_view, std::string_view> maps{
        {"?getMetadata@SimpleNetworkInterfaceImpl@NetherNet@@CA?AV?$Result@V?$basic_string@DU?$char_traits@D@std@@V?$"
         "allocator@D@2@@std@@Verror_code@2@@Bedrock@@AEBVHttpMetadataProvider@12@@Z",
         "brstd::detail::function::function_base_impl<0,Bedrock::Result<std::basic_string<char,std::char_traits<char>,"
         "std::allocator<char> >,std::error_code>,0>::vtable_impl<`lambda at "
         "D:\\a\\_work\\1\\s\\handheld\\src-deps\\NetherNet\\src\\NetherNetTransportInterfaceImpl.cpp:60:5',`lambda at "
         "D:\\a\\_work\\1\\s\\handheld\\src-deps\\NetherNet\\src\\NetherNetTransportInterfaceImpl.cpp:60:5' "
         "&,0>::invoke"}};
    return maps;
}

void *resolve_symbol(std::string_view symbol)
{
    if (symbol.starts_with('?') && !symbol.starts_with("??_")) {
        auto it = get_maps().find(symbol);
        if (it != get_maps().end()) {
            return ll::memory::SymbolView{it->second}.resolve();
        }

        using namespace demangler::ms_demangle;
        using namespace demangler;
        Demangler demangler;

        std::string_view fn = symbol.substr(1);
        SymbolNode *symbolNode = demangler.demangleDeclarator(fn);
        if (!demangler.Error && symbolNode && symbolNode->kind() == NodeKind::FunctionSymbol) {
            auto &funcNode = reinterpret_cast<FunctionSymbolNode *>(symbolNode)->Signature->FunctionClass;
            bool modified = false;
            size_t funcNodeSize = funcNode.toString().size();
            if (funcNode.has(FC_Protected)) {
                funcNode.remove(FC_Protected);
                funcNode.add(FC_Public);
                modified = true;
            }
            if (funcNode.has(FC_Private)) {
                funcNode.remove(FC_Private);
                funcNode.add(FC_Public);
                modified = true;
            }
            if (modified) {
                std::string fakeSymbol{symbol};
                std::string fakeFuncNode = funcNode.toString();
                size_t offset = symbol.size() - funcNode.pos.size();
                fakeSymbol.replace(offset, funcNodeSize, fakeFuncNode);
                return ll::memory::SymbolView{fakeSymbol}.resolve();
            }
        }
    }
    else {
        auto it = get_vars().find(symbol);
        if (it != get_vars().end()) {
            return reinterpret_cast<void *(*)()>(ll::memory::SymbolView{it->second}.resolve())();
        }
    }
    return ll::memory::SymbolView{symbol}.resolve();
}

}  // namespace endstone::runtime
