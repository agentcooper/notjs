#include "main.h"

const bool kDebug = false;

template <typename First, typename... Rest>
void log(First first, Rest... rest)
{
  if (!kDebug) {
    return;
  }
  
  std::string sep = " ";
  std::string end = "\n";
  
  std::cout << first;
  ((std::cout << sep << rest), ...);
  std::cout << end;
}

class JSNumber : public JSValue {
public:
  const double value;
  
  std::string serialize() const override { return std::to_string(value); };
  
  JSNumber(double value) : value(value){};
  
  std::shared_ptr<JSValue>
  plus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSNumber>(value + right->as_number()->value);
  };
  
  std::shared_ptr<JSValue>
  minus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSNumber>(value - right->as_number()->value);
  };
  
  std::shared_ptr<JSBoolean>
  equalsequalsequals_operator(std::shared_ptr<JSValue> right) const override {
    bool res = fabs(value - right->as_number()->value) < 0.0001f;
    return std::make_shared<JSBoolean>(res);
  };
  
  std::shared_ptr<JSNumber> as_number() const override {
    return std::make_shared<JSNumber>(*this);
  }
  
  std::shared_ptr<JSBoolean> as_boolean() const override {
    return std::make_shared<JSBoolean>(true);
  }
};

class JSBoolean : public JSValue {
public:
  const bool value;
  
  std::string serialize() const override { return value ? "true" : "false"; };
  
  JSBoolean(const bool value) : value(value){};
  
  std::shared_ptr<JSValue>
  plus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSNumber>(value + right->as_number()->value);
  };
  
  std::shared_ptr<JSValue>
  minus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSNumber>(value - right->as_number()->value);
  };
  
  std::shared_ptr<JSBoolean>
  equalsequalsequals_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSBoolean>(false);
  };
  
  std::shared_ptr<JSNumber> as_number() const override {
    return std::make_shared<JSNumber>(0);
  }
  
  std::shared_ptr<JSBoolean> as_boolean() const override {
    return std::make_shared<JSBoolean>(*this);
  }
};

class JSString : public JSValue {
public:
  const std::string value;
  
  std::string serialize() const override { return value; };
  
  std::shared_ptr<JSValue>
  plus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSNumber>(0);
  };
  
  std::shared_ptr<JSValue>
  minus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSNumber>(0);
  };
  
  std::shared_ptr<JSBoolean>
  equalsequalsequals_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSBoolean>(false);
  };
  
  std::shared_ptr<JSNumber> as_number() const override {
    return std::make_shared<JSNumber>(0);
  }
  
  std::shared_ptr<JSBoolean> as_boolean() const override {
    return std::make_shared<JSBoolean>(false);
  }
};

class JSUndefined : public JSValue {
public:
  std::string serialize() const override { return "undefined"; };
  
  std::shared_ptr<JSValue>
  plus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSNumber>(0);
  };
  
  std::shared_ptr<JSValue>
  minus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSNumber>(0);
  };
  
  std::shared_ptr<JSBoolean>
  equalsequalsequals_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSBoolean>(false);
  };
  
  std::shared_ptr<JSNumber> as_number() const override {
    return std::make_shared<JSNumber>(0);
  }
  
  std::shared_ptr<JSBoolean> as_boolean() const override {
    return std::make_shared<JSBoolean>(false);
  }
};

class Block : public Node {
public:
  std::vector<std::shared_ptr<Statement>> statements;
  Block(std::vector<std::shared_ptr<Statement>> statements): statements(statements) {};
  
  void visit() const override {
    printf("Visit Block\n");
    for (auto &statement : statements) {
      statement->visit();
    }
  }
};

class Parameter {
public:
  const Identifier name;
  Parameter(const Identifier name) : name(name){};
};

class FunctionDeclaration : public Statement {
public:
  const StatementKind kind;
  const Identifier name;
  const Block body;
  const std::vector<Parameter> parameters;
  
  FunctionDeclaration(const Identifier name, const Block &body,
                      const std::vector<Parameter> parameters)
  : kind(StatementKind::FunctionDeclaration), name(name), body(body), parameters(parameters){};
  
  void visit() const override {
    printf("Visit FunctionDeclaration\n");
    name.visit();
    body.visit();
  }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    chain.scopes.back().functions.insert(
                                         {name.text, std::make_shared<FunctionDeclaration>(*this)});
    return std::make_shared<JSUndefined>();
  }
  
  std::shared_ptr<JSValue> execute(Chain &chain) const {
    log("FunctionDeclaration::execute", name.text);
    for (const auto &statement : body.statements) {
      auto value = statement->evaluate(chain);
      if (value) {
        return value;
      }
    }
    return nullptr;
  }
  
  StatementKind getKind() const override { return kind; }
};

