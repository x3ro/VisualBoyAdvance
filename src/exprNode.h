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

typedef struct Node {
  Type *type;
  u32 location;
  u32 objLocation;
  LocationType locType;
  int value;
  int index;
  char *name;
  struct Node *expression;
  Member *member;
  void (*print)(struct Node *);
  bool (*resolve)(struct Node *, Function *f, CompileUnit *u);
} Node;

extern void exprNodeCleanUp();

extern Node *exprNodeIdentifier();
extern void exprNodeIdentifierPrint(Node *);
extern bool exprNodeIdentifierResolve(Node *, Function *, CompileUnit *);

extern Node *exprNodeNumber();
extern void exprNodeNumberPrint(Node *);
extern bool exprNodeNumberResolve(Node *, Function *, CompileUnit *);

extern Node *exprNodeStar(Node *);
extern void exprNodeStarPrint(Node *);
extern bool exprNodeStarResolve(Node *, Function *, CompileUnit *);

extern Node *exprNodeDot(Node *, Node *);
extern void exprNodeDotPrint(Node *);
extern bool exprNodeDotResolve(Node *, Function *, CompileUnit *);

extern Node *exprNodeArrow(Node *, Node *);
extern void exprNodeArrowPrint(Node *);
extern bool exprNodeArrowResolve(Node *, Function *, CompileUnit *);

extern Node *exprNodeAddr(Node *);
extern void exprNodeAddrPrint(Node *);
extern bool exprNodeAddrResolve(Node *, Function *, CompileUnit *);

extern Node *exprNodeSizeof(Node *);
extern void exprNodeSizeofPrint(Node *);
extern bool exprNodeSizeofResolve(Node *, Function *, CompileUnit *);

extern Node *exprNodeArray(Node *, Node *);
extern void exprNodeArrayPrint(Node *);
extern bool exprNodeArrayResolve(Node *, Function *, CompileUnit *);

#define YYSTYPE struct Node *
