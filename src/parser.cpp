#include "parser.hpp"
#include <iostream>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

bool Parser::isAtEnd() { return peek().type == TokenType::END_OF_FILE; }
Token Parser::peek() { return tokens[current]; }
Token Parser::previous() { return tokens[current - 1]; }
Token Parser::advance() { if (!isAtEnd()) current++; return previous(); }
bool Parser::check(TokenType type) { if (isAtEnd()) return false; return peek().type == type; }

bool Parser::match(std::vector<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, std::string message) {
    if (check(type)) return advance();
    throw std::runtime_error("Hata (Satir " + std::to_string(peek().line) + "): " + message);
}

std::vector<std::shared_ptr<Stmt>> Parser::parse() {
    std::vector<std::shared_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        if (match({TokenType::SEMICOLON})) continue; 
        statements.push_back(statement());
    }
    return statements;
}

std::shared_ptr<Stmt> Parser::statement() {
    if (match({TokenType::TASK})) return functionDeclaration();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::LEFT_BRACE})) return std::make_shared<BlockStmt>(block());
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::PRINT, TokenType::PRINTLN})) return printStatement();
    
    if (match({TokenType::VAR, TokenType::CONST, TokenType::INT, 
               TokenType::STR, TokenType::NUM, TokenType::BOOL, 
               TokenType::CHAR})) {
        return varDeclaration();
    }

    return expressionStatement();
}

std::shared_ptr<Stmt> Parser::ifStatement() {
    std::vector<IfStmt::Branch> branches;
    std::shared_ptr<Stmt> elseBranch = nullptr; // Başta tanımlamalıyız

    // 1. İlk 'if' kısmı
    consume(TokenType::LEFT_PAREN, "'if' den sonra '(' bekleniyor.");
    std::shared_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Kosuldan sonra ')' bekleniyor.");
    std::shared_ptr<Stmt> thenBranch = statement();
    branches.push_back({condition, thenBranch});

    // 2. 'else if' ve 'else' kontrolü
    while (match({TokenType::ELSE})) { 
        if (match({TokenType::IF})) {  
            consume(TokenType::LEFT_PAREN, "'if' den sonra '(' bekleniyor.");
            std::shared_ptr<Expr> elifCondition = expression();
            consume(TokenType::RIGHT_PAREN, "Kosuldan sonra ')' bekleniyor.");
            std::shared_ptr<Stmt> elifBranch = statement();
            branches.push_back({elifCondition, elifBranch});
        } else {
            // ELSE'den sonra IF gelmiyorsa, bu son duraktır.
            elseBranch = statement();
            break; 
        }
    }

    // 3. AST nesnesini oluştur ve döndür
    return std::make_shared<IfStmt>(branches, elseBranch);
}

std::vector<std::shared_ptr<Stmt>> Parser::block() {
    std::vector<std::shared_ptr<Stmt>> statements;
    
    // Blok boş olabilir veya sadece noktalı virgüllerden oluşabilir
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        // Blok içindeki gereksiz boş noktalı virgülleri temizle
        if (match({TokenType::SEMICOLON})) continue;
        statements.push_back(statement());
    }
    
    consume(TokenType::RIGHT_BRACE, "Blok sonunda '}' bekleniyor.");
    return statements;
}

std::shared_ptr<Stmt> Parser::varDeclaration() {
    Token keyword = previous();
    bool isConstant = (keyword.type == TokenType::CONST);
    TokenType type = TokenType::VAR;

    if (isConstant) {
        if (match({TokenType::INT, TokenType::STR, TokenType::NUM, 
                   TokenType::BOOL, TokenType::CHAR, TokenType::VAR})) {
            type = previous().type;
        }
    } else {
        type = keyword.type;
    }

    Token name = consume(TokenType::IDENTIFIER, "Degisken adi bekleniyor.");
    std::shared_ptr<Expr> initializer = nullptr;

    if (match({TokenType::EQUALS})) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Degisken tanimindan sonra ';' bekleniyor.");
    return std::make_shared<VarStmt>(name, initializer, type, isConstant);
}

std::shared_ptr<Stmt> Parser::printStatement() {
    bool isNewLine = (previous().type == TokenType::PRINTLN);
    consume(TokenType::LEFT_PAREN, "'(' bekleniyor.");
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::RIGHT_PAREN, "')' bekleniyor.");
    consume(TokenType::SEMICOLON, "';' bekleniyor.");
    return std::make_shared<PrintStmt>(expr, isNewLine);
}

std::shared_ptr<Stmt> Parser::expressionStatement() {
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Ifade sonuna ';' bekleniyor.");
    return std::make_shared<ExpressionStmt>(expr);
}

// --- IFADE (EXPRESSION) HIYERARSISI ---

std::shared_ptr<Expr> Parser::expression() {
    return assignment(); 
}
std::shared_ptr<Expr> Parser::assignment() {
    std::shared_ptr<Expr> expr = orExpression();

    if (match({TokenType::EQUALS})) {
        Token equals = previous();
        std::shared_ptr<Expr> value = assignment(); // Sağ taraf özyinelemeli (recursive)

        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            return std::make_shared<AssignExpr>(varExpr->name, value);
        }
        throw std::runtime_error("Hata (Satir " + std::to_string(equals.line) + "): Gecersiz atama hedefi.");
    }
    return expr;
}

