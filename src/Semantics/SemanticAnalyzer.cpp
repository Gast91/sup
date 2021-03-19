//#include <sstream>    // TODO: REMOVE ME WHEN YOU DECIDE AGOUT ID

#include "SemanticAnalyzer.h"
#include "../Util/Error.h"
#include "../Util/Logger.h"
#include "../Util/Utility.h"

SemanticAnalyzer::~SemanticAnalyzer() { for (const auto& scope : symbolTable) delete scope; }

void SemanticAnalyzer::Visit(ASTNode& n)       { assert(("Semantic Analyzer visited base ASTNode class?!"      , false)); }
void SemanticAnalyzer::Visit(UnaryASTNode& n)  { assert(("Semantic Analyzer visited base UnaryASTNode class?!" , false)); }
void SemanticAnalyzer::Visit(BinaryASTNode& n) { assert(("Semantic Analyzer visited base BinaryASTNode class?!", false)); }
void SemanticAnalyzer::Visit(IntegerNode& n) {}

void SemanticAnalyzer::Visit(IdentifierNode& n)   // Every identifier will be hit here, so the address must be set here to the one in the symbol table
{
    if (const auto sym = currentScope->LookUpSymbol(std::get<0>(n.tokenInfo)); !sym)
    {
        failState = true;
        const auto [variableName, variableType, line, col] = n.tokenInfo;
        throw SymbolNotFoundException(n.tokenInfo, GetSourceLine ? GetSourceLine(line) : "");
    }
    else n.offset = sym->offset;
}

void SemanticAnalyzer::Visit(UnaryOperationNode& n) { n.expr->Accept(*this); }

void SemanticAnalyzer::Visit(BinaryOperationNode& n)
{
    n.left->Accept(*this);
    n.right->Accept(*this);
}

void SemanticAnalyzer::Visit(ConditionNode& n)
{
    n.left->Accept(*this);
    n.right->Accept(*this);
}

SymbolTable* SemanticAnalyzer::CreateNewScope(const ASTNode* n, const char* tag)
{
    // Generate a name for the new nested scope and add it as a symbol into the parent scope (current)
    const std::string nestedScopeName = GenerateID(n, tag);
    currentScope->DefineSymbol(new NestedScope(nestedScopeName));
    
    // New nested scope with the nested scope name, at a greater depth than the current with the current scope as its parent
    SymbolTable* nestedScope = new SymbolTable(nestedScopeName, currentScope->scopeLevel + 1, currentScope);
    
    // Current scope becomes this new scope
    symbolTable.push_back(nestedScope);
    currentScope = nestedScope;
    
    return nestedScope;
}

void SemanticAnalyzer::Visit(IfNode& n)
{
    // Visit Condition, Identifiers in the condition belong to the current scope
    n.condition->Accept(*this);

    // Make a new nested scope for the body of this if (or else_if)
    SymbolTable* nestedScope = CreateNewScope(&n, (n.type + "_").c_str());

    // Perform Semantic Analysis to the "contents" of this new scope
    n.body->Accept(*this);

    // After we are done with the body of this nested statement we go back to the parent scope
    currentScope = nestedScope->parentScope;
}

void SemanticAnalyzer::Visit(IfStatementNode& n)
{
    // Visit all if-else if statements under this umbrella if statement and perform semantic analysis
    for (const auto& ifN : n.ifNodes) ifN->Accept(*this);

    // Make a new nested scope for the body of the else
    SymbolTable* nestedScope = CreateNewScope(&n, "ELSE_");

    // Perform Semantic Analysis to the "contents" of this new scope
    if (n.elseBody) n.elseBody->Accept(*this);

    // After we are done with the body of this nested statement we go back to the parent scope
    currentScope = nestedScope->parentScope;
}

void SemanticAnalyzer::Visit(IterationNode& n) { assert(("Semantic Analyzer visited base IterationNode class?!", false)); }

void SemanticAnalyzer::Visit(WhileNode& n)
{
    // Visit Condition, Identifiers in the condition belong to the current scope
    n.condition->Accept(*this);

    // Make a new nested scope for the body of this while
    SymbolTable* nestedScope = CreateNewScope(&n, "WHILE_");

    // Perform Semantic Analysis to the "contents" of this new scope
    n.body->Accept(*this);

    // After we are done with the body of this nested statement we go back to the parent scope
    currentScope = nestedScope->parentScope;
}

