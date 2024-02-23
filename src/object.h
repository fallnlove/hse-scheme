#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "error.h"
#include "classes.h"
#include "heap.h"
#include "tokenizer.h"

class Object {
public:
    virtual ~Object() = default;

    virtual std::string ToString();

    virtual Object* DeepCopy();

    virtual Object* Calculate();

    virtual Object* operator()(Object* root);

    virtual void AddScope(Object* scope);

    void ThrowScope();

    Object* GetScope();

protected:
    Object* scope_;
    std::set<Object*> neighbours_;
    bool is_achivable_ = true;

    void AddDependency(Object* other);

    void RemoveDependency(Object* other);

    void Mark();

    Object() = default;

    friend Heap;
};

class Number : public Object {
public:
    int64_t GetValue() const;

    virtual std::string ToString() override;

    virtual Object* DeepCopy() override;

    virtual Object* Calculate() override;

protected:
    int64_t value_;

    Number(int64_t value);

    friend Heap;
};

class Symbol : public Object {
public:
    const std::string& GetName() const;

    virtual std::string ToString() override;

    virtual Object* DeepCopy() override;

    virtual Object* Calculate() override;

protected:
    std::string str_;

    explicit Symbol(std::string str);

    explicit Symbol(const char* str);

    explicit Symbol(bool boolean);

    friend Heap;
};

class Cell : public Object {
public:
    Object* GetFirst() const;
    Object* GetSecond() const;

    virtual std::string ToString() override;

    virtual Object* DeepCopy() override;

    virtual Object* Calculate() override;

private:
    Object* first_;
    Object* second_;

    friend class SetCar;
    friend class SetCdr;

    friend Object* Read(Tokenizer* tokenizer);

    friend Object* ReadList(Tokenizer* tokenizer);

    Cell() = default;

    Cell(Object* f, Object* s);

    friend Heap;
};

class Scope : public Object {
public:
    void Add(std::string name, Object* value);

    void Set(std::string name, Object* new_value);

    Object* Get(std::string name);

    virtual Object* DeepCopy() override;

private:
    Object* parent_;
    std::unordered_map<std::string, Object*> scope_names_;

    Scope() = delete;

    Scope(Object* parent);

    friend Heap;
};

/////////////////////////////////HELPERS///////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

std::vector<Object*> GetArgs(Object* root);

std::vector<Object*> GetArgsWithoutCalculating(Object* root);

void RequireArgsRE(const std::vector<Object*>& args, size_t min_cnt, size_t max_cnt);

void RequireArgsSE(const std::vector<Object*>& args, size_t min_cnt, size_t max_cnt);

void DfsList(Object* root, size_t& depth, bool& is_end_null);

template <class T>
bool IsExpectedType(const std::vector<Object*>& args) {
    for (const auto& i : args) {
        if (!Is<T>(i)) {
            return false;
        }
    }
    return true;
}

template <class T>
void CheckExpectedType(const std::vector<Object*>& args) {
    if (!IsExpectedType<T>(args)) {
        throw RuntimeError("Invalid type of argument");
    }
}

template <class T, int64_t StartingValue, size_t MaxArgs, size_t MinArgs>
class FoldingInt : public Object {
public:
    virtual Object* operator()(Object* root) override {
        ThrowScope();
        auto args = GetArgs(root);
        CheckExpectedType<Number>(args);
        RequireArgsRE(args, MinArgs, MaxArgs);

        int64_t result = StartingValue;
        for (const auto& i : args) {
            result = func_(result, As<Number>(i)->GetValue());
        }

        return GetInstance<Heap>().Make<Number>(result);
    }

    virtual Object* DeepCopy() override {
        return GetInstance<Heap>().Make<FoldingInt<T, StartingValue, MaxArgs, MinArgs>>();
    }

private:
    T func_;

    FoldingInt() = default;

    friend Heap;
};

template <class T, int64_t StartingValue, size_t MaxArgs>
class FoldingInt<T, StartingValue, MaxArgs, 2> : public Object {
public:
    virtual Object* operator()(Object* root) override {
        ThrowScope();
        auto args = GetArgs(root);
        CheckExpectedType<Number>(args);
        RequireArgsRE(args, 2, std::numeric_limits<size_t>::max());

        int64_t result = func_(As<Number>(args[0])->GetValue(), As<Number>(args[1])->GetValue());
        for (size_t i = 2; i < args.size(); ++i) {
            result = func_(result, As<Number>(args[i])->GetValue());
        }

        return GetInstance<Heap>().Make<Number>(result);
    }

