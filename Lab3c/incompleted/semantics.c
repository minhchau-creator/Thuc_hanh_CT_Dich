/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include "error.h"

extern SymTab* symtab;
extern Token* currentToken;

Object* lookupObject(char *name) {
  // Search for the object in the current scope and outer scopes
  Scope* scope = symtab->currentScope;
  Object* obj;
  
  // Search through all scopes from current to outer
  while (scope != NULL) {
    obj = findObject(scope->objList, name);
    if (obj != NULL) return obj;
    scope = scope->outer;
  }
  
  // Search in global object list (predefined functions/procedures)
  obj = findObject(symtab->globalObjectList, name);
  return obj;
}

void checkFreshIdent(char *name) {
  // Check if the identifier is NOT already declared in the current scope
  Object* obj = findObject(symtab->currentScope->objList, name);
  if (obj != NULL) {
    error(ERR_DUPLICATE_IDENT, currentToken->lineNo, currentToken->colNo);
  }
}

Object* checkDeclaredIdent(char* name) {
  // Check if the identifier is declared
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

Object* checkDeclaredConstant(char* name) {
  // Check if a constant identifier is declared
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
  }
  if (obj->kind != OBJ_CONSTANT) {
    error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

Object* checkDeclaredType(char* name) {
  // Check if a type identifier is declared
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
  }
  if (obj->kind != OBJ_TYPE) {
    error(ERR_INVALID_TYPE, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

Object* checkDeclaredVariable(char* name) {
  // Check if a variable identifier is declared
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
  }
  if (obj->kind != OBJ_VARIABLE) {
    error(ERR_INVALID_VARIABLE, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

Object* checkDeclaredFunction(char* name) {
  // Check if a function identifier is declared
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo);
  }
  if (obj->kind != OBJ_FUNCTION) {
    error(ERR_INVALID_FUNCTION, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

Object* checkDeclaredProcedure(char* name) {
  // Check if a procedure identifier is declared
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo);
  }
  if (obj->kind != OBJ_PROCEDURE) {
    error(ERR_INVALID_PROCEDURE, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

Object* checkDeclaredLValueIdent(char* name) {
  // Check if the identifier is a valid lvalue (variable, parameter, or function)
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
  }
  
  // Valid lvalue identifiers are: variables, parameters, and functions (for return value assignment)
  if (obj->kind != OBJ_VARIABLE && obj->kind != OBJ_PARAMETER && obj->kind != OBJ_FUNCTION) {
    error(ERR_INVALID_LVALUE, currentToken->lineNo, currentToken->colNo);
  }
  
  return obj;
}

