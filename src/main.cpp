#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "interpreter.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Kullanim: ./uzum <dosya_adi.uz>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Dosya acilamadi: " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();

    // 1. Lexer: Metni simgelere ayır
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.scanTokens();

    // 2. Parser: Simgeleri mantıksal ağaca (AST) çevir
    Parser parser(tokens);
    std::vector<std::shared_ptr<Stmt>> statements;
    
    try {
        statements = parser.parse();
    } catch (const std::exception& e) {
        std::cerr << "SOZDIZIMI HATASI: " << e.what() << std::endl;
        return 1;
    }

    // 3. Interpreter: Ağacı çalıştır
    std::cout << "\n[Uzum Ciktisi]" << std::endl;
    std::cout << "------------------------" << std::endl;
    Interpreter interpreter;
    interpreter.interpret(statements);
    std::cout << "------------------------" << std::endl;

    return 0;
}