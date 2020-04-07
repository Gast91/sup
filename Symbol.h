#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>

class Symbol
{
public:
    std::string name;
    // The Symbol of an identifier - not all identifiers have a symbol
    // BuiltIn Type Definitions and Nested Scopes in a symbol table dont have a type, just a name
    Symbol* type;             
    friend class SymbolTable;

    Symbol(std::string n, Symbol* t = nullptr);
    virtual ~Symbol() = default;  // SymbolTable class will handle the deleting of Symbols

    virtual void Print() = 0;
};

// Symbols built into the language - Integers, Floats, Chars etc.
// Type is indicated by name, used as a lookup
class BuiltInSymbol : public Symbol
{
public:
    BuiltInSymbol(std::string n);

    void Print() override;
};

// Actual Variable Symbols containing a variable name and a type 
// (of type Symbol(BuiltInSymbol for now, later can be user defined))
class VariableSymbol : public Symbol
{
public:
    VariableSymbol(std::string n, Symbol* t);

    void Print() override;
};

// Symbols indicating a nested scope within the current scope
class NestedScope : public Symbol
{
public:
    NestedScope(std::string n);

    void Print() override;
};

// class FunctionSymbol : public Symbol ...
// class UserDefinedSymbol : public Symbol ...

// Abstract Data Type for tracking various symbols in the source code
// Each SymbolTable represents a scope in the source code
class SymbolTable
{
private:
    std::map<std::string, Symbol*> symbols;
    std::string scopeName;
    int scopeLevel;
    SymbolTable* parentScope;
    friend class SemanticAnalyzer;
public:
    SymbolTable(const std::string& name, const int level, SymbolTable* parent = nullptr);
    ~SymbolTable();  // Deleting of Symbols will be handled here

    bool DefineSymbol(Symbol* s);
    Symbol* LookUpSymbol(const std::string& symName);

    void Print();
};