#include <sstream>
#include <fstream>

#include "ASTPrinterJson.h"
#include "AbstractSyntaxTree.h"
#include "../Util/Logger.h"
#include "../Util/Utility.h"

void ASTPrinterJson::PrintAST()
{
    if (!root) return;

    out.open("AST.js");

    if (!out) 
    {
        out.close(); 
        Logger::Error("File writing failed..\n"); 
        return;
    }

    // First node is the parent of all other nodes and doesnt have a parent itself, its parentID is itself
    root->parentID = GenerateJSONHeader(out, root, "ROOT", config);
    // The next node in line will look for this one's id and that is why it's value is set to its own id rather than null
    root->SetChildrenPrintID(root->parentID);

    // Recursively visit each of the tree's nodes and print JSON to file
    root->Accept(*this);

    // Footer of the file must be a configuration list of all the nodes and their ids
    GenerateJSONFooter(out, config);
    out.close();
    Logger::Info("AST Visualisation File Successfully created\n");
}

std::string ASTPrinterJson::GenerateJSONHeader(std::ofstream& out, const ASTNode* root, const char* rootID, std::vector<std::string>& config) const
{
    out << "config = {\n\tcontainer: \"#AST\"\n};\n\n";
    config.push_back("config");
    const std::string id = Util::GenerateID(root, rootID);
    out << id << " = {\n\ttext: { name: \"ROOT\" }\n};\n\n";
    config.push_back(id);
    return id;
}

void ASTPrinterJson::GenerateJSONFooter(std::ofstream& out, const std::vector<std::string>& config) const
{
    out << "simple_chart_config = [\n    ";
    for (unsigned int i = 0; i < config.size(); ++i) { if (i != 0) out << ", "; out << config.at(i); }
    out << "\n];";
}

std::string ASTPrinterJson::GenerateJSON(std::ofstream& out, const ASTNode* node, const char* ID, const std::string& parentID, const std::string& name, std::vector<std::string>& config) const
{
    const std::string nodeID = Util::GenerateID(node, ID);
    out << nodeID << " = {\n\tparent: " << parentID <<
        ",\n\ttext: { name: \"" << name << "\" }\n};\n\n";
    config.push_back(nodeID);
    return nodeID;
}

void ASTPrinterJson::Visit(ASTNode& n)       { assert(("ASTVisualizer visited base ASTNode class?!"      , false)); }
void ASTPrinterJson::Visit(UnaryASTNode& n)  { assert(("ASTVisualizer visited base UnaryASTNode class?!" , false)); }
void ASTPrinterJson::Visit(BinaryASTNode& n) { assert(("ASTVisualizer visited base BinaryASTNode class?!", false)); }

void ASTPrinterJson::Visit(IntegerNode& n)
{
    (void)GenerateJSON(out, &n, "INT", n.parentID, std::to_string(n.value), config);
}

void ASTPrinterJson::Visit(IdentifierNode& n)
{
    (void)GenerateJSON(out, &n, "ID", n.parentID, n.token.str, config);
}

void ASTPrinterJson::Visit(UnaryOperationNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "UNARY", n.parentID, n.op.str, config));
    n.expr->Accept(*this);
}

void ASTPrinterJson::Visit(BinaryOperationNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "BINOP", n.parentID, n.op.str, config));
    n.left->Accept(*this);
    n.right->Accept(*this);
}

void ASTPrinterJson::Visit(ConditionNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "COND", n.parentID, n.op.str, config));
    n.left->Accept(*this);
    n.right->Accept(*this);
}

void ASTPrinterJson::Visit(IfNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, n.type.c_str(), n.parentID, n.type.c_str(), config));
    n.condition->Accept(*this);
    if (n.body) n.body->Accept(*this);
}

void ASTPrinterJson::Visit(IfStatementNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "_IF_", n.parentID, "_IF_", config));  // else is compound so parent automatically becomes _IF_ shows no else!
    for (const auto& ifN : n.ifNodes) ifN->Accept(*this);
    if (n.elseBody) n.elseBody->Accept(*this);
}

void ASTPrinterJson::Visit(IterationNode& n) { assert(("ASTVisualizer visited base IterationNode class?!", false)); }
void ASTPrinterJson::Visit(WhileNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "WHILE", n.parentID, "WHILE", config));
    n.condition->Accept(*this);
    if (n.body) n.body->Accept(*this);
}

void ASTPrinterJson::Visit(DoWhileNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "DO", n.parentID, "DO", config));
    if (n.body) n.body->Accept(*this);
    n.condition->Accept(*this);
}

void ASTPrinterJson::Visit(CompoundStatementNode& n)
{
    // Each child of the compound has this compound as a parent (but we never visualise compound statements)
    // so the parent of the children is in fact the parent of the compound node
    n.SetChildrenPrintID(n.parentID);
    for (const auto& statement : n.statements) statement->Accept(*this);
}

void ASTPrinterJson::Visit(StatementBlockNode& n)
{
    // Each child of the compound has this compound as a parent (but we never visualise compound statements)
    // so the parent of the children is in fact the parent of the compound node
    n.SetChildrenPrintID(n.parentID);
    for (const auto& statement : n.statements) statement->Accept(*this);
}

void ASTPrinterJson::Visit(DeclareStatementNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "DECL", n.parentID, n.type.str, config));
    n.identifier->Accept(*this);
}

void ASTPrinterJson::Visit(DeclareAssignNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "DECL_ASSIGN", n.parentID, n.op.str, config));
    n.left->Accept(*this);
    n.right->Accept(*this);
}

void ASTPrinterJson::Visit(AssignStatementNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "ASSIGN", n.parentID, n.op.str, config));
    n.left->Accept(*this);
    n.right->Accept(*this);
}

void ASTPrinterJson::Visit(ReturnStatementNode& n)
{
    n.SetChildrenPrintID(GenerateJSON(out, &n, "RETURN", n.parentID, "RETURN", config));
    n.expr->Accept(*this);
}

void ASTPrinterJson::Visit(EmptyStatementNode& n) {}