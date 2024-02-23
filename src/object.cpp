#include "object.h"
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "classes.h"
#include "error.h"

std::string Object::ToString() {
    throw RuntimeError("Not Implemented");
}

Object* Object::DeepCopy() {
    return this;  // maybe problems TODO
}

Object* Object::Calculate() {
    throw RuntimeError("Not Implemented");
}

Object* Object::operator()([[maybe_unused]] Object* root) {
    throw RuntimeError("Not Implemented");
}

void Object::AddScope(Object* scope) {
    scope_ = scope;
}

void Object::ThrowScope() {
    auto cell = As<Cell>(this);
    if (cell == nullptr) {
        return;
    }
    if (cell->GetFirst() != nullptr) {
        cell->GetFirst()->AddScope(scope_);
    }
    if (cell->GetSecond() != nullptr) {
        cell->GetSecond()->AddScope(scope_);
    }
}

Object* Object::GetScope() {
    return scope_;
}

void Object::AddDependency(Object* other) {
    neighbours_.insert(other);
}

void Object::RemoveDependency(Object* other) {
    neighbours_.erase(other);
}

void Object::Mark() {
    is_achivable_ = true;
    for (auto v : neighbours_) {
        if (v != nullptr && !v->is_achivable_) {
            v->Mark();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////

Number::Number(int64_t value) : value_(value) {
}

int64_t Number::GetValue() const {
    return value_;
}

std::string Number::ToString() {
    return std::to_string(value_);
}

Object* Number::DeepCopy() {
    return GetInstance<Heap>().Make<Number>(value_);
}

Object* Number::Calculate() {
    return this;
}

///////////////////////////////////////////////////////////////////////////////////////////

Symbol::Symbol(std::string str) : str_(str) {
}

Symbol::Symbol(const char* str) : str_(str) {
}

Symbol::Symbol(bool boolean) : str_(boolean ? "#t" : "#f") {
}

const std::string& Symbol::GetName() const {
    return str_;
}

std::string Symbol::ToString() {
    return str_;
}

Object* Symbol::DeepCopy() {
    return GetInstance<Heap>().Make<Symbol>(str_);
}

Object* Symbol::Calculate() {
    return As<Scope>(scope_)->Get(str_);
}

///////////////////////////////////////////////////////////////////////////////////////////

Cell::Cell(Object* f, Object* s) : first_(f), second_(s) {
    AddDependency(first_);
    AddDependency(second_);
}

Object* Cell::GetFirst() const {
    return first_;
}

Object* Cell::GetSecond() const {
    return second_;
}

std::string Cell::ToString() {
    std::string ans = "(";
    Object* root = this;
    while (root != nullptr) {
        if (!Is<Cell>(root)) {
            ans += ". ";
            ans += root->ToString();
            break;
        }
        auto fisrt = As<Cell>(root)->first_;
        if (fisrt == nullptr) {
            ans += "()";
        } else {
            ans += As<Cell>(root)->first_->ToString();
        }
        if (As<Cell>(root)->second_ != nullptr) {
            ans += " ";
        }
        root = As<Cell>(root)->second_;
    }
    return ans + ")";
}

Object* Cell::DeepCopy() {
    return GetInstance<Heap>().Make<Cell>(first_ ? first_->DeepCopy() : nullptr,
                                          second_ ? second_->DeepCopy() : nullptr);
}

Object* Cell::Calculate() {
    if (first_ == nullptr) {
        throw RuntimeError("List can't be self calculated");
    }

    ThrowScope();
    auto func = first_->Calculate();  // maybe here problem TODO

    if (func == nullptr) {
        throw RuntimeError("List can't be self calculated");
    }

    func->AddScope(scope_);

    return (*func)(second_);
}

///////////////////////////////////////////////////////////////////////////////////////////

Scope::Scope(Object* parent) : parent_(parent) {
}

void Scope::Add(std::string name, Object* value) {
    RemoveDependency(scope_names_[name]);
    scope_names_[name] = value;
    AddDependency(scope_names_[name]);
}

void Scope::Set(std::string name, Object* new_value) {
    if (scope_names_.contains(name)) {
        Add(name, new_value);
        return;
    }
    if (parent_ == nullptr) {
        throw NameError("Name is not defined");
    }
    As<Scope>(parent_)->Set(name, new_value);
}

Object* Scope::Get(std::string name) {
    if (scope_names_.contains(name)) {
        return scope_names_[name];
    }
    if (parent_ == nullptr) {
        throw NameError("Unknown name");
    }
    return As<Scope>(parent_)->Get(name);
}

Object* Scope::DeepCopy() {  /// maybe problem here TODO
    return this;
}

/////////////////////////////////HELPERS///////////////////////////////////////////////////

std::vector<Object*> GetArgs(Object* root) {
    std::vector<Object*> args;
    while (root != nullptr) {
        root->ThrowScope();
        if (Is<Cell>(root)) {
            if (As<Cell>(root)->GetFirst() != nullptr) {
                args.push_back(As<Cell>(root)->GetFirst()->Calculate());
            } else {
                args.push_back(nullptr);
            }
            root = As<Cell>(root)->GetSecond();
        } else {
            args.push_back(root->Calculate());
            root = nullptr;
        }
    }
    return args;
}

std::vector<Object*> GetArgsWithoutCalculating(Object* root) {
    std::vector<Object*> args;
    while (root != nullptr) {
        root->ThrowScope();
        if (Is<Cell>(root)) {
            args.push_back(As<Cell>(root)->GetFirst());
            root = As<Cell>(root)->GetSecond();
        } else {
            args.push_back(root);
            root = nullptr;
        }
    }
    return args;
}

void RequireArgsRE(const std::vector<Object*>& args, size_t min_cnt, size_t max_cnt) {
    if (args.size() < min_cnt || args.size() > max_cnt) {
        throw RuntimeError("Invalid number of arguments");
    }
}

void RequireArgsSE(const std::vector<Object*>& args, size_t min_cnt, size_t max_cnt) {
    if (args.size() < min_cnt || args.size() > max_cnt) {
        throw SyntaxError("Invalid number of arguments");
    }
}

void DfsList(Object* root, size_t& depth, bool& is_end_null) {
    depth = 0;
    is_end_null = true;
    while (root != nullptr) {
        ++depth;
        if (!Is<Cell>(root)) {
            is_end_null = false;
            root = nullptr;
        } else {
            root = As<Cell>(root)->GetSecond();
        }
    }
}

////////////////////////////////FUNCTIONS//////////////////////////////////////////////////

int64_t SpecialMin::operator()(int64_t lhs, int64_t rhs) {
    return std::min(lhs, rhs);
}

int64_t SpecialMax::operator()(int64_t lhs, int64_t rhs) {
    return std::max(lhs, rhs);
}

int64_t SpecialAbs::operator()([[maybe_unused]] int64_t lhs, int64_t rhs) {
    return std::abs(rhs);
}

Object* BooleanPredicate::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    bool f = IsExpectedType<Symbol>(args) &&
             (args[0]->ToString() == "#f" || args[0]->ToString() == "#t");
    return GetInstance<Heap>().Make<Symbol>(f);
}

Object* BooleanPredicate::DeepCopy() {
    return GetInstance<Heap>().Make<BooleanPredicate>();
}

Object* NotFunction::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    bool f = IsExpectedType<Symbol>(args) && (args[0]->ToString() == "#f");
    return GetInstance<Heap>().Make<Symbol>(f);
}

Object* NotFunction::DeepCopy() {
    return GetInstance<Heap>().Make<NotFunction>();
}

Object* AndFunction::operator()(Object* root) {
    ThrowScope();
    if (root == nullptr) {
        return GetInstance<Heap>().Make<Symbol>(true);
    }
    Object* res = nullptr;
    while (root != nullptr) {
        root->ThrowScope();
        if (Is<Cell>(root)) {
            res = As<Cell>(root)->GetFirst()->Calculate();
            root = As<Cell>(root)->GetSecond();
        } else {
            res = root->Calculate();
            root = nullptr;
        }
        if (Is<Symbol>(res) && (res->ToString() == "#f")) {
            return res;
        }
    }
    return res;
}

Object* AndFunction::DeepCopy() {
    return GetInstance<Heap>().Make<AndFunction>();
}

Object* OrFunction::operator()(Object* root) {
    ThrowScope();
    if (root == nullptr) {
        return GetInstance<Heap>().Make<Symbol>(false);
    }
    Object* res = nullptr;
    while (root != nullptr) {
        root->ThrowScope();
        if (Is<Cell>(root)) {
            res = As<Cell>(root)->GetFirst()->Calculate();
            root = As<Cell>(root)->GetSecond();
        } else {
            res = root->Calculate();
            root = nullptr;
        }
        if (!Is<Symbol>(res) || (res->ToString() != "#f")) {
            return res;
        }
    }
    return res;
}

Object* OrFunction::DeepCopy() {
    return GetInstance<Heap>().Make<OrFunction>();
}

Object* QuoteFunction::operator()(Object* root) {
    ThrowScope();
    if (root == nullptr || !Is<Cell>(root)) {
        throw RuntimeError("Quote should have arguments");
    }
    return As<Cell>(root)->GetFirst();
}

Object* QuoteFunction::DeepCopy() {
    return GetInstance<Heap>().Make<QuoteFunction>();
}

Object* IntegerPredicate::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    return GetInstance<Heap>().Make<Symbol>(IsExpectedType<Number>(args));
}

