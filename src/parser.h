#pragma once

#include <memory>

#include "object.h"
#include "tokenizer.h"

Object* Read(Tokenizer* tokenizer);

Object* ReadList(Tokenizer* tokenizer);

bool IsCloseBracket(Token token);

bool IsDot(Token token);

void CheckEnd(Tokenizer* tokenizer);
