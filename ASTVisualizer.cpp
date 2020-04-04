#include "ASTVisualizer.h"
#include "AbstractSyntaxTree.h"

ASTVisualizer::ASTVisualizer(bool console) : consoleOutput(console) {}
ASTVisualizer::~ASTVisualizer() {}

void ASTVisualizer::PrintAST(ASTNode& n)
{
	out.open("AST.js");    // error checking - unable to open file? also must set to completely overwrite
	// First node is the parent of all other nodes and doesnt have a parent itself, its parentID is itself
	n.parentID = GenerateJSONHeader(out, &n, "ROOT", config);
	// The next node in line will look for this one's id and that is why it's value is set to its own id rather than null
	n.SetChildrenPrintID(n.parentID);
	// Recursively visit each of the tree's nodes and print it to the console
	if (consoleOutput)
	{
		std::cout << "-> Console Dump:\n";
		n.Accept(*this);
		consoleOutput = false;
	}
	else std::cout << "\n";
	// Recursively visit each of the tree's nodes and print JSON to file
	n.Accept(*this);

	// Footer of the file must be a configuration list of all the nodes and their ids
	GenerateJSONFooter(out, config);
	std::cout << "\n\nAST Visualisation File Successfully created\n";
}

void ASTVisualizer::Visit(ASTNode& n) { assert(("ASTVisualizer visited base ASTNode class?!", false)); }

void ASTVisualizer::Visit(UnaryASTNode& n) { n.expr->Accept(*this); }

void ASTVisualizer::Visit(BinaryASTNode& n)
{
	n.left->Accept(*this);
	n.right->Accept(*this);
}

void ASTVisualizer::Visit(IntegerNode& n)
{
	// Print to console
	if   (consoleOutput) std::cout << "Int: " << n.value;
	else GenerateJSON(out, &n, "INT", n.parentID, std::to_string(n.value), config); // Print JSON
}

void ASTVisualizer::Visit(IdentifierNode& n)
{
	// Print to console
	if    (consoleOutput) std::cout << "Ident: " << n.value;
	else  GenerateJSON(out, &n, "ID", n.parentID, n.value, config); // Print JSON
}

void ASTVisualizer::Visit(UnaryOperationNode& n)
{
	// Print to console
	if (consoleOutput)
	{
		std::cout << "Unary: '" << n.op.first << "' [";
		n.expr->Accept(*this);
		std::cout << "]";
	}
	else // Print JSON
	{
		n.SetChildrenPrintID(GenerateJSON(out, &n, "UNARY", n.parentID, n.op.first, config));
		n.expr->Accept(*this);
	}
}

void ASTVisualizer::Visit(BinaryOperationNode& n)
{
	// Print to console
	if (consoleOutput)
	{
		std::cout << "BinOp: '" << n.op.first << "' [L: ";
		n.left->Accept(*this);
		std::cout << " R: ";
		n.right->Accept(*this);
		std::cout << "]";
	}
	else // Print JSON
	{
		n.SetChildrenPrintID(GenerateJSON(out, &n, "BINOP", n.parentID, n.op.first, config));
		n.left->Accept(*this);
		n.right->Accept(*this);
	}
}

void ASTVisualizer::Visit(ConditionNode& n)
{
	// Print to console
	if (consoleOutput)
	{
		std::cout << "COND: '" << n.op.first << "' [L: ";
		n.left->Accept(*this);
		std::cout << " R: ";
		n.right->Accept(*this);
		std::cout << "]";
	}
	else // Print JSON
	{
		n.SetChildrenPrintID(GenerateJSON(out, &n, "COND", n.parentID, n.op.first, config));
		n.left->Accept(*this);
		n.right->Accept(*this);
	}
}

void ASTVisualizer::Visit(IfNode& n)
{
	// Print to console
	if (consoleOutput)
	{
		std::cout << "\nIF: " << "[";
		n.condition->Accept(*this);
		std::cout << " BODY: ";
		// If body can be empty
		if (n.body) n.body->Accept(*this);
		std::cout << "]";
	}
	else // Print JSON
	{
		n.SetChildrenPrintID(GenerateJSON(out, &n, "IF", n.parentID, "IF", config));
		n.condition->Accept(*this);
		if (n.body) n.body->Accept(*this);
	}
}

void ASTVisualizer::Visit(WhileNode& n)
{
	// Print to console
	if (consoleOutput)
	{
		std::cout << "\nWHILE: " << "[";
		n.condition->Accept(*this);
		std::cout << " BODY: ";
		// While-body can be empty
		if (n.body) n.body->Accept(*this);
		std::cout << "]";
	}
	else // Print JSON
	{
		n.SetChildrenPrintID(GenerateJSON(out, &n, "WHILE", n.parentID, "WHILE", config));
		n.condition->Accept(*this);
		if (n.body) n.body->Accept(*this);
	}
}

void ASTVisualizer::Visit(CompoundStatementNode& n)
{
	// Each child of the compound has this compound as a parent (but we never visualise compound statements)
	// so the parent of the children is in fact the parent of the compound node
	n.SetChildrenPrintID(n.parentID);
	for (const auto& statement : n.statements) statement->Accept(*this);
}

void ASTVisualizer::Visit(DeclareStatementNode& n)
{
	// Print to console
	if (consoleOutput)
	{
		std::cout << "\nDECL: [";
		n.identifier->Accept(*this);
		std::cout << "]";
	}
	else // Print JSON
	{
		//std::string t;
		//if (n.type == Token::INT_TYPE) t = "INTEGER"; // ugh
		n.SetChildrenPrintID(GenerateJSON(out, &n, "DECL", n.parentID, n.type.first, config));
		n.identifier->Accept(*this);
	}
}

void ASTVisualizer::Visit(AssignStatementNode& n)
{
	// Print to console
	if (consoleOutput)
	{
		std::cout << "\nASSIGN: '" << n.op.first << "' [L: ";
		n.left->Accept(*this);
		std::cout << " R: ";
		n.right->Accept(*this);
		std::cout << "]";
	}
	else // Print JSON
	{
		n.SetChildrenPrintID(GenerateJSON(out, &n, "ASSIGN", n.parentID, n.op.first, config));
		n.left->Accept(*this);
		n.right->Accept(*this);
	}
}

void ASTVisualizer::Visit(ReturnStatementNode& n)
{
	// Print to console
	if (consoleOutput)
	{
		std::cout << "\nRETURN: [";
		n.expr->Accept(*this);
		std::cout << "]";
	}
	else // Print JSON
	{
		n.SetChildrenPrintID(GenerateJSON(out, &n, "RETURN", n.parentID, "RETURN", config));
		n.expr->Accept(*this);
	}
}

void ASTVisualizer::Visit(EmptyStatementNode& n) {}