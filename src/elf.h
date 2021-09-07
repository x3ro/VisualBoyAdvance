// -*- C++ -*-
// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef VBA_ELF_H
#define VBA_ELF_H

#include "System.h"
#include <stdio.h>

typedef enum LocationType {
  LOCATION_register,
  LOCATION_memory,
  LOCATION_value
} LocationType;

#define DW_ATE_boolean       0x02
#define DW_ATE_signed        0x05
#define DW_ATE_unsigned      0x07
#define DW_ATE_unsigned_char 0x08

typedef struct ELFHeader {
  u32 magic;
  u8 clazz;
  u8 data;
  u8 version;
  u8 pad[9];
  u16 e_type;
  u16 e_machine;
  u32 e_version;
  u32 e_entry;
  u32 e_phoff;
  u32 e_shoff;
  u32 e_flags;
  u16 e_ehsize;
  u16 e_phentsize;
  u16 e_phnum;
  u16 e_shentsize;
  u16 e_shnum;
  u16 e_shstrndx;
} ELFHeader;

typedef struct ELFProgramHeader {
  u32 type;
  u32 offset;
  u32 vaddr;
  u32 paddr;
  u32 filesz;
  u32 memsz;
  u32 flags;
  u32 align;
} ELFProgramHeader;

typedef struct ELFSectionHeader {
  u32 name;
  u32 type;
  u32 flags;
  u32 addr;
  u32 offset;
  u32 size;
  u32 link;
  u32 info;
  u32 addralign;
  u32 entsize;
} ELFSectionHeader;

typedef struct ELFSymbol {
  u32 name;
  u32 value;
  u32 size;
  u8 info;
  u8 other;
  u16 shndx;
} ELFSymbol;

typedef struct ELFBlock {
  int length;
  u8 *data;
} ELFBlock;

typedef struct ELFAttr {
  u32 name;
  u32 form;
  union {
    u32 value;
    char *string;
    u8 *data;
    bool flag;
    ELFBlock *block;
  };
} ELFAttr;

typedef struct ELFAbbrev {
  u32 number;
  u32 tag;
  bool hasChildren;
  int numAttrs;
  struct ELFAttr *attrs;
  struct ELFAbbrev *next;
} ELFAbbrev;

typedef enum TypeEnum {
  TYPE_base,
  TYPE_pointer,
  TYPE_function,
  TYPE_void,
  TYPE_array,
  TYPE_struct,
  TYPE_reference,
  TYPE_enum,
  TYPE_union
} TypeEnum;

struct Type;
struct Object;

typedef struct FunctionType {
  struct Type *returnType;
  struct Object *args;
} FunctionType;

typedef struct Member {
  char *name;
  struct Type *type;
  int bitSize;
  int bitOffset;
  int byteSize;
  ELFBlock *location;
} Member;

typedef struct Struct {
  int memberCount;
  Member *members;
} Struct;

typedef struct Array {
  struct Type *type;
  int maxBounds;
  int *bounds;
} Array;

typedef struct EnumMember {
  char *name;
  u32 value;
} EnumMember;

typedef struct Enum {
  int count;
  EnumMember *members;
} Enum;

typedef struct Type {
  u32 offset;
  TypeEnum type;
  char *name;
  int encoding;
  int size;
  int bitSize;
  union {
    struct Type *pointer;
    FunctionType *function;
    Array *array;
    Struct *structure;
    Enum *enumeration;
  };
  struct Type *next;
} Type;

typedef struct Object {
  char *name;
  int file;
  int line;
  bool external;
  Type *type;
  ELFBlock *location;
  u32 startScope;
  u32 endScope;
  struct Object *next;
} Object;

typedef struct Function {
  char *name;
  u32 lowPC;
  u32 highPC;
  int file;
  int line;
  bool external;
  Type *returnType;
  Object *parameters;
  Object *variables;
  ELFBlock *frameBase;
  struct Function *next;
} Function;

typedef struct LineInfoItem {
  u32 address;
  char *file;  
  int line;
} LineInfoItem;

typedef struct LineInfo {
  int fileCount;
  char **files;
  int number;
  LineInfoItem *lines;
} LineInfo;

typedef struct ARange {
  u32 lowPC;
  u32 highPC;
} ARange;

typedef struct ARanges {
  u32 offset;
  int count;
  ARange *ranges;
} ARanges;

typedef struct CompileUnit {
  u32 length;
  u8 *top;
  u32 offset;
  ELFAbbrev **abbrevs;
  ARanges *ranges;
  char *name;
  char *compdir;  
  u32 lowPC;
  u32 highPC;
  bool hasLineInfo;
  u32 lineInfo;
  LineInfo *lineInfoTable;
  Function *functions;
  Function *lastFunction;
  Object *variables;
  Type *types;
  struct CompileUnit *next;  
} CompileUnit;

typedef struct DebugInfo {
  u8 *debugfile;
  u8 *abbrevdata;
  u8 *debugdata;
  u8 *infodata;
  int numRanges;
  ARanges *ranges;
} DebugInfo;

typedef struct Symbol {
  char *name;
  int type;
  int binding;
  u32 address;
  u32 value;
  u32 size;
} Symbol;

extern u32 elfReadLEB128(u8 *, int *);
extern s32 elfReadSignedLEB128(u8 *, int *);
extern bool elfRead(const char *, int*, FILE *f);
extern bool elfGetSymbolAddress(char *,u32 *, u32 *, int *);
extern char *elfGetAddressSymbol(u32);
extern char *elfGetSymbol(int, u32 *, u32 *, int *);
extern void elfCleanUp(void);
extern bool elfGetCurrentFunction(u32, Function **, CompileUnit **c);
extern bool elfGetObject(char *, Function *, CompileUnit *, Object **);
extern bool elfFindLineInUnit(u32 *, CompileUnit *, int);
extern bool elfFindLineInModule(u32 *, char *, int);
u32 elfDecodeLocation(Function *, ELFBlock *, LocationType *);
u32 elfDecodeLocation4(Function *, ELFBlock *, LocationType *, u32);
int elfFindLine(CompileUnit *unit, Function *func, u32 addr, char **);
#endif