void Identifier::visit() const { printf("Visit Identifier\n"); }
std::shared_ptr<JSValue> Identifier::evaluate(Chain &chain) const {
  return chain.lookup_value(text);
}
std::shared_ptr<JSValue>
Identifier::call(Chain &chain,
                 std::vector<std::shared_ptr<JSValue>> values) const {
  auto function = chain.lookup_function(text);
  
  if (function) {
    auto function_scope = Scope{};
    
    for (std::size_t i = 0; i != values.size(); ++i) {
      auto value = values[i];
      auto s = function->parameters.at(i).name.text;
      function_scope.values.insert({s, value});
    }
    
    log("Identifier::call", text, function_scope.serialize());
    
    chain.scopes.push_back(function_scope);
    auto function_return_value = function->execute(chain);
    chain.scopes.pop_back();
    
    return function_return_value;
  }
  
  return std::make_shared<JSUndefined>();
}

class TrueKeyword : public Expression {
public:
  void visit() const override { printf("Visit TrueKeyword\n"); }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    return std::make_shared<JSBoolean>(true);
  }
  
  std::shared_ptr<JSValue>
  call(Chain &chain,
       std::vector<std::shared_ptr<JSValue>> values) const override {
    return std::make_shared<JSUndefined>();
  }
};

class FalseKeyword : public Expression {
public:
  void visit() const override { printf("Visit FalseKeyword\n"); }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    return std::make_shared<JSBoolean>(false);
  }
  
  std::shared_ptr<JSValue>
  call(Chain &chain,
       std::vector<std::shared_ptr<JSValue>> values) const override {
    return std::make_shared<JSUndefined>();
  }
};

class NumericLiteral : public Expression {
public:
  const std::string text;
  
  NumericLiteral(const std::string text) : text(text){};
  
  void visit() const override { printf("Visit NumericLiteral\n"); }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    return std::make_shared<JSNumber>(std::stod(text));
  }
  
  std::shared_ptr<JSValue>
  call(Chain &chain,
       std::vector<std::shared_ptr<JSValue>> values) const override {
    return std::make_shared<JSUndefined>();
  }
};

class BinaryExpression : public Expression {
public:
  const std::shared_ptr<Expression> left;
  const std::shared_ptr<Expression> right;
  const Token operatorToken;
  
  BinaryExpression(const std::shared_ptr<Expression> left, const Token operatorToken,
                   const std::shared_ptr<Expression> right)
  : left(left), right(right), operatorToken(operatorToken) {};
  
  void visit() const override {
    printf("Visit BinaryExpression\n");
    left->visit();
    right->visit();
  }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    log("BinaryExpression::evaluate");
    switch (operatorToken) {
      case Token::Plus: {
        auto left_value = left->evaluate(chain);
        auto right_value = right->evaluate(chain);
        return left_value->plus_operator(std::move(right_value));
      }
      case Token::Minus: {
        auto left_value = left->evaluate(chain);
        auto right_value = right->evaluate(chain);
        return left_value->minus_operator(std::move(right_value));
      }
      case Token::EqualsEqualsEquals: {
        auto left_value = left->evaluate(chain);
        auto right_value = right->evaluate(chain);
        return left_value->equalsequalsequals_operator(std::move(right_value));
      }
    }
  }
  
  std::shared_ptr<JSValue>
  call(Chain &chain,
       std::vector<std::shared_ptr<JSValue>> values) const override {
    return std::make_shared<JSUndefined>();
  }
};

class ConditionalExpression : public Expression {
public:
  const std::shared_ptr<Expression> condition;
  const std::shared_ptr<Expression> whenTrue;
  const std::shared_ptr<Expression> whenFalse;
  
  ConditionalExpression(const std::shared_ptr<Expression> condition, const std::shared_ptr<Expression> whenTrue,
                        const std::shared_ptr<Expression> whenFalse)
  : condition(condition), whenTrue(whenTrue), whenFalse(whenFalse){};
  
  void visit() const override { printf("Visit ConditionalExpression\n"); }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    log("ConditionalExpression::evaluate");
    bool result = condition->evaluate(chain)->as_boolean()->value;
    
    if (result) {
      return whenTrue->evaluate(chain);
    } else {
      return whenFalse->evaluate(chain);
    }
  }
  
  std::shared_ptr<JSValue>
  call(Chain &chain,
       std::vector<std::shared_ptr<JSValue>> values) const override {
    return std::make_shared<JSUndefined>();
  }
};

