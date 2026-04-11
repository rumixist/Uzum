#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <cmath>
#include "ast.hpp"
#include "environment.hpp"

class Interpreter {
public:
    void interpret(const std::vector<std::shared_ptr<Stmt>>& statements) {
        try {
            for (const auto& stmt : statements) {
                execute(stmt, environment);
            }
        } catch (const std::exception& e) {
            std::cerr << "Calisma Zamani Hatasi: " << e.what() << std::endl;
        } catch (const std::string& returnVal) {
            std::cerr << "Hata: 'return' komutu yalnizca bir gorev (task) icinde kullanilabilir!" << std::endl;
        }
    }

private:
    Environment environment; // Ana (Global) Bellek
    std::map<std::string, std::shared_ptr<FunctionStmt>> functions;

    bool checkType(TokenType type, const std::string& value) {
        if (type == TokenType::VAR) return true; // 'var' her şeyi kabul eder
        
        if (type == TokenType::INT || type == TokenType::NUM) {
            try {
                std::stod(value);
                return true;
            } catch (...) { return false; }
        }
        
        if (type == TokenType::BOOL) {
            return (value == "true" || value == "false");
        }
        
        if (type == TokenType::STR) {
            // Stringler genelde çift tırnakla başlar ve biter
            return (value.front() == '"' && value.back() == '"');
        }
        
        return true;
    }

    bool isTrue(std::string value) {
        if (value == "true") return true;
        if (value == "false") return false;
        // Diğer her şey (nil, "merhaba", 5) mantıksal olarak geçersiz/false sayılmalı 
        // veya hata fırlatmalı.
        return false; 
    }

    void execute(std::shared_ptr<Stmt> stmt, Environment& env) {
        if (auto varStmt = std::dynamic_pointer_cast<VarStmt>(stmt)) {
            std::string value = "nil";
            if (varStmt->initializer != nullptr) {
                value = evaluate(varStmt->initializer, env);
            }

            // TİP KONTROLÜ BURADA:
            if (value != "nil" && !checkType(varStmt->type, value)) {
                throw std::runtime_error("Hata: '" + varStmt->name.lexeme + "' degiskenine gecersiz tip atamasi!");
            }

            env.define(varStmt->name.lexeme, value, varStmt->type, varStmt->isConstant);
        }
        else if (auto printStmt = std::dynamic_pointer_cast<PrintStmt>(stmt)) {
            std::string value = evaluate(printStmt->expression, env);
            
            // Metin temizleme mantığı
            if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            } else {
                try {
                    size_t pos;
                    double num = std::stod(value, &pos);
                    if (pos == value.length()) {
                        long long integral = static_cast<long long>(num);
                        if (num == integral) value = std::to_string(integral);
                        else {
                            value.erase(value.find_last_not_of('0') + 1, std::string::npos);
                            if (value.back() == '.') value.pop_back();
                        }
                    }
                } catch (...) {}
            }
            std::cout << value;
            if (printStmt->newLine) std::cout << std::endl;
        }
        else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
            bool executed = false;

            // Tüm dalları (if ve else if) sırayla kontrol et
            for (const auto& branch : ifStmt->branches) {
                if (isTrue(evaluate(branch.condition, env))) {
                    execute(branch.statement, env);
                    executed = true;
                    break; // İlk doğru koşulu bulduysan diğerlerine bakma!
                }
            }