Object* IntegerPredicate::DeepCopy() {
    return GetInstance<Heap>().Make<IntegerPredicate>();
}

Object* PairPredicate::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    size_t depth = 0;
    bool is_end_null = true;
    DfsList(args[0], depth, is_end_null);
    return GetInstance<Heap>().Make<Symbol>(depth == 2 || (depth == 1 && !is_end_null));
}

Object* PairPredicate::DeepCopy() {
    return GetInstance<Heap>().Make<PairPredicate>();
}

Object* NullPredicate::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    size_t depth = 0;
    bool is_end_null = true;
    DfsList(args[0], depth, is_end_null);
    return GetInstance<Heap>().Make<Symbol>(depth == 0);
}

Object* NullPredicate::DeepCopy() {
    return GetInstance<Heap>().Make<NullPredicate>();
}

Object* ListPredicate::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    size_t depth = 0;
    bool is_end_null = true;
    DfsList(args[0], depth, is_end_null);
    return GetInstance<Heap>().Make<Symbol>(is_end_null);
}

Object* ListPredicate::DeepCopy() {
    return GetInstance<Heap>().Make<ListPredicate>();
}

Object* Cons::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 2, 2);
    return GetInstance<Heap>().Make<Cell>(args[0]->DeepCopy(), args[1]->DeepCopy());
}