void SemanticAnalyzer::Visit(DoWhileNode& n)
{
    // Make a new nested scope for the body of this do_while
    SymbolTable* nestedScope = CreateNewScope(&n, "DO_");

    // After we are done with the body of this nested statement we go back to the parent scope
    currentScope = nestedScope->parentScope;

    // Visit Condition. Identifiers in the condition belong to the parent scope (set back above)
    n.condition->Accept(*this);
}

void SemanticAnalyzer::Visit(CompoundStatementNode& n) { for (const auto& statement : n.statements) statement->Accept(*this); }

void SemanticAnalyzer::Visit(StatementBlockNode& n)
{
    // Make a new nested scope for the body of this block
    SymbolTable* nestedScope = CreateNewScope(&n, "BLOCK_");

    // Visit all the statements in this block
    for (const auto& statement : n.statements) statement->Accept(*this);

    // After we are done with the body of this nested statement block we go back to the parent scope
    currentScope = nestedScope->parentScope;
}

void SemanticAnalyzer::Visit(DeclareStatementNode& n)
{
    // Look up Declaration Node's type in the symbol table
    Symbol* symbolType = currentScope->LookUpSymbol(n.type.first);
    // Get the variable name from the Declaration's Identifier Node
    const auto [variableName, variableType, line, col] = n.identifier->tokenInfo;
    addressOffset -= 4;  // This shouldnt be hardcoded for int32's but for now we only have ints
    // Define a new VarSymbol using variable name and symbolType
    Symbol* variableSymbol = new VariableSymbol(variableName, std::to_string(addressOffset), symbolType);
    n.identifier->offset = std::to_string(addressOffset);
    if (!currentScope->DefineSymbol(variableSymbol))
    {
        failState = true;
        // The redefined identifier will not be stored as the program is semantically wrong and Semantic Analysis will stop
        delete variableSymbol; // Clean up this temporary and throw the RedefinitionException
        throw SymbolRedefinitionException(n.identifier->tokenInfo, GetSourceLine ? GetSourceLine(line) : "");
    }
}

void SemanticAnalyzer::Visit(DeclareAssignNode& n)
{
    n.left->Accept(*this);
    n.right->Accept(*this);
}

void SemanticAnalyzer::Visit(AssignStatementNode& n)
{
    IdentifierNode* identifier = dynamic_cast<IdentifierNode*>(n.left.get());
    if (const auto sym = currentScope->LookUpSymbol(std::get<0>(identifier->tokenInfo)); !sym)
    {
        failState = true;
        const auto& [tokName, tokType, line, col] = identifier->tokenInfo;
        throw SymbolNotFoundException(identifier->tokenInfo, GetSourceLine ? GetSourceLine(line) : "");
    }
    else identifier->offset = sym->offset;
    // Identifier's name (left) has been extracted. If we reached here we know the symbol's in the table so no need to visit left node
    n.right->Accept(*this);
}

void SemanticAnalyzer::Visit(ReturnStatementNode& n) { n.expr->Accept(*this); }

void SemanticAnalyzer::Visit(EmptyStatementNode& n) {}

void SemanticAnalyzer::Render(int isOpen) const
{
    if (failState || !root) return;
    for (const auto& scope : symbolTable) scope->Render(isOpen);
}

void SemanticAnalyzer::Run()
{
    if (!shouldRun || !root) return;

    failState = false;
    addressOffset = 0;
    
    symbolTable.clear();
    symbolTable.push_back(new SymbolTable("GLOBAL_SCOPE", 1));
    currentScope = symbolTable.back();

    try
    {
        root->Accept(*this);
        shouldRun = false;
    }
    catch (const std::exception& ex)
    {
        failState = true;
        Logger::Error("{}\n", ex.what());
    }

    failState ? Logger::Error("Semantic Analysis failed..\n") : Logger::Info("Semantic Analysis Complete\n");
}

/* TODO:
    -Type Checking (no types other than ints atm but for the future)
*/