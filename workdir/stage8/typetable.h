#ifndef TYPETABLE_H
#define TYPETABLE_H

struct Typetable {
    char *name;
    int size;
    struct Fieldlist *fields;
    struct Typetable *next;
};

struct Fieldlist {
    char *name;
    int fieldIndex;
    struct Typetable *type;
    struct Classtable *Ctype; // Stage 7: For class fields
    struct Fieldlist *next;
};

void TypeTableCreate();
void TInstall(char *name, int size, struct Fieldlist *fields);
struct Typetable* TLookup(char *name);
struct Fieldlist* FLookup(struct Typetable *type, char *name);
void PrintTypeTable();

#endif