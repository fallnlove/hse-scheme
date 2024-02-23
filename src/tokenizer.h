#pragma once

#include <variant>
#include <optional>
#include <istream>

struct SymbolToken {
    std::string name;

    SymbolToken(char character);

    SymbolToken(std::string str);

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    ConstantToken(int val);

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    std::istream* in_;
    Token cur_token_;
    bool is_end_;

    void RemoveSpaces();

    bool IsNextSpace();

    bool IsNextDigit();

    int ReadInt();

    std::string ReadString();

    bool IsReachedEnd();

    bool IsSymbolF(char c);

    bool IsSymbol(char c);
};
