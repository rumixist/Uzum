#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <memory>
#include "token.hpp"
#include "ast.hpp"

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::shared_ptr<Stmt>> parse();

private:
    std::vector<Token> tokens;
    int current = 0;

    // Yardımcı fonksiyonlar
    Token peek();
    Token previous();
    bool isAtEnd();
    bool check(TokenType type);
    Token advance();
    bool match(std::vector<TokenType> types);
    Token consume(TokenType type, std::string message);

    // Dil kuralları
    std::shared_ptr<Stmt> statement();
    std::shared_ptr<Stmt> varDeclaration();
    std::shared_ptr<Stmt> printStatement();
    std::shared_ptr<Expr> expression();
    std::shared_ptr<Stmt> ifStatement();
    std::vector<std::shared_ptr<Stmt>> block();
    std::shared_ptr<Expr> primary();
    std::shared_ptr<Expr> term();
    std::shared_ptr<Expr> factor();
    std::shared_ptr<Expr> comparison();
    std::shared_ptr<Stmt> whileStatement();
    std::shared_ptr<Expr> orExpression();
    std::shared_ptr<Expr> andExpression();
    std::shared_ptr<Stmt> expressionStatement();
    std::shared_ptr<Stmt> functionDeclaration();
    std::shared_ptr<Expr> finishCall(std::shared_ptr<Expr> callee);
    std::shared_ptr<Stmt> returnStatement();
    std::shared_ptr<Expr> assignment();
    std::shared_ptr<Expr> unary();
    std::shared_ptr<Expr> xorExpression();
    std::shared_ptr<Expr> equality();

};

#endif