    virtual Object* DeepCopy() override {
        return GetInstance<Heap>().Make<FoldingInt<T, StartingValue, MaxArgs, 2>>();
    }

private:
    T func_;

    FoldingInt() = default;

    friend Heap;
};

template <class T>
class FoldingBoolean : public Object {
public:
    virtual Object* operator()(Object* root) override {
        ThrowScope();
        auto args = GetArgs(root);
        CheckExpectedType<Number>(args);

        bool result = true;
        for (size_t i = 1; i < args.size(); ++i) {
            result &= func_(As<Number>(args[i - 1])->GetValue(), As<Number>(args[i])->GetValue());
        }

        return GetInstance<Heap>().Make<Symbol>(result);
    }

    virtual Object* DeepCopy() override {
        return GetInstance<Heap>().Make<FoldingBoolean>();
    }

private:
    T func_;

    FoldingBoolean() = default;

    friend Heap;
};

////////////////////////////////FUNCTIONS//////////////////////////////////////////////////

class BooleanPredicate : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    BooleanPredicate() = default;
};

class NotFunction : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    NotFunction() = default;
};

class AndFunction : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    AndFunction() = default;
};

class OrFunction : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    OrFunction() = default;
};

class QuoteFunction : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    QuoteFunction() = default;
};

class IntegerPredicate : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    IntegerPredicate() = default;
};

using GreateOrEqual = FoldingBoolean<std::greater_equal<int64_t>>;
using Greate = FoldingBoolean<std::greater<int64_t>>;
using Equal = FoldingBoolean<std::equal_to<int64_t>>;
using LessOrEqual = FoldingBoolean<std::less_equal<int64_t>>;
using Less = FoldingBoolean<std::less<int64_t>>;

using Plus = FoldingInt<std::plus<int64_t>, 0, std::numeric_limits<size_t>::max(), 0>;
using Minus = FoldingInt<std::minus<int64_t>, 0, std::numeric_limits<size_t>::max(), 2>;
using Mul = FoldingInt<std::multiplies<int64_t>, 1, std::numeric_limits<size_t>::max(), 0>;
using Div = FoldingInt<std::divides<int64_t>, 0, std::numeric_limits<size_t>::max(), 2>;

class SpecialMin {
public:
    int64_t operator()(int64_t lhs, int64_t rhs);
};

class SpecialMax {
public:
    int64_t operator()(int64_t lhs, int64_t rhs);
};

class SpecialAbs {
public:
    int64_t operator()(int64_t lhs, int64_t rhs);
};

using Min = FoldingInt<SpecialMin, std::numeric_limits<int64_t>::max(),
                       std::numeric_limits<size_t>::max(), 1>;
using Max = FoldingInt<SpecialMax, std::numeric_limits<int64_t>::min(),
                       std::numeric_limits<size_t>::max(), 1>;
using Abs = FoldingInt<SpecialAbs, 0, 1, 1>;

class PairPredicate : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    PairPredicate() = default;
};

class NullPredicate : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    NullPredicate() = default;
};

class ListPredicate : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    ListPredicate() = default;
};

class Cons : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    Cons() = default;
};

class Car : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    Car() = default;
};

class Cdr : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    Cdr() = default;
};

class ListFunction : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    ListFunction() = default;
};

class ListRef : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    ListRef() = default;
};

class ListTail : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    ListTail() = default;
};

class SymbolPredicate : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    SymbolPredicate() = default;
};

class Define : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    Define() = default;

private:
    Object* DefineLambda(Object* root);
};

class SetVariable : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    SetVariable() = default;
};

class IfFunc : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    IfFunc() = default;
};

class LambdaDefinition : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    LambdaDefinition() = default;
};

class Lambda : public Object {
public:
    virtual void AddScope(Object* scope) override;

    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    std::vector<Object*> local_variables_;
    Object* body_;

    friend Heap;

    Lambda(std::vector<Object*> local_variables, Object* body, Object* scope)
        : local_variables_(local_variables), body_(body) {
        scope_ = scope;
        AddDependency(scope_);
        AddDependency(body_);
        for (auto i : local_variables_) {
            AddDependency(i);
        }
    }
};

class SetCar : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    SetCar() = default;
};

class SetCdr : public Object {
public:
    virtual Object* operator()(Object* root) override;

    virtual Object* DeepCopy() override;

private:
    friend Heap;

    SetCdr() = default;
};

///////////////////////////////////////////////////////////////////////////////
