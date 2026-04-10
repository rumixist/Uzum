#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory>
#include "token.hpp"

// Tüm ifadelerin (2 + 2, "merhaba", x) temel sınıfı
class Expr {
public:
    virtual ~Expr() = default;
};

// Sabit değerler (Sayılar, Metinler)
class LiteralExpr : public Expr {
public:
    LiteralExpr(Token value) : value(value) {}
    Token value;
};

// Değişken okuma (Örn: print(x) içindeki x)
class VariableExpr : public Expr {
public:
    VariableExpr(Token name) : name(name) {}
    Token name;
};

// --- Komutlar (Statements) ---

class Stmt {
public:
    virtual ~Stmt() = default;
};

// Değişken Tanımlama: var x = 5;
class VarStmt : public Stmt {
public:
    Token name;
    std::shared_ptr<Expr> initializer;
    TokenType type;    // VAR, INT, STR, NUM, BOOL, CHAR
    bool isConstant;   // const eki var mı?

    VarStmt(Token name, std::shared_ptr<Expr> initializer, TokenType type, bool isConstant)
        : name(name), initializer(initializer), type(type), isConstant(isConstant) {}
};

// Yazdırma: print("...") veya println(x)
class PrintStmt : public Stmt {
public:
    PrintStmt(std::shared_ptr<Expr> expression, bool newLine) 
        : expression(expression), newLine(newLine) {}
    std::shared_ptr<Expr> expression;
    bool newLine;
};

// Kod bloklarını (süslü parantez içi) temsil eder
class BlockStmt : public Stmt {
public:
    BlockStmt(std::vector<std::shared_ptr<Stmt>> statements) 
        : statements(statements) {}
    std::vector<std::shared_ptr<Stmt>> statements;
};

// if yapısını temsil eder
class IfStmt : public Stmt {
public:
    IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch) 
        : condition(condition), thenBranch(thenBranch) {}
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
};

// Karşılaştırma ifadeleri (x > 10 gibi)
class BinaryExpr : public Expr {
public:
    BinaryExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {}
    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body)
        : condition(condition), body(body) {}
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
};

// ast.hpp içinde Stmt sınıflarının olduğu yere ekle:
class ExpressionStmt : public Stmt {
public:
    ExpressionStmt(std::shared_ptr<Expr> expression) 
        : expression(expression) {}
    std::shared_ptr<Expr> expression;
};

// Ayrıca atama işlemini temsil edecek bir ifade sınıfına ihtiyacın var:
class AssignExpr : public Expr {
public:
    AssignExpr(Token name, std::shared_ptr<Expr> value) 
        : name(name), value(value) {}
    Token name;
    std::shared_ptr<Expr> value;
};

#endif