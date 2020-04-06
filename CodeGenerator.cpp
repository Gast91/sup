#include "CodeGenerator.h"
#include "AbstractSyntaxTree.h"

ThreeAddressCode CodeGenerator::instructions;
int Temporary::tempCount = 0;
int Label::labelCount = 0;

void CodeGenerator::GenerateAssembly(ASTNode* n)
{
    // set up stack/main;
    /*std::cout << "\npush rbp\nmov rbp, rsp\n";
    n->Accept(*this);
    std::cout << "pop rbp\nret\n";*/
    std::cout << "Intermediate Language Representation:\nmain:\n";
    PlainVisit(n);  // Start Traversing the AST
    for (const auto& instruction : instructions)   // no string comparisons pls - how about string_view or something else - merge stuff
    {
        // if there is a second operand then the instruction represent's a full three address code (3 addresses, 2 max operands on the right)
        if (instruction.src2.has_value())
            std::cout << '\t' << instruction.dest.value() << " = " << instruction.src1.value() << ' ' << instruction.op.value() << ' ' << instruction.src2.value() << ";\n";
        else if (instruction.op.value() == "=")       std::cout << '\t' << instruction.dest.value() << ' ' << instruction.op.value() << ' ' << instruction.src1.value() << ";\n";
        else if (instruction.op.value() == "IfZ")     std::cout << '\t' << instruction.op.value() << ' ' << instruction.src1.value() << " Goto " << instruction.dest.value() << ";\n";
        else if (instruction.op.value() == "Label")   std::cout << instruction.dest.value() << ":\n";
        else if (instruction.op.value() == "Return")  std::cout << '\t' << instruction.op.value() << ' ' << instruction.dest.value() << ";\n";
        else if (instruction.op.value() == "Goto")    std::cout << '\t' << instruction.op.value() << ' ' << instruction.dest.value() << ":\n";
        // Unary Instruction
        else std::cout << '\t' << instruction.dest.value() << " = " << instruction.op.value() << ' ' << instruction.src1.value() << ";\n";
    }
}

void CodeGenerator::Visit(ASTNode& n)        { assert(("Code Generator visited base ASTNode class?!", false)); }
void CodeGenerator::Visit(UnaryASTNode& n)   { assert(("Code Generator visited base UnaryASTNode class?!", false)); }
void CodeGenerator::Visit(BinaryASTNode& n)  { assert(("Code Generator visited base BinaryASTNode class?!", false)); }
// Integer and Identifier Leaf Nodes. A throwaway Quadruple is returned that effectively passes back their value or name
void CodeGenerator::Visit(IntegerNode& n)    { Return({ std::nullopt, std::nullopt, std::nullopt, std::to_string(n.value) }); }
void CodeGenerator::Visit(IdentifierNode& n) { Return({ std::nullopt, std::nullopt,  std::nullopt, n.name }); }

void CodeGenerator::Visit(UnaryOperationNode& n)
{
    instructions.push_back({ n.op.first, GetValue(n.expr).dest, std::nullopt, Temporary::NewTemporary() });
    Return(instructions.back());
}

void CodeGenerator::Visit(BinaryOperationNode& n)
{
    instructions.push_back({ n.op.first, GetValue(n.left).dest, GetValue(n.right).dest, Temporary::NewTemporary() });
    Return(instructions.back());
}

void CodeGenerator::Visit(ConditionNode& n)  // we will need logical expressions? what the fuck is happening here - AST might need modification or assert in not needed
{
    //std::cout << "HERE";
    /*Quadruples q1 = GetValue(n.left);
    Quadruples q2 = GetValue(n.right);
    instructions.push_back({ n.op.first, q1.dest, q2.dest, "t" + std::to_string(temporaries) });
    std::cout << "\nCHECK " << "t" + std::to_string(temporaries) << '\n';
    Return(instructions.at(temporaries));
    ++temporaries;*/
}

void CodeGenerator::Visit(IfNode& n)   // no else etc - would be nice, probably no if else - but first parser must be able to understand it
{
    const auto falseLabel = Label::NewLabel();
    instructions.push_back({ "IfZ", GetValue(n.condition).dest, std::nullopt, falseLabel });  // better encoding here? can be others than IfFalse(Z) based on cond operator?
    if (n.body) PlainVisit(n.body);
    instructions.push_back({ "Label", std::nullopt, std::nullopt, falseLabel });
}

void CodeGenerator::Visit(WhileNode& n)
{
    const auto startLabel = Label::NewLabel();
    instructions.push_back({ "Label", std::nullopt, std::nullopt, startLabel });
    const auto endLabel = Label::NewLabel();
    instructions.push_back({ "IfZ", GetValue(n.condition).dest, std::nullopt, endLabel });
    if (n.body) PlainVisit(n.body);
    instructions.push_back({ "Goto", std::nullopt, std::nullopt, startLabel });
    instructions.push_back({ "Label", std::nullopt, std::nullopt, endLabel });
}

void CodeGenerator::Visit(CompoundStatementNode& n)
{
    for (const auto& statement : n.statements) PlainVisit(statement);
    // the way it is set up here, generation will start here, maybe have a ProgramEntryNode?  also label main:
    // for setting up stack etc? for exit as well - wont work atm - hack atm is generate
}

void CodeGenerator::Visit(DeclareStatementNode& n)
{
    // no visiting probs
    // just visit? or do nothing? Semantic Analyzer has taken care of variables being declared etc
    // it will need to do type checking also (in the future) and tell code generator size of var?
}

void CodeGenerator::Visit(AssignStatementNode& n)
{
    instructions.push_back({ n.op.first, GetValue(n.right).dest, std::nullopt, GetValue(n.left).dest });
}

void CodeGenerator::Visit(ReturnStatementNode& n)
{
    instructions.push_back({ "Return", std::nullopt, std::nullopt, GetValue(n.expr).dest });
    // we are done here we must jump to the end or something
}

void CodeGenerator::Visit(EmptyStatementNode& n) {}

// TODO:
/*
    -Fix/Add Nodes into the ast to accomodate logical operations, main/entry point - potentially more? 
    -Start filling out the functions - output TAC into the console for start (expressions done - googo ifs!)
    -Rename+cpp+h to CodeGeneratorIR?

    check asserts if work
    conserve temporaries - the t0 = a * b --> c = t0 is annoying!!!
    optional's in place?
    need beginfunc and endfunc, jump for return to the end and allocating space at begin func?

    -CHECK STD::VISITOR-VARIANT - nah
    https://web.stanford.edu/class/archive/cs/cs143/cs143.1128/handouts/240%20TAC%20Examples.pdf page 7-8 for reduced temporaries
*/