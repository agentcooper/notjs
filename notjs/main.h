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

enum class Token {
    Plus,
    Minus,
    EqualsEqualsEquals,
};

enum class StatementKind {
    FunctionDeclaration,
    Return,
    If,
};

class Node {
public:
    virtual void visit() const = 0;
};

class JSString;
class JSNumber;
class JSBoolean;

class JSValue {
public:
    virtual std::string serialize() const = 0;
    virtual std::shared_ptr<JSValue> plus_operator(std::shared_ptr<JSValue> right) const = 0;
    virtual std::shared_ptr<JSValue> minus_operator(std::shared_ptr<JSValue> right) const = 0;
    virtual std::shared_ptr<JSBoolean> equalsequalsequals_operator(std::shared_ptr<JSValue> right) const = 0;
    virtual std::shared_ptr<JSNumber> as_number() const = 0;
    virtual std::shared_ptr<JSBoolean> as_boolean() const = 0;
    virtual ~JSValue() {};
};

class Identifier;
class Block;
class Statement;
class SourceFile;

class FunctionDeclaration;

class Scope {
public:
    std::map<std::string, std::shared_ptr<FunctionDeclaration>> functions {};
    std::map<std::string, std::shared_ptr<JSValue>> values {};
    std::string serialize() const;
};

class Chain {
public:
    std::vector<Scope> scopes {};
    std::shared_ptr<JSValue> lookup_value(const std::string name) const;
    std::shared_ptr<FunctionDeclaration> lookup_function(const std::string name) const;
    void load(const SourceFile& sourceFile);
};

class Expression: public Node {
public:
    virtual std::shared_ptr<JSValue> evaluate(Chain& chain) const = 0;
    virtual std::shared_ptr<JSValue> call(Chain& chain, std::vector<std::shared_ptr<JSValue>> values) const = 0;
    virtual ~Expression() {};
};

class Identifier: public Expression {
public:
    const std::string text;
    Identifier(const std::string text): text(text) {};
    void visit() const override;
    std::shared_ptr<JSValue> evaluate(Chain& chain) const override;
    std::shared_ptr<JSValue> call(Chain& chain, std::vector<std::shared_ptr<JSValue>> values) const override;
};

class Statement: public Node {
public:
    virtual std::shared_ptr<JSValue> evaluate(Chain& chain) const = 0;
    virtual StatementKind getKind() const = 0;
    virtual ~Statement() {};
};

#endif /* main_h */
