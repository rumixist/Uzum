#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include "ast.hpp"
#include "environment.hpp"

class Interpreter {
public:
    void interpret(const std::vector<std::shared_ptr<Stmt>>& statements) {
        try {
            for (const auto& stmt : statements) {
                execute(stmt);
            }
        } catch (const std::exception& e) {
            std::cerr << "Calisma Zamani Hatasi: " << e.what() << std::endl;
        }
    }

private:
    Environment environment;

    // Yardımcı mantık fonksiyonu
    bool isTrue(std::string value) {
        if (value == "true") return true;
        if (value == "false" || value == "nil") return false;
        try {
            return std::stoi(value) != 0;
        } catch (...) {
            return !value.empty();
        }
    }

    void execute(std::shared_ptr<Stmt> stmt) {
        if (auto varStmt = std::dynamic_pointer_cast<VarStmt>(stmt)) {
            std::string value = "nil";
            if (varStmt->initializer != nullptr) {
                value = evaluate(varStmt->initializer);
            }
            // ESKİ SATIRI SİLDİK, YERİNE BU BLOK GELDİ:
            try {
                // Değişken zaten var mı diye kontrol et
                environment.get(varStmt->name.lexeme);
                
                // Varsa: 'assign' çağır ki içindeki 'isConstant' kontrolü çalışsın
                environment.assign(varStmt->name.lexeme, value);
            } catch (const std::runtime_error&) {
                // Yoksa: Yeni bir değişken olarak tanımla
                environment.define(varStmt->name.lexeme, value, varStmt->type, varStmt->isConstant);
            }
        }
        else if (auto printStmt = std::dynamic_pointer_cast<PrintStmt>(stmt)) {
            std::string value = evaluate(printStmt->expression);
    
            // Metinleri tırnaklardan arındırma (Zaten sende olabilir, kontrol et)
            if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            } 
            else {
                // SAYI GÜZELLEŞTİRME MANTIĞI:
                try {
                    size_t pos;
                    double num = std::stod(value, &pos);
                    
                    // Eğer değer gerçekten bir sayıysa ve sonuna kadar okunabilmişse
                    if (pos == value.length()) {
                        long long integral = static_cast<long long>(num);
                        // Eğer sayı tam sayıya eşitse (örn: 55.0000 == 55)
                        if (num == integral) {
                            value = std::to_string(integral);
                        } else {
                            // Eğer ondalıklıysa, gereksiz sondaki sıfırları sil
                            value.erase(value.find_last_not_of('0') + 1, std::string::npos);
                            if (value.back() == '.') value.pop_back();
                        }
                    }
                } catch (...) {
                    // Sayı değilse (true/false/nil vb.) dokunmadan devam et
                }
            }

            std::cout << value;
            if (printStmt->newLine) std::cout << std::endl;
        }
        else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
            if (isTrue(evaluate(ifStmt->condition))) {
                execute(ifStmt->thenBranch);
            }
        }
        else if (auto blockStmt = std::dynamic_pointer_cast<BlockStmt>(stmt)) {
            for (auto& s : blockStmt->statements) execute(s);
        }
        else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
            while (isTrue(evaluate(whileStmt->condition))) {
                execute(whileStmt->body);
            }
        }
        // execute fonksiyonunun içine ekle:
        else if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStmt>(stmt)) {
            evaluate(exprStmt->expression); 
        }
    }

    std::string evaluate(std::shared_ptr<Expr> expr) {
        if (auto assign = std::dynamic_pointer_cast<AssignExpr>(expr)) {
            std::string value = evaluate(assign->value);
            
            // İŞTE KRİTİK KONTROL BURADA ÇALIŞACAK:
            environment.assign(assign->name.lexeme, value); 
            
            return value;
        }
        if (auto literal = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
            return literal->value.lexeme;
        }
        if (auto variable = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            return environment.get(variable->name.lexeme);
        }
        if (auto binary = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
            std::string leftStr = evaluate(binary->left);
            std::string rightStr = evaluate(binary->right);

            switch (binary->op.type) {
                // --- MATEMATİKSEL VE KIYASLAMA OPERATÖRLERİ (Sayı gerektirir) ---
                case TokenType::PLUS:  return std::to_string(std::stod(leftStr) + std::stod(rightStr));
                case TokenType::MINUS: return std::to_string(std::stod(leftStr) - std::stod(rightStr));
                case TokenType::STAR:  return std::to_string(std::stod(leftStr) * std::stod(rightStr));
                case TokenType::SLASH: {
                    double r = std::stod(rightStr);
                    if (r == 0) throw std::runtime_error("Sifira bolme hatasi!");
                    return std::to_string(std::stod(leftStr) / r);
                }
                case TokenType::GREATER:     return (std::stod(leftStr) >  std::stod(rightStr)) ? "true" : "false";
                case TokenType::LESS:        return (std::stod(leftStr) <  std::stod(rightStr)) ? "true" : "false";
                case TokenType::EQUAL_EQUAL: return (leftStr == rightStr) ? "true" : "false";
                case TokenType::BANG_EQUAL:  return (leftStr != rightStr) ? "true" : "false";

                // --- MANTIKSAL OPERATÖRLER (Sayı gerektirmez, isTrue kullanır) ---
                case TokenType::AND: return (isTrue(leftStr) && isTrue(rightStr)) ? "true" : "false";
                case TokenType::OR:  return (isTrue(leftStr) || isTrue(rightStr)) ? "true" : "false";
                case TokenType::XOR: return (isTrue(leftStr) != isTrue(rightStr)) ? "true" : "false";

                default: return "nil";
            }
        }
        return "nil";
    }
};

#endif