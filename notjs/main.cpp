#include <iostream>
#include <vector>
#include <map>

#include "main.h"

struct JSNumber: JSValue {
    double value;
    
    std::string serialize() const override {
        return std::to_string(value);
    };
    
    JSNumber(double value): value(value) {};
    
    std::unique_ptr<JSValue> plus_operator(std::unique_ptr<JSValue> right) const override {
        return std::make_unique<JSNumber>(value + right->as_number()->value);
    };
    
    std::unique_ptr<JSNumber> as_number() const override {
        return std::make_unique<JSNumber>(*this);
    }
};

struct JSString: JSValue {
    std::string value;
    
    std::string serialize() const override {
        return value;
    };
    
    std::unique_ptr<JSValue> plus_operator(std::unique_ptr<JSValue> right) const override {
        return std::make_unique<JSNumber>(0);
    };
    
    std::unique_ptr<JSNumber> as_number() const override {
        return std::make_unique<JSNumber>(0);
    }
};

struct JSUndefined: JSValue {
    std::string serialize() const override {
        return "undefined";
    };
    
    std::unique_ptr<JSValue> plus_operator(std::unique_ptr<JSValue> right) const override {
        return std::make_unique<JSNumber>(0);
    };
    
    std::unique_ptr<JSNumber> as_number() const override {
        return std::make_unique<JSNumber>(0);
    }
};

struct Block: Node {
    std::vector<std::unique_ptr<Statement>> statements;

    void visit() const override {
        printf("Visit Block\n");
        for (auto& statement: statements) {
            statement->visit();
        }
    }
};

struct FunctionDeclaration: Statement {
    StatementKind kind;
    Identifier& name;
    Block& body;
    
    FunctionDeclaration(Identifier& name, Block& body): name(name), body(body) {};
    
    void visit() const override {
        printf("Visit FunctionDeclaration\n");
        name.visit();
        body.visit();
    }
    
    std::unique_ptr<JSValue> evaluate(Scope& scope) const override {
        scope.functions.insert({ name.text, std::make_unique<FunctionDeclaration>(*this) });
        return std::make_unique<JSUndefined>();
    }
    
    std::unique_ptr<JSValue> execute(Scope& scope) const {
        for (auto& statement: body.statements) {
            statement->evaluate(scope);
            if (statement->getKind() == StatementKind::Return) {
                return statement->evaluate(scope);
            }
        }
        return std::make_unique<JSUndefined>();
    }
    
    StatementKind getKind() const override {
        return kind;
    }
};

void Identifier::visit() const {
    printf("Visit Identifier\n");
}
std::unique_ptr<JSValue> Identifier::evaluate(Scope& scope) const {
    return std::make_unique<JSUndefined>();
}
std::unique_ptr<JSValue> Identifier::call(Scope &scope) const {
    auto it = scope.functions.find(text);
    if (it == scope.functions.end() ) {
        return std::make_unique<JSUndefined>();
    } else {
        return it->second->execute(scope);
    }

    return std::make_unique<JSUndefined>();
}

struct NumericLiteral: Expression {
    std::string text;
    
    NumericLiteral(std::string text): text(text) {};
    
    void visit() const override {
        printf("Visit NumericLiteral\n");
    }
    
    std::unique_ptr<JSValue> evaluate(Scope& scope) const override {
        return std::make_unique<JSNumber>(std::stod(text));
    }
    
    std::unique_ptr<JSValue> call(Scope &scope) const override {
        return std::make_unique<JSUndefined>();
    }
};

struct BinaryExpression: Expression {
    Expression& left;
    Expression& right;
    Token operatorToken;
    
    BinaryExpression(Expression& left, Token operatorToken, Expression& right): left(left), operatorToken(operatorToken), right(right) {};
    
    void visit() const override {
        printf("Visit BinaryExpression\n");
        left.visit();
        right.visit();
    }
    
    std::unique_ptr<JSValue> evaluate(Scope& scope) const override {
        switch (operatorToken) {
        case Token::Plus:
            auto left_value = left.evaluate(scope);
            auto right_value = right.evaluate(scope);
            return left_value->plus_operator(std::move(right_value));
        }
    }
    
    std::unique_ptr<JSValue> call(Scope &scope) const override {
        return std::make_unique<JSUndefined>();
    }
};

struct CallExpression: Expression {
    Expression& expression;
    
    CallExpression(Expression& expression): expression(expression) {};
    
    void visit() const override {
        printf("Visit CallExpression\n");
    }
    
    std::unique_ptr<JSValue> evaluate(Scope& scope) const override {
        return expression.call(scope);
    }
    
    std::unique_ptr<JSValue> call(Scope &scope) const override {
        return std::make_unique<JSUndefined>();
    }
};

struct ReturnStatement: Statement {
    StatementKind kind;
    Expression& expression;
    
    ReturnStatement(Expression& expression): kind(StatementKind::Return), expression(expression) {};
    
    void visit() const override {
        printf("Visit ReturnStatement\n");
        expression.visit();
    }
    
    std::unique_ptr<JSValue> evaluate(Scope& scope) const override {
        return expression.evaluate(scope);
    }
    
    StatementKind getKind() const override {
        return kind;
    }
};

struct SourceFile {
    std::vector<std::unique_ptr<Statement>> statements;
    
    void visit() const {
        printf("Visit SourceFile\n");
        for (auto& statement: statements) {
            statement->visit();
        }
    }
    
    void evaluate(Scope& scope) const {
        for (auto& statement: statements) {
            statement->evaluate(scope);
        }
    }
};

void Scope::load(SourceFile& sourceFile) {
    sourceFile.evaluate(*this);
}

int main(int argc, const char* argv[]) {
    auto globalScope = Scope {};
    
    // function a() { return 666; }
    auto identifier_a = Identifier { "a" };
    auto a_block = Block();
    auto literal_666 = NumericLiteral("666");
    a_block.statements.push_back(std::make_unique<ReturnStatement>(literal_666));
    auto function_declaration_a = FunctionDeclaration { identifier_a, a_block };
    
    // function main() { return 1 + a(); }
    auto identifier_main = Identifier { "main" };
    auto block_main = Block();
    auto literal_10 = NumericLiteral("10");
    auto call_expression_a = CallExpression(identifier_a);
    auto binary_expression = BinaryExpression(literal_10, Token::Plus, call_expression_a);
    block_main.statements.push_back(std::make_unique<ReturnStatement>(binary_expression));
    auto function_declaration_main = FunctionDeclaration { identifier_main, block_main };
    
    auto source_file = SourceFile {};
    source_file.statements.push_back(std::make_unique<FunctionDeclaration>(function_declaration_a));
    source_file.statements.push_back(std::make_unique<FunctionDeclaration>(function_declaration_main));
    
    globalScope.load(source_file);
    
    auto it = globalScope.functions.find("main");
    if (it == globalScope.functions.end() ) {
         std::cout << "No main function!" << std::endl;
    } else {
         std::cout << it->second->execute(globalScope)->serialize() << std::endl;
    }
    
    return 0;
}
