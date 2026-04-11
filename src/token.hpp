#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum class TokenType {
    // Tek karakterli simgeler
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    EQUALS, BANG, GREATER, LESS, COLON,

    // İki karakterli simgeler
    BANG_EQUAL, EQUAL_EQUAL, GREATER_EQUAL, LESS_EQUAL, MINUS_EQUAL, PLUS_EQUAL,

    // Literaller
    IDENTIFIER, STRING, NUMBER,

    //Mantıksal operatörler
    AND, OR, NOT, XOR, NAND, NOR, XNOR,

    // İşaretler
    MODULO,

    // Anahtar kelimeler (Üzüm'e özel)
    VAR, INT, NUM, STR, BOOL, CHAR, CONST, STATIC, QUBIT, FUNCTION, IF, ELSE, ELSE_IF,
    FOR, WHILE, RETURN, PRINT, PRINTLN, CONSOLE, TASK,
    TRUE, FALSE, NIL,

    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
};

#endif