            // Hiçbir koşul tutmadıysa ve 'else' varsa onu çalıştır
            if (!executed && ifStmt->elseBranch != nullptr) {
                execute(ifStmt->elseBranch, env);
            }
        }
        else if (auto blockStmt = std::dynamic_pointer_cast<BlockStmt>(stmt)) {
            Environment blockEnv(&env);
            for (auto& s : blockStmt->statements) execute(s, blockEnv);
        }
        else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
            while (isTrue(evaluate(whileStmt->condition, env))) {
                execute(whileStmt->body, env); // env eklendi
            }
        }
        else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
            evaluate(exprStmt->expression, env);
        }
        else if (auto funcStmt = std::dynamic_pointer_cast<FunctionStmt>(stmt)) {
            functions[funcStmt->name.lexeme] = funcStmt;
        }
        else if (auto returnStmt = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
            std::string value = "nil";
            if (returnStmt->value != nullptr) {
                value = evaluate(returnStmt->value, env);
            }
            throw value; 
        }
    }

    std::string evaluate(std::shared_ptr<Expr> expr, Environment& env) {
        if (auto assign = std::dynamic_pointer_cast<AssignExpr>(expr)) {
            std::string value = evaluate(assign->value, env);
            env.assign(assign->name.lexeme, value); // env üzerinden atama
            return value;
        }
        if (auto literal = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
            // Eğer Lexer'dan gelen token tipi zaten TRUE veya FALSE ise
            if (literal->value.type == TokenType::TRUE) return "true";
            if (literal->value.type == TokenType::FALSE) return "false";
            
            // Değilse (Sayı veya String ise) lexeme'i döndür
            return literal->value.lexeme;
        }
        if (auto variable = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            return env.get(variable->name.lexeme); // env üzerinden erişim
        }
        if (auto call = std::dynamic_pointer_cast<CallExpr>(expr)) {
            auto varExpr = std::dynamic_pointer_cast<VariableExpr>(call->callee);
            if (!varExpr) throw std::runtime_error("Gecersiz gorev cagrisi.");
            
            std::string name = varExpr->name.lexeme;
            if (functions.find(name) == functions.end()) {
                throw std::runtime_error("Hata: '" + name + "' adinda bir gorev bulunamadi.");
            }

            auto function = functions[name];

            // Lexical Scope: Fonksiyonlar globali (environment) parent olarak görmeli
            Environment localEnv(&environment);

            for (size_t i = 0; i < function->params.size(); i++) {
                // Argümanlar, fonksiyonun ÇAĞRILDIĞI ortamda (env) hesaplanır
                std::string val = evaluate(call->arguments[i], env);
                localEnv.define(function->params[i].lexeme, val, TokenType::VAR, false);
            }

            try {
                for (auto& s : function->body) execute(s, localEnv);
            } catch (const std::string& returnVal) {
                return returnVal;
            }
            return "nil";
        }
        if (auto binary = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
            std::string leftStr = evaluate(binary->left, env);
            std::string rightStr = evaluate(binary->right, env);

            switch (binary->op.type) {
                case TokenType::PLUS:  return std::to_string(std::stod(leftStr) + std::stod(rightStr));
                case TokenType::MINUS: return std::to_string(std::stod(leftStr) - std::stod(rightStr));
                case TokenType::STAR:  return std::to_string(std::stod(leftStr) * std::stod(rightStr));
                case TokenType::SLASH: {
                    double r = std::stod(rightStr);
                    if (r == 0) throw std::runtime_error("Sifira bolme hatasi!");
                    return std::to_string(std::stod(leftStr) / r);
                }
                case TokenType::MODULO: {
                    double left = std::stod(leftStr);
                    double right = std::stod(rightStr);
                    if (right == 0) throw std::runtime_error("Hata: Sifira bolme (modulo) hatasi!");
                    
                    // C++'ta double için modulo fmod ile yapılır
                    return std::to_string(std::fmod(left, right));
                }
                case TokenType::GREATER:
                    return (std::stod(leftStr) > std::stod(rightStr)) ? "true" : "false";

                case TokenType::LESS:
                    return (std::stod(leftStr) < std::stod(rightStr)) ? "true" : "false";

                // --- EKSİK OLAN VE EKLEMEN GEREKEN KISIM BURASI ---
                case TokenType::GREATER_EQUAL:
                    return (std::stod(leftStr) >= std::stod(rightStr)) ? "true" : "false";

                case TokenType::LESS_EQUAL:
                    return (std::stod(leftStr) <= std::stod(rightStr)) ? "true" : "false";
                case TokenType::EQUAL_EQUAL: {
                    // 1. Sayısal Karşılaştırma Denemesi (0.000000 == 0 durumu için)
                    try {
                        size_t pos1, pos2;
                        double l = std::stod(leftStr, &pos1);
                        double r = std::stod(rightStr, &pos2);
                        
                        // Eğer her iki değer de geçerli birer sayıysa, onları double olarak karşılaştır
                        if (pos1 == leftStr.length() && pos2 == rightStr.length()) {
                            return (l == r) ? "true" : "false";
                        }
                    } catch (...) {
                        // Eğer değerlerden biri sayı değilse (örneğin "test" == 5), 
                        // catch bloğuna düşer ve aşağıdan metinsel karşılaştırmaya devam eder.
                    }
                    
                    // 2. Metinsel veya Mantıksal Karşılaştırma (Varsayılan)
                    return (leftStr == rightStr) ? "true" : "false";
                }

                case TokenType::BANG_EQUAL: {
                    // Aynı mantığı Eşit Değil (!=) için de uygulayalım:
                    try {
                        size_t pos1, pos2;
                        double l = std::stod(leftStr, &pos1);
                        double r = std::stod(rightStr, &pos2);
                        if (pos1 == leftStr.length() && pos2 == rightStr.length()) {
                            return (l != r) ? "true" : "false";
                        }
                    } catch (...) {}
                    return (leftStr != rightStr) ? "true" : "false";
                }
                case TokenType::AND:
                case TokenType::OR:
                case TokenType::XOR: {
                    // 1. Tip Kontrolü (Gümrük)
                    bool leftIsBool = (leftStr == "true" || leftStr == "false");
                    bool rightIsBool = (rightStr == "true" || rightStr == "false");

                    if (!leftIsBool || !rightIsBool) {
                        throw std::runtime_error("Mantiksal operatorler sadece bool tipleriyle kullanilabilir!");
                    }

                    // 2. İşlem ve Kesin Dönüş
                    if (binary->op.type == TokenType::AND) {
                        return (isTrue(leftStr) && isTrue(rightStr)) ? "true" : "false";
                    }
                    if (binary->op.type == TokenType::OR) {
                        return (isTrue(leftStr) || isTrue(rightStr)) ? "true" : "false";
                    }
                    if (binary->op.type == TokenType::XOR) {
                        return (isTrue(leftStr) != isTrue(rightStr)) ? "true" : "false";
                    }
                    
                    // Eğer buraya kadar sızarsa (teorik olarak imkansız ama havacılık kuralı: her zaman break koy!)
                    break; 
                }

                case TokenType::NOT: // Eğer 'not' anahtar kelimesini unary olarak kullanacaksan
                    // Bu kısım genellikle unary içinde çözülür ama binary switch'inde hata vermemesi için:
                    return isTrue(rightStr) ? "false" : "true";

                default: return "nil";
            }
        }
        if (auto unary = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
            std::string rightStr = evaluate(unary->right, env);

            switch (unary->op.type) {
                case TokenType::MINUS: {
                    // Bilimsel olarak: Sayıyı negatife çevir
                    try {
                        double val = std::stod(rightStr);
                        return std::to_string(-val);
                    } catch (...) {
                        throw std::runtime_error("Hata: '-' operatoru sadece sayilarla kullanilabilir.");
                    }
                }
                case TokenType::BANG:
                case TokenType::NOT: {
                    // Mantıksal tersini al (!true -> false)
                    return isTrue(rightStr) ? "false" : "true";
                }
                default:
                    return "nil";
            }
        }
        return "nil";
    }
};

#endif