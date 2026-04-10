#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>
#include "token.hpp"

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> scanTokens();

private:
    std::string source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;

    // Yardımcı Fonksiyonlar
    bool isAtEnd();
    char advance();
    char peek();
    void scanToken();
    void addToken(TokenType type);
    void scanString();
    void scanNumber();
    void scanIdentifier();
    bool match(char expected);
};

#endif