class CallExpression : public Expression {
public:
  const std::shared_ptr<Expression> expression;
  const std::vector<std::shared_ptr<Expression>> arguments;
  
  CallExpression(const std::shared_ptr<Expression> expression,
                 const std::vector<std::shared_ptr<Expression>> arguments)
  : expression(expression), arguments(arguments){};
  
  void visit() const override { printf("Visit CallExpression\n"); }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    
    std::vector<std::shared_ptr<JSValue>> values{};
    
    for (const auto& argument : arguments) {
      auto value = argument->evaluate(chain);
      values.push_back(value);
    }
    
    log("CallExpression::evaluate", values.size());
    
    return expression->call(chain, values);
  }
  
  std::shared_ptr<JSValue>
  call(Chain &chain,
       std::vector<std::shared_ptr<JSValue>> values) const override {
    return std::make_shared<JSUndefined>();
  }
};

class IfStatement : public Statement {
public:
  const StatementKind kind;
  const Block thenStatement;
  const BinaryExpression expression;
  
  IfStatement(const BinaryExpression expression, const Block thenStatement)
  : kind(StatementKind::If), thenStatement(thenStatement),
  expression(expression){};
  
  void visit() const override {
    printf("Visit IfStatement\n");
    expression.visit();
  }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    auto value = expression.evaluate(chain);
    
    if (value->as_boolean()->value) {
      for (auto &statement : thenStatement.statements) {
        return statement->evaluate(chain);
      }
    }
    
    return nullptr;
  }
  
  StatementKind getKind() const override { return kind; }
};

class ReturnStatement : public Statement {
public:
  const StatementKind kind;
  const std::shared_ptr<Expression> expression;
  
  ReturnStatement(const std::shared_ptr<Expression> expression)
  : kind(StatementKind::Return), expression(expression){};
  
  void visit() const override {
    printf("Visit ReturnStatement\n");
    expression->visit();
  }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    log("ReturnStatement::evaluate");
    return expression->evaluate(chain);
  }
  
  StatementKind getKind() const override { return kind; }
};

struct VariableDeclaration {
  const Identifier name;
  const std::shared_ptr<Expression> initializer;
};

struct VariableDeclarationList {
  const std::vector<VariableDeclaration> declarations;
};

class VariableStatement : public Statement {
public:
  const StatementKind kind_;
  const VariableDeclarationList declarationList_;
  
  VariableStatement(const VariableDeclarationList declarationList)
  : kind_(StatementKind::VariableStatement), declarationList_(declarationList) {};
  
  void visit() const override {
    printf("Visit VariableStatement\n");
  }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    log("ReturnStatement::evaluate");
    
    for (const auto& declaration: declarationList_.declarations) {
      chain.set_value(declaration.name.text, declaration.initializer->evaluate(chain));
    }
    
    return nullptr;
  }
  
  StatementKind getKind() const override { return kind_; }
};

class SourceFile {
public:
  const std::string fileName;
  std::vector<std::shared_ptr<Statement>> statements;
  
  void visit() const {
    printf("Visit SourceFile\n");
    for (auto &statement : statements) {
      statement->visit();
    }
  }
  
  void evaluate(Chain &chain) const {
    log("SourceFile::evaluate");
    for (const auto &statement : statements) {
      statement->evaluate(chain);
    }
  }
};

void Chain::load(const SourceFile &sourceFile) { sourceFile.evaluate(*this); }

std::string Scope::serialize() const {
  std::string result = "Scope {";
  
  for (const auto &value : values) {
    result += value.first + " = " + value.second->serialize();
  }
  
  return result + "}";
};

std::shared_ptr<JSValue> Chain::lookup_value(const std::string name) const {
  for (size_t i = scopes.size() - 1; i >= 0; --i) {
    auto scope = scopes[i];
    
    auto it = scope.values.find(name);
    if (it != scope.values.end()) {
      return it->second;
    }
  }
  
  return std::make_shared<JSUndefined>();
}

void Chain::set_value(const std::string name, std::shared_ptr<JSValue> value) {
  scopes.back().values.insert({ name, value });
}

std::shared_ptr<FunctionDeclaration>
Chain::lookup_function(const std::string name) const {
  for (auto scope : scopes) {
    auto it = scope.functions.find(name);
    if (it != scope.functions.end()) {
      return it->second;
    }
  }
  return nullptr;
}

