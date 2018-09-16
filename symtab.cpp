#include "symtab.hpp"


SymbolType TokenToSymbolType(Token type) {
    switch(type) {
        case Token::Type_array:
            return SymbolType::Array;
        case Token::Type_int:
            return SymbolType::Int;
        case Token::Type_bool:
            return SymbolType::Bool;
        case Token::Type_void:
            return SymbolType::Void;
        default:
            return SymbolType::Err;
    }
}
