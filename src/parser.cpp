#include "parser.h"
#include "tokenizer.h"
#include <variant>
#include "classes.h"
#include "error.h"
#include "heap.h"
#include "object.h"

Object* Read(Tokenizer* tokenizer) {
    auto token = tokenizer->GetToken();
    CheckEnd(tokenizer);
    tokenizer->Next();
    if (BracketToken* bracket = std::get_if<BracketToken>(&token)) {
        if (*bracket == BracketToken::CLOSE) {
            throw SyntaxError("Closed bracket before open");
        }
        return ReadList(tokenizer);
    }
    if (ConstantToken* number = std::get_if<ConstantToken>(&token)) {
        return GetInstance<Heap>().Make<Number>(number->value);
    } else if (SymbolToken* symbol = std::get_if<SymbolToken>(&token)) {
        return GetInstance<Heap>().Make<Symbol>(symbol->name);
    } else if ([[maybe_unused]] QuoteToken* quote = std::get_if<QuoteToken>(&token)) {
        auto argument = Read(tokenizer);
        auto second_cell = GetInstance<Heap>().Make<Cell>(argument, nullptr);
        return GetInstance<Heap>().Make<Cell>(GetInstance<Heap>().Make<Symbol>("quote"),
                                              second_cell);
    }
    throw SyntaxError("Unknown token");
}

Object* ReadList(Tokenizer* tokenizer) {
    if (IsCloseBracket(tokenizer->GetToken())) {
        tokenizer->Next();
        return nullptr;
    }

    Object* cell = GetInstance<Heap>().Make<Cell>();
    Object* cur = cell;

    bool flag_should_be_end = false;

    while (!IsCloseBracket(tokenizer->GetToken())) {
        if (flag_should_be_end) {
            throw SyntaxError("Dot in incorrect place");
        }

        auto tmp_token = Read(tokenizer);
        CheckEnd(tokenizer);

        if (IsDot(tokenizer->GetToken())) {
            tokenizer->Next();
            flag_should_be_end = true;
        }

        if (flag_should_be_end) {
            *(As<Cell>(cur)) = Cell(tmp_token, Read(tokenizer));
            CheckEnd(tokenizer);
        } else {
            Object* tmp =
                IsCloseBracket(tokenizer->GetToken()) ? nullptr : GetInstance<Heap>().Make<Cell>();
            *(As<Cell>(cur)) = Cell(tmp_token, tmp);
            cur = tmp;
        }
    }

    tokenizer->Next();
    return cell;
}

bool IsCloseBracket(Token token) {
    if (BracketToken* bracket = std::get_if<BracketToken>(&token)) {
        return *bracket == BracketToken::CLOSE;
    }
    return false;
}

bool IsDot(Token token) {
    return std::get_if<DotToken>(&token) != nullptr;
}

void CheckEnd(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Not found close bracket");
    }
}
