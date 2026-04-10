#include "parser.hpp"
#include <iostream>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

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
        statements.push_back(statement());
    }
    return statements;
}

std::shared_ptr<Stmt> Parser::statement() {
    if (match({TokenType::LEFT_BRACE})) return std::make_shared<BlockStmt>(block());
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::PRINT, TokenType::PRINTLN})) return printStatement();
    
    if (match({TokenType::VAR, TokenType::CONST, TokenType::INT, 
               TokenType::STR, TokenType::NUM, TokenType::BOOL, 
               TokenType::CHAR})) {
        return varDeclaration();
    }

    return expressionStatement(); // Sadece bu kalsın
}

// --- Fonksiyonlar Ayrıldı ---

std::shared_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "'if' kosulundan once '(' bekleniyor."); // EKLE
    std::shared_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "'if' kosulundan sonra ')' bekleniyor."); // EKLE
    
    consume(TokenType::LEFT_BRACE, "'if' blogu icin '{' bekleniyor.");
    std::shared_ptr<Stmt> thenBranch = std::make_shared<BlockStmt>(block());
    return std::make_shared<IfStmt>(condition, thenBranch);
}

std::vector<std::shared_ptr<Stmt>> Parser::block() {
    std::vector<std::shared_ptr<Stmt>> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(statement());
    }
    consume(TokenType::RIGHT_BRACE, "Blok sonunda '}' bekleniyor.");
    return statements;
}

std::shared_ptr<Stmt> Parser::varDeclaration() {
    // statement() içinde zaten bir token tükettik (match ile), 
    // o token'ı previous() ile alıp ne olduğunu anlayalım.
    Token keyword = previous();
    bool isConstant = (keyword.type == TokenType::CONST);
    TokenType type = TokenType::VAR;

    // Eğer ilk kelime const ise, arkasından bir tip (int, str vb.) veya 'var' gelebilir
    if (isConstant) {
        if (match({TokenType::INT, TokenType::STR, TokenType::NUM, 
                   TokenType::BOOL, TokenType::CHAR, TokenType::VAR})) {
            type = previous().type;
        }
    } else {
        // İlk kelime const değilse, zaten tiplerden biridir (int, str, var vb.)
        type = keyword.type;
    }

    Token name = consume(TokenType::IDENTIFIER, "Degisken adi bekleniyor.");
    std::shared_ptr<Expr> initializer = nullptr;

    if (match({TokenType::EQUALS})) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Satir sonuna ';' bekleniyor.");

    return std::make_shared<VarStmt>(name, initializer, type, isConstant);
}

std::shared_ptr<Stmt> Parser::printStatement() {
    TokenType type = previous().type; 
    bool isNewLine = (type == TokenType::PRINTLN);
    
    consume(TokenType::LEFT_PAREN, "'(' bekleniyor.");
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::RIGHT_PAREN, "')' bekleniyor.");
    consume(TokenType::SEMICOLON, "';' bekleniyor.");
    
    return std::make_shared<PrintStmt>(expr, isNewLine);
}

// 1. En düşük öncelik: Karşılaştırmalar (>, <, ==)
// 1. En düşük öncelik: OR ve XOR (Giriş noktası)
std::shared_ptr<Stmt> Parser::expressionStatement() {
    // Önce ifadeyi bir oku (Bu bir atama da olabilir, düz ifade de)
    std::shared_ptr<Expr> expr = expression();
    
    // Satır sonu kontrolü
    consume(TokenType::SEMICOLON, "Ifade sonuna ';' bekleniyor.");
    return std::make_shared<ExpressionStmt>(expr);
}

// expression() fonksiyonun içine atama desteği ekleyelim (En düşük öncelik)
std::shared_ptr<Expr> Parser::expression() {
    if (check(TokenType::IDENTIFIER)) {
        if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::EQUALS) {
            Token name = advance(); // Değişken adı
            consume(TokenType::EQUALS, "Atama operatoru '=' bekleniyor.");
            std::shared_ptr<Expr> value = expression();
            return std::make_shared<AssignExpr>(name, value);
        }
    }
    return orExpression();
}