// see js/fib.js
SourceFile createFibonacciProgram() {
  // fib
  auto identifier_fib = Identifier{"fib"};
  auto parameter_n = Parameter{Identifier{"n"}};
  
  auto first_if_block = Block({ std::make_shared<ReturnStatement>(std::make_shared<NumericLiteral>("1")) });
  auto first_if_condition = BinaryExpression { std::make_shared<Identifier>("n"), Token::EqualsEqualsEquals, std::make_shared<NumericLiteral>("1") };
  
  auto second_if_block = Block({ std::make_shared<ReturnStatement>(std::make_shared<NumericLiteral>("1")) });
  auto second_if_condition = BinaryExpression { std::make_shared<Identifier>("n"), Token::EqualsEqualsEquals, std::make_shared<NumericLiteral>("2") };
  
  std::vector<std::shared_ptr<Expression>> fib_arguments_left {
    std::make_shared<BinaryExpression>(std::make_shared<Identifier>("n"), Token::Minus, std::make_shared<NumericLiteral>("1"))
  };
  auto binary_left = std::make_shared<CallExpression>(std::make_shared<Identifier>("fib"), fib_arguments_left);
  
  std::vector<std::shared_ptr<Expression>> fib_arguments_right {
    std::make_shared<BinaryExpression>(std::make_shared<Identifier>("n"), Token::Minus, std::make_shared<NumericLiteral>("2"))
  };
  auto binary_right = std::make_shared<CallExpression>(std::make_shared<Identifier>("fib"), fib_arguments_right);
  
  auto sum = std::make_shared<BinaryExpression>(binary_left, Token::Plus, binary_right);
  
  auto function_declaration_fib = FunctionDeclaration {
    identifier_fib,
    Block({
      std::make_shared<IfStatement>(first_if_condition, first_if_block),
      std::make_shared<IfStatement>(second_if_condition, second_if_block),
      std::make_shared<ReturnStatement>(sum),
    }),
    {parameter_n}
  };
  
  // main
  auto identifier_main = Identifier{"main"};
  std::vector<std::shared_ptr<Expression>> args = {std::make_shared<NumericLiteral>("25")};
  auto ce = std::make_shared<CallExpression>(std::make_shared<Identifier>("fib"), args);
  auto function_declaration_main = FunctionDeclaration {
    identifier_main,
    Block({ std::make_shared<ReturnStatement>(ce) }),
    {}
  };
  
  auto source_file = SourceFile{ "./js/fib.js" };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_fib));
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_main));
  
  return source_file;
}

// see js/let.js
SourceFile createLetProgram() {
  auto source_file = SourceFile{ "./js/let.js" };
  
  auto identifier_main = Identifier { "main" };
  
  auto let_a = std::make_shared<VariableStatement>(VariableDeclarationList {{
    VariableDeclaration { Identifier { "a" }, std::make_shared<NumericLiteral>("1") }
  }});
  source_file.statements.push_back(let_a);
  
  auto let_b = std::make_shared<VariableStatement>(VariableDeclarationList {{
    VariableDeclaration { Identifier { "b" }, std::make_shared<NumericLiteral>("2") }
  }});
  source_file.statements.push_back(let_b);

  auto function_declaration_main = FunctionDeclaration {
    identifier_main,
    Block({ std::make_shared<ReturnStatement>(std::make_shared<BinaryExpression>(std::make_shared<Identifier>("a"), Token::Plus, std::make_shared<Identifier>("b"))) }),
    {}
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_main));
  
  return source_file;
}

std::shared_ptr<JSValue> createScopeAndEvaluate(SourceFile source_file) {
  auto globalScope = Scope{};
  auto chain = Chain{};
  chain.scopes.push_back(globalScope);
  
  chain.load(source_file);
  
  auto main_function = chain.lookup_function("main");
  if (!main_function) {
    log("No main function!");
    return nullptr;
  }
  
  return main_function->execute(chain);
}

int main(int argc, const char *argv[]) {
  {
    auto source_file = createFibonacciProgram();
    auto value = createScopeAndEvaluate(source_file);
    if (!value) {
      return 1;
    }
    auto serialized_value = value->serialize();
    std::cout << source_file.fileName << ": " << serialized_value << std::endl;
    assert(serialized_value == "75025.000000");
  }
  
  {
    auto source_file = createLetProgram();
    auto value = createScopeAndEvaluate(source_file);
    if (!value) {
      return 1;
    }
    auto serialized_value = value->serialize();
    std::cout << source_file.fileName << ": " << serialized_value << std::endl;
    assert(serialized_value == "3.000000");
  }

  return 0;
}
