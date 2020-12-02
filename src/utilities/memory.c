#include "compiler.h"
#include "utilities/memory.h"

void* reallocate(VM* vm, void* pointer, size_t oldest, size_t newest) {
  vm->allocate += newest - oldest;

  if (newest > oldest)
    if (vm->allocate > vm->garbage)
      recycle(vm);

  if (newest == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, newest);

  if (result == NULL) exit(1);

  return result;
}

void recycle(VM* vm) {
  Parents parents;

  parents.count = 0;
  parents.capacity = 0;
  
  parents.content = NULL;

  roots(vm, &parents);
  traverse(vm, &parents);

  sweep(vm);

  vm->garbage = vm->allocate * GARBAGE_COLLECTOR_GROW_FACTOR;

  free(parents.content);
}

void roots(VM* vm, Parents* parents) {
  for (Value* slot = vm->stack.content; slot < vm->stack.top; slot++)
    mark(parents, *slot);

  for (Upvalue* upvalue = vm->upvalues; upvalue != NULL; upvalue = upvalue->next)
    mark(parents, OBJECT(upvalue));

  for (int i = 0; i < vm->count; i++)
    mark(parents, OBJECT(vm->frames[i].closure));

  Table* table = &vm->globals;

  for (int i = 0; i < table->capacity; i++) {
    Entry* entry = &table->entries[i];
    mark(parents, OBJECT(entry->key));
    mark(parents, entry->value);
  }

  Compiler* compiler = vm->parser->compiler;

  while (compiler != NULL) {
    mark(parents, OBJECT(compiler->function));
    
    compiler = compiler->enclosing;
  }
}

void traverse(VM* vm, Parents* parents) {
  while (parents->count > 0) {
    Object* object = parents->content[--parents->count];

    switch (object->type) {
      case OBJECT_UPVALUE: {
        mark(parents, ((Upvalue*)object)->closed);

        break;
      }

      case OBJECT_FUNCTION: {
        Function* function = (Function*)object;

        mark(parents, OBJECT(function->identifier));

        Constants* constants = &function->chunk.constants;
        
        for (int i = 0; i < constants->count; i++)
          mark(parents, constants->values[i]);

        break;
      }

      case OBJECT_CLOSURE: {
        Closure* closure = (Closure*)object;

        mark(parents, OBJECT(closure->function));

        for (int i = 0; i < closure->count; i++)
          mark(parents, OBJECT(closure->upvalues[i]));

        break;
      }
    }
  }

  table_clear(&vm->strings);
}

void mark(Parents* parents, Value value) {
  if (IS_OBJECT(value) == true) {
    Object* object = AS_OBJECT(value);

    if (object != NULL) {
      if (object->mark == true) return;

      object->mark = true;

      if (object->type == OBJECT_STRING || object->type == OBJECT_NATIVE) return;

      if (parents->capacity < parents->count + 1) {
        parents->capacity = GROW_CAPACITY(parents->capacity);
        parents->content = realloc(parents->content, sizeof(Object*) * parents->capacity);
      }

      parents->content[parents->count++] = object;
    }
  }
}

void sweep(VM* vm) {
  Object* previous = NULL;

  Object* object = vm->objects;

  while (object != NULL) {
    if (object->mark) {
      previous = object;

      object = object->next;

      continue;
    }

    Object* unreachable = object;

    object = object->next;

    if (previous != NULL)
      previous->next = object;
    else vm->objects = object;

    free_object(vm, unreachable);
  }
}

void free_object(VM* vm, Object* object) {
  switch (object->type) {
    case OBJECT_UPVALUE: {
      FREE(vm, Upvalue, object);
      break;
    }

    case OBJECT_NUMBER: {
      Number* number = (Number*)object;
      mpf_clear(number->content);
      FREE(vm, Number, number);
      break;
    }

    case OBJECT_STRING: {
      String* string = (String*)object;
      FREE_ARRAY(vm, char, string->content, string->length + 1);
      FREE(vm, String, object);
      break;
    }

    case OBJECT_FUNCTION: {
      Function* function = (Function*)object;
      free_chunk(&function->chunk);
      FREE(vm, Function, object);
      break;
    }

    case OBJECT_CLOSURE: {
      Closure* closure = (Closure*)object;
      FREE_ARRAY(vm, Upvalue*, closure->upvalues, closure->count);
      FREE(vm, Closure, object);
      break;
    }

    case OBJECT_NATIVE: {
      FREE(vm, Native, object);
      break;
    }
  }
}
