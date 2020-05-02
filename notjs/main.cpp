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
  
  std::shared_ptr<JSValue> call(Chain &chain, std::vector<std::shared_ptr<JSValue>> values) const override {
    throw std::runtime_error(this->serialize() + " is not a function");
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
  
  std::shared_ptr<JSValue> call(Chain &chain, std::vector<std::shared_ptr<JSValue> > values) const override {
    throw std::runtime_error(this->serialize() + " is not a function");
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
  
  std::shared_ptr<JSValue> call(Chain &chain, std::vector<std::shared_ptr<JSValue>> values) const override {
    throw std::runtime_error(this->serialize() + " is not a function");
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
    // @TO-DO: find a better way :-)
    if (right->serialize() == "undefined") {
      return std::make_shared<JSBoolean>(true);
    }
    
    return std::make_shared<JSBoolean>(false);
  };
  
  std::shared_ptr<JSNumber> as_number() const override {
    return std::make_shared<JSNumber>(0);
  }
  
  std::shared_ptr<JSBoolean> as_boolean() const override {
    return std::make_shared<JSBoolean>(false);
  }
  
  std::shared_ptr<JSValue> call(Chain &chain, std::vector<std::shared_ptr<JSValue>> values) const override {
    throw std::runtime_error("TypeError: undefined not a function.");
  }
};

class JSFunction : public JSValue {
public:
  const FunctionDeclaration declaration;
  Chain local_chain_;
  
  JSFunction(const FunctionDeclaration declaration, Chain& local_chain) :
  declaration(declaration), local_chain_(local_chain) {};
  
  std::string serialize() const override { return "Function {}"; };
  
  std::shared_ptr<JSValue>
  plus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSUndefined>();
  };
  
  std::shared_ptr<JSValue>
  minus_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSUndefined>();
  };
  
  std::shared_ptr<JSBoolean>
  equalsequalsequals_operator(std::shared_ptr<JSValue> right) const override {
    return std::make_shared<JSBoolean>(false);
  };
  
  std::shared_ptr<JSNumber> as_number() const override {
    return std::make_shared<JSNumber>(0);
  }
  
  std::shared_ptr<JSBoolean> as_boolean() const override {
    return std::make_shared<JSBoolean>(true);
  }
  
  std::shared_ptr<JSValue> call(Chain &chain, std::vector<std::shared_ptr<JSValue>> values) const override {
    auto function_scope = Scope{};
    
    for (std::size_t i = 0; i != values.size(); ++i) {
      auto value = values[i];
      auto s = declaration.parameters.at(i).name.text;
      function_scope.values.insert({s, value});
    }
    
    log(
        "JSFunction::call push, name =",
        declaration.name.text,
        "function_scope =",
        function_scope.serialize(),
        "local_chain =",
        local_chain_.serialize()
        );
    
    auto new_chain = chain.add(local_chain_).add(function_scope);
    auto function_return_value = declaration.execute(new_chain);
    log("JSFunction::call pop, name =", declaration.name.text);
    
    return function_return_value;
  }
};

// FunctionDeclaration
std::shared_ptr<JSValue> FunctionDeclaration::evaluate(Chain &chain) const {
  log("FunctionDeclaration::evaluate", name.text);
  auto function_value = std::make_shared<JSFunction>(*this, chain);
  chain.scopes.back().values.insert({ name.text, function_value });
  return nullptr;
}
std::shared_ptr<JSValue> FunctionDeclaration::execute(Chain &chain) const {
  log("FunctionDeclaration::execute", name.text);
  for (const auto &statement : body.statements) {
    auto value = statement->evaluate(chain);
    if (value) {
      return value;
    }
  }
  return nullptr;
}
StatementKind FunctionDeclaration::getKind() const { return kind; }

std::string FunctionDeclaration::serialize() const {
  std::string result = "function " + name.text + "(";
  
  for (const auto& parameter: parameters) {
    result += parameter.name.text + ", ";
  }
  result += ") {\n";
  
  result += body.serialize("  ");
  
  return result + "}";
}

void Identifier::visit() const { printf("Visit Identifier\n"); }
std::shared_ptr<JSValue> Identifier::evaluate(Chain &chain) const {
  auto value = chain.lookup_value(text);
  log("Identifier::evaluate", text, "=", value->serialize());
  return value;
}

std::string Identifier::serialize() const {
  return text;
}

class TrueKeyword : public Expression {
public:
  void visit() const override { printf("Visit TrueKeyword\n"); }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    return std::make_shared<JSBoolean>(true);
  }
  
  std::string serialize() const override {
    return "true";
  }
};

class FalseKeyword : public Expression {
public:
  void visit() const override { printf("Visit FalseKeyword\n"); }
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    return std::make_shared<JSBoolean>(false);
  }
  
  std::string serialize() const override {
    return "false";
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
  
  std::string serialize() const override {
    return text;
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
  
  std::string serialize() const override {
    std::string result = left->serialize();
    
    switch (operatorToken) {
      case Token::Plus: result += " + "; break;
      case Token::Minus: result += " - "; break;
      case Token::EqualsEqualsEquals: result += " === "; break;
    }
    
    return result + right->serialize();
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
  
  std::string serialize() const override {
    return "ConditionalExpression";
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
    
    log("CallExpression::evaluate,", values.size(), "argument(s)");
    
    auto value = expression->evaluate(chain);
    
    log("CallExpression::evaluate, got value", value->serialize());
    
    return value->call(chain, values);
  }
  
  std::string serialize() const override {
    std::string result = expression->serialize() + "(";
    
    for (const auto& argument : arguments) {
      result += argument->serialize() + ", ";
    }
    
    return result + ")";
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
  
  std::string serialize() const override {
    return "if (" + expression.serialize() + ") {\n" + thenStatement.serialize("  ") + "}";
  }
};

std::string Block::serialize(const std::string offset) const {
  std::string result = "";
  
  for (const auto& statement: statements) {
    result += offset + statement->serialize() + ";\n";
  }
  
  return result;
};

class ReturnStatement : public Statement {
public:
  const StatementKind kind;
  const std::shared_ptr<Expression> expression;
  
  ReturnStatement(const std::shared_ptr<Expression> expression)
  : kind(StatementKind::Return), expression(expression){};
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    log("ReturnStatement::evaluate");
    return expression->evaluate(chain);
  }
  
  StatementKind getKind() const override { return kind; }
  
  std::string serialize() const override {
    return "return " + expression->serialize();
  }
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
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override {
    log("ReturnStatement::evaluate");
    
    for (const auto& declaration: declarationList_.declarations) {
      chain.set_value(declaration.name.text, declaration.initializer->evaluate(chain));
    }
    
    return nullptr;
  }
  
  StatementKind getKind() const override { return kind_; }
  
  std::string serialize() const override {
    return "VariableStatement";
  }
};

class SourceFile {
public:
  const std::string fileName;
  std::vector<std::shared_ptr<Statement>> statements;
  
  void evaluate(Chain &chain) const {
    log("SourceFile::evaluate");
    for (const auto &statement : statements) {
      statement->evaluate(chain);
    }
  }
  
  std::string serialize() {
    std::string result = "";
    
    for (const auto &statement : statements) {
      result += statement->serialize() + "\n";
    }
    
    return result;
  }
};

void Chain::load(const SourceFile &sourceFile) {
  sourceFile.evaluate(*this);
}

Chain Chain::add(const Chain& chain) const {
  auto result = Chain{};
  result.scopes.insert(result.scopes.end(), this->scopes.begin(), this->scopes.end());
  result.scopes.insert(result.scopes.end(), chain.scopes.begin(), chain.scopes.end());
  return result;
}

Chain Chain::add(const Scope& scope) const {
  auto result = Chain{};
  result.scopes.insert(result.scopes.end(), this->scopes.begin(), this->scopes.end());
  result.scopes.push_back(scope);
  return result;
}

std::string Scope::serialize() const {
  std::string result = "Scope {";
  
  bool first = true;
  for (const auto &value : values) {
    if (!first) {
      result += ", ";
    }
    result += value.first + " = " + value.second->serialize();
    first = false;
  }
  
  return result + "}";
};

std::string Chain::serialize() const {
  std::string result = "Chain {";
  
  bool first = true;
  for (const auto& scope: scopes) {
    if (!first) {
      result += ", ";
    }
    result += scope.serialize();
    first = false;
  }
  
  return result + "}";
};

std::shared_ptr<JSValue> Chain::lookup_value(const std::string name) const {
  for (std::vector<Scope>::const_reverse_iterator i = scopes.rbegin(); i != scopes.rend(); ++i) {
    auto scope = *i;
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
  
  auto let_a = std::make_shared<VariableStatement>(VariableDeclarationList {{
    VariableDeclaration { Identifier { "a" }, std::make_shared<NumericLiteral>("1") }
  }});
  source_file.statements.push_back(let_a);
  
  auto let_b = std::make_shared<VariableStatement>(VariableDeclarationList {{
    VariableDeclaration { Identifier { "b" }, std::make_shared<NumericLiteral>("2") }
  }});
  source_file.statements.push_back(let_b);
  
  auto function_declaration_main = FunctionDeclaration {
    Identifier { "main" },
    Block({ std::make_shared<ReturnStatement>(std::make_shared<BinaryExpression>(std::make_shared<Identifier>("a"), Token::Plus, std::make_shared<Identifier>("b"))) }),
    {}
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_main));
  
  return source_file;
}

// see js/closure.js
SourceFile createClosureProgram() {
  auto source_file = SourceFile{ "./js/closure.js" };
  
  // inner
  auto function_declaration_inner = FunctionDeclaration {
    Identifier { "inner" },
    Block({
      std::make_shared<ReturnStatement>(std::make_shared<BinaryExpression>(std::make_shared<Identifier>("a"), Token::Plus, std::make_shared<Identifier>("b")))
    }),
    {
      Parameter{ Identifier{ "b" } }
    }
  };
  
  // sum
  auto function_declaration_sum = FunctionDeclaration {
    Identifier { "sum" },
    Block({
      std::make_shared<FunctionDeclaration>(function_declaration_inner),
      std::make_shared<ReturnStatement>(std::make_shared<Identifier>("inner"))
    }),
    {
      Parameter{ Identifier{ "a" } }
    }
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_sum));
  
  // main
  std::vector<std::shared_ptr<Expression>> args_inner = {std::make_shared<NumericLiteral>("40")};
  auto call_expression_inner = std::make_shared<CallExpression>(std::make_shared<Identifier>("sum"), args_inner);
  
  std::vector<std::shared_ptr<Expression>> args_outer = {std::make_shared<NumericLiteral>("2")};
  auto call_expression_outer = std::make_shared<CallExpression>(call_expression_inner, args_outer);
  
  auto function_declaration_main = FunctionDeclaration {
    Identifier { "main" },
    Block({ std::make_shared<ReturnStatement>(call_expression_outer) }),
    {}
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_main));
  
  return source_file;
}

// see js/list.js
SourceFile createListProgram() {
  auto source_file = SourceFile{ "./js/list.js" };
  
  // inner
  std::vector<std::shared_ptr<Expression>> args_getter = {
    std::make_shared<Identifier>("a"),
    std::make_shared<Identifier>("b")
  };
  auto function_declaration_inner = FunctionDeclaration {
    Identifier { "inner" },
    Block({
      std::make_shared<ReturnStatement>(std::make_shared<CallExpression>(std::make_shared<Identifier>("getter"), args_getter))
    }),
    {
      Parameter{ Identifier{ "getter" } }
    }
  };
  // pair
  auto function_declaration_pair = FunctionDeclaration {
    Identifier { "pair" },
    Block({
      std::make_shared<FunctionDeclaration>(function_declaration_inner),
      std::make_shared<ReturnStatement>(std::make_shared<Identifier>("inner"))
    }),
    {
      Parameter{ Identifier{ "a" } },
      Parameter{ Identifier{ "b" } }
    }
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_pair));
  
  // getFirst
  auto function_declaration_getFirst = FunctionDeclaration {
    Identifier { "getFirst" },
    Block({
      std::make_shared<ReturnStatement>(std::make_shared<Identifier>("a"))
    }),
    {
      Parameter{ Identifier{ "a" } },
      Parameter{ Identifier{ "b" } }
    }
  };
  // first
  std::vector<std::shared_ptr<Expression>> args_pair = {
    std::make_shared<Identifier>("getFirst")
  };
  auto function_declaration_first = FunctionDeclaration {
    Identifier { "first" },
    Block({
      std::make_shared<FunctionDeclaration>(function_declaration_getFirst),
      std::make_shared<ReturnStatement>(std::make_shared<CallExpression>(std::make_shared<Identifier>("pair"), args_pair))
    }),
    {
      Parameter{ Identifier{ "pair" } }
    }
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_first));
  
  // getSecond
  auto function_declaration_getSecond = FunctionDeclaration {
    Identifier { "getSecond" },
    Block({
      std::make_shared<ReturnStatement>(std::make_shared<Identifier>("b"))
    }),
    {
      Parameter{ Identifier{ "a" } },
      Parameter{ Identifier{ "b" } }
    }
  };
  // second
  std::vector<std::shared_ptr<Expression>> args_pair_2 = {
    std::make_shared<Identifier>("getSecond")
  };
  auto function_declaration_second = FunctionDeclaration {
    Identifier { "second" },
    Block({
      std::make_shared<FunctionDeclaration>(function_declaration_getSecond),
      std::make_shared<ReturnStatement>(std::make_shared<CallExpression>(std::make_shared<Identifier>("pair"), args_pair_2))
    }),
    {
      Parameter{ Identifier{ "pair" } }
    }
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_second));
  
  // sum
  auto first_if_condition = BinaryExpression { std::make_shared<Identifier>("list"), Token::EqualsEqualsEquals, std::make_shared<Identifier>("undefined") };
  auto first_if_block = Block({ std::make_shared<ReturnStatement>(std::make_shared<NumericLiteral>("0")) });
  
  std::vector<std::shared_ptr<Expression>> left_args = {
    std::make_shared<Identifier>("list")
  };
  auto left = std::make_shared<CallExpression>(std::make_shared<Identifier>("first"), left_args);
  
  
  std::vector<std::shared_ptr<Expression>> right_inner_args = {
    std::make_shared<Identifier>("list")
  };
  auto right_inner = std::make_shared<CallExpression>(std::make_shared<Identifier>("second"), left_args);
  std::vector<std::shared_ptr<Expression>> right_args = {
    right_inner
  };
  auto right = std::make_shared<CallExpression>(std::make_shared<Identifier>("sum"), right_args);
  
  auto function_declaration_sum = FunctionDeclaration {
    Identifier { "sum" },
    Block({
      std::make_shared<IfStatement>(first_if_condition, first_if_block),
      std::make_shared<ReturnStatement>(std::make_shared<BinaryExpression>(left, Token::Plus, right))
    }),
    {
      Parameter{ Identifier{ "list" } }
    }
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_sum));
  
  // main
  std::vector<std::shared_ptr<Expression>> pair_4_args = {
    std::make_shared<NumericLiteral>("4"),
  };
  
  std::vector<std::shared_ptr<Expression>> pair_3_args = {
    std::make_shared<NumericLiteral>("3"),
    std::make_shared<CallExpression>(std::make_shared<Identifier>("pair"), pair_4_args)
  };
  
  std::vector<std::shared_ptr<Expression>> pair_2_args = {
    std::make_shared<NumericLiteral>("2"),
    std::make_shared<CallExpression>(std::make_shared<Identifier>("pair"), pair_3_args)
  };
  
  std::vector<std::shared_ptr<Expression>> pair_1_args = {
    std::make_shared<NumericLiteral>("1"),
    std::make_shared<CallExpression>(std::make_shared<Identifier>("pair"), pair_2_args)
  };
  std::vector<std::shared_ptr<Expression>> pair_1 = {std::make_shared<CallExpression>(std::make_shared<Identifier>("pair"), pair_1_args)};
  
  auto call_expression_first = std::make_shared<CallExpression>(std::make_shared<Identifier>("sum"), pair_1);
  
  auto function_declaration_main = FunctionDeclaration {
    Identifier { "main" },
    Block({ std::make_shared<ReturnStatement>(call_expression_first) }),
    {}
  };
  source_file.statements.push_back(std::make_shared<FunctionDeclaration>(function_declaration_main));
  
  return source_file;
}

std::shared_ptr<JSValue> createScopeAndEvaluate(SourceFile source_file) {
  auto globalScope = Scope {};
  auto chain = Chain {};
  
  chain.scopes.push_back(globalScope);
  
  chain.load(source_file);
  
  auto main_function = chain.lookup_value("main");
  if (!main_function) {
    log("No main function!");
    return nullptr;
  }
  
  return main_function->call(chain, {});
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
  
  {
    auto source_file = createClosureProgram();
    auto value = createScopeAndEvaluate(source_file);
    if (!value) {
      return 1;
    }
    auto serialized_value = value->serialize();
    std::cout << source_file.fileName << ": " << serialized_value << std::endl;
    assert(serialized_value == "42.000000");
  }
  
  {
    auto source_file = createListProgram();
    auto value = createScopeAndEvaluate(source_file);
    if (!value) {
      return 1;
    }
    auto serialized_value = value->serialize();
    std::cout << source_file.fileName << ": " << serialized_value << std::endl;
    assert(serialized_value == "10.000000");
  }
  
  return 0;
}
