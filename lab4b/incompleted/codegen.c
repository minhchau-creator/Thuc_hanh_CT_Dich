/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include "codegen.h"
#include "reader.h"
#include <stdio.h>

#define CODE_SIZE 10000
extern SymTab *symtab;

extern Object *readiFunction;
extern Object *readcFunction;
extern Object *writeiProcedure;
extern Object *writecProcedure;
extern Object *writelnProcedure;

CodeBlock *codeBlock;

void genVariableAddress(Object *var) {
  // TODO
  int currentLevel;
  Object *owner = symtab->currentScope->owner;

  if (owner->kind == OBJ_PROGRAM) {
    currentLevel = 0;
  } else if (owner->kind == OBJ_FUNCTION || owner->kind == OBJ_PROCEDURE) {
    Scope *outerScope = (owner->kind == OBJ_FUNCTION)
                            ? owner->funcAttrs->scope->outer
                            : owner->procAttrs->scope->outer;

    if (outerScope != NULL && outerScope->owner->kind == OBJ_PROGRAM) {
      currentLevel = 1;
    } else {
      currentLevel = 2;
    }
  } else {
    currentLevel = 0;
  }

  switch (var->kind) {
  case OBJ_VARIABLE: {
    int varLevel;
    if (var->varAttrs->scope->owner->kind == OBJ_PROGRAM) {
      varLevel = 0;
    } else {
      varLevel = 1;
    }

    int deltaLevel = currentLevel - varLevel;

    genLA(deltaLevel, var->varAttrs->localOffset);
    break;
  }

  case OBJ_PARAMETER: {
    int paramLevel;
    if (var->paramAttrs->scope->outer == NULL ||
        var->paramAttrs->scope->outer->owner->kind == OBJ_PROGRAM) {
      paramLevel = 1;
    } else {
      paramLevel = 2;
    }

    int deltaLevel = currentLevel - paramLevel;

    if (var->paramAttrs->kind == PARAM_VALUE) {
      genLA(deltaLevel, var->paramAttrs->localOffset);
    } else {
      genLV(deltaLevel, var->paramAttrs->localOffset);
    }
    break;
  }

  default:
    break;
  }
}

void genVariableValue(Object *var) {
  // TODO
  int currentLevel;
  Object *owner = symtab->currentScope->owner;

  // Xác định level hiện tại
  if (owner->kind == OBJ_PROGRAM) {
    currentLevel = 0;
  } else if (owner->kind == OBJ_FUNCTION || owner->kind == OBJ_PROCEDURE) {
    Scope *outerScope = (owner->kind == OBJ_FUNCTION)
                            ? owner->funcAttrs->scope->outer
                            : owner->procAttrs->scope->outer;

    if (outerScope != NULL && outerScope->owner->kind == OBJ_PROGRAM) {
      currentLevel = 1;
    } else {
      currentLevel = 2;
    }
  } else {
    currentLevel = 0;
  }

  switch (var->kind) {
  case OBJ_VARIABLE: {
    int varLevel;
    if (var->varAttrs->scope->owner->kind == OBJ_PROGRAM) {
      varLevel = 0;
    } else {
      varLevel = 1;
    }

    int deltaLevel = currentLevel - varLevel;

    // Sử dụng LV để load giá trị trực tiếp
    genLV(deltaLevel, var->varAttrs->localOffset);
    break;
  }

  case OBJ_PARAMETER: {
    int paramLevel;
    if (var->paramAttrs->scope->outer == NULL ||
        var->paramAttrs->scope->outer->owner->kind == OBJ_PROGRAM) {
      paramLevel = 1;
    } else {
      paramLevel = 2;
    }

    int deltaLevel = currentLevel - paramLevel;

    if (var->paramAttrs->kind == PARAM_VALUE) {
      genLV(deltaLevel, var->paramAttrs->localOffset);
    } else {
      // load địa chỉ rồi load gián tiếp
      genLV(deltaLevel, var->paramAttrs->localOffset);
      genLI();
    }
    break;
  }

  default:
    break;
  }
}

int isPredefinedFunction(Object *func) {
  return ((func == readiFunction) || (func == readcFunction));
}

int isPredefinedProcedure(Object *proc) {
  return ((proc == writeiProcedure) || (proc == writecProcedure) ||
          (proc == writelnProcedure));
}

void genPredefinedProcedureCall(Object *proc) {
  if (proc == writeiProcedure)
    genWRI();
  else if (proc == writecProcedure)
    genWRC();
  else if (proc == writelnProcedure)
    genWLN();
}

void genPredefinedFunctionCall(Object *func) {
  if (func == readiFunction)
    genRI();
  else if (func == readcFunction)
    genRC();
}

void genLA(int level, int offset) { emitLA(codeBlock, level, offset); }

void genLV(int level, int offset) { emitLV(codeBlock, level, offset); }

void genLC(WORD constant) { emitLC(codeBlock, constant); }

void genLI(void) { emitLI(codeBlock); }

void genINT(int delta) { emitINT(codeBlock, delta); }

void genDCT(int delta) { emitDCT(codeBlock, delta); }

Instruction *genJ(CodeAddress label) {
  Instruction *inst = codeBlock->code + codeBlock->codeSize;
  emitJ(codeBlock, label);
  return inst;
}

Instruction *genFJ(CodeAddress label) {
  Instruction *inst = codeBlock->code + codeBlock->codeSize;
  emitFJ(codeBlock, label);
  return inst;
}

void genHL(void) { emitHL(codeBlock); }

void genST(void) { emitST(codeBlock); }

void genCALL(int level, CodeAddress label) {
  emitCALL(codeBlock, level, label);
}

void genEP(void) { emitEP(codeBlock); }

void genEF(void) { emitEF(codeBlock); }

void genRC(void) { emitRC(codeBlock); }

void genRI(void) { emitRI(codeBlock); }

void genWRC(void) { emitWRC(codeBlock); }

void genWRI(void) { emitWRI(codeBlock); }

void genWLN(void) { emitWLN(codeBlock); }

void genAD(void) { emitAD(codeBlock); }

void genSB(void) { emitSB(codeBlock); }

void genML(void) { emitML(codeBlock); }

void genDV(void) { emitDV(codeBlock); }

void genNEG(void) { emitNEG(codeBlock); }

void genCV(void) { emitCV(codeBlock); }

void genEQ(void) { emitEQ(codeBlock); }

void genNE(void) { emitNE(codeBlock); }

void genGT(void) { emitGT(codeBlock); }

void genGE(void) { emitGE(codeBlock); }

void genLT(void) { emitLT(codeBlock); }

void genLE(void) { emitLE(codeBlock); }

void updateJ(Instruction *jmp, CodeAddress label) { jmp->q = label; }

void updateFJ(Instruction *jmp, CodeAddress label) { jmp->q = label; }

CodeAddress getCurrentCodeAddress(void) { return codeBlock->codeSize; }

void initCodeBuffer(void) { codeBlock = createCodeBlock(CODE_SIZE); }

void printCodeBuffer(void) { printCodeBlock(codeBlock); }

void cleanCodeBuffer(void) { freeCodeBlock(codeBlock); }

int serialize(char *fileName) {
  FILE *f;

  f = fopen(fileName, "wb");
  if (f == NULL)
    return IO_ERROR;
  saveCode(codeBlock, f);
  fclose(f);
  return IO_SUCCESS;
}