Object* Cons::DeepCopy() {
    return GetInstance<Heap>().Make<Cons>();
}

Object* Car::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    CheckExpectedType<Cell>(args);
    return As<Cell>(args[0])->GetFirst();
}

Object* Car::DeepCopy() {
    return GetInstance<Heap>().Make<Car>();
}

Object* Cdr::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    CheckExpectedType<Cell>(args);
    return As<Cell>(args[0])->GetSecond();
}

Object* Cdr::DeepCopy() {
    return GetInstance<Heap>().Make<Cdr>();
}

Object* ListFunction::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    Object* ptr = nullptr;
    for (auto it = args.rbegin(); it != args.rend(); ++it) {
        ptr = GetInstance<Heap>().Make<Cell>((*it)->DeepCopy(), ptr);
    }
    return ptr;
}

Object* ListFunction::DeepCopy() {
    return GetInstance<Heap>().Make<ListFunction>();
}

Object* ListRef::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 2, 2);
    CheckExpectedType<Cell>({args[0]});
    CheckExpectedType<Number>({args[1]});

    size_t pos = As<Number>(args[1])->GetValue();

    Object* ptr = args[0];
    while (ptr != nullptr) {
        ptr->ThrowScope();
        if (pos == 0) {
            return Is<Cell>(ptr) ? As<Cell>(ptr)->GetFirst() : ptr;
        }
        --pos;
        ptr = Is<Cell>(ptr) ? As<Cell>(ptr)->GetSecond() : nullptr;
    }
    throw RuntimeError("Index overflow");
}

Object* ListRef::DeepCopy() {
    return GetInstance<Heap>().Make<ListRef>();
}

Object* ListTail::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 2, 2);
    CheckExpectedType<Cell>({args[0]});
    CheckExpectedType<Number>({args[1]});

    size_t pos = As<Number>(args[1])->GetValue();

    Object* ptr = args[0];
    while (ptr != nullptr) {
        ptr->ThrowScope();
        if (pos == 0) {
            return ptr;
        }
        --pos;
        ptr = Is<Cell>(ptr) ? As<Cell>(ptr)->GetSecond() : nullptr;
    }
    if (pos == 0) {
        return nullptr;
    }
    throw RuntimeError("Index overflow");
}

Object* ListTail::DeepCopy() {
    return GetInstance<Heap>().Make<ListTail>();
}

Object* SymbolPredicate::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgs(root);
    RequireArgsRE(args, 1, 1);
    bool f = IsExpectedType<Symbol>(args) &&
             (args[0]->ToString() != "#f" || args[0]->ToString() != "#t");
    return GetInstance<Heap>().Make<Symbol>(f);
}

Object* SymbolPredicate::DeepCopy() {
    return GetInstance<Heap>().Make<SymbolPredicate>();
}

Object* Define::operator()(Object* root) {
    ThrowScope();
    if (Is<Cell>(root) && Is<Cell>(As<Cell>(root)->GetFirst())) {
        return DefineLambda(root);
    }
    auto args = GetArgsWithoutCalculating(root);

    RequireArgsSE(args, 2, 2);
    CheckExpectedType<Symbol>({args[0]});

    As<Scope>(root->GetScope())->Add(args[0]->ToString(), args[1]->Calculate()->DeepCopy());

    return args[0];
}

