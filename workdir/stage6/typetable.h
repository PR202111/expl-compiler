#ifndef TYPETABLE_H
#define TYPETABLE_H

struct Typetable {
    char *name;                 // Type name (e.g., "int", "bst")
    int size;                   // Size of the type (max 8 for UDTs)
    struct Fieldlist *fields;   // Pointer to the head of fields list
    struct Typetable *next;     // Pointer to the next type table entry
};

struct Fieldlist {
    char *name;                 // Name of the field
    int fieldIndex;             // Position of the field (0 to 7)
    struct Typetable *type;     // Pointer to type table entry of the field
    struct Fieldlist *next;     // Pointer to the next field
};

void TypeTableCreate();
void TInstall(char *name, int size, struct Fieldlist *fields);
struct Typetable* TLookup(char *name);
struct Fieldlist* FLookup(struct Typetable *type, char *name);
int GetSize(struct Typetable *type);
void PrintTypeTable();

#endif