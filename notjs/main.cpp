#include "main.h"

struct JSNumber : JSValue {
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

struct JSBoolean : JSValue {
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

struct JSString : JSValue {
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

struct JSUndefined : JSValue {
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

struct Block : Node {
  std::vector<std::shared_ptr<Statement>> statements;

  void visit() const override {
    printf("Visit Block\n");
    for (auto &statement : statements) {
      statement->visit();
    }
  }
};

struct Parameter {
  const Identifier name;

  Parameter(const Identifier name) : name(name){};
};

struct FunctionDeclaration : Statement {
  StatementKind kind;
  const Identifier &name;
  Block &body;
  std::vector<Parameter> parameters;

  FunctionDeclaration(const Identifier &name, Block &body,
                      std::vector<Parameter> parameters)
      : name(name), body(body), parameters(parameters){};

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
    for (auto &statement : body.statements) {
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

    chain.scopes.push_back(function_scope);
    auto function_return_value = function->execute(chain);
    chain.scopes.pop_back();

    return function_return_value;
  }

  return std::make_shared<JSUndefined>();
}

struct TrueKeyword : Expression {
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

struct FalseKeyword : Expression {
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

struct NumericLiteral : Expression {
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

struct BinaryExpression : Expression {
  const Expression &left;
  const Expression &right;
  const Token operatorToken;

  BinaryExpression(const Expression &left, const Token operatorToken,
                   const Expression &right)
      : left(left), operatorToken(operatorToken), right(right){};

  void visit() const override {
    printf("Visit BinaryExpression\n");
    left.visit();
    right.visit();
  }

  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    switch (operatorToken) {
    case Token::Plus: {
      auto left_value = left.evaluate(chain);
      auto right_value = right.evaluate(chain);
      //                std::cout << left_value->serialize() << " + " <<
      //                right_value->serialize() << std::endl;
      return left_value->plus_operator(std::move(right_value));
    }
    case Token::Minus: {
      auto left_value = left.evaluate(chain);
      auto right_value = right.evaluate(chain);
      //                std::cout << left_value->serialize() << " - " <<
      //                right_value->serialize() << std::endl;
      return left_value->minus_operator(std::move(right_value));
    }
    case Token::EqualsEqualsEquals: {
      auto left_value = left.evaluate(chain);
      auto right_value = right.evaluate(chain);
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

struct ConditionalExpression : Expression {
  const Expression &condition;
  const Expression &whenTrue;
  const Expression &whenFalse;
  Token operatorToken;

  ConditionalExpression(const Expression &condition, const Expression &whenTrue,
                        const Expression &whenFalse)
      : condition(condition), whenTrue(whenTrue), whenFalse(whenFalse){};

  void visit() const override { printf("Visit ConditionalExpression\n"); }

  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    bool result = condition.evaluate(chain)->as_boolean()->value;

    if (result) {
      return whenTrue.evaluate(chain);
    } else {
      return whenFalse.evaluate(chain);
    }
  }

  std::shared_ptr<JSValue>
  call(Chain &chain,
       std::vector<std::shared_ptr<JSValue>> values) const override {
    return std::make_shared<JSUndefined>();
  }
};

struct CallExpression : Expression {
  const Expression &expression;
  std::vector<std::shared_ptr<Expression>> arguments;

  CallExpression(const Expression &expression,
                 std::vector<std::shared_ptr<Expression>> arguments)
      : expression(expression), arguments(arguments){};

  void visit() const override { printf("Visit CallExpression\n"); }

  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    std::vector<std::shared_ptr<JSValue>> values{};

    for (auto argument : arguments) {
      auto value = argument->evaluate(chain);
      values.push_back(value);
    }

    return expression.call(chain, values);
  }

  std::shared_ptr<JSValue>
  call(Chain &chain,
       std::vector<std::shared_ptr<JSValue>> values) const override {
    return std::make_shared<JSUndefined>();
  }
};

struct IfStatement : Statement {
  const StatementKind kind;
  const Block &thenStatement;
  const BinaryExpression &expression;

  IfStatement(const BinaryExpression &expression, const Block &thenStatement)
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

struct ReturnStatement : Statement {
  const StatementKind kind;
  const Expression &expression;

  ReturnStatement(const Expression &expression)
      : kind(StatementKind::Return), expression(expression){};

  void visit() const override {
    printf("Visit ReturnStatement\n");
    expression.visit();
  }

  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    return expression.evaluate(chain);
  }

  StatementKind getKind() const override { return kind; }
};

struct SourceFile {
  std::vector<std::shared_ptr<Statement>> statements;

  void visit() const {
    printf("Visit SourceFile\n");
    for (auto &statement : statements) {
      statement->visit();
    }
  }

  void evaluate(Chain &chain) const {
    for (auto &statement : statements) {
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

int main(int argc, const char *argv[]) {
  auto globalScope = Scope{};
  auto chain = Chain{};
  chain.scopes.push_back(globalScope);

  // fib
  auto identifier_fib = Identifier{"fib"};
  auto parameter_n = Parameter{Identifier{"n"}};
  auto block_fib = Block();

  auto first_if_block = Block();
  first_if_block.statements.push_back(
      std::make_shared<ReturnStatement>(NumericLiteral("1")));
  block_fib.statements.push_back(std::make_shared<IfStatement>(
      BinaryExpression{Identifier{"n"}, Token::EqualsEqualsEquals,
                       NumericLiteral{"1"}},
      first_if_block));

  auto second_if_block = Block();
  second_if_block.statements.push_back(
      std::make_shared<ReturnStatement>(NumericLiteral("1")));
  block_fib.statements.push_back(std::make_shared<IfStatement>(
      BinaryExpression{Identifier{"n"}, Token::EqualsEqualsEquals,
                       NumericLiteral{"2"}},
      second_if_block));

  auto bb_left =
      CallExpression{Identifier("fib"),
                     {std::make_shared<BinaryExpression>(
                         Identifier{"n"}, Token::Minus, NumericLiteral("1"))}};
  auto bb_right =
      CallExpression{Identifier("fib"),
                     {std::make_shared<BinaryExpression>(
                         Identifier{"n"}, Token::Minus, NumericLiteral("2"))}};
  auto bb = BinaryExpression{bb_left, Token::Plus, bb_right};
  block_fib.statements.push_back(std::make_shared<ReturnStatement>(bb));

  auto function_declaration_fib =
      FunctionDeclaration{identifier_fib, block_fib, {parameter_n}};

  // main
  auto identifier_main = Identifier{"main"};
  auto block_main = Block();
  auto ce = CallExpression{Identifier("fib"),
                           {std::make_shared<NumericLiteral>("25")}};
  block_main.statements.push_back(std::make_shared<ReturnStatement>(ce));
  auto function_declaration_main =
      FunctionDeclaration{identifier_main, block_main, {}};

  auto source_file = SourceFile{};
  source_file.statements.push_back(
      std::make_shared<FunctionDeclaration>(function_declaration_fib));
  source_file.statements.push_back(
      std::make_shared<FunctionDeclaration>(function_declaration_main));

  chain.load(source_file);

  auto main_function = chain.lookup_function("main");
  if (!main_function) {
    std::cout << "No main function!" << std::endl;
    return 1;
  }

  std::cout << main_function->execute(chain)->serialize() << std::endl;

  return 0;
}