Object* Define::DeepCopy() {
    return GetInstance<Heap>().Make<Define>();
}

Object* SetVariable::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgsWithoutCalculating(root);
    RequireArgsSE(args, 2, 2);
    CheckExpectedType<Symbol>({args[0]});
    As<Scope>(root->GetScope())->Set(args[0]->ToString(), args[1]->Calculate()->DeepCopy());
    return As<Scope>(root->GetScope())->Get(args[0]->ToString());
}

Object* SetVariable::DeepCopy() {
    return GetInstance<Heap>().Make<SetVariable>();
}

Object* IfFunc::operator()(Object* root) {
    ThrowScope();
    auto args = GetArgsWithoutCalculating(root);
    RequireArgsSE(args, 2, 3);
    auto predicate = args[0]->Calculate();
    bool f = IsExpectedType<Symbol>({predicate}) &&
             (predicate->ToString() != "#f" || predicate->ToString() != "#t");
    if (!f) {
        throw RuntimeError("if should have boolean");
    }
    return predicate->ToString() == "#t" ? args[1]->Calculate()
                                         : (args.size() == 2 ? nullptr : args[2]->Calculate());
}

Object* IfFunc::DeepCopy() {
    return GetInstance<Heap>().Make<IfFunc>();
}

Object* Define::DefineLambda(Object* root) {
    auto args = GetArgsWithoutCalculating(As<Cell>(root)->GetFirst());
    CheckExpectedType<Symbol>(args);
    RequireArgsSE(args, 1, std::numeric_limits<size_t>::max());

    auto name = args[0];

    std::vector<Object*> variables;
    variables.reserve(args.size() - 1);
    for (size_t i = 1; i < args.size(); ++i) {
        variables.push_back(args[i]);
    }

    if (As<Cell>(root)->GetSecond() == nullptr) {
        throw SyntaxError("Lambda should return something");
    }
    auto lambda = GetInstance<Heap>().Make<Lambda>(variables, As<Cell>(root)->GetSecond(), scope_);

    As<Scope>(scope_)->Add(name->ToString(), lambda);

    return name;
}

Object* LambdaDefinition::operator()(Object* root) {  // lambda initialization
    if (!Is<Cell>(root)) {
        throw SyntaxError("Null lamnda");
    }

    root->ThrowScope();
    auto args = GetArgsWithoutCalculating(As<Cell>(root)->GetFirst());
    CheckExpectedType<Symbol>(args);

    if (As<Cell>(root)->GetSecond() == nullptr) {
        throw SyntaxError("Lambda should return something");
    }
    return GetInstance<Heap>().Make<Lambda>(args, As<Cell>(root)->GetSecond(), scope_);
}

Object* LambdaDefinition::DeepCopy() {
    return GetInstance<Heap>().Make<LambdaDefinition>();
}

Object* Lambda::operator()(Object* root) {
    auto local_scope = GetInstance<Heap>().Make<Scope>(scope_);
    auto new_body = body_->DeepCopy();

    auto args = GetArgs(root);
    RequireArgsRE(args, local_variables_.size(), local_variables_.size());
    for (size_t i = 0; i < args.size(); ++i) {
        As<Scope>(local_scope)->Add(local_variables_[i]->ToString(), args[i]);
    }

    new_body->AddScope(local_scope);
    new_body->ThrowScope();

    auto return_values = GetArgs(new_body);
    return return_values.back();
}

Object* Lambda::DeepCopy() {
    return GetInstance<Heap>().Make<Lambda>(local_variables_, body_, scope_);
}

void Lambda::AddScope([[maybe_unused]] Object* scope) {
}

Object* SetCar::operator()(Object* root) {
    auto args = GetArgsWithoutCalculating(root);
    RequireArgsSE(args, 2, 2);
    auto pair = args[0]->Calculate();

    CheckExpectedType<Cell>({pair});

    RemoveDependency(As<Cell>(pair)->first_);
    As<Cell>(pair)->first_ = args[1]->Calculate();
    AddDependency(As<Cell>(pair)->first_);

    return args[0];
}

Object* SetCar::DeepCopy() {
    return GetInstance<Heap>().Make<SetCar>();
}

Object* SetCdr::operator()(Object* root) {
    auto args = GetArgsWithoutCalculating(root);
    RequireArgsSE(args, 2, 2);
    auto pair = args[0]->Calculate();

    CheckExpectedType<Cell>({pair});

    RemoveDependency(As<Cell>(pair)->second_);
    As<Cell>(pair)->second_ = args[1]->Calculate();
    AddDependency(As<Cell>(pair)->second_);

    return args[0];
}

Object* SetCdr::DeepCopy() {
    return GetInstance<Heap>().Make<SetCdr>();
}
