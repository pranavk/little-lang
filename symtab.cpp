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

Token SymbolToTokenType(SymbolType type) {
  switch(type) {
        case SymbolType::Array:
            return Token::Type_array;
        case SymbolType::Int:
            return Token::Type_int;
        case SymbolType::Void:
            return Token::Type_void;
        case SymbolType::Bool:
            return Token::Type_bool;
        default:
            return Token::Err;
    }
}