std::shared_ptr<Expr> Parser::orExpression() {
    std::shared_ptr<Expr> expr = andExpression();

    while (match({TokenType::OR, TokenType::XOR})) {
        Token op = previous();
        std::shared_ptr<Expr> right = andExpression();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// 2. Orta-Düşük: AND
std::shared_ptr<Expr> Parser::andExpression() {
    std::shared_ptr<Expr> expr = comparison();

    while (match({TokenType::AND})) {
        Token op = previous();
        std::shared_ptr<Expr> right = comparison();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// 3. Karşılaştırmalar (==, !=, >, <)
std::shared_ptr<Expr> Parser::comparison() {
    std::shared_ptr<Expr> expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, // EKLE
                  TokenType::LESS, TokenType::LESS_EQUAL,       // EKLE
                  TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        Token op = previous();
        std::shared_ptr<Expr> right = term();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// 4. Matematiksel işlemler (Zaten sende olan term ve factor fonksiyonları aynen kalıyor)
// Sadece comparison fonksiyonunun term'i çağırdığından emin ol.

// 2. Orta öncelik: Toplama ve Çıkarma
std::shared_ptr<Expr> Parser::term() {
    std::shared_ptr<Expr> expr = factor();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        std::shared_ptr<Expr> right = factor();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// 3. En yüksek öncelik: Çarpma ve Bölme
std::shared_ptr<Expr> Parser::factor() {
    std::shared_ptr<Expr> expr = primary();

    while (match({TokenType::STAR, TokenType::SLASH})) {
        Token op = previous();
        std::shared_ptr<Expr> right = primary();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

// 4. En temel birim (Sayı, Değişken ve Parantez)
std::shared_ptr<Expr> Parser::primary() {
    // Sayı veya Metin
    if (match({TokenType::NUMBER, TokenType::STRING})) {
        return std::make_shared<LiteralExpr>(previous());
    }

    // Değişken
    if (match({TokenType::IDENTIFIER})) {
        return std::make_shared<VariableExpr>(previous());
    }

    // GRUPLAMA: Parantez ( ) desteği
    if (match({TokenType::LEFT_PAREN})) {
        std::shared_ptr<Expr> expr = expression(); // Parantez içini baştan çöz
        consume(TokenType::RIGHT_PAREN, "Parantez kapatilmadi, ')' bekleniyor.");
        return expr;
    }
    
    // Hata mesajını detaylandıralım
    throw std::runtime_error("Deger veya degisken bekleniyor. Bulunan simge: '" + peek().lexeme + "'");
}

std::shared_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "'while' kosulundan once '(' bekleniyor."); // EKLE
    std::shared_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "'while' kosulundan sonra ')' bekleniyor."); // EKLE
    
    consume(TokenType::LEFT_BRACE, "Dongu blogu icin '{' bekleniyor.");
    std::shared_ptr<Stmt> body = std::make_shared<BlockStmt>(block());
    return std::make_shared<WhileStmt>(condition, body);
}

std::shared_ptr<Stmt> Parser::assignment() {
    // Eğer if (....) gibi bir şey assignment'a düştüyse burada patlar.
    // O yüzden önce IDENTIFIER kontrolü yapıyoruz ama '=' olup olmadığına bakıyoruz.
    if (check(TokenType::IDENTIFIER)) {
        // Bir sonraki token '=' mi? (current + 1 kontrolü)
        if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::EQUALS) {
            Token name = advance(); 
            consume(TokenType::EQUALS, "Atama operatoru '=' bekleniyor.");
            std::shared_ptr<Expr> value = expression();
            consume(TokenType::SEMICOLON, "Ifadenin sonuna ';' bekleniyor.");
            return std::make_shared<ExpressionStmt>(std::make_shared<AssignExpr>(name, value));
        }
    }

    // Eğer atama değilse, sadece bir ifadedir (Expression Statement)
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Ifade sonuna ';' bekleniyor.");
    return std::make_shared<ExpressionStmt>(expr);
}