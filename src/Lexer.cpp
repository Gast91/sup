#include "Lexer.h"

void Lexer::Tokenize(const std::vector<std::string>& srcLines)
{
    // Reset Lexer state for next run
    if (!sourceTokens.empty())
    {
        sourceTokens.clear();
        currentTokenIndex = 0;
        lineStartIndex = 0;
    }
    size_t lineNo = 0;
    for (const auto& line : srcLines)
    {
        ++lineNo;
        size_t prev = 0, pos;
        while ((pos = line.find_first_of(" \t+-*/()=!><&|;{}%", prev)) != std::string::npos)
        {
            if (pos < prev) continue;
            if (pos > prev) AddToken(line.substr(prev, pos - prev), lineNo, prev + 1);

            // This delimiter is a special character or an operator (normal or compound) and must be preserved as a token 
            // or it is a form of a whitespace character and must be discarded
            const std::string delimiter = line.substr(pos, 1);
            // Discard whitespaces, tabs, newlines etc
            if (!IsDiscardableCharacter(delimiter))
            {
                // If the next character and the current delimiter form a compound operator,
                // they must be preserved as one token
                const std::string nextCharacter = line.substr(pos + 1, 1);
                if (IsCompoundOperator(delimiter, nextCharacter))  // doesnt check for >>= or <<=
                {
                    AddToken(delimiter + nextCharacter, lineNo, prev + 1);
                    // Skip the next character since it has been processed already (part of a compound operator)
                    prev = pos + 2;
                    continue;
                }
                // Comment start at any point in this line means we skip the rest of the line
                else if (IsComment(delimiter, nextCharacter)) { pos = prev = std::string::npos; break; }
                // Delimiter is just a normal operator, preserve it
                else AddToken(delimiter, lineNo, prev + 1);
            }
            prev = pos + 1;
        }
        if (prev < line.length()) AddToken(line.substr(prev, std::string::npos), lineNo, prev + 1);
    }
    tokenized = true;
}

void Lexer::AddToken(const std::string& tok, const size_t lineNo, const size_t col)
{
    // Get iterator to token if already in the map (this will be the case for defined language features and operators)
    // but not for literals and identifiers or things that do not fall in either of the aforementioned categories.
    const auto[it, success] = tokens.emplace(tok, Token::UNKNOWN);
    if (success)
    {
        // Succesful insertion has the identifiers, literal or garbage token marked as unknown
        // Update it if it satisfies the criteria for a literal or identifier
        if (IsIdentifier(tok, true)) it->second = Token::IDENTIFIER;
        else if (IsInteger(tok))     it->second = Token::INT_LITERAL;
        // Or it is a not yet defined language feature or garbage token so it stays unknown
    }
    sourceTokens.push_back(std::make_tuple(it->first, it->second, lineNo, col));
}

bool Lexer::IsDiscardableCharacter(const std::string& delimiter) const noexcept { return delimiter.find_first_of(" \t\r\n") != std::string::npos; }

bool Lexer::IsCompoundOperator(const std::string& delimiter, const std::string& next) const
{
    switch (delimiter.front())  // shiftleft and shiftright missing
    {
    case '*': case '/': case '!': case '=': case '%': case '^': return next == "=";
    case '+': return next == "=" || next == "+";
    case '-': return next == "=" || next == "-";
    case '&': return next == "&" || next == "=";
    case '|': return next == "|" || next == "=";
    case '>': return next == ">" || next == "=";
    case '<': return next == "<" || next == "=";
    default:  return false;
    }
}

bool Lexer::IsComment(const std::string& delimiter, const std::string& next) const
{
    return delimiter.front() == '/' && next == "/";
}

bool Lexer::IsInteger(const std::string& num) const
{
    unsigned int i = 0;
    const unsigned char digit = num.front();
    if (isdigit(digit))
    {
        if (++i >= num.length()) return true;
        else return IsInteger(num.substr(i, std::string::npos));
    }
    return false;
}

// Checks if the character pass to it is one of the alphabet characters (lower or upper) or a digit or underscore
bool Lexer::IsCharacter(const unsigned char c) const noexcept { return isalpha(c) || c == '_' || isdigit(c); }

constexpr bool Lexer::IsIdentifier(const std::string& identifier, const bool firstCall) const
{
    if (firstCall) if (isdigit(identifier.front())) return false;
    unsigned int i = 0;
    const char character = identifier.front();
    if (IsCharacter(character))
    {
        if (++i >= identifier.length()) return true;
        else return IsIdentifier(identifier.substr(i, std::string::npos), false);
    }
    return false;
}

void Lexer::PrintTokens() const
{
    if (failState) return;
    std::cout << "Tokenized Input (Split by whitespace):\n";
    for (const auto& [name, type, line, col] : sourceTokens) if (type != Token::NLINE && type != Token::FILE_END) std::cout << name << ' ';
    std::cout << '\n';
}

const std::vector<TokenInfo>& Lexer::GetTokens() const
{
    return sourceTokens;
}

bool Lexer::Failure() const { return failState; }
bool Lexer::Done() const { return currentTokenIndex == sourceTokens.size(); }  // ??

const std::string Lexer::GetLine() const { return std::to_string(std::get<2>(sourceTokens.at(currentTokenIndex))); }

const ErrorInfo Lexer::GetErrorInfo() const  // THIS SHIT IS UGLY AF - MAKE BETTER - ALSO MAKE IT FIT FOR SEMANTICS
{                                            // WONKY after semicolons, shows line below?... ALSO check how you showed the expected token before (git)
    // Source column and line for the error
    const auto& [tok, type, line, col] = sourceTokens.at(currentTokenIndex);
    const std::string loc = "<source>:" + std::to_string(line) + ":" + std::to_string(col) + ":";
    const std::string seperator = "\t|\t";


    // Format the erroneous line of code
    unsigned int index = lineStartIndex + 1;   // line's not correct without +1
    const auto& token = std::get<0>(sourceTokens.at(index++));
    std::string codeLine = seperator + std::get<0>(sourceTokens.at(index++)) + " ";
    while (std::get<2>(sourceTokens.at(index)) == lineStartIndex)
        codeLine.append(std::get<0>(sourceTokens.at(index++)) + " ");
    const size_t len = codeLine.size() - col - seperator.size();
    codeLine.append("\n" + seperator).append(len, ' ').append("^").append(std::get<0>(sourceTokens.at(currentTokenIndex)).size(), '~');
    return { loc, codeLine };
}

void Lexer::Consume(const Token tokenType)
{
    const auto& [tok, type, line, col] = sourceTokens.at(currentTokenIndex);
    if (tokenType == type && currentTokenIndex < sourceTokens.size())
    {
        ++currentTokenIndex;
        if (currentTokenIndex == sourceTokens.size()) return;
        if (std::get<2>(sourceTokens.at(currentTokenIndex)) != line) ++lineStartIndex;
    }
    else throw UnexpectedTokenException(GetErrorInfo(), "Encountered Unexpected Token");
}

const TokenInfo& Lexer::GetCurrentToken() { return sourceTokens.at(currentTokenIndex); }
bool Lexer::hasTokenized() const { return tokenized; }