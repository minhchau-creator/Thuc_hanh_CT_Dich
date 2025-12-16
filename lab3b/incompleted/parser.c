/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include "debug.h"

Token *currentToken;
Token *lookAhead;

extern Type* intType;
extern Type* charType;
extern SymTab* symtab;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  // Tạo program object, enter scope, compile block, và exit scope
  Object* program;
  
  eat(KW_PROGRAM);
  // Lưu tên program từ token
  eat(TK_IDENT);
  program = createProgramObject(currentToken->string);
  // Enter vào scope của program
  enterBlock(program->progAttrs->scope);
  
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  
  // Exit khỏi program scope
  exitBlock();
}

void compileBlock(void) {
  // Xử lý khai báo CONST: tạo và đăng ký constant objects
  Object* constObj;
  ConstantValue* constValue;
  
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);

    do {
      eat(TK_IDENT);
      // Tạo constant object với tên từ token
      constObj = createConstantObject(currentToken->string);
      
      eat(SB_EQ);
      // Compile constant value
      constValue = compileConstant();
      // Gán giá trị cho constant object
      constObj->constAttrs->value = constValue;
      
      // Declare constant vào symbol table
      declareObject(constObj);
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock2();
  } 
  else compileBlock2();
}

void compileBlock2(void) {
  // Xử lý khai báo TYPE: tạo và đăng ký type objects
  Object* typeObj;
  Type* type;
  
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);

    do {
      eat(TK_IDENT);
      // Tạo type object với tên từ token
      typeObj = createTypeObject(currentToken->string);
      
      eat(SB_EQ);
      // Compile type definition
      type = compileType();
      // Gán actual type cho type object
      typeObj->typeAttrs->actualType = type;
      
      // Declare type vào symbol table
      declareObject(typeObj);
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  // Xử lý khai báo VAR: tạo và đăng ký variable objects
  Object* varObj;
  Type* type;
  
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);

    do {
      eat(TK_IDENT);
      // Tạo variable object với tên từ token
      varObj = createVariableObject(currentToken->string);
      
      eat(SB_COLON);
      // Compile type
      type = compileType();
      // Gán type cho variable object
      varObj->varAttrs->type = type;
      
      // Declare variable vào symbol table
      declareObject(varObj);
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileSubDecls(void) {
  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else compileProcDecl();
  }
}

void compileFuncDecl(void) {
  // Tạo function object, compile params, return type, và body
  Object* funcObj;
  Type* returnType;
  
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  // Tạo function object với tên từ token
  funcObj = createFunctionObject(currentToken->string);
  
  // Declare function vào symbol table trước khi compile params
  declareObject(funcObj);
  
  // Enter vào scope của function để compile params
  enterBlock(funcObj->funcAttrs->scope);
  
  // Compile parameters
  compileParams();
  
  eat(SB_COLON);
  // Compile return type
  returnType = compileBasicType();
  // Gán return type cho function
  funcObj->funcAttrs->returnType = returnType;
  
  eat(SB_SEMICOLON);
  
  // Compile function body
  compileBlock();
  
  // Exit khỏi function scope
  exitBlock();
  
  eat(SB_SEMICOLON);
}

void compileProcDecl(void) {
  // Tạo procedure object, compile params và body
  Object* procObj;
  
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  // Tạo procedure object với tên từ token
  procObj = createProcedureObject(currentToken->string);
  
  // Declare procedure vào symbol table trước khi compile params
  declareObject(procObj);
  
  // Enter vào scope của procedure để compile params
  enterBlock(procObj->procAttrs->scope);
  
  // Compile parameters
  compileParams();
  
  eat(SB_SEMICOLON);
  
  // Compile procedure body
  compileBlock();
  
  // Exit khỏi procedure scope
  exitBlock();
  
  eat(SB_SEMICOLON);
}

ConstantValue* compileUnsignedConstant(void) {
  // Tạo và trả về unsigned constant value
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    // Tạo int constant từ giá trị token
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    // Lookup constant đã được define trước đó
    obj = lookupObject(currentToken->string);
    if (obj != NULL && obj->kind == OBJ_CONSTANT) {
      // Duplicate constant value
      constValue = duplicateConstantValue(obj->constAttrs->value);
    } else {
      error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
    }
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    // Tạo char constant từ giá trị token
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

ConstantValue* compileConstant(void) {
  // Tạo và trả về constant (có thể có dấu +/-)
  ConstantValue* constValue;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    // Dấu + không thay đổi giá trị
    constValue = compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    // Dấu - đảo dấu giá trị
    constValue = compileConstant2();
    if (constValue->type == TP_INT) {
      constValue->intValue = -(constValue->intValue);
    } else {
      error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
    }
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    // Tạo char constant
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    constValue = compileConstant2();
    break;
  }
  return constValue;
}

