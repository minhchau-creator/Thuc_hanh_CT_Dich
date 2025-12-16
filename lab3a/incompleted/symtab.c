/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

void freeObject(Object* obj);
void freeScope(Scope* scope);
void freeObjectList(ObjectNode *objList);
void freeReferenceList(ObjectNode *objList);

SymTab* symtab;
Type* intType;
Type* charType;

/******************* Type utilities ******************************/

Type* makeIntType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_INT;
  return type;
}

Type* makeCharType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_CHAR;
  return type;
}

Type* makeArrayType(int arraySize, Type* elementType) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_ARRAY;
  type->arraySize = arraySize;
  type->elementType = elementType;
  return type;
}

Type* duplicateType(Type* type) {
  Type* newType = (Type*) malloc(sizeof(Type));
  newType->typeClass = type->typeClass;
  // If it's an array type, we need to recursively duplicate the element type
  if (type->typeClass == TP_ARRAY) {
    newType->arraySize = type->arraySize;
    newType->elementType = duplicateType(type->elementType);
  }
  return newType;
}

int compareType(Type* type1, Type* type2) {
  // check the type classes are the same
  if (type1->typeClass != type2->typeClass)
    return 0; // Different type classes -> 0
  // check array size and element type
  if (type1->typeClass == TP_ARRAY) {
    if (type1->arraySize != type2->arraySize)
      return 0; // Different array sizes -> 0
    return compareType(type1->elementType, type2->elementType);
  }
  return 1;
}

void freeType(Type* type) {
  // If it's an array type, recursively free the element type first
  if (type->typeClass == TP_ARRAY) {
    freeType(type->elementType);
  }
  // Free the current type structure
  free(type);
}

/******************* Constant utility ******************************/

ConstantValue* makeIntConstant(int i) {
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  // Set type to integer
  value->type = TP_INT;
  // Store the integer value in the union
  value->intValue = i;
  return value;
}

ConstantValue* makeCharConstant(char ch) {
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  // Set type to character
  value->type = TP_CHAR;
  // Store the character value in the union
  value->charValue = ch;
  return value;
}

ConstantValue* duplicateConstantValue(ConstantValue* v) {
  ConstantValue* newValue = (ConstantValue*) malloc(sizeof(ConstantValue));
  // Copy the type
  newValue->type = v->type;
  // Copy the appropriate value based on type (union can hold either int or char)
  if (v->type == TP_INT)
    newValue->intValue = v->intValue;
  else if (v->type == TP_CHAR)
    newValue->charValue = v->charValue;
  return newValue;
}

/******************* Object utilities ******************************/

Scope* createScope(Object* owner, Scope* outer) {
  Scope* scope = (Scope*) malloc(sizeof(Scope));
  scope->objList = NULL;
  scope->owner = owner;
  scope->outer = outer;
  return scope;
}

Object* createProgramObject(char *programName) {
  Object* program = (Object*) malloc(sizeof(Object));
  strcpy(program->name, programName);
  program->kind = OBJ_PROGRAM;
  program->progAttrs = (ProgramAttributes*) malloc(sizeof(ProgramAttributes));
  program->progAttrs->scope = createScope(program,NULL);
  symtab->program = program;

  return program;
}

Object* createConstantObject(char *name) {
  // Allocate memory for the object
  Object* obj = (Object*) malloc(sizeof(Object));
  // Copy the constant name
  strcpy(obj->name, name);
  // Set object kind to constant
  obj->kind = OBJ_CONSTANT;
  // Allocate and initialize constant attributes
  obj->constAttrs = (ConstantAttributes*) malloc(sizeof(ConstantAttributes));
  // Value will be set later by caller
  obj->constAttrs->value = NULL;
  return obj;
}

Object* createTypeObject(char *name) {
  // Allocate memory for the object
  Object* obj = (Object*) malloc(sizeof(Object));
  // Copy the type name
  strcpy(obj->name, name);
  // Set object kind to type
  obj->kind = OBJ_TYPE;
  // Allocate and initialize type attributes
  obj->typeAttrs = (TypeAttributes*) malloc(sizeof(TypeAttributes));
  // Actual type will be set later by caller
  obj->typeAttrs->actualType = NULL;
  return obj;
}