std::shared_ptr<Expr> Parser::orExpression() {
    std::shared_ptr<Expr> expr = xorExpression();
    while (match({TokenType::OR})) {
        Token op = previous();
        std::shared_ptr<Expr> right = xorExpression();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// 2. XOR
std::shared_ptr<Expr> Parser::xorExpression() {
    std::shared_ptr<Expr> expr = andExpression();
    while (match({TokenType::XOR})) {
        Token op = previous();
        std::shared_ptr<Expr> right = andExpression();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// 3. AND
std::shared_ptr<Expr> Parser::andExpression() {
    std::shared_ptr<Expr> expr = equality(); // == ve != seviyesine bağlanır
    while (match({TokenType::AND})) {
        Token op = previous();
        std::shared_ptr<Expr> right = equality();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::equality() {
    std::shared_ptr<Expr> expr = comparison(); // Bir alt basamağa ( <, >, <=, >= ) iner

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        std::shared_ptr<Expr> right = comparison();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::comparison() {
    std::shared_ptr<Expr> expr = term();
    // Sadece büyüklük/küçüklük kontrolleri kalmalı
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, 
                  TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        std::shared_ptr<Expr> right = term();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::term() {
    std::shared_ptr<Expr> expr = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        std::shared_ptr<Expr> right = factor();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::factor() {
    std::shared_ptr<Expr> expr = unary();

    while (match({TokenType::STAR, TokenType::SLASH, TokenType::MODULO})) { // MODULO eklendi
        Token op = previous();
        std::shared_ptr<Expr> right = unary();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }

    return expr;
}

// YENİ FONKSİYON: Tekli operatörleri işler
std::shared_ptr<Expr> Parser::unary() {
    if (match({TokenType::BANG, TokenType::NOT, TokenType::MINUS})) {
        Token op = previous();
        std::shared_ptr<Expr> right = unary();
        return std::make_shared<UnaryExpr>(op, right);
    }
    return primary();
}

std::shared_ptr<Expr> Parser::primary() {
    // 1. Mantıksal Değerleri Ekle (TRUE ve FALSE)
    if (match({TokenType::TRUE, TokenType::FALSE})) {
        return std::make_shared<LiteralExpr>(previous());
    }

    // 2. Sayılar ve Metinler
    if (match({TokenType::NUMBER, TokenType::STRING})) {
        return std::make_shared<LiteralExpr>(previous());
    }
    
    // 3. Değişkenler ve Görev Çağrıları
    if (match({TokenType::IDENTIFIER})) {
        std::shared_ptr<Expr> expr = std::make_shared<VariableExpr>(previous());
        // IDENTIFIER'dan sonra hemen '(' geliyorsa çağrıdır
        while (match({TokenType::LEFT_PAREN})) {
            expr = finishCall(expr);
        }
        return expr;
    }
    
    // 4. Gruplandırma (Parantezler)
    if (match({TokenType::LEFT_PAREN})) {
        std::shared_ptr<Expr> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Parantez kapatilmadi, ')' bekleniyor.");
        return expr;
    }

    throw std::runtime_error("Deger veya degisken bekleniyor. Bulunan simge: '" + peek().lexeme + "'");
}

std::shared_ptr<Expr> Parser::finishCall(std::shared_ptr<Expr> callee) {
    std::vector<std::shared_ptr<Expr>> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255) {
                std::cerr << "Cok fazla arguman!" << std::endl;
            }
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    Token paren = consume(TokenType::RIGHT_PAREN, "Fonksiyon cagrisindan sonra ')' bekleniyor.");
    return std::make_shared<CallExpr>(callee, paren, arguments);
}

// --- DIGER DEYIMLER ---

std::shared_ptr<Stmt> Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Gorev ismi bekleniyor.");
    consume(TokenType::LEFT_PAREN, "'(' bekleniyor.");
    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            parameters.push_back(consume(TokenType::IDENTIFIER, "Parametre ismi bekleniyor."));
            if (match({TokenType::COLON})) consume(TokenType::IDENTIFIER, "Tip bekleniyor."); 
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "')' bekleniyor.");
    if (match({TokenType::COLON})) consume(TokenType::IDENTIFIER, "Donus tipi bekleniyor.");

    consume(TokenType::LEFT_BRACE, "'{' bekleniyor.");
    return std::make_shared<FunctionStmt>(name, parameters, block());
}

std::shared_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "'(' bekleniyor.");
    std::shared_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "')' bekleniyor.");
    
    // block() çağırmak yerine doğrudan statement() çağırıyoruz.
    // Eğer kullanıcı { } kullanmışsa statement() zaten bunu BlockStmt olarak işleyecek.
    std::shared_ptr<Stmt> body = statement(); 
    
    return std::make_shared<WhileStmt>(condition, body);
}

std::shared_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    std::shared_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) value = expression();
    consume(TokenType::SEMICOLON, "Return'den sonra ';' bekleniyor.");
    return std::make_shared<ReturnStmt>(keyword, value);
}