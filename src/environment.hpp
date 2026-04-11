#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <string>
#include <map>
#include <iostream>
#include <stdexcept>
#include "lexer.hpp" 

struct Variable {
    std::string value;
    TokenType type;
    bool isConstant;
};

class Environment {
public:
    Environment* enclosing; // Üst bellek zinciri

    // Global bellek için
    Environment() : enclosing(nullptr) {}

    // Yerel bellek (fonksiyonlar) için
    Environment(Environment* enclosing) : enclosing(enclosing) {}

    // Değişken Tanımlama
    void define(const std::string& name, const std::string& value, TokenType type, bool isConstant) {
        // Tanımlama anında tip kontrolü
        if (value != "nil" && type != TokenType::VAR) {
            if (!checkTypeCompatibility(type, value)) {
                throw std::runtime_error("Tip Hatasi: '" + name + "' degiskeni bu degeri kabul etmiyor!");
            }
        }

        std::string finalValue = value;
        if (type == TokenType::INT && value != "nil") {
            try {
                finalValue = std::to_string(static_cast<long long>(std::stod(value)));
            } catch (...) { }
        }
        
        values[name] = {finalValue, type, isConstant};
    }

    // Değişken Atama (a = x)
    void assign(const std::string& name, const std::string& value) {
        // 1. Kendi içimizde ara
        if (values.find(name) != values.end()) {
            Variable& var = values[name];

            if (var.isConstant) {
                throw std::runtime_error("Hata: '" + name + "' sabittir, degeri degistirilemez!");
            }

            if (var.type != TokenType::VAR) {
                if (!checkTypeCompatibility(var.type, value)) {
                    throw std::runtime_error("Tip Hatasi: '" + name + "' degiskeni bu degeri kabul etmiyor!");
                }
            }

            if (var.type == TokenType::INT) {
                try {
                    var.value = std::to_string(static_cast<long long>(std::stod(value)));
                } catch (...) { var.value = value; }
            } else {
                var.value = value;
            }
            return;
        }

        // 2. Üst belleğe sor
        if (enclosing != nullptr) {
            enclosing->assign(name, value);
            return;
        }

        throw std::runtime_error("Hata: Tanimlanmamis degisken: '" + name + "'");
    }

    // Değişken Okuma
    std::string get(const std::string& name) {
        if (values.find(name) != values.end()) {
            return values[name].value;
        }

        if (enclosing != nullptr) {
            return enclosing->get(name);
        }

        throw std::runtime_error("Tanimsiz degisken okunmaya calisildi: '" + name + "'");
    }

private:
    std::map<std::string, Variable> values;

    // Tip Uyumluluk Denetimi
    bool checkTypeCompatibility(TokenType type, const std::string& value) {
        if (value == "nil") return true;
        try {
            switch (type) {
                case TokenType::INT:
                case TokenType::NUM:
                    std::stod(value); // Sayıya çevrilebiliyor mu?
                    return true;
                case TokenType::BOOL:
                    return (value == "true" || value == "false");
                case TokenType::STR:
                    // String disiplini: Başında ve sonunda tırnak olmalı
                    return (value.length() >= 2 && value.front() == '"' && value.back() == '"');
                case TokenType::CHAR:
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