Object* createVariableObject(char *name) {
  // Allocate memory for the object
  Object* obj = (Object*) malloc(sizeof(Object));
  // Copy the variable name
  strcpy(obj->name, name);
  // Set object kind to variable
  obj->kind = OBJ_VARIABLE;
  // Allocate and initialize variable attributes
  obj->varAttrs = (VariableAttributes*) malloc(sizeof(VariableAttributes));
  // Type will be set later by caller
  obj->varAttrs->type = NULL;
  // Link variable to current scope
  obj->varAttrs->scope = symtab->currentScope;
  return obj;
}

Object* createFunctionObject(char *name) {
  // Allocate memory for the object
  Object* obj = (Object*) malloc(sizeof(Object));
  // Copy the function name
  strcpy(obj->name, name);
  // Set object kind to function
  obj->kind = OBJ_FUNCTION;
  // Allocate and initialize function attributes
  obj->funcAttrs = (FunctionAttributes*) malloc(sizeof(FunctionAttributes));
  // Parameter list starts empty
  obj->funcAttrs->paramList = NULL;
  // Return type will be set later by caller
  obj->funcAttrs->returnType = NULL;
  // Create a new scope for the function body (owner=this function, outer=current scope)
  obj->funcAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createProcedureObject(char *name) {
  // Allocate memory for the object
  Object* obj = (Object*) malloc(sizeof(Object));
  // Copy the procedure name
  strcpy(obj->name, name);
  // Set object kind to procedure
  obj->kind = OBJ_PROCEDURE;
  // Allocate and initialize procedure attributes
  obj->procAttrs = (ProcedureAttributes*) malloc(sizeof(ProcedureAttributes));
  // Parameter list starts empty
  obj->procAttrs->paramList = NULL;
  // Create a new scope for the procedure body (owner=this procedure, outer=current scope)
  obj->procAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createParameterObject(char *name, enum ParamKind kind, Object* owner) {
  // Allocate memory for the object
  Object* obj = (Object*) malloc(sizeof(Object));
  // Copy the parameter name
  strcpy(obj->name, name);
  // Set object kind to parameter
  obj->kind = OBJ_PARAMETER;
  // Allocate and initialize parameter attributes
  obj->paramAttrs = (ParameterAttributes*) malloc(sizeof(ParameterAttributes));
  // Set parameter kind (PARAM_VALUE for pass-by-value, PARAM_REFERENCE for pass-by-reference)
  obj->paramAttrs->kind = kind;
  // Type will be set later by caller
  obj->paramAttrs->type = NULL;
  // Link parameter to its owner function/procedure
  obj->paramAttrs->function = owner;
  return obj;
}

void freeObject(Object* obj) {
  // Free object based on its kind - each kind has different attributes to clean up
  switch (obj->kind) {
  case OBJ_CONSTANT:
    // Free constant value if it exists
    if (obj->constAttrs->value != NULL)
      free(obj->constAttrs->value);
    free(obj->constAttrs);
    break;
  case OBJ_TYPE:
    // Recursively free the actual type definition
    if (obj->typeAttrs->actualType != NULL)
      freeType(obj->typeAttrs->actualType);
    free(obj->typeAttrs);
    break;
  case OBJ_VARIABLE:
    // Recursively free the variable's type
    if (obj->varAttrs->type != NULL)
      freeType(obj->varAttrs->type);
    free(obj->varAttrs);
    break;
  case OBJ_FUNCTION:
    // Free parameter list (including all parameter objects)
    freeObjectList(obj->funcAttrs->paramList);
    // Free return type
    if (obj->funcAttrs->returnType != NULL)
      freeType(obj->funcAttrs->returnType);
    // Free function's local scope and all objects within it
    freeScope(obj->funcAttrs->scope);
    free(obj->funcAttrs);
    break;
  case OBJ_PROCEDURE:
    // Free parameter list (including all parameter objects)
    freeObjectList(obj->procAttrs->paramList);
    // Free procedure's local scope and all objects within it
    freeScope(obj->procAttrs->scope);
    free(obj->procAttrs);
    break;
  case OBJ_PROGRAM:
    // Free program's global scope and all objects within it
    freeScope(obj->progAttrs->scope);
    free(obj->progAttrs);
    break;
  case OBJ_PARAMETER:
    // Free parameter's type
    if (obj->paramAttrs->type != NULL)
      freeType(obj->paramAttrs->type);
    free(obj->paramAttrs);
    break;
  }
  // Finally free the object itself
  free(obj);
}

void freeScope(Scope* scope) {
  // Free all objects declared in this scope
  freeObjectList(scope->objList);
  // Free the scope structure itself
  free(scope);
}

void freeObjectList(ObjectNode *objList) {
  ObjectNode *node = objList;
  // Traverse the linked list of object nodes
  while (node != NULL) {
    ObjectNode *nextNode = node->next;
    // Free the object contained in this node
    freeObject(node->object);
    // Free the node itself
    free(node);
    // Move to next node
    node = nextNode;
  }
}

void freeReferenceList(ObjectNode *objList) {
  ObjectNode *node = objList;
  // Traverse the linked list of object nodes
  while (node != NULL) {
    ObjectNode *nextNode = node->next;
    // Only free the node, NOT the object (it's just a reference)
    // The actual objects are owned and freed elsewhere
    free(node);
    // Move to next node
    node = nextNode;
  }
}

void addObject(ObjectNode **objList, Object* obj) {
  ObjectNode* node = (ObjectNode*) malloc(sizeof(ObjectNode));
  node->object = obj;
  node->next = NULL;
  if ((*objList) == NULL) 
    *objList = node;
  else {
    ObjectNode *n = *objList;
    while (n->next != NULL) 
      n = n->next;
    n->next = node;
  }
}

Object* findObject(ObjectNode *objList, char *name) {
  ObjectNode *node = objList;
  // Traverse the linked list searching for an object with matching name
  while (node != NULL) {
    // Compare object name with the search name
    if (strcmp(node->object->name, name) == 0)
      return node->object; // Found!
    node = node->next;
  }
  // Object not found in the list
  return NULL;
}

/******************* others ******************************/

void initSymTab(void) {
  Object* obj;
  Object* param;

  symtab = (SymTab*) malloc(sizeof(SymTab));
  symtab->globalObjectList = NULL;
  
  obj = createFunctionObject("READC");
  obj->funcAttrs->returnType = makeCharType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createFunctionObject("READI");
  obj->funcAttrs->returnType = makeIntType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEI");
  param = createParameterObject("i", PARAM_VALUE, obj);
  param->paramAttrs->type = makeIntType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEC");
  param = createParameterObject("ch", PARAM_VALUE, obj);
  param->paramAttrs->type = makeCharType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITELN");
  addObject(&(symtab->globalObjectList), obj);

  intType = makeIntType();
  charType = makeCharType();
}

void cleanSymTab(void) {
  freeObject(symtab->program);
  freeObjectList(symtab->globalObjectList);
  free(symtab);
  freeType(intType);
  freeType(charType);
}

void enterBlock(Scope* scope) {
  symtab->currentScope = scope;
}

void exitBlock(void) {
  symtab->currentScope = symtab->currentScope->outer;
}

void declareObject(Object* obj) {
  if (obj->kind == OBJ_PARAMETER) {
    Object* owner = symtab->currentScope->owner;
    switch (owner->kind) {
    case OBJ_FUNCTION:
      addObject(&(owner->funcAttrs->paramList), obj);
      break;
    case OBJ_PROCEDURE:
      addObject(&(owner->procAttrs->paramList), obj);
      break;
    default:
      break;
    }
  }
 
  addObject(&(symtab->currentScope->objList), obj);
}


