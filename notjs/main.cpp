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
    
    std::shared_ptr<JSValue> plus_operator(std::shared_ptr<JSValue> right) const override {
        return std::make_shared<JSNumber>(value + right->as_number()->value);
    };
    
    std::shared_ptr<JSNumber> as_number() const override {
        return std::make_shared<JSNumber>(*this);
    }
};

struct JSString: JSValue {
    std::string value;
    
    std::string serialize() const override {
        return value;
    };
    
    std::shared_ptr<JSValue> plus_operator(std::shared_ptr<JSValue> right) const override {
        return std::make_shared<JSNumber>(0);
    };
    
    std::shared_ptr<JSNumber> as_number() const override {
        return std::make_shared<JSNumber>(0);
    }
};

struct JSUndefined: JSValue {
    std::string serialize() const override {
        return "undefined";
    };
    
    std::shared_ptr<JSValue> plus_operator(std::shared_ptr<JSValue> right) const override {
        return std::make_shared<JSNumber>(0);
    };
    
    std::shared_ptr<JSNumber> as_number() const override {
        return std::make_shared<JSNumber>(0);
    }
};

struct Block: Node {
    std::vector<std::shared_ptr<Statement>> statements;

    void visit() const override {
        printf("Visit Block\n");
        for (auto& statement: statements) {
            statement->visit();
        }
    }
};

struct Parameter {
    Identifier name;
    
    Parameter(Identifier name): name(name) {};
};

struct FunctionDeclaration: Statement {
    StatementKind kind;
    Identifier& name;
    Block& body;
    std::vector<Parameter> parameters;
    
    FunctionDeclaration(Identifier& name, Block& body, std::vector<Parameter> parameters):
        name(name),
        body(body),
        parameters(parameters)
        {};
    
    void visit() const override {
        printf("Visit FunctionDeclaration\n");
        name.visit();
        body.visit();
    }
    
    std::shared_ptr<JSValue> evaluate(Scope& scope) const override {
        scope.functions.insert({ name.text, std::make_shared<FunctionDeclaration>(*this) });
        return std::make_shared<JSUndefined>();
    }
    
    std::shared_ptr<JSValue> execute(Scope& scope) const {
        for (auto& statement: body.statements) {
            statement->evaluate(scope);
            if (statement->getKind() == StatementKind::Return) {
                return statement->evaluate(scope);
            }
        }
        return std::make_shared<JSUndefined>();
    }
    
    StatementKind getKind() const override {
        return kind;
    }
};

void Identifier::visit() const {
    printf("Visit Identifier\n");
}
std::shared_ptr<JSValue> Identifier::evaluate(Scope& scope) const {
    auto it = scope.values.find(text);
    if (it == scope.values.end()) {
        return std::make_shared<JSUndefined>();
    } else {
        return it->second;
    }
    return std::make_shared<JSUndefined>();
}
std::shared_ptr<JSValue> Identifier::call(Scope &scope, std::vector<std::shared_ptr<JSValue>> values) const {
    auto it = scope.functions.find(text);
    if (it == scope.functions.end() ) {
        return std::make_shared<JSUndefined>();
    } else {
        auto function = it->second;

        for (std::size_t i = 0; i != values.size(); ++i) {
            auto value = values[i];
            auto s = function->parameters.at(i).name.text;
            scope.values.insert({ s, value });
        }
        
        // @TODO: push and then pop scope
        return function->execute(scope);
    }

    return std::make_shared<JSUndefined>();
}

struct NumericLiteral: Expression {
    std::string text;
    
    NumericLiteral(std::string text): text(text) {};
    
    void visit() const override {
        printf("Visit NumericLiteral\n");
    }
    
    std::shared_ptr<JSValue> evaluate(Scope& scope) const override {
        return std::make_shared<JSNumber>(std::stod(text));
    }
    
    std::shared_ptr<JSValue> call(Scope &scope, std::vector<std::shared_ptr<JSValue>> values) const override {
        return std::make_shared<JSUndefined>();
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
    
    std::shared_ptr<JSValue> evaluate(Scope& scope) const override {
        switch (operatorToken) {
        case Token::Plus:
            auto left_value = left.evaluate(scope);
            auto right_value = right.evaluate(scope);
            return left_value->plus_operator(std::move(right_value));
        }
    }
    
    std::shared_ptr<JSValue> call(Scope &scope, std::vector<std::shared_ptr<JSValue>> values) const override {
        return std::make_shared<JSUndefined>();
    }
};

struct CallExpression: Expression {
    Expression& expression;
    std::vector<std::shared_ptr<Expression>> arguments;
    
    CallExpression(Expression& expression, std::vector<std::shared_ptr<Expression>> arguments):
        expression(expression),
        arguments(arguments)
        {};
    
    void visit() const override {
        printf("Visit CallExpression\n");
    }
    
    std::shared_ptr<JSValue> evaluate(Scope& scope) const override {
        std::vector<std::shared_ptr<JSValue>> values {};
        
        for (auto argument: arguments) {
            auto value = argument->evaluate(scope);
            values.push_back(value);
        }
        
        return expression.call(scope, values);
    }
    
    std::shared_ptr<JSValue> call(Scope &scope, std::vector<std::shared_ptr<JSValue>> values) const override {
        return std::make_shared<JSUndefined>();
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
    
    std::shared_ptr<JSValue> evaluate(Scope& scope) const override {
        return expression.evaluate(scope);
    }
    
    StatementKind getKind() const override {
        return kind;
    }
};

struct SourceFile {
    std::vector<std::shared_ptr<Statement>> statements;
    
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
    
    // function foo(n) { return n + 99; }
    auto identifier_foo = Identifier { "foo" };
    auto identifier_n = Identifier { "n" };
    auto parameter_n = Parameter { identifier_n };
    auto a_block = Block();
    auto literal_99 = NumericLiteral("99");
    auto binary_expression_n_plus_99 = BinaryExpression(identifier_n, Token::Plus, literal_99);
    a_block.statements.push_back(std::make_shared<ReturnStatement>(binary_expression_n_plus_99));
    auto function_declaration_foo = FunctionDeclaration { identifier_foo, a_block, { parameter_n } };
    
    // function main() { return 50 + foo(1); }
    auto identifier_main = Identifier { "main" };
    auto block_main = Block();
    auto literal_50 = NumericLiteral("50");
    auto call_expression_foo = CallExpression(identifier_foo, { std::make_shared<NumericLiteral>("1") });
    auto binary_expression = BinaryExpression(literal_50, Token::Plus, call_expression_foo);
    block_main.statements.push_back(std::make_shared<ReturnStatement>(binary_expression));
    auto function_declaration_main = FunctionDeclaration { identifier_main, block_main, {} };
    
    auto source_file = SourceFile {};
    source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_foo));
    source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_main));
    
    globalScope.load(source_file);
    
    auto it = globalScope.functions.find("main");
    if (it == globalScope.functions.end() ) {
        std::cout << "No main function!" << std::endl;
    } else {
        std::cout << it->second->execute(globalScope)->serialize() << std::endl;
    }
    
    return 0;
}
