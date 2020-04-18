//
//  main.h
//  notjs
//
//  Created by Artem Tyurin on 4/13/20.
//  Copyright © 2020 Artem Tyurin. All rights reserved.
//

#ifndef main_h
#define main_h

enum class Token {
    Plus,
};

enum class StatementKind {
    FunctionDeclaration,
    Return,
};

struct Node {
    virtual void visit() const = 0;
};

struct JSString;
struct JSNumber;

struct JSValue {
    virtual std::string serialize() const = 0;
    virtual std::shared_ptr<JSValue> plus_operator(std::shared_ptr<JSValue> right) const = 0;
    virtual std::shared_ptr<JSNumber> as_number() const = 0;
    virtual ~JSValue() {};
};

struct Identifier;
struct Block;
struct Statement;
struct SourceFile;

struct FunctionDeclaration;

struct Scope {
    std::map<std::string, std::shared_ptr<FunctionDeclaration>> functions {};
    std::map<std::string, std::shared_ptr<JSValue>> values {};
    void load(SourceFile& sourceFile);
};

struct Expression: Node {
    virtual std::shared_ptr<JSValue> evaluate(Scope& scope) const = 0;
    virtual std::shared_ptr<JSValue> call(Scope& scope, std::vector<std::shared_ptr<JSValue>> values) const = 0;
    virtual ~Expression() {};
};

struct Identifier: Expression {
    std::string text;
    Identifier(std::string text): text(text) {};
    void visit() const override;
    std::shared_ptr<JSValue> evaluate(Scope &scope) const override;
    std::shared_ptr<JSValue> call(Scope &scope, std::vector<std::shared_ptr<JSValue>> values) const override;
};

struct Statement: Node {
    virtual std::shared_ptr<JSValue> evaluate(Scope& scope) const = 0;
    virtual StatementKind getKind() const = 0;
    virtual ~Statement() {};
};

#endif /* main_h */
