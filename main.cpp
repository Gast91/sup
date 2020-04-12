#include "Parser.h"
#include "ASTVisualizer.h"
#include "SemanticAnalyzer.h"
#include "CodeGenerator.h"

int main(int argc, char* argv[])  // change success checks to !success -> return, verbose mode? set fail flag for lexer or exc if cannot find file to read
{
    // Create lexer and pass to it the file to be tokenized
    Lexer lexer("source.txt");    //argv[argc - 1]);  This for actual programs
    lexer.PrintTokens();

    // Create parser, pass to it the lexer and parse input from lexer
    Parser parser(lexer);
    if (parser.Success())
    {
        // Parse Successful, print the AST
        ASTVisualizer ASTPrinter;
        ASTPrinter.PrintAST(*parser.GetAST());
        // Perform Semantic Analysis to the AST
        SemanticAnalyzer semanticAnalyzer;
        try { parser.GetAST()->Accept(semanticAnalyzer); }
        catch (const std::exception& ex) { std::cout << '\n' << ex.what(); }
        // Show symbol table info gathered by the semantic analysis

        if (semanticAnalyzer.Success())
        {
            semanticAnalyzer.PrintAnalysisInfo();
            // If semantic analysis was a success then generate intermediate code
            CodeGenerator codeGenerator;
            codeGenerator.GenerateTAC(parser.GetAST());
            // Transform intermediate code to assembly
            // Print|Output to file
            // Pass assembly to assembler?!
        }
    }
    std::cin.get(); // Debug Only - Get rid for actual
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);  // _CrtDumpMemoryLeaks() will be called AFTER main has been exited
}