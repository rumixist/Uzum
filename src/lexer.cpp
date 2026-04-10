#include "lexer.hpp"
#include <map>

// Üzüm dilinin tüm anahtar kelimeleri
static std::map<std::string, TokenType> keywords = {
    {"import",   TokenType::VAR}, // Dosya dahil etme
    {"var",      TokenType::VAR},
    {"int",      TokenType::INT},
    {"num",      TokenType::NUM},
    {"str",      TokenType::STR},
    {"bool",     TokenType::BOOL},
    {"char",     TokenType::CHAR},
    {"qubit",    TokenType::QUBIT},
    {"print",    TokenType::PRINT},
    {"println",  TokenType::PRINTLN},
    {"console",  TokenType::CONSOLE},
    {"const",    TokenType::CONST},
    {"static",   TokenType::STATIC},
    {"true",     TokenType::TRUE},
    {"false",    TokenType::FALSE},
    {"and",      TokenType::AND}, // Mantıksal operatörler
    {"or",       TokenType::OR},
    {"not",      TokenType::NOT},
    {"xor",      TokenType::XOR},
    {"nand",     TokenType::NAND},
    {"nor",      TokenType::NOR},
    {"xnor",     TokenType::XNOR},
    {"switch",   TokenType::IF}, // Parser aşamasında ayrılacak
    {"case",     TokenType::IF},
    {"default",  TokenType::IF},
    {"otherwise",TokenType::ELSE},
    {"if",       TokenType::IF},
    {"else",     TokenType::ELSE},
    {"for",      TokenType::FOR},
    {"while",    TokenType::WHILE},
    {"function", TokenType::FUNCTION},
    {"class",    TokenType::FUNCTION},
    {"nil",      TokenType::NIL},
    {"break",    TokenType::RETURN},
    {"continue", TokenType::RETURN},
    {"repeat",   TokenType::RETURN},
    {"in",       TokenType::FOR},
    {"return",   TokenType::RETURN},
    {"do",       TokenType::WHILE},
    {"until",    TokenType::WHILE}
};

Lexer::Lexer(const std::string& source) : source(source) {}

bool Lexer::isAtEnd() {
    return current >= (int)source.length();
}

char Lexer::advance() {
    return source[current++];
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}

// Bir sonraki karakter beklenen karakter ise tüketir ve true döner
bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

void Lexer::addToken(TokenType type) {
    std::string text = source.substr(start, current - start);
    tokens.push_back({type, text, line});
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        // --- TEK KARAKTERLİ SİMGELER ---
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case '%': addToken(TokenType::MODULO); break;

        // --- OPERATÖRLER (DÜZELTİLMİŞ) ---
        case '-': 
            addToken(match('=') ? TokenType::MINUS_EQUAL : TokenType::MINUS); break; 
        case '+': 
            addToken(match('=') ? TokenType::PLUS_EQUAL : TokenType::PLUS); break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUALS); break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
        case '>':
            // BURASI: match('=') TRUE dönerse GREATER_EQUAL ekler ve '=' karakterini tüketir.
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;

        // --- BÖLME VE YORUM SATIRI ---
        case '/':
            if (match('/')) {
                // Yorum satırı: Satır sonuna kadar oku ama token ekleme
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::SLASH);
            }
            break;

        // --- GÖRMEZDEN GELİNECEK KARAKTERLER ---
        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            line++;
            break;

        // --- METİN VE DİĞERLERİ ---
        case '"': scanString(); break;

        default:
            if (isdigit(c)) {
                scanNumber();
            } else if (isalpha(c) || c == '_') {
                scanIdentifier();
            } else {
                // Bilinmeyen karakterleri şimdilik sessizce geçiyoruz
                // İstersen buraya bir cerr yazdırabilirsin
            }
            break;
    }
}

void Lexer::scanString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }
    if (isAtEnd()) return; 
    advance(); // Kapanış tırnağı
    addToken(TokenType::STRING);
}

void Lexer::scanNumber() {
    while (isdigit(peek())) advance();
    if (peek() == '.' && isdigit(source[current + 1])) {
        advance(); 
        while (isdigit(peek())) advance();
    }
    addToken(TokenType::NUMBER);
}

void Lexer::scanIdentifier() {
    while (isalnum(peek()) || peek() == '_') advance();
    std::string text = source.substr(start, current - start);
    TokenType type = keywords.count(text) ? keywords[text] : TokenType::IDENTIFIER;
    addToken(type);
}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.push_back({TokenType::END_OF_FILE, "", line});
    return tokens;
}