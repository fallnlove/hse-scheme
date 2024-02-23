#include "error.h"
#include "tokenizer.h"
#include <cctype>
#include <string>

SymbolToken::SymbolToken(std::string str) : name(str) {
}

SymbolToken::SymbolToken(char character) {
    name += character;
}

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

ConstantToken::ConstantToken(int val) : value(val) {
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream* in) : in_(in), cur_token_(QuoteToken()), is_end_(false) {
    Next();
}

bool Tokenizer::IsEnd() {
    return is_end_;
}

bool Tokenizer::IsReachedEnd() {
    return in_->eof();
}

void Tokenizer::Next() {
    RemoveSpaces();
    if (IsReachedEnd()) {
        is_end_ = true;
        return;
    }
    char character = in_->get();
    if (character == '\047') {  //  quote
        cur_token_ = QuoteToken();
    } else if (character == '(') {
        cur_token_ = BracketToken::OPEN;
    } else if (character == ')') {
        cur_token_ = BracketToken::CLOSE;
    } else if (character == '.') {
        cur_token_ = DotToken();
    } else if (std::isdigit(character)) {
        in_->unget();
        cur_token_ = ConstantToken(ReadInt());
    } else if (character == '+' || character == '-') {
        if (IsNextDigit()) {
            cur_token_ = ConstantToken(ReadInt() * (character == '-' ? -1 : 1));
        } else {
            cur_token_ = SymbolToken(character);
        }
    } else if (IsSymbolF(character)) {
        in_->unget();
        cur_token_ = SymbolToken(ReadString());
    } else {
        throw SyntaxError("Unknown symbol");
    }
}

Token Tokenizer::GetToken() {
    return cur_token_;
}

void Tokenizer::RemoveSpaces() {
    while (IsNextSpace() && !IsReachedEnd()) {
        in_->get();
    }
}

bool Tokenizer::IsNextSpace() {
    return std::isspace(in_->peek());
}

bool Tokenizer::IsNextDigit() {
    return std::isdigit(in_->peek());
}

int Tokenizer::ReadInt() {
    int res = 0;
    while (IsNextDigit() && !IsReachedEnd()) {
        res *= 10;
        res += in_->get() - '0';
    }
    return res;
}

std::string Tokenizer::ReadString() {
    std::string str;
    while (IsSymbol(in_->peek()) && !IsReachedEnd()) {
        str += in_->get();
    }
    return str;
}

bool Tokenizer::IsSymbolF(char c) {
    return std::isalpha(c) || c == '<' || c == '=' || c == '>' || c == '*' || c == '/' || c == '#';
}

bool Tokenizer::IsSymbol(char c) {
    return IsSymbolF(c) || std::isdigit(c) || c == '?' || c == '!' || c == '-';
}
