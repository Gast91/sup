#include "Symbol.h"
#include "../AST/Visitor.h"
#include "../AST/AbstractSyntaxTree.h"
#include "../Util/ModuleManager.h"

class SemanticAnalyzer : public ASTNodeVisitor, public IObserver
{
private:
    std::vector<SymbolTable*> symbolTable;
    SymbolTable* currentScope;

    ASTNode* root = nullptr;

    int addressOffset = 0;
    bool failState = false;
    bool shouldRun = false;

    SymbolTable* CreateNewScope(const ASTNode* n, const char* tag);
public:
    SemanticAnalyzer() = default;
    ~SemanticAnalyzer();

    // Inherited via ASTNodeVisitor
    void Visit(ASTNode& n)               override;
    void Visit(UnaryASTNode& n)          override;
    void Visit(BinaryASTNode& n)         override;
    void Visit(IntegerNode& n)           override;
    void Visit(IdentifierNode& n)        override;
    void Visit(UnaryOperationNode& n)    override;
    void Visit(BinaryOperationNode& n)   override;
    void Visit(ConditionNode& n)         override;
    void Visit(IfNode& n)                override;
    void Visit(IfStatementNode& n)       override;
    void Visit(IterationNode& n)         override;
    void Visit(WhileNode& n)             override;
    void Visit(DoWhileNode& n)           override;
    void Visit(StatementBlockNode& n)    override;
    void Visit(CompoundStatementNode& n) override;
    void Visit(DeclareStatementNode& n)  override;
    void Visit(DeclareAssignNode& n)     override;
    void Visit(AssignStatementNode& n)   override;
    void Visit(ReturnStatementNode& n)   override;
    void Visit(EmptyStatementNode& n)    override;

    void PrintAnalysisInfo() const;
    void Render() const;  // ??

    // Inherited via IObserver Interface
    virtual bool ShouldRun()  const override { return shouldRun; }
    virtual void SetToRun()         override { shouldRun = true; }
    virtual void Update(ASTNode* n) override { root = n; }
    virtual void Run() override;
};