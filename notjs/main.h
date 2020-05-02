//
//  main.h
//  notjs
//
//  Created by Artem Tyurin on 4/13/20.
//  Copyright © 2020 Artem Tyurin. All rights reserved.
//

#ifndef main_h
#define main_h

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <math.h>
#include <cassert>
#include <stdexcept>

enum class Token {
  Plus,
  Minus,
  EqualsEqualsEquals,
};

enum class StatementKind {
  VariableStatement,
  FunctionDeclaration,
  Return,
  If,
};

class Node {
public:
  virtual void visit() const = 0;
};

class FunctionDeclaration;


class JSUndefined;
class JSString;
class JSNumber;
class JSBoolean;

class Chain;

class JSValue {
public:
  virtual std::string serialize() const = 0;
  virtual std::shared_ptr<JSValue> plus_operator(std::shared_ptr<JSValue> right) const = 0;
  virtual std::shared_ptr<JSValue> minus_operator(std::shared_ptr<JSValue> right) const = 0;
  virtual std::shared_ptr<JSBoolean> equalsequalsequals_operator(std::shared_ptr<JSValue> right) const = 0;
  virtual std::shared_ptr<JSNumber> as_number() const = 0;
  virtual std::shared_ptr<JSBoolean> as_boolean() const = 0;
  virtual std::shared_ptr<JSValue> call(Chain& chain, std::vector<std::shared_ptr<JSValue>> values) const = 0;
  virtual ~JSValue() {};
};

class Identifier;
class SourceFile;

class Scope {
public:
  std::map<std::string, std::shared_ptr<JSValue>> values {};
  std::string serialize() const;
};

class Chain {
public:
  std::vector<Scope> scopes {};
  std::shared_ptr<JSValue> lookup_value(const std::string name) const;
  void load(const SourceFile& sourceFile);
  void set_value(const std::string name, std::shared_ptr<JSValue> value);
  Chain add(const Chain& chain) const;
  
  Chain() {};
  Chain(const Chain &other) { scopes = other.scopes; }
  
  Chain add(const Scope& scope) const;
  
  std::string serialize() const;
};

class Expression: public Node {
public:
  virtual std::shared_ptr<JSValue> evaluate(Chain& chain) const = 0;
  virtual std::string serialize() const = 0;
  virtual ~Expression() {};
};

class Identifier: public Expression {
public:
  const std::string text;
  Identifier(const std::string text): text(text) {};
  void visit() const override;
  std::shared_ptr<JSValue> evaluate(Chain& chain) const override;
  std::string serialize() const override;
};

class Statement;

class Block {
public:
  std::vector<std::shared_ptr<Statement>> statements;
  Block(std::vector<std::shared_ptr<Statement>> statements): statements(statements) {};
  std::string serialize(const std::string offset) const;
};

class Statement {
public:
  virtual std::shared_ptr<JSValue> evaluate(Chain& chain) const = 0;
  virtual StatementKind getKind() const = 0;
  virtual std::string serialize() const = 0;
  virtual ~Statement() {};
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
  
  std::shared_ptr<JSValue> evaluate(Chain &chain) const override;
  StatementKind getKind() const override;
  
  std::shared_ptr<JSValue> execute(Chain &chain) const;
  
  std::string serialize() const override;
};

#endif /* main_h */
