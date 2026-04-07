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
        {"getI18n::result", "?getI18n@@YAAEAVI18n@@XZ"},
    };
    return vars;
}

void *resolve_symbol(std::string_view symbol, bool disable_error_output)
{
    if (symbol.starts_with('?') && !symbol.starts_with("??_")) {
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
                return ll::memory::SymbolView{fakeSymbol}.resolve(disable_error_output);
            }
        }
    }
    else {
        auto it = get_vars().find(symbol);
        if (it != get_vars().end()) {
            return reinterpret_cast<void *(*)()>(ll::memory::SymbolView{it->second}.resolve())();
        }
    }
    return ll::memory::SymbolView{symbol}.resolve(disable_error_output);
}

}  // namespace endstone::runtime
