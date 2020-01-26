#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class UnexpectedTokenException : public std::exception
{
public:
	UnexpectedTokenException(std::string msg) : exception(msg.c_str()) {}  // ugh
};

class Lexer
{
private:
	friend class Parser;

	std::vector<std::string> sourceTokens;  // switch of array of Tokens (since it has already been processed it can same time)
	int currentTokenIndex = 0;
	bool tokensLeft = true;

	// Tokenize source based on the specified delimiters (preserving those delimiters if not whitespace characters)
	void tokenizeSource(std::ifstream& infile);
	// Token recognizers - some can be const char* - more efficient than memory alloc
	bool IsDiscardableCharacter(const std::string& delimiter);
	bool IsCompoundOperator(const std::string& delimiter, const std::string& next);  // UNUSED
	bool IsComment(const std::string& delimiter, const std::string& next);
	bool IsDigit(char c);
	bool IsInteger(const std::string& num);
	bool IsCharacter(char c);
	bool IsIdentifier(const std::string& identifier, const bool firstCall);
	bool IsKeyword(const std::string& keyword);   // UNUSED
public:
	Lexer(char* sourcePath);  // const
	~Lexer();

	void printTokenizedInput();
	bool consume(std::string token); // const ptr?
	bool advance();  // this should raise an error if the expected token passed from the parser isnt the one given - else advance
	std::string& getCurrentToken();
};