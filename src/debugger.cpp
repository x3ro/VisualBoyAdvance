/*
 * VisualBoyAdvanced - Nintendo Gameboy/GameboyAdvance (TM) emulator
 * Copyrigh(c) 1999-2002 Forgotten (vb@emuhq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GBA.h"
#include "Port.h"
#include "armdis.h"
#include "elf.h"
#include "exprNode.h"

extern bool debugger;
extern int emulating;

extern void (*emuUpdateCPSR)();
extern void (*emuMain)(int);

#define debuggerReadMemory(addr) \
  READ32LE((&map[(addr)>>24].address[(addr) & map[(addr)>>24].mask]))

#define debuggerReadHalfWord(addr) \
  READ16LE((&map[(addr)>>24].address[(addr) & map[(addr)>>24].mask]))

#define debuggerReadByte(addr) \
  map[(addr)>>24].address[(addr) & map[(addr)>>24].mask]

#define debuggerWriteMemory(addr, value) \
  WRITE32LE(&map[(addr)>>24].address[(addr) & map[(addr)>>24].mask], value)

#define debuggerWriteHalfWord(addr, value) \
  WRITE16LE(&map[(addr)>>24].address[(addr) & map[(addr)>>24].mask], value)

#define debuggerWriteByte(addr, value) \
  map[(addr)>>24].address[(addr) & map[(addr)>>24].mask] = (value)

struct breakpointInfo {
  u32 address;
  u32 value;
  int size;
};

struct DebuggerCommand {
  char *name;
  void (*function)(int,char **);
  char *help;
  char *syntax;
};

void debuggerContinueAfterBreakpoint();

void debuggerHelp(int,char **);
void debuggerNext(int,char **);
void debuggerContinue(int, char **);
void debuggerRegisters(int, char **);
void debuggerBreak(int, char **);
void debuggerBreakDelete(int, char **);
void debuggerBreakList(int, char **);
void debuggerBreakArm(int, char **);
void debuggerBreakWriteClear(int, char **);
void debuggerBreakThumb(int, char **);
void debuggerBreakWrite(int, char **);
void debuggerDebug(int, char **);
void debuggerDisassemble(int, char **);
void debuggerDisassembleArm(int, char **);
void debuggerDisassembleThumb(int, char **);
void debuggerEditByte(int, char **);
void debuggerEditHalfWord(int, char **);
void debuggerEdit(int, char **);
void debuggerIo(int, char **);
void debuggerLocals(int, char **);
void debuggerMemoryByte(int, char **);
void debuggerMemoryHalfWord(int, char **);
void debuggerMemory(int, char **);
void debuggerPrint(int, char **);
void debuggerQuit(int, char **);
void debuggerSetRadix(int, char **);
void debuggerSymbols(int, char **);
void debuggerVerbose(int, char **);
void debuggerWhere(int, char **);

DebuggerCommand debuggerCommands[] = {
  { "?", debuggerHelp,        "Shows this help information. Type ? <command> for command help", "[<command>]" },
  { "ba", debuggerBreakArm,   "Adds an ARM breakpoint", "<address>" },
  { "bd", debuggerBreakDelete,"Deletes a breakpoint", "<number>" },
  { "bl", debuggerBreakList,  "Lists breakpoints" },
  { "bpw", debuggerBreakWrite, "Break on write", "<address> <size>" },
  { "bpwc", debuggerBreakWriteClear, "Clear break on write", NULL },
  { "break", debuggerBreak,    "Adds a breakpoint on the given function", "<function>|<line>|<file:line>" },
  { "bt", debuggerBreakThumb, "Adds a THUMB breakpoint", "<address>" },
  { "c", debuggerContinue,    "Continues execution" , NULL },
  { "d", debuggerDisassemble, "Disassembles instructions", "[<address> [<number>]]" },
  { "da", debuggerDisassembleArm, "Disassembles ARM instructions", "[<address> [<number>]]" },
  { "dt", debuggerDisassembleThumb, "Disassembles THUMB instructions", "[<address> [<number>]]" },
  { "eb", debuggerEditByte,   "Modify memory location (byte)", "<address> <hex value>" },
  { "eh", debuggerEditHalfWord,"Modify memory location (half-word)","<address> <hex value>" },
  { "ew", debuggerEdit,       "Modify memory location (word)", "<address> <hex value" },
  { "h", debuggerHelp,        "Shows this help information. Type h <command> for command help", "[<command>]" },
  { "io", debuggerIo,         "Show I/O registers status", "[video|video2|dma|timer|misc]" },
  { "locals", debuggerLocals, "Shows local variables", NULL },
  { "mb", debuggerMemoryByte, "Shows memory contents (bytes)", "<address>" },
  { "mh", debuggerMemoryHalfWord, "Shows memory contents (half-words)", "<address>"},
  { "mw", debuggerMemory,     "Shows memory contents (words)", "<address>" },
  { "n", debuggerNext,        "Executes the next instruction", "[<count>]" },
  { "print", debuggerPrint,   "Print the value of a expression (if known)", "[/x|/o|/d] <expression>" },
  { "q", debuggerQuit,        "Quits the emulator", NULL },
  { "r", debuggerRegisters,   "Shows ARM registers", NULL },
  { "radix", debuggerSetRadix,   "Sets the print radix", "<radix>" },
  { "symbols", debuggerSymbols, "List symbols", "[<symbol>]" },
#ifndef FINAL_VERSION
  { "trace", debuggerDebug,       "Sets the trace level", "<value>" },
#endif
#ifdef DEV_VERSION
  { "verbose", debuggerVerbose,     "Change verbose setting", "<value>" },
#endif
  { "where", debuggerWhere,   "Shows call chain", NULL },
  { NULL, NULL, NULL, NULL} // end marker
};

breakpointInfo debuggerBreakpointList[100];

int debuggerNumOfBreakpoints = 0;
bool debuggerAtBreakpoint = false;
int debuggerBreakpointNumber = 0;
int debuggerRadix = 0;

void debuggerApplyBreakpoint(u32 address, int num, int size)
{
  if(size)
    debuggerWriteMemory(address, (u32)(0xe1200070 | 
                                       (num & 0xf) | 
                                       ((num<<4)&0xf0)));
  else
    debuggerWriteHalfWord(address, 
                          (u16)(0xbe00 | num));
}

void debuggerDisableBreakpoints()
{
  for(int i = 0; i < debuggerNumOfBreakpoints; i++) {
    if(debuggerBreakpointList[i].size)
      debuggerWriteMemory(debuggerBreakpointList[i].address,
                          debuggerBreakpointList[i].value);
    else
      debuggerWriteHalfWord(debuggerBreakpointList[i].address,
                            debuggerBreakpointList[i].value);      
  }
}

void debuggerEnableBreakpoints(bool skipPC)
{
  for(int i = 0; i < debuggerNumOfBreakpoints; i++) {
    if(debuggerBreakpointList[i].address == armNextPC && skipPC)
      continue;

    debuggerApplyBreakpoint(debuggerBreakpointList[i].address,
                            i,
                            debuggerBreakpointList[i].size);
  }  
}

void debuggerUsage(char *cmd)
{
  for(int i = 0; ; i++) {
    if(debuggerCommands[i].name) {
      if(!strcmp(debuggerCommands[i].name, cmd)) {
        printf("%s %s\t%s\n", 
               debuggerCommands[i].name, 
               debuggerCommands[i].syntax ? debuggerCommands[i].syntax : "",
               debuggerCommands[i].help);
        break;
      }
    } else {
      printf("Unrecognized command '%s'.", cmd);
      break;
    }
  }  
}

void debuggerPrintBaseType(Type *t, u32 value, u32 location,
                           LocationType type,
                           int bitSize, int bitOffset)
{
  if(bitSize) {
    if(bitOffset)
      value >>= ((t->size*8)-bitOffset-bitSize);
    value &= (1 << bitSize)-1;
  } else {
    if(t->size == 2)
      value &= 0xFFFF;
    else if(t->size == 1)
      value &= 0xFF;
  }

  if(t->size == 8) {
    u64 value = 0;
    if(type == LOCATION_memory) {
      value = debuggerReadMemory(location) |
        ((u64)debuggerReadMemory(location+4)<<32);
    } else if(type == LOCATION_register) {
      value = reg[location].I | ((u64)reg[location+1].I << 32);
    }
    switch(t->encoding) {
    case DW_ATE_signed:
      switch(debuggerRadix) {
      case 0:
        printf("%lld", value);
        break;
      case 1:
        printf("0x%llx", value);
        break;
      case 2:
        printf("0%llo", value);
        break;
      }
      break;
    case DW_ATE_unsigned:
      switch(debuggerRadix) {
      case 0:
        printf("%llu", value);
        break;
      case 1:
        printf("0x%llx", value);
        break;
      case 2:
        printf("0%llo", value);
        break;
      }
      break;
    default:
      printf("Unknowing 64-bit encoding\n");
    }
    return;
  }
  
  switch(t->encoding) {
  case DW_ATE_boolean:
    if(value)
      printf("true");
    else
      printf("false");
    break;
  case DW_ATE_signed:
    switch(debuggerRadix) {
    case 0:
      printf("%d", value);
      break;
    case 1:
      printf("0x%x", value);
      break;
    case 2:
      printf("0%o", value);
      break;
    }
    break;
  case DW_ATE_unsigned:
  case DW_ATE_unsigned_char:
    switch(debuggerRadix) {
    case 0:
      printf("%u", value);
      break;
    case 1:
      printf("0x%x", value);
      break;
    case 2:
      printf("0%o", value);
      break;
    }
    break;
  default:
    printf("UNKNOWN BASE %d %08x", t->encoding, value);
  }
}

char *debuggerPrintType(Type *t)
{
  char buffer[1024];  
  static char buffer2[1024];
  
  if(t->type == TYPE_pointer) {
    if(t->pointer)
      strcpy(buffer, debuggerPrintType(t->pointer));
    else
      strcpy(buffer, "void");
    sprintf(buffer2, "%s *", buffer);
    return buffer2;
  } else if(t->type == TYPE_reference) {
    strcpy(buffer, debuggerPrintType(t->pointer));
    sprintf(buffer2, "%s &", buffer);
    return buffer2;    
  }
  return t->name;
}

void debuggerPrintValueInternal(Function *, Type *, ELFBlock *, int, int, u32);
void debuggerPrintValueInternal(Function *f, Type *t,
                                int bitSize, int bitOffset,
                                u32 objLocation, LocationType type);

u32 debuggerGetValue(u32 location, LocationType type)
{
  switch(type) {
  case LOCATION_memory:
    return debuggerReadMemory(location);
  case LOCATION_register:
    return reg[location].I;
  case LOCATION_value:
    return location;
  }
  return 0;
}

void debuggerPrintPointer(Type *t, u32 value)
{
  printf("(%s)0x%08x", debuggerPrintType(t), value);
}

void debuggerPrintReference(Type *t, u32 value)
{
  printf("(%s)0x%08x", debuggerPrintType(t), value);
}

void debuggerPrintFunction(Type *t, u32 value)
{
  printf("(%s)0x%08x", debuggerPrintType(t), value);
}

void debuggerPrintArray(Type *t, u32 value)
{
  // todo
  printf("(%s[])0x%08x", debuggerPrintType(t->array->type), value);
}

void debuggerPrintMember(Function *f,
                         Member *m,
                         u32 objLocation,
                         u32 location)
{
  int bitSize = m->bitSize;
  if(bitSize) {
    u32 value = 0;
    int off = m->bitOffset;
    int size = m->byteSize;
    u32 v = 0;
    if(size == 1)
      v = debuggerReadByte(location);
      else if(size == 2)
        v = debuggerReadHalfWord(location);
      else if(size == 4)
        v = debuggerReadMemory(location);
      
      while(bitSize) {
        int top = size*8 - off;
        int bot = top - bitSize;
        top--;
        if(bot >= 0) {
          value = (v >> (size*8 - bitSize - off)) & ((1 << bitSize)-1);
          bitSize = 0;
        } else {
          value |= (v & ((1 << top)-1)) << (bitSize - top);
          bitSize -= (top+1);
          location -= size;
          off = 0;
          if(size == 1)
            v = debuggerReadByte(location);
          else if(size == 2)
            v = debuggerReadHalfWord(location);
          else
            v = debuggerReadMemory(location);
        }
      }
      debuggerPrintBaseType(m->type, value, location, LOCATION_memory,
                            bitSize, 0);
    } else {
      debuggerPrintValueInternal(f, m->type, m->location, m->bitSize,
                                 m->bitOffset, objLocation);
    }  
}

void debuggerPrintStructure(Function *f, Type *t, u32 objLocation)
{
  printf("{");
  int count = t->structure->memberCount;
  int i = 0;
  while(i < count) {
    Member *m = &t->structure->members[i];
    printf("%s=", m->name);
    LocationType type;
    u32 location = elfDecodeLocation(f, m->location, &type, objLocation);
    debuggerPrintMember(f, m, objLocation, location);
    i++;
    if(i < count)
      printf(",");
  }
  printf("}");
}

void debuggerPrintUnion(Function *f, Type *t, u32 objLocation)
{
  // todo
  printf("{");
  int count = t->structure->memberCount;
  int i = 0;
  while(i < count) {
    Member *m = &t->structure->members[i];
    printf("%s=", m->name);
    debuggerPrintMember(f, m, objLocation, 0);
    i++;
    if(i < count)
      printf(",");
  }
  printf("}");
}

void debuggerPrintEnum(Type *t, u32 value)
{
  int i;
  for(i = 0; i < t->enumeration->count; i++) {
    EnumMember *m = (EnumMember *)&t->enumeration->members[i];
    if(value == m->value) {
      printf(m->name);
      return;
    }
  }
  printf("(UNKNOWN VALUE) %d", value);
}

void debuggerPrintValueInternal(Function *f, Type *t,
                                int bitSize, int bitOffset,
                                u32 objLocation, LocationType type)
{
  u32 value = debuggerGetValue(objLocation, type);
  if(!t) {
    printf("void");
    return;
  }
  switch(t->type) {
  case TYPE_base:
    debuggerPrintBaseType(t, value, objLocation, type, bitSize, bitOffset);
    break;
  case TYPE_pointer:
    debuggerPrintPointer(t, value);
    break;
  case TYPE_reference:
    debuggerPrintReference(t, value);
    break;
  case TYPE_function:
    debuggerPrintFunction(t, value);
    break;
  case TYPE_array:
    debuggerPrintArray(t, objLocation);
    break;
  case TYPE_struct:
    debuggerPrintStructure(f, t, objLocation);
    break;
  case TYPE_union:
    debuggerPrintUnion(f, t, objLocation);
    break;
  case TYPE_enum:
    debuggerPrintEnum(t, value);
    break;
  default:
    printf("%08x", value);
    break;
  }  
}

void debuggerPrintValueInternal(Function *f, Type *t, ELFBlock *loc,
                                int bitSize, int bitOffset, u32 objLocation)
{
  LocationType type;  
  u32 location;
  if(loc) {
    if(objLocation)
      location = elfDecodeLocation(f, loc, &type, objLocation);
    else
      location = elfDecodeLocation(f, loc,&type);
  } else {
    location = objLocation;
    type = LOCATION_memory;
  }

  debuggerPrintValueInternal(f, t, bitSize, bitOffset, location, type);
}

void debuggerPrintValue(Function *f, Object *o)
{
  debuggerPrintValueInternal(f, o->type, o->location, 0, 0, 0);
  
  printf("\n");
}

void debuggerSymbols(int argc, char **argv)
{
  int i = 0;
  u32 value;
  u32 size;
  int type;
  bool match = false;
  int matchSize = 0;
  char *matchStr = NULL;
  
  if(argc == 2) {
    match = true;
    matchSize = strlen(argv[1]);
    matchStr = argv[1];
  }
  printf("Symbol               Value    Size     Type   \n");
  printf("-------------------- -------  -------- -------\n");
  char *s = NULL;
  while((s = elfGetSymbol(i, &value, &size, &type))) {
    if(*s) {
      if(match) {
        if(strncmp(s, matchStr, matchSize) != 0) {
          i++;
          continue;
        }
      }
      char *ts = "?";
      switch(type) {
      case 2:
        ts = "ARM";
        break;
      case 0x0d:
        ts = "THUMB";
        break;
      case 1:
        ts = "DATA";
        break;
      }
      printf("%-20s %08x %08x %-7s\n",
             s, value, size, ts);
    }
    i++;
  }
}

void debuggerSetRadix(int argc, char **argv)
{
  if(argc != 2)
    debuggerUsage(argv[0]);
  else {
    int r = atoi(argv[1]);

    bool error = false;
    switch(r) {
    case 10:
      debuggerRadix = 0;
      break;
    case 8:
      debuggerRadix = 2;
      break;
    case 16:
      debuggerRadix = 1;
      break;
    default:
      error = true;
      printf("Unknown radix %d. Valid values are 8, 10 and 16.\n", r);
      break;
    }
    if(!error)
      printf("Radix set to %d\n", r);
  }
}

void debuggerPrint(int argc, char **argv)
{
  if(argc != 2 && argc != 3) {
    debuggerUsage(argv[0]);
  } else {
    u32 pc = armNextPC;
    Function *f = NULL;
    CompileUnit *u = NULL;
    
    elfGetCurrentFunction(pc,
                          &f, &u);

    int oldRadix = debuggerRadix;
    if(argc == 3) {
      if(argv[1][0] == '/') {
        if(argv[1][1] == 'x')
          debuggerRadix = 1;
        else if(argv[1][1] == 'o')
          debuggerRadix = 2;
        else if(argv[1][1] == 'd')
          debuggerRadix = 0;
        else {
          printf("Unknown format %c\n", argv[1][1]);
          return;
        }
      } else {
        printf("Unknown option %s\n", argv[1]);
        return;
      }
    } 
    
    char *s = argc == 2 ? argv[1] : argv[2];

    extern char *exprString;
    extern int exprCol;
    extern int yyparse();
    exprString = s;
    exprCol = 0;
    if(!yyparse()) {
      extern Node *result;
      if(result->resolve(result, f, u)) {
        if(result->member)
          debuggerPrintMember(f,
                              result->member,
                              result->objLocation,
                              result->location);
        else
          debuggerPrintValueInternal(f, result->type, 0, 0,
                                     result->location,
                                     result->locType);
        printf("\n");
      } else {
        printf("Error resolving expression\n");
      }
    } else {
      printf("Error parsing expression:\n");
      printf("%s\n", s);
      exprCol--;
      for(int i = 0; i < exprCol; i++)
        printf(" ");
      printf("^\n");
    }
    extern void exprCleanBuffer();
    exprCleanBuffer();
    exprNodeCleanUp();
    debuggerRadix = oldRadix;
  }
}

void debuggerHelp(int n, char **args)
{
  if(n == 2) {
    debuggerUsage(args[1]);
  } else {
    for(int i = 0; ; i++) {
      if(debuggerCommands[i].name) {
        printf("%s\t%s\n", debuggerCommands[i].name, debuggerCommands[i].help);
      } else
        break;
    }
  }
}

void debuggerDebug(int n, char **args)
{
  if(n == 2) {
    int v = 0;
    sscanf(args[1], "%d", &v);
    systemDebug = v;
    printf("Debug level set to %d\n", systemDebug);
  } else
    debuggerUsage("trace");      
}

void debuggerVerbose(int n, char **args)
{
  if(n == 2) {
    int v = 0;
    sscanf(args[1], "%d", &v);
    systemVerbose = v;
    printf("Verbose level set to %d\n", systemVerbose);
  } else
    debuggerUsage("verbose");    
}

void debuggerWhere(int n, char **args)
{
  void elfPrintCallChain(u32);
  elfPrintCallChain(armNextPC);
}

void debuggerLocals(int n, char **args)
{
  Function *f = NULL;
  CompileUnit *u = NULL;
  u32 pc = armNextPC;
  if(elfGetCurrentFunction(pc,
                           &f, &u)) {
    Object *o = f->parameters;
    while(o) {
      printf("%s=", o->name);
      debuggerPrintValue(f, o);
      o = o->next;
    }

    o = f->variables;
    while(o) {
      bool visible = o->startScope ? pc>=o->startScope : true;
      if(visible)
        visible = o->endScope ? pc < o->endScope : true;
      if(visible) {
        printf("%s=", o->name);
        debuggerPrintValue(f, o);
      }
      o = o->next;      
    }
  } else {
    printf("No information for current address\n");
  }  
}

void debuggerNext(int n, char **args)
{
  int count = 1;
  if(n == 2) {
    sscanf(args[1], "%d", &count);
  }
  for(int i = 0; i < count; i++) {
    if(debuggerAtBreakpoint) {
      debuggerContinueAfterBreakpoint();
      debuggerEnableBreakpoints(false);
    } else 
      emuMain(1);
  }
  debuggerDisableBreakpoints();
  Function *f = NULL;
  CompileUnit *u = NULL;
  u32 a = armNextPC;
  if(elfGetCurrentFunction(a, &f, &u)) {
    char *file;
    int line = elfFindLine(u, f, a, &file);
    
    printf("File %s, function %s, line %d\n", file, f->name,
           line);
  }
  debuggerRegisters(0, NULL);
}

void debuggerContinue(int n, char **args)
{
  if(debuggerAtBreakpoint)
    debuggerContinueAfterBreakpoint();
  debuggerEnableBreakpoints(false);
  debugger = false;
}

void debuggerSignal(int sig,int number)
{
  switch(sig) {
  case 4:
    {
      printf("Illegal instruction at %08x\n", armNextPC);
      debugger = true;
    }
    break;
  case 5:
    {
      printf("Breakpoint %d reached\n", number);
      debugger = true;
      debuggerAtBreakpoint = true;
      debuggerBreakpointNumber = number;
      debuggerDisableBreakpoints();
      
      Function *f = NULL;
      CompileUnit *u = NULL;
      
      if(elfGetCurrentFunction(armNextPC, &f, &u)) {
        char *file;
        int line = elfFindLine(u,f,armNextPC,&file);
        printf("File %s, function %s, line %d\n", file, f->name,
               line);
      }
    }
    break;
  default:
    printf("Unknown signal %d\n", sig);
    break;
  }
}

void debuggerBreakList(int, char **)
{
  printf("Num Address  Type  Symbol\n");
  printf("--- -------- ----- ------\n");
  for(int i = 0; i < debuggerNumOfBreakpoints; i++) {
    printf("%3d %08x %s %s\n",i, debuggerBreakpointList[i].address,
           debuggerBreakpointList[i].size ? "ARM" : "THUMB",
           elfGetAddressSymbol(debuggerBreakpointList[i].address));
  }
}

void debuggerBreakDelete(int n, char **args)
{
  if(n == 2) {
    int n = 0;
    sscanf(args[1], "%d", &n);
    printf("Deleting breakpoint %d (%d)\n", n, debuggerNumOfBreakpoints);
    if(n >= 0 && n < debuggerNumOfBreakpoints) {
      n++;
      if(n < debuggerNumOfBreakpoints) {
        for(int i = n; i < debuggerNumOfBreakpoints; i++) {
          debuggerBreakpointList[i-1].address = 
            debuggerBreakpointList[i].address;
          debuggerBreakpointList[i-1].value = 
            debuggerBreakpointList[i].value;
          debuggerBreakpointList[i-1].size = 
            debuggerBreakpointList[i].size;
        }
      }
      debuggerNumOfBreakpoints--;
    }
  } else
    debuggerUsage("bd");    
}

void debuggerBreak(int n, char **args)
{
  if(n == 2) {
    u32 address = 0;
    u32 value = 0;
    int type = 0;
    char *s = args[1];
    char c = *s;
    if(strchr(s, ':')) {
      char *name = s;
      char *l = strchr(s, ':');
      *l++ = 0;
      int line = atoi(l);

      u32 addr;
      Function *f;
      CompileUnit *u;
      
      if(elfFindLineInModule(&addr, name, line)) {
        if(elfGetCurrentFunction(addr, &f, &u)) {
          u32 addr2;
          if(elfGetSymbolAddress(f->name, &addr2, &value, &type)) {
            address = addr;
          } else {
            printf("Unable to get function symbol data\n");
            return;
          }
        } else {
          printf("Unable to find function for address\n");
          return;
        }
      } else {
        printf("Unable to find module or line\n");
        return;
      }
    } else if(c >= '0' && c <= '9') {
      int line = atoi(s);
      Function *f;
      CompileUnit *u;
      u32 addr;
      
      if(elfGetCurrentFunction(armNextPC, &f, &u)) {
        if(elfFindLineInUnit(&addr, u, line)) {
          if(elfGetCurrentFunction(addr, &f, &u)) {
            u32 addr2;
            if(elfGetSymbolAddress(f->name, &addr2, &value, &type)) {
              address = addr;
            } else {
              printf("Unable to get function symbol data\n");
              return;
            }
          } else {
            printf("Unable to find function for address\n");
            return;
          }
        } else {
          printf("Unable to find line\n");
          return;
        }
      } else {
        printf("Cannot find current function\n");
        return;
      }
    } else {
      if(!elfGetSymbolAddress(s, &address, &value, &type)) {
        printf("Function %s not found\n", args[1]);
        return;
      }
    }
    if(type == 0x02 || type == 0x0d) {
      int i = debuggerNumOfBreakpoints;
      int size = 0;
      if(type == 2)
        size = 1;
      debuggerBreakpointList[i].address = address;
      debuggerBreakpointList[i].value = type == 0x02 ?
        debuggerReadMemory(address) : debuggerReadHalfWord(address);
      debuggerBreakpointList[i].size = size;
      //      debuggerApplyBreakpoint(address, i, size);
      debuggerNumOfBreakpoints++;
      if(size)
        printf("Added ARM breakpoint at %08x\n", address);        
      else
        printf("Added THUMB breakpoint at %08x\n", address);
    } else {
      printf("%s is not a function symbol\n", args[1]); 
    }
  } else
    debuggerUsage("break");  
}

void debuggerBreakThumb(int n, char **args)
{
  if(n == 2) {
    u32 address = 0;
    sscanf(args[1],"%x", &address);
    int i = debuggerNumOfBreakpoints;
    debuggerBreakpointList[i].address = address;
    debuggerBreakpointList[i].value = debuggerReadHalfWord(address);
    debuggerBreakpointList[i].size = 0;
    //    debuggerApplyBreakpoint(address, i, 0);
    debuggerNumOfBreakpoints++;
    printf("Added THUMB breakpoint at %08x\n", address);
  } else
    debuggerUsage("bt");    
}

void debuggerBreakArm(int n, char **args)
{
  if(n == 2) {
    u32 address = 0;
    sscanf(args[1],"%x", &address);
    int i = debuggerNumOfBreakpoints;
    debuggerBreakpointList[i].address = address;
    debuggerBreakpointList[i].value = debuggerReadMemory(address);
    debuggerBreakpointList[i].size = 1;
    //    debuggerApplyBreakpoint(address, i, 1);
    debuggerNumOfBreakpoints++;
    printf("Added ARM breakpoint at %08x\n", address);
  } else
    debuggerUsage("ba");
}

void debuggerBreakOnWrite(u32 *mem, u32 oldvalue, u32 value, int size)
{
  u32 address = 0;
  if(mem >= (u32*)&workRAM[0] && mem <= (u32*)&workRAM[0x3ffff])
    address = 0x2000000 + ((u32)mem - (u32)&workRAM[0]);
  else
    address = 0x3000000 + ((u32)mem - (u32)&internalRAM[0]);

  if(size == 2)
    printf("Breakpoint (on write) address %08x old:%08x new:%08x\n", 
           address, oldvalue, value);
  else if(size == 1)
    printf("Breakpoint (on write) address %08x old:%04x new:%04x\n", 
           address, (u16)oldvalue,(u16)value);
  else
    printf("Breakpoint (on write) address %08x old:%02x new:%02x\n", 
           address, (u8)oldvalue, (u8)value);
  debugger = true;
}

void debuggerBreakWriteClear(int n, char **args)
{
  memset(freezeWorkRAM, false, 0x40000);
  memset(freezeInternalRAM, false, 0x8000);
  printf("Cleared all break on write\n");
}

void debuggerBreakWrite(int n, char **args)
{
  if(n == 3) {
    if(cheatsNumber != 0) {
      printf("Cheats are enabled. Cannot continue.\n");
      return;
    }
    u32 address = 0;
    sscanf(args[1], "%x", &address);
    int n = 0;
    sscanf(args[2], "%d", &n);
    
    if(address < 0x2000000 || address > 0x3007fff) {
      printf("Invalid address: %08x\n", address);
      return;
    }
    
    if(address > 0x203ffff && address < 0x3000000) {
      printf("Invalid address: %08x\n", address);
      return;
    }

    u32 final = address + n;

    if(address < 0x2040000 && final > 0x2040000) {
      printf("Invalid byte count: %d\n", n);
      return;
    } else if(address < 0x3008000 && final > 0x3008000) {
      printf("Invalid byte count: %d\n", n);
      return;
    }
    printf("Added break on write at %08x for %d bytes\n", address, n);
    for(int i = 0; i < n; i++) {
      if((address >> 24) == 2)
        freezeWorkRAM[address & 0x3ffff] = true;
      else
        freezeInternalRAM[address & 0x7fff] = true;
      address++;
    }
  } else
    debuggerUsage("bpw");    
}

void debuggerDisassembleArm(int n, char **args)
{
  char buffer[80];
  u32 pc = reg[15].I;
  pc -= 4;
  int count = 20;
  if(n >= 2) {
    sscanf(args[1], "%x", &pc);
  }
  if(pc & 3) {
    printf("Misaligned address %08x\n", pc);
    pc &= 0xfffffffc;
  }
  if(n >= 3) {
    sscanf(args[2], "%d", &count);
  }
  int i = 0;
  int len = 0;
  char format[30];
  for(i = 0; i < count; i++) {
    int l = strlen(elfGetAddressSymbol(pc+4*i));
    if(l > len)
      len = l;
  }
  sprintf(format, "%%08x %%-%ds %%s\n", len);
  for(i = 0; i < count; i++) {
    u32 addr = pc;
    pc += disArm(pc, buffer, 2);
    printf(format, addr, elfGetAddressSymbol(addr), buffer);
  }
}

void debuggerDisassembleThumb(int n, char **args)
{
  char buffer[80];
  u32 pc = reg[15].I;
  pc -= 2;
  int count = 20;
  if(n >= 2) {
    sscanf(args[1], "%x", &pc);
  }
  if(pc & 1) {
    printf("Misaligned address %08x\n", pc);
    pc &= 0xfffffffe;
  }
  if(n >= 3) {
    sscanf(args[2], "%d", &count);
  }

  int i = 0;
  int len = 0;
  char format[30];
  for(i = 0; i < count; i++) {
    int l = strlen(elfGetAddressSymbol(pc+2*i));
    if(l > len)
      len = l;
  }
  sprintf(format, "%%08x %%-%ds %%s\n", len);  
  
  for(i = 0; i < count; i++) {
    u32 addr = pc;
    pc += disThumb(pc, buffer, 2);
    printf(format, addr, elfGetAddressSymbol(addr), buffer);
  }
}

void debuggerDisassemble(int n, char **args)
{
  if(armState)
    debuggerDisassembleArm(n, args);
  else
    debuggerDisassembleThumb(n, args);
}

void debuggerContinueAfterBreakpoint()
{
  printf("Continuing after breakpoint\n");
  debuggerEnableBreakpoints(true);
  emuMain(1);
  debuggerAtBreakpoint = false;
}

void debuggerRegisters(int, char **)
{
  char *command[3];
  char buffer[10];

  printf("R00=%08x R04=%08x R08=%08x R12=%08x\n",
         reg[0].I, reg[4].I, reg[8].I, reg[12].I);
  printf("R01=%08x R05=%08x R09=%08x R13=%08x\n",
         reg[1].I, reg[5].I, reg[9].I, reg[13].I);
  printf("R02=%08x R06=%08x R10=%08x R14=%08x\n",
         reg[2].I, reg[6].I, reg[10].I, reg[14].I);
  printf("R03=%08x R07=%08x R11=%08x R15=%08x\n",
         reg[3].I, reg[7].I, reg[11].I, reg[15].I);
  printf("CPSR=%08x (%c%c%c%c%c%c%c Mode: %02x)\n",
         reg[16].I,
         (N_FLAG ? 'N' : '.'),
         (Z_FLAG ? 'Z' : '.'),
         (C_FLAG ? 'C' : '.'),
         (V_FLAG ? 'V' : '.'),
         (armIrqEnable ? '.' : 'I'),
         ((!(reg[16].I & 0x40)) ? '.' : 'F'),
         (armState ? '.' : 'T'),
         armMode);
  sprintf(buffer,"%08x", armState ? reg[15].I - 4 : reg[15].I - 2);
  command[0]="m";
  command[1]=buffer;
  command[2]="1";
  debuggerDisassemble(3, command);
}

void debuggerIoVideo()
{
  printf("DISPCNT  = %04x\n", DISPCNT);
  printf("DISPSTAT = %04x\n", DISPSTAT);
  printf("VCOUNT   = %04x\n", VCOUNT);
  printf("BG0CNT   = %04x\n", BG0CNT);
  printf("BG1CNT   = %04x\n", BG1CNT);
  printf("BG2CNT   = %04x\n", BG2CNT);
  printf("BG3CNT   = %04x\n", BG3CNT);
  printf("WIN0H    = %04x\n", WIN0H);
  printf("WIN0V    = %04x\n", WIN0V);
  printf("WIN1H    = %04x\n", WIN1H);
  printf("WIN1V    = %04x\n", WIN1V);
  printf("WININ    = %04x\n", WININ);
  printf("WINOUT   = %04x\n", WINOUT);
  printf("MOSAIC   = %04x\n", MOSAIC);
  printf("BLDMOD   = %04x\n", BLDMOD);
  printf("COLEV    = %04x\n", COLEV);
  printf("COLY     = %04x\n", COLY);
}

void debuggerIoVideo2()
{
  printf("BG0HOFS  = %04x\n", BG0HOFS);
  printf("BG0VOFS  = %04x\n", BG0VOFS);
  printf("BG1HOFS  = %04x\n", BG1HOFS);
  printf("BG1VOFS  = %04x\n", BG1VOFS);
  printf("BG2HOFS  = %04x\n", BG2HOFS);
  printf("BG2VOFS  = %04x\n", BG2VOFS);
  printf("BG3HOFS  = %04x\n", BG3HOFS);
  printf("BG3VOFS  = %04x\n", BG3VOFS);
  printf("BG2PA    = %04x\n", BG2PA);
  printf("BG2PB    = %04x\n", BG2PB);
  printf("BG2PC    = %04x\n", BG2PC);
  printf("BG2PD    = %04x\n", BG2PD);
  printf("BG2X     = %08x\n", (BG2X_H<<16)|BG2X_L);
  printf("BG2Y     = %08x\n", (BG2Y_H<<16)|BG2Y_L);
  printf("BG3PA    = %04x\n", BG3PA);
  printf("BG3PB    = %04x\n", BG3PB);
  printf("BG3PC    = %04x\n", BG3PC);
  printf("BG3PD    = %04x\n", BG3PD);
  printf("BG3X     = %08x\n", (BG3X_H<<16)|BG3X_L);
  printf("BG3Y     = %08x\n", (BG3Y_H<<16)|BG3Y_L);
}

void debuggerIoDMA()
{
  printf("DM0SAD   = %08x\n", (DM0SAD_H<<16)|DM0SAD_L);
  printf("DM0DAD   = %08x\n", (DM0DAD_H<<16)|DM0DAD_L);
  printf("DM0CNT   = %08x\n", (DM0CNT_H<<16)|DM0CNT_L);  
  printf("DM1SAD   = %08x\n", (DM1SAD_H<<16)|DM1SAD_L);
  printf("DM1DAD   = %08x\n", (DM1DAD_H<<16)|DM1DAD_L);
  printf("DM1CNT   = %08x\n", (DM1CNT_H<<16)|DM1CNT_L);  
  printf("DM2SAD   = %08x\n", (DM2SAD_H<<16)|DM2SAD_L);
  printf("DM2DAD   = %08x\n", (DM2DAD_H<<16)|DM2DAD_L);
  printf("DM2CNT   = %08x\n", (DM2CNT_H<<16)|DM2CNT_L);  
  printf("DM3SAD   = %08x\n", (DM3SAD_H<<16)|DM3SAD_L);
  printf("DM3DAD   = %08x\n", (DM3DAD_H<<16)|DM3DAD_L);
  printf("DM3CNT   = %08x\n", (DM3CNT_H<<16)|DM3CNT_L);    
}

void debuggerIoTimer()
{
  printf("TM0D     = %04x\n", TM0D);
  printf("TM0CNT   = %04x\n", TM0CNT);
  printf("TM1D     = %04x\n", TM1D);
  printf("TM1CNT   = %04x\n", TM1CNT);
  printf("TM2D     = %04x\n", TM2D);
  printf("TM2CNT   = %04x\n", TM2CNT);
  printf("TM3D     = %04x\n", TM3D);
  printf("TM3CNT   = %04x\n", TM3CNT);
}

void debuggerIoMisc()
{
  printf("P1       = %04x\n", P1);  
  printf("IE       = %04x\n", IE);
  printf("IF       = %04x\n", IF);
  printf("IME      = %04x\n", IME);
}

void debuggerIo(int n, char **args)
{
  if(n == 1) {
    debuggerIoVideo();
    return;
  }
  if(!strcmp(args[1], "video"))
    debuggerIoVideo();
  else if(!strcmp(args[1], "video2"))
    debuggerIoVideo2();
  else if(!strcmp(args[1], "dma"))
    debuggerIoDMA();
  else if(!strcmp(args[1], "timer"))
    debuggerIoTimer();
  else if(!strcmp(args[1], "misc"))
    debuggerIoMisc();
  else printf("Unrecognized option %s\n", args[1]);
}

void debuggerEditByte(int n, char **args)
{
  if(n == 3) {
    u32 address;
    u32 byte;
    sscanf(args[1], "%x", &address);
    sscanf(args[2], "%x", &byte);
    debuggerWriteByte(address, (u8)byte);
  } else
    debuggerUsage("eb");    
}

void debuggerEditHalfWord(int n, char **args)
{
  if(n == 3) {
    u32 address;
    u32 byte;
    sscanf(args[1], "%x", &address);
    if(address & 1) {
      printf("Error: address must be half-word aligned\n");
      return;
    }
    sscanf(args[2], "%x", &byte);
    debuggerWriteHalfWord(address, (u16)byte);
  } else
    debuggerUsage("eh");        
}

void debuggerEdit(int n, char **args)
{
  if(n == 3) {
    u32 address;
    u32 byte;
    sscanf(args[1], "%x", &address);
    if(address & 3) {
      printf("Error: address must be word aligned\n");
      return;
    }
    sscanf(args[2], "%x", &byte);
    debuggerWriteMemory(address, (u32)byte);
  } else
    debuggerUsage("ew");    
}


#define ASCII(c) (c) < 32 ? '.' : (c) > 127 ? '.' : (c)

void debuggerMemoryByte(int n, char **args)
{
  if(n == 2) {
    u32 addr = 0;
    sscanf(args[1], "%x", &addr);
    for(int i = 0; i < 16; i++) {
      int a = debuggerReadByte(addr);
      int b = debuggerReadByte(addr+1);
      int c = debuggerReadByte(addr+2);
      int d = debuggerReadByte(addr+3);
      int e = debuggerReadByte(addr+4);
      int f = debuggerReadByte(addr+5);
      int g = debuggerReadByte(addr+6);
      int h = debuggerReadByte(addr+7);
      int i = debuggerReadByte(addr+8);
      int j = debuggerReadByte(addr+9);
      int k = debuggerReadByte(addr+10);
      int l = debuggerReadByte(addr+11);
      int m = debuggerReadByte(addr+12);
      int n = debuggerReadByte(addr+13);
      int o = debuggerReadByte(addr+14);
      int p = debuggerReadByte(addr+15);
      
      printf("%08x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
             addr,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,
             ASCII(a),ASCII(b),ASCII(c),ASCII(d),
             ASCII(e),ASCII(f),ASCII(g),ASCII(h),
             ASCII(i),ASCII(j),ASCII(k),ASCII(l),
             ASCII(m),ASCII(n),ASCII(o),ASCII(p));
      addr += 16;
    }
  } else
    debuggerUsage("mb");    
}

void debuggerMemoryHalfWord(int n, char **args)
{
  if(n == 2) {
    u32 addr = 0;
    sscanf(args[1], "%x", &addr);
    addr = addr & 0xfffffffe;
    for(int i = 0; i < 16; i++) {
      int a = debuggerReadByte(addr);
      int b = debuggerReadByte(addr+1);
      int c = debuggerReadByte(addr+2);
      int d = debuggerReadByte(addr+3);
      int e = debuggerReadByte(addr+4);
      int f = debuggerReadByte(addr+5);
      int g = debuggerReadByte(addr+6);
      int h = debuggerReadByte(addr+7);
      int i = debuggerReadByte(addr+8);
      int j = debuggerReadByte(addr+9);
      int k = debuggerReadByte(addr+10);
      int l = debuggerReadByte(addr+11);
      int m = debuggerReadByte(addr+12);
      int n = debuggerReadByte(addr+13);
      int o = debuggerReadByte(addr+14);
      int p = debuggerReadByte(addr+15);
      
      printf("%08x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
             addr,b,a,d,c,f,e,h,g,j,i,l,k,n,m,p,o,
             ASCII(a),ASCII(b),ASCII(c),ASCII(d),
             ASCII(e),ASCII(f),ASCII(g),ASCII(h),
             ASCII(i),ASCII(j),ASCII(k),ASCII(l),
             ASCII(m),ASCII(n),ASCII(o),ASCII(p));
      addr += 16;
    }
  } else
    debuggerUsage("mh");    
}

void debuggerMemory(int n, char **args)
{
  if(n == 2) {
    u32 addr = 0;
    sscanf(args[1], "%x", &addr);
    addr = addr & 0xfffffffc;
    for(int i = 0; i < 16; i++) {
      int a = debuggerReadByte(addr);
      int b = debuggerReadByte(addr+1);
      int c = debuggerReadByte(addr+2);
      int d = debuggerReadByte(addr+3);

      int e = debuggerReadByte(addr+4);
      int f = debuggerReadByte(addr+5);
      int g = debuggerReadByte(addr+6);
      int h = debuggerReadByte(addr+7);

      int i = debuggerReadByte(addr+8);
      int j = debuggerReadByte(addr+9);
      int k = debuggerReadByte(addr+10);
      int l = debuggerReadByte(addr+11);

      int m = debuggerReadByte(addr+12);
      int n = debuggerReadByte(addr+13);
      int o = debuggerReadByte(addr+14);
      int p = debuggerReadByte(addr+15);
      
      printf("%08x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
             addr,d,c,b,a,h,g,f,e,l,k,j,i,p,o,n,m,
             ASCII(a),ASCII(b),ASCII(c),ASCII(d),
             ASCII(e),ASCII(f),ASCII(g),ASCII(h),
             ASCII(i),ASCII(j),ASCII(k),ASCII(l),
             ASCII(m),ASCII(n),ASCII(o),ASCII(p));
      addr += 16;
    }
  } else
    debuggerUsage("mw");    
}

void debuggerQuit(int, char **)
{
  char buffer[10];
  printf("Are you sure you want to quit (y/n)? ");
  fgets(buffer, 1024, stdin);
  
  if(buffer[0] == 'y' || buffer[0] == 'Y') {
    debugger = false;
    emulating = false;
  }
}

void debuggerOutput(char *s, u32 addr)
{
  if(s)
    printf(s);
  else {
    char c;

    c = debuggerReadByte(addr);
    addr++;
    while(c) {
      putchar(c);
      c = debuggerReadByte(addr);
      addr++;
    }
  }
}

void debuggerMain()
{
  char buffer[1024];
  char *commands[10];
  char commandCount = 0;
  
  emuUpdateCPSR();
  debuggerRegisters(0, NULL);
  
  while(debugger) {
    systemSoundPause();
    printf("debugger> ");
    commandCount = 0;
    char *s = fgets(buffer, 1024, stdin);

    commands[0] = strtok(s, " \t\n");
    if(commands[0] == NULL)
      continue;
    commandCount++;
    while((s = strtok(NULL, " \t\n"))) {
      commands[commandCount++] = s;
      if(commandCount == 10)
        break;
    }

    for(int j = 0; ; j++) {
      if(debuggerCommands[j].name == NULL) {
        printf("Unrecognized command %s. Type h for help.\n", commands[0]);
        break;
      }
      if(!strcmp(commands[0], debuggerCommands[j].name)) {
        debuggerCommands[j].function(commandCount, commands);
        break;
      }
    } 
  }
}
