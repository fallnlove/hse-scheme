#include "scheme.h"
#include <iostream>
#include <memory>
#include <sstream>

#include "classes.h"
#include "error.h"
#include "object.h"
#include "parser.h"
#include "tokenizer.h"
#include "heap.h"

Interpreter::Interpreter() : scope_(GetInstance<Heap>().Make<Scope>(nullptr)) {
    std::vector<std::pair<std::string, Object*>> functions = {
        {"boolean?", GetInstance<Heap>().Make<BooleanPredicate>()},
        {"not", GetInstance<Heap>().Make<NotFunction>()},
        {"and", GetInstance<Heap>().Make<AndFunction>()},
        {"or", GetInstance<Heap>().Make<OrFunction>()},
        {"quote", GetInstance<Heap>().Make<QuoteFunction>()},
        {"number?", GetInstance<Heap>().Make<IntegerPredicate>()},
        {">=", GetInstance<Heap>().Make<GreateOrEqual>()},
        {">", GetInstance<Heap>().Make<Greate>()},
        {"=", GetInstance<Heap>().Make<Equal>()},
        {"<=", GetInstance<Heap>().Make<LessOrEqual>()},
        {"<", GetInstance<Heap>().Make<Less>()},
        {"+", GetInstance<Heap>().Make<Plus>()},
        {"-", GetInstance<Heap>().Make<Minus>()},
        {"*", GetInstance<Heap>().Make<Mul>()},
        {"/", GetInstance<Heap>().Make<Div>()},
        {"min", GetInstance<Heap>().Make<Min>()},
        {"max", GetInstance<Heap>().Make<Max>()},
        {"abs", GetInstance<Heap>().Make<Abs>()},
        {"pair?", GetInstance<Heap>().Make<PairPredicate>()},
        {"null?", GetInstance<Heap>().Make<NullPredicate>()},
        {"list?", GetInstance<Heap>().Make<ListPredicate>()},
        {"cons", GetInstance<Heap>().Make<Cons>()},
        {"car", GetInstance<Heap>().Make<Car>()},
        {"cdr", GetInstance<Heap>().Make<Cdr>()},
        {"list", GetInstance<Heap>().Make<ListFunction>()},
        {"list-ref", GetInstance<Heap>().Make<ListRef>()},
        {"list-tail", GetInstance<Heap>().Make<ListTail>()},
        {"#t", GetInstance<Heap>().Make<Symbol>(true)},
        {"#f", GetInstance<Heap>().Make<Symbol>(false)},
        {"symbol?", GetInstance<Heap>().Make<SymbolPredicate>()},
        {"define", GetInstance<Heap>().Make<Define>()},
        {"set!", GetInstance<Heap>().Make<SetVariable>()},
        {"if", GetInstance<Heap>().Make<IfFunc>()},
        {"lambda", GetInstance<Heap>().Make<LambdaDefinition>()},
        {"set-car!", GetInstance<Heap>().Make<SetCar>()},
        {"set-cdr!", GetInstance<Heap>().Make<SetCdr>()},
    };
    for (auto& [name, value] : functions) {
        As<Scope>(scope_)->Add(name, value);
    }
}

Interpreter::~Interpreter() {
    GetInstance<Heap>() = Heap();
}

std::string Interpreter::Run(const std::string& str) {
    std::stringstream stream(str);
    Tokenizer tokenizer(&stream);
    auto input_ast = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Read is end, but input is not null");
    }
    if (input_ast == nullptr) {
        throw RuntimeError("No command");
    }

    input_ast->AddScope(scope_);

    auto output_ast = input_ast->Calculate();

    if (output_ast == nullptr) {
        return "()";
    }

    auto res = output_ast->ToString();
    GetInstance<Heap>().Check(scope_);
    return res;
}
