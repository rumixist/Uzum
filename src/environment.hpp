#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <string>
#include <map>
#include <iostream>
#include <stdexcept>
#include "lexer.hpp" // TokenType erişimi için

// Üzüm'ün her bir değişken hücresi
struct Variable {
    std::string value;
    TokenType type;
    bool isConstant;
};

class Environment {
public:
    // Değişkeni ilk kez belleğe yaz (Tanımlama)
    void define(const std::string& name, const std::string& value, TokenType type, bool isConstant) {
        // Tip kontrolü: Eğer int ise ondalığı otomatik budayalım (Pratik yaklaşım)
        std::string finalValue = value;
        if (type == TokenType::INT && value != "nil") {
            try {
                finalValue = std::to_string(static_cast<long long>(std::stod(value)));
            } catch (...) { /* Sayı değilse olduğu gibi bırak */ }
        }
        
        values[name] = {finalValue, type, isConstant};
    }

    // Var olan değişkenin değerini değiştir (Atama)
    void assign(const std::string& name, const std::string& value) {
        if (values.find(name) != values.end()) {
            Variable& var = values[name];

            // 1. SABİTLİK KONTROLÜ
            if (var.isConstant) {
                throw std::runtime_error("'" + name + "' sabittir, degeri degistirilemez!");
            }

            // 2. TİP GÜVENLİĞİ KONTROLÜ
            // Değişken dinamik (var) değilse, yeni gelen değerin tipine bakmalıyız
            if (var.type != TokenType::VAR) {
                if (!checkTypeCompatibility(var.type, value)) {
                    throw std::runtime_error("Tip uyumsuzlugu: '" + name + "' degiskeni bu degeri alamaz!");
                }
            }

            // Atama yaparken int budama kuralını buraya da uygulayalım
            if (var.type == TokenType::INT) {
                try {
                    var.value = std::to_string(static_cast<long long>(std::stod(value)));
                } catch (...) { var.value = value; }
            } else {
                var.value = value;
            }
            return;
        }
        throw std::runtime_error("Tanimsiz degisken uzerine atama: '" + name + "'");
    }

    // Değişkenin değerini oku
    std::string get(const std::string& name) {
        if (values.find(name) != values.end()) {
            return values[name].value;
        }
        throw std::runtime_error("Tanimsiz degisken okunmaya calisildi: '" + name + "'");
    }

private:
    std::map<std::string, Variable> values;

    // Basit bir tip doğrulama laboratuvarı
    bool checkTypeCompatibility(TokenType type, const std::string& value) {
        if (value == "nil") return true; // nil her kaba girer

        try {
            switch (type) {
                case TokenType::INT:
                case TokenType::NUM:
                    std::stod(value); // Sayıya çevrilebiliyor mu?
                    return true;
                case TokenType::BOOL:
                    return (value == "true" || value == "false");
                case TokenType::STR:
                    return true; // Şimdilik her şey string olabilir
                case TokenType::CHAR:
                    // Tırnaklar dahil 3 karakter ('A') veya tırnaksız 1 karakter
                    return (value.length() == 1 || (value.length() == 3 && value[0] == '\''));
                default:
                    return true;
            }
        } catch (...) {
            return false;
        }
    }
};

#endif