ConstantValue* compileConstant2(void) {
  // Tạo và trả về constant value (số hoặc identifier)
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    // Tạo int constant từ giá trị token
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    // Lookup constant đã được define trước đó
    obj = lookupObject(currentToken->string);
    if (obj != NULL && obj->kind == OBJ_CONSTANT) {
      // Duplicate constant value
      constValue = duplicateConstantValue(obj->constAttrs->value);
    } else {
      error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
    }
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

Type* compileType(void) {
  // Tạo và trả về type (Integer, Char, Array, hoặc user-defined)
  Type* type;
  Type* elementType;
  int arraySize;
  Object* obj;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER);
    // Trả về duplicate của intType
    type = makeIntType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR);
    // Trả về duplicate của charType
    type = makeCharType();
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);
    // Lưu array size từ token
    arraySize = currentToken->value;
    eat(SB_RSEL);
    eat(KW_OF);
    // Compile element type (đệ quy)
    elementType = compileType();
    // Tạo array type
    type = makeArrayType(arraySize, elementType);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    // Lookup type đã được define trước đó
    obj = lookupObject(currentToken->string);
    if (obj != NULL && obj->kind == OBJ_TYPE) {
      // Duplicate actual type
      type = duplicateType(obj->typeAttrs->actualType);
    } else {
      error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
    }
    break;
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

Type* compileBasicType(void) {
  // Tạo và trả về basic type (Integer hoặc Char)
  Type* type;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER);
    // Trả về duplicate của intType
    type = makeIntType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR);
    // Trả về duplicate của charType
    type = makeCharType();
    break;
  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON) {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParam(void) {
  // Tạo và đăng ký parameter (value hoặc reference)
  Object* param;
  Type* type;
  enum ParamKind kind;
  
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    // Pass-by-value parameter
    eat(TK_IDENT);
    // Tạo parameter object với kind = PARAM_VALUE
    param = createParameterObject(currentToken->string, PARAM_VALUE, symtab->currentScope->owner);
    
    eat(SB_COLON);
    // Compile type
    type = compileBasicType();
    // Gán type cho parameter
    param->paramAttrs->type = type;
    
    // Declare parameter
    declareObject(param);
    break;
  case KW_VAR:
    // Pass-by-reference parameter
    eat(KW_VAR);
    eat(TK_IDENT);
    // Tạo parameter object với kind = PARAM_REFERENCE
    param = createParameterObject(currentToken->string, PARAM_REFERENCE, symtab->currentScope->owner);
    
    eat(SB_COLON);
    // Compile type
    type = compileBasicType();
    // Gán type cho parameter
    param->paramAttrs->type = type;
    
    // Declare parameter
    declareObject(param);
    break;
  default:
    error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileStatements(void) {
  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileLValue(void) {
  eat(TK_IDENT);
  compileIndexes();
}

void compileAssignSt(void) {
  compileLValue();
  eat(SB_ASSIGN);
  compileExpression();
}

void compileCallSt(void) {
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
}

void compileGroupSt(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void) {
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
}

void compileForSt(void) {
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
}

void compileArgument(void) {
  compileExpression();
}

void compileArguments(void) {
  switch (lookAhead->tokenType) {
  case SB_LPAR:
    eat(SB_LPAR);
    compileArgument();

    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      compileArgument();
    }

    eat(SB_RPAR);
    break;
    // Check FOLLOW set 
  case SB_TIMES:
  case SB_SLASH:
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileCondition(void) {
  compileExpression();
  switch (lookAhead->tokenType) {
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  default:
    error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }

  compileExpression();
}

void compileExpression(void) {
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileExpression2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileExpression2();
    break;
  default:
    compileExpression2();
  }
}

void compileExpression2(void) {
  compileTerm();
  compileExpression3();
}


void compileExpression3(void) {
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileTerm();
    compileExpression3();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileTerm();
    compileExpression3();
    break;
    // check the FOLLOW set
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileTerm(void) {
  compileFactor();
  compileTerm2();
}

void compileTerm2(void) {
  switch (lookAhead->tokenType) {
  case SB_TIMES:
    eat(SB_TIMES);
    compileFactor();
    compileTerm2();
    break;
  case SB_SLASH:
    eat(SB_SLASH);
    compileFactor();
    compileTerm2();
    break;
    // check the FOLLOW set
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileFactor(void) {
  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    switch (lookAhead->tokenType) {
    case SB_LPAR:
      compileArguments();
      break;
    case SB_LSEL:
      compileIndexes();
      break;
    default:
      break;
    }
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileIndexes(void) {
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();

  compileProgram();

  printObject(symtab->program,0);

  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
