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
#ifdef BKPT_SUPPORT
#define CONSOLE_OUTPUT(a,b) \
    extern void (*dbgOutput)(char *, u32);\
    if((opcode == 0xe0000000) && (reg[0].I == 0xC0DED00D)) {\
      dbgOutput((a), (b));\
    }
#else
#define CONSOLE_OUTPUT(a,b)
#endif

#define OP_AND \
      reg[dest].I = reg[(opcode>>16)&15].I & value;\
      CONSOLE_OUTPUT(NULL,reg[2].I);

#define OP_ANDS \
      reg[dest].I = reg[(opcode>>16)&15].I & value;\
      \
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
      Z_FLAG = (reg[dest].I) ? false : true;\
      C_FLAG = C_OUT;

#define OP_EOR \
      reg[dest].I = reg[(opcode>>16)&15].I ^ value;

#define OP_EORS \
      reg[dest].I = reg[(opcode>>16)&15].I ^ value;\
      \
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
      Z_FLAG = (reg[dest].I) ? false : true;\
      C_FLAG = C_OUT;
#ifdef C_CORE
#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)
#define ADDCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & NEG(b)) |\
            (NEG(a) & POS(c)) |\
            (NEG(b) & POS(c))) ? true : false;
#define ADDOVERFLOW(a, b, c) \
  V_FLAG = ((NEG(a) & NEG(b) & POS(c)) |\
            (POS(a) & POS(b) & NEG(c))) ? true : false;
#define SUBCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & POS(b)) |\
            (NEG(a) & POS(c)) |\
            (POS(b) & POS(c))) ? true : false;
#define SUBOVERFLOW(a, b, c)\
  V_FLAG = ((NEG(a) & POS(b) & POS(c)) |\
            (POS(a) & NEG(b) & NEG(c))) ? true : false;
#define OP_SUB \
    {\
      reg[dest].I = reg[base].I - value;\
    }
#define OP_SUBS \
   {\
     u32 lhs = reg[base].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_RSB \
    {\
      reg[dest].I = value - reg[base].I;\
    }
#define OP_RSBS \
   {\
     u32 lhs = reg[base].I;\
     u32 rhs = value;\
     u32 res = rhs - lhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define OP_ADD \
    {\
      reg[dest].I = reg[base].I + value;\
    }
#define OP_ADDS \
   {\
     u32 lhs = reg[base].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define OP_ADC \
    {\
      reg[dest].I = reg[base].I + value + (u32)C_FLAG;\
    }
#define OP_ADCS \
   {\
     u32 lhs = reg[base].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs + (u32)C_FLAG;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define OP_SBC \
    {\
      reg[dest].I = reg[base].I - value - !((u32)C_FLAG);\
    }
#define OP_SBCS \
   {\
     u32 lhs = reg[base].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs - !((u32)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_RSC \
    {\
      reg[dest].I = value - reg[base].I - !((u32)C_FLAG);\
    }
#define OP_RSCS \
   {\
     u32 lhs = reg[base].I;\
     u32 rhs = value;\
     u32 res = rhs - lhs - !((u32)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define OP_CMP \
   {\
     u32 lhs = reg[base].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define OP_CMN \
   {\
     u32 lhs = reg[base].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }

#define LOGICAL_LSL_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (32 - shift)) & 1 ? true : false;\
     value = v << shift;\
   }
#define LOGICAL_LSR_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = v >> shift;\
   }
#define LOGICAL_ASR_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     C_OUT = ((s32)v >> (int)(shift - 1)) & 1 ? true : false;\
     value = (s32)v >> (int)shift;\
   }
#define LOGICAL_ROR_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define LOGICAL_RRX_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     shift = (int)C_FLAG;\
     C_OUT = (v  & 1) ? true : false;\
     value = ((v >> 1) |\
              (shift << 31));\
   }
#define LOGICAL_ROR_IMM \
   {\
     u32 v = opcode & 0xff;\
     C_OUT = (v >> (shift - 1)) & 1 ? true : false;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ARITHMETIC_LSL_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     value = v << shift;\
   }
#define ARITHMETIC_LSR_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     value = v >> shift;\
   }
#define ARITHMETIC_ASR_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     value = (s32)v >> (int)shift;\
   }
#define ARITHMETIC_ROR_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ARITHMETIC_RRX_REG \
   {\
     u32 v = reg[opcode & 0x0f].I;\
     shift = (int)C_FLAG;\
     value = ((v >> 1) |\
              (shift << 31));\
   }
#define ARITHMETIC_ROR_IMM \
   {\
     u32 v = opcode & 0xff;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ROR_IMM_MSR \
   {\
     u32 v = opcode & 0xff;\
     value = ((v << (32 - shift)) |\
              (v >> shift));\
   }
#define ROR_VALUE \
   {\
     value = ((value << (32 - shift)) |\
              (value >> shift));\
   }
#define RCR_VALUE \
   {\
     shift = (int)C_FLAG;\
     value = ((value >> 1) |\
              (shift << 31));\
   }
#else
#ifdef __GNUC__
#define OP_SUB \
     asm ("sub %1, %%ebx;"\
                  : "=b" (reg[dest].I)\
                  : "r" (value), "b" (reg[base].I));

#define OP_SUBS \
     asm ("sub %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
                  : "=b" (reg[dest].I)\
                  : "r" (value), "b" (reg[base].I));

#define OP_RSB \
            asm  ("sub %1, %%ebx;"\
                 : "=b" (reg[dest].I)\
                 : "r" (reg[base].I), "b" (value));

#define OP_RSBS \
            asm  ("sub %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setncb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (reg[dest].I)\
                 : "r" (reg[base].I), "b" (value));

#define OP_ADD \
            asm  ("add %1, %%ebx;"\
                 : "=b" (reg[dest].I)\
                 : "r" (value), "b" (reg[base].I));

#define OP_ADDS \
            asm  ("add %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setcb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (reg[dest].I)\
                 : "r" (value), "b" (reg[base].I));

#define OP_ADC \
            asm  ("bt $0, C_FLAG;"\
                  "adc %1, %%ebx;"\
                 : "=b" (reg[dest].I)\
                 : "r" (value), "b" (reg[base].I));

#define OP_ADCS \
            asm  ("bt $0, C_FLAG;"\
                  "adc %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setcb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (reg[dest].I)\
                 : "r" (value), "b" (reg[base].I));

#define OP_SBC \
            asm  ("bt $0, C_FLAG;"\
                  "cmc;"\
                  "sbb %1, %%ebx;"\
                 : "=b" (reg[dest].I)\
                 : "r" (value), "b" (reg[base].I));

#define OP_SBCS \
            asm  ("bt $0, C_FLAG;"\
                  "cmc;"\
                  "sbb %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setncb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (reg[dest].I)\
                 : "r" (value), "b" (reg[base].I));
#define OP_RSC \
            asm  ("bt $0, C_FLAG;"\
                  "cmc;"\
                  "sbb %1, %%ebx;"\
                 : "=b" (reg[dest].I)\
                 : "r" (reg[base].I), "b" (value));

#define OP_RSCS \
            asm  ("bt $0, C_FLAG;"\
                  "cmc;"\
                  "sbb %1, %%ebx;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setncb C_FLAG;"\
                  "setob V_FLAG;"\
                 : "=b" (reg[dest].I)\
                 : "r" (reg[base].I), "b" (value));
#define OP_CMP \
            asm  ("sub %0, %1;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setncb C_FLAG;"\
                  "setob V_FLAG;"\
                 :\
                 : "r" (value), "r" (reg[base].I));

#define OP_CMN \
            asm  ("add %0, %1;"\
                  "setsb N_FLAG;"\
                  "setzb Z_FLAG;"\
                  "setcb C_FLAG;"\
                  "setob V_FLAG;"\
                 : \
                 : "r" (value), "r" (reg[base].I));
#define LOGICAL_LSL_REG \
       asm("shl %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (reg[opcode & 0x0f].I), "c" (shift));

#define LOGICAL_LSR_REG \
       asm("shr %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (reg[opcode & 0x0f].I), "c" (shift));

#define LOGICAL_ASR_REG \
       asm("sar %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (reg[opcode & 0x0f].I), "c" (shift));

#define LOGICAL_ROR_REG \
       asm("ror %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (reg[opcode & 0x0f].I), "c" (shift));       

#define LOGICAL_RRX_REG \
       asm("bt $0, C_FLAG;"\
           "rcr $1, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (reg[opcode & 0x0f].I));       

#define LOGICAL_ROR_IMM \
       asm("ror %%cl, %%eax;"\
           "setcb %%cl;"\
           : "=a" (value), "=c" (C_OUT)\
           : "a" (opcode & 0xff), "c" (shift));
#define ARITHMETIC_LSL_REG \
       asm("\
             shl %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (reg[opcode & 0x0f].I), "c" (shift));

#define ARITHMETIC_LSR_REG \
       asm("\
             shr %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (reg[opcode & 0x0f].I), "c" (shift));

#define ARITHMETIC_ASR_REG \
       asm("\
             sar %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (reg[opcode & 0x0f].I), "c" (shift));

#define ARITHMETIC_ROR_REG \
       asm("\
             ror %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (reg[opcode & 0x0f].I), "c" (shift));       

#define ARITHMETIC_RRX_REG \
       asm("\
             bt $0, C_FLAG;\
             rcr $1, %%eax;"\
           : "=a" (value)\
           : "a" (reg[opcode & 0x0f].I));       

#define ARITHMETIC_ROR_IMM \
       asm("\
             ror %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (opcode & 0xff), "c" (shift));
#define ROR_IMM_MSR \
      asm ("ror %%cl, %%eax;"\
           : "=a" (value)\
           : "a" (opcode & 0xFF), "c" (shift));
#define ROR_VALUE \
      asm("ror %%cl, %0"\
          : "=r" (value)\
          : "r" (value), "c" (shift));
#define RCR_VALUE \
      asm("bt $0, C_FLAG;"\
          "rcr $1, %0"\
          : "=r" (value)\
          : "r" (value));
#else
#define OP_SUB \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
        __asm sub ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg+4*eax], ebx\
      }

#define OP_SUBS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
        __asm sub ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg+4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setnc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }

#define OP_RSB \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
        __asm mov eax, value\
        __asm sub eax, ebx\
        __asm mov ebx, dest\
        __asm mov dword ptr [OFFSET reg+4*ebx], eax\
      }

#define OP_RSBS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
        __asm mov eax, value\
        __asm sub eax, ebx\
        __asm mov ebx, dest\
        __asm mov dword ptr [OFFSET reg+4*ebx], eax\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setnc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }

#define OP_ADD \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
        __asm add ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg+4*eax], ebx\
      }

#define OP_ADDS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
        __asm add ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg+4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }

#define OP_ADC \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
        __asm bt word ptr C_FLAG, 0\
        __asm adc ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg+4*eax], ebx\
      }

#define OP_ADCS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
        __asm bt word ptr C_FLAG, 0\
        __asm adc ebx, value\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg+4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }

#define OP_SBC \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg + 4*ebx]\
        __asm mov eax, value\
        __asm bt word ptr C_FLAG, 0\
        __asm cmc\
        __asm sbb ebx, eax\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg + 4*eax], ebx\
      }

#define OP_SBCS \
      {\
        __asm mov ebx, base\
        __asm mov ebx, dword ptr [OFFSET reg + 4*ebx]\
        __asm mov eax, value\
        __asm bt word ptr C_FLAG, 0\
        __asm cmc\
        __asm sbb ebx, eax\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg + 4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setnc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }
#define OP_RSC \
      {\
        __asm mov ebx, value\
        __asm mov eax, base\
        __asm mov eax, dword ptr[OFFSET reg + 4*eax]\
        __asm bt word ptr C_FLAG, 0\
        __asm cmc\
        __asm sbb ebx, eax\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg + 4*eax], ebx\
      }

#define OP_RSCS \
      {\
        __asm mov ebx, value\
        __asm mov eax, base\
        __asm mov eax, dword ptr[OFFSET reg + 4*eax]\
        __asm bt word ptr C_FLAG, 0\
        __asm cmc\
        __asm sbb ebx, eax\
        __asm mov eax, dest\
        __asm mov dword ptr [OFFSET reg + 4*eax], ebx\
        __asm sets byte ptr N_FLAG\
        __asm setz byte ptr Z_FLAG\
        __asm setnc byte ptr C_FLAG\
        __asm seto byte ptr V_FLAG\
      }
#define OP_CMP \
     {\
       __asm mov eax, base\
       __asm mov ebx, dword ptr [OFFSET reg+4*eax]\
       __asm sub ebx, value\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setnc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }

#define OP_CMN \
     {\
       __asm mov eax, base\
       __asm mov ebx, dword ptr [OFFSET reg+4*eax]\
       __asm add ebx, value\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#define LOGICAL_LSL_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm shl eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_LSR_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm shr eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_ASR_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm sar eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_ROR_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0F\
        __asm mov eax, dword ptr [OFFSET reg + 4*eax]\
        __asm mov cl, byte ptr shift\
        __asm ror eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_RRX_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0F\
        __asm mov eax, dword ptr [OFFSET reg + 4*eax]\
        __asm bt word ptr C_OUT, 0\
        __asm rcr eax, 1\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT

#define LOGICAL_ROR_IMM \
        __asm mov eax, opcode\
        __asm and eax, 0xff\
        __asm mov cl, byte ptr shift\
        __asm ror eax, cl\
        __asm mov value, eax\
        __asm setc byte ptr C_OUT
#define ARITHMETIC_LSL_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm shl eax, cl\
        __asm mov value, eax

#define ARITHMETIC_LSR_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm shr eax, cl\
        __asm mov value, eax

#define ARITHMETIC_ASR_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0f\
        __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
        __asm mov cl, byte ptr shift\
        __asm sar eax, cl\
        __asm mov value, eax

#define ARITHMETIC_ROR_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0F\
        __asm mov eax, dword ptr [OFFSET reg + 4*eax]\
        __asm mov cl, byte ptr shift\
        __asm ror eax, cl\
        __asm mov value, eax

#define ARITHMETIC_RRX_REG \
        __asm mov eax, opcode\
        __asm and eax, 0x0F\
        __asm mov eax, dword ptr [OFFSET reg + 4*eax]\
        __asm bt word ptr C_FLAG, 0\
        __asm rcr eax, 1\
        __asm mov value, eax

#define ARITHMETIC_ROR_IMM \
        __asm mov eax, opcode\
        __asm and eax, 0xff\
        __asm mov cl, byte ptr shift\
        __asm ror eax, cl\
        __asm mov value, eax
#define ROR_IMM_MSR \
      {\
        __asm mov eax, opcode\
        __asm and eax, 0xff\
        __asm mov cl, byte ptr shift\
        __asm ror eax, CL\
        __asm mov value, eax\
      }
#define ROR_VALUE \
      {\
        __asm mov cl, byte ptr shift\
        __asm ror dword ptr value, cl\
      }
#define RCR_VALUE \
      {\
        __asm mov cl, byte ptr shift\
        __asm bt word ptr C_FLAG, 0\
        __asm rcr dword ptr value, 1\
      }
#endif
#endif

#define OP_TST \
      u32 res = reg[base].I & value;\
      N_FLAG = (res & 0x80000000) ? true : false;\
      Z_FLAG = (res) ? false : true;\
      C_FLAG = C_OUT;

#define OP_TEQ \
      u32 res = reg[base].I ^ value;\
      N_FLAG = (res & 0x80000000) ? true : false;\
      Z_FLAG = (res) ? false : true;\
      C_FLAG = C_OUT;

#define OP_ORR \
    reg[dest].I = reg[base].I | value;

#define OP_ORRS \
    reg[dest].I = reg[base].I | value;\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_MOV \
    reg[dest].I = value;

#define OP_MOVS \
    reg[dest].I = value;\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_BIC \
    reg[dest].I = reg[base].I & (~value);

#define OP_BICS \
    reg[dest].I = reg[base].I & (~value);\
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define OP_MVN \
    reg[dest].I = ~value;

#define OP_MVNS \
    reg[dest].I = ~value; \
    N_FLAG = (reg[dest].I & 0x80000000) ? true : false;\
    Z_FLAG = (reg[dest].I) ? false : true;\
    C_FLAG = C_OUT;

#define CASE_16(BASE) \
  case BASE:\
  case BASE+1:\
  case BASE+2:\
  case BASE+3:\
  case BASE+4:\
  case BASE+5:\
  case BASE+6:\
  case BASE+7:\
  case BASE+8:\
  case BASE+9:\
  case BASE+10:\
  case BASE+11:\
  case BASE+12:\
  case BASE+13:\
  case BASE+14:\
  case BASE+15:

#define CASE_256(BASE) \
  CASE_16(BASE)\
  CASE_16(BASE+0x10)\
  CASE_16(BASE+0x20)\
  CASE_16(BASE+0x30)\
  CASE_16(BASE+0x40)\
  CASE_16(BASE+0x50)\
  CASE_16(BASE+0x60)\
  CASE_16(BASE+0x70)\
  CASE_16(BASE+0x80)\
  CASE_16(BASE+0x90)\
  CASE_16(BASE+0xa0)\
  CASE_16(BASE+0xb0)\
  CASE_16(BASE+0xc0)\
  CASE_16(BASE+0xd0)\
  CASE_16(BASE+0xe0)\
  CASE_16(BASE+0xf0)

#define LOGICAL_DATA_OPCODE(OPCODE, OPCODE2, BASE) \
  case BASE: \
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      \
      if(shift) {\
        LOGICAL_LSL_REG\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
       /* OP Rd,Rb,Rm LSR # */ \
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      if(shift) {\
        LOGICAL_LSR_REG\
      } else {\
        value = 0;\
        C_OUT = (reg[opcode & 0x0F].I & 0x80000000) ? true : false;\
      }\
      \
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
       /* OP Rd,Rb,Rm ASR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      if(shift) {\
        LOGICAL_ASR_REG\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }                   \
      }\
      \
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
       /* OP Rd,Rb,Rm ROR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      if(shift) {\
        LOGICAL_ROR_REG\
      } else {\
        LOGICAL_RRX_REG\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
       /* OP Rd,Rb,Rm LSL Rs */\
      clockTicks++;\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (reg[opcode & 0x0F].I & 1 ? true : false);\
        } else if(shift < 32) {\
           LOGICAL_LSL_REG\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
       /* OP Rd,Rb,Rm LSR Rs */ \
      clockTicks++;\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
          C_OUT = (reg[opcode & 0x0F].I & 0x80000000 ? true : false);\
        } else if(shift < 32) {\
            LOGICAL_LSR_REG\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
       /* OP Rd,Rb,Rm ASR Rs */ \
      clockTicks++;\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      if(shift < 32) {\
        if(shift) {\
          LOGICAL_ASR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
          C_OUT = true;\
        } else {\
          value = 0;\
          C_OUT = false;\
        }\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
       /* OP Rd,Rb,Rm ROR Rs */\
      clockTicks++;\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
          LOGICAL_ROR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
          C_OUT = (value & 0x80000000 ? true : false);\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
        C_OUT = (value & 0x80000000 ? true : false);\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int base = (opcode >> 16) & 0x0F;\
      int dest = (opcode >> 12) & 0x0F;\
      bool C_OUT = C_FLAG;\
      u32 value;\
      if(shift) {\
        LOGICAL_ROR_IMM\
      } else {\
        value = opcode & 0xff;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;

#define ARITHMETIC_DATA_OPCODE(OPCODE, OPCODE2, BASE) \
  case BASE:\
  case BASE+8:\
    {\
      /* OP Rd,Rb,Rm LSL # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      u32 value;\
      if(shift) {\
        ARITHMETIC_LSL_REG\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+2:\
  case BASE+10:\
    {\
      /* OP Rd,Rb,Rm LSR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      u32 value;\
      if(shift) {\
        ARITHMETIC_LSR_REG\
      } else {\
        value = 0;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+4:\
  case BASE+12:\
    {\
      /* OP Rd,Rb,Rm ASR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      u32 value;\
      if(shift) {\
        ARITHMETIC_ASR_REG\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
        } else value = 0;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+6:\
  case BASE+14:\
    {\
      /* OP Rd,Rb,Rm ROR # */\
      int base = (opcode >> 16) & 0x0F;\
      int shift = (opcode >> 7) & 0x1F;\
      int dest = (opcode>>12) & 15;\
      u32 value;\
      if(shift) {\
         ARITHMETIC_ROR_REG\
      } else {\
         ARITHMETIC_RRX_REG\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+1:\
    {\
      /* OP Rd,Rb,Rm LSL Rs */\
      clockTicks++;\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      u32 value;\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
        } else if(shift < 32) {\
           ARITHMETIC_LSL_REG\
        } else value = 0;\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+3:\
    {\
      /* OP Rd,Rb,Rm LSR Rs */\
      clockTicks++;\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      u32 value;\
      if(shift) {\
        if(shift == 32) {\
          value = 0;\
        } else if(shift < 32) {\
           ARITHMETIC_LSR_REG\
        } else value = 0;\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+5:\
    {\
      /* OP Rd,Rb,Rm ASR Rs */\
      clockTicks++;\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      u32 value;\
      if(shift < 32) {\
        if(shift) {\
           ARITHMETIC_ASR_REG\
        } else {\
          value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        if(reg[opcode & 0x0F].I & 0x80000000){\
          value = 0xFFFFFFFF;\
        } else value = 0;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+7:\
    {\
      /* OP Rd,Rb,Rm ROR Rs */\
      clockTicks++;\
      int base = (opcode >> 16) & 0x0F;\
      int shift = reg[(opcode >> 8)&15].B.B0;\
      int dest = (opcode>>12) & 15;\
      u32 value;\
      if(shift) {\
        shift &= 0x1f;\
        if(shift) {\
           ARITHMETIC_ROR_REG\
        } else {\
           value = reg[opcode & 0x0F].I;\
        }\
      } else {\
        value = reg[opcode & 0x0F].I;\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;\
  case BASE+0x200:\
  case BASE+0x201:\
  case BASE+0x202:\
  case BASE+0x203:\
  case BASE+0x204:\
  case BASE+0x205:\
  case BASE+0x206:\
  case BASE+0x207:\
  case BASE+0x208:\
  case BASE+0x209:\
  case BASE+0x20a:\
  case BASE+0x20b:\
  case BASE+0x20c:\
  case BASE+0x20d:\
  case BASE+0x20e:\
  case BASE+0x20f:\
    {\
      int shift = (opcode & 0xF00) >> 7;\
      int base = (opcode >> 16) & 0x0F;\
      int dest = (opcode >> 12) & 0x0F;\
      u32 value;\
      {\
        ARITHMETIC_ROR_IMM\
      }\
      if(dest == 15) {\
        OPCODE2\
        /* todo */\
        if(opcode & 0x00100000) {\
          clockTicks++;\
          CPUSwitchMode(reg[17].I & 0x1f, false);\
        }\
        if(armState) {\
          reg[15].I &= 0xFFFFFFFC;\
          armNextPC = reg[15].I;\
          reg[15].I += 4;\
        } else {\
          reg[15].I &= 0xFFFFFFFE;\
          armNextPC = reg[15].I;\
          reg[15].I += 2;\
        }\
      } else {\
        OPCODE \
      }\
    }\
    break;

  u32 opcode = CPUReadMemoryQuick(armNextPC);

#ifndef FINAL_VERSION
  if(armNextPC == stop) {
    armNextPC++;
  }
#endif

  armNextPC = reg[15].I;
  reg[15].I += 4;
  int cond = opcode >> 28;
  // suggested optimization for frequent cases
  bool cond_res;
  if(cond == 0x0e) {
    cond_res = true;
  } else {
    switch(cond) { 
    case 0x00: // EQ 
      cond_res = Z_FLAG;
      break;
    case 0x01: // NE
      cond_res = !Z_FLAG;
      break; 
    case 0x02: // CS
      cond_res = C_FLAG;
      break;
    case 0x03: // CC
      cond_res = !C_FLAG;
      break;
    case 0x04: // MI
      cond_res = N_FLAG;
      break;
    case 0x05: // PL
      cond_res = !N_FLAG;
      break;
    case 0x06: // VS
      cond_res = V_FLAG;
      break;
    case 0x07: // VC
      cond_res = !V_FLAG;
      break;
    case 0x08: // HI
      cond_res = C_FLAG && !Z_FLAG;
      break;
    case 0x09: // LS
      cond_res = !C_FLAG || Z_FLAG;
      break;
    case 0x0A: // GE
      cond_res = N_FLAG == V_FLAG;
      break;
    case 0x0B: // LT
      cond_res = N_FLAG != V_FLAG;
      break;
    case 0x0C: // GT
      cond_res = !Z_FLAG &&(N_FLAG == V_FLAG);
      break;    
    case 0x0D: // LE
      cond_res = Z_FLAG || (N_FLAG != V_FLAG);
      break; 
    case 0x0E: 
      cond_res = true; 
      break;
    case 0x0F:
    default:
      // ???
      cond_res = false;
      break;
    }
  }
  
if(cond_res) {
  switch(((opcode>>16)&0xFF0) | ((opcode>>4)&0x0F)) {
    LOGICAL_DATA_OPCODE(OP_AND,  OP_AND, 0x000);
    LOGICAL_DATA_OPCODE(OP_ANDS, OP_AND, 0x010);
  case 0x009:
    {
      // MUL Rd, Rm, Rs
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      int rs = (opcode >> 8) & 0x0F;
      clockTicks = 4;
      reg[dest].I = reg[mult].I * reg[rs].I;
    }
    break;
  case 0x019:
    {
      // MULS Rd, Rm, Rs
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      int rs = (opcode >> 8) & 0x0F;
      clockTicks = 4;
      reg[dest].I = reg[mult].I * reg[rs].I;
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;
      Z_FLAG = (reg[dest].I) ? false : true;
    }
    break;
  case 0x00b:
  case 0x02b:
    {
      // STRH Rd, [Rn], -Rm
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x04b:
  case 0x06b:
    {
      // STRH Rd, [Rn], #-offset
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x08b:
  case 0x0ab:
    {
      // STRH Rd, [Rn], Rm
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x0cb:
  case 0x0eb:
    {
      // STRH Rd, [Rn], #offset
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x10b:
    {
      // STRH Rd, [Rn, -Rm]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
    }
    break;
  case 0x12b:
    {
      // STRH Rd, [Rn, -Rm]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
      reg[base].I = address;
    }
    break;
  case 0x14b:
    {
      // STRH Rd, [Rn, -#offset]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
    }
    break;
  case 0x16b:
    {
      // STRH Rd, [Rn, -#offset]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
      reg[base].I = address;
    }
    break;
  case 0x18b:
    {
      // STRH Rd, [Rn, Rm]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
    }
    break;
  case 0x1ab:
    {
      // STRH Rd, [Rn, Rm]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
      reg[base].I = address;
    }
    break;
  case 0x1cb:
    {
      // STRH Rd, [Rn, #offset]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
    }
    break;
  case 0x1eb:
    {
      // STRH Rd, [Rn, #offset]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 4;
      CPUWriteHalfWord(address, reg[dest].W.W0);
      reg[base].I = address;
    }
    break;
  case 0x01b:
  case 0x03b:
    {
      // LDRH Rd, [Rn], -Rm
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x05b:
  case 0x07b:
    {
      // LDRH Rd, [Rn], #-offset
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x09b:
  case 0x0bb:
    {
      // LDRH Rd, [Rn], Rm
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x0db:
  case 0x0fb:
    {
      // LDRH Rd, [Rn], #offset
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x11b:
    {
      // LDRH Rd, [Rn, -Rm]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
    }
    break;
  case 0x13b:
    {
      // LDRH Rd, [Rn, -Rm]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
      reg[base].I = address;
    }
    break;
  case 0x15b:
    {
      // LDRH Rd, [Rn, -#offset]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
    }
    break;
  case 0x17b:
    {
      // LDRH Rd, [Rn, -#offset]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
      reg[base].I = address;
    }
    break;
  case 0x19b:
    {
      // LDRH Rd, [Rn, Rm]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
    }
    break;
  case 0x1bb:
    {
      // LDRH Rd, [Rn, Rm]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
      reg[base].I = address;
    }
    break;
  case 0x1db:
    {
      // LDRH Rd, [Rn, #offset]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
    }
    break;
  case 0x1fb:
    {
      // LDRH Rd, [Rn, #offset]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = CPUReadHalfWord(address);
      reg[base].I = address;
    }
    break;
  case 0x01d:
  case 0x03d:
    {
      // LDRSB Rd, [Rn], -Rm
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x05d:
  case 0x07d:
    {
      // LDRSB Rd, [Rn], #-offset
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x09d:
  case 0x0bd:
    {
      // LDRSB Rd, [Rn], Rm
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x0dd:
  case 0x0fd:
    {
      // LDRSB Rd, [Rn], #offset
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x11d:
    {
      // LDRSB Rd, [Rn, -Rm]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
    }
    break;
  case 0x13d:
    {
      // LDRSB Rd, [Rn, -Rm]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
      reg[base].I = address;
    }
    break;
  case 0x15d:
    {
      // LDRSB Rd, [Rn, -#offset]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
    }
    break;
  case 0x17d:
    {
      // LDRSB Rd, [Rn, -#offset]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
      reg[base].I = address;
    }
    break;
  case 0x19d:
    {
      // LDRSB Rd, [Rn, Rm]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
    }
    break;
  case 0x1bd:
    {
      // LDRSB Rd, [Rn, Rm]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
      reg[base].I = address;
    }
    break;
  case 0x1dd:
    {
      // LDRSB Rd, [Rn, #offset]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
    }
    break;
  case 0x1fd:
    {
      // LDRSB Rd, [Rn, #offset]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = (s8)CPUReadByte(address);
      reg[base].I = address;
    }
    break;
  case 0x01f:
  case 0x03f:
    {
      // LDRSH Rd, [Rn], -Rm
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x05f:
  case 0x07f:
    {
      // LDRSH Rd, [Rn], #-offset
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
      address -= offset;
      reg[base].I = address;
    }
    break;
  case 0x09f:
  case 0x0bf:
    {
      // LDRSH Rd, [Rn], Rm
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x0df:
  case 0x0ff:
    {
      // LDRSH Rd, [Rn], #offset
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I;
      int offset = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
      address += offset;
      reg[base].I = address;
    }
    break;
  case 0x11f:
    {
      // LDRSH Rd, [Rn, -Rm]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
    }
    break;
  case 0x13f:
    {
      // LDRSH Rd, [Rn, -Rm]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
      reg[base].I = address;
    }
    break;
  case 0x15f:
    {
      // LDRSH Rd, [Rn, -#offset]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
    }
    break;
  case 0x17f:
    {
      // LDRSH Rd, [Rn, -#offset]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I - ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
      reg[base].I = address;
    }
    break;
  case 0x19f:
    {
      // LDRSH Rd, [Rn, Rm]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
    }
    break;
  case 0x1bf:
    {
      // LDRSH Rd, [Rn, Rm]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + reg[opcode & 0x0F].I;
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
      reg[base].I = address;
    }
    break;
  case 0x1df:
    {
      // LDRSH Rd, [Rn, #offset]
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
    }
    break;
  case 0x1ff:
    {
      // LDRSH Rd, [Rn, #offset]!
      int base = (opcode >> 16) & 0x0F;
      int dest = (opcode >> 12) & 0x0F;
      u32 address = reg[base].I + ((opcode & 0x0F)|((opcode>>4)&0xF0));
      clockTicks = 3;
      reg[dest].I = (s16)CPUReadHalfWordSigned(address);
      reg[base].I = address;
    }
    break;
    LOGICAL_DATA_OPCODE(OP_EOR,  OP_EOR, 0x020);
    LOGICAL_DATA_OPCODE(OP_EORS, OP_EOR, 0x030);
  case 0x029:
    {
      // MLA Rd, Rm, Rs, Rn
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      int rs = (opcode >> 8) & 0x0F;
      clockTicks = 4;
      reg[dest].I = reg[mult].I * reg[rs].I + reg[(opcode>>12)&0x0f].I;
    }
    break;
  case 0x039:
    {
      // MLAS Rd, Rm, Rs, Rn
      int dest = (opcode >> 16) & 0x0F;
      int mult = (opcode & 0x0F);
      int rs = (opcode >> 8) & 0x0F;
      clockTicks = 4;
      reg[dest].I = reg[mult].I * reg[rs].I + reg[(opcode>>12)&0x0f].I;
      N_FLAG = (reg[dest].I & 0x80000000) ? true : false;
      Z_FLAG = (reg[dest].I) ? false : true;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_SUB,  OP_SUB, 0x040);
    ARITHMETIC_DATA_OPCODE(OP_SUBS, OP_SUB, 0x050);
    ARITHMETIC_DATA_OPCODE(OP_RSB,  OP_RSB, 0x060);
    ARITHMETIC_DATA_OPCODE(OP_RSBS, OP_RSB, 0x070);
    ARITHMETIC_DATA_OPCODE(OP_ADD,  OP_ADD, 0x080);
    ARITHMETIC_DATA_OPCODE(OP_ADDS, OP_ADD, 0x090);
  case 0x089:
    {
      // UMULL RdLo, RdHi, Rn, Rs
      u32 umult = reg[(opcode & 0x0F)].I;
      u32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u64 uTemp = ((u64)umult)*((u64)usource);
      reg[destLo].I = (u32)uTemp;
      reg[destHi].I = (u32)(uTemp >> 32);
      clockTicks = 4;
    }
    break;
  case 0x099:
    {
      // UMULLS RdLo, RdHi, Rn, Rs
      u32 umult = reg[(opcode & 0x0F)].I;
      u32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u64 uTemp = ((u64)umult)*((u64)usource);
      reg[destLo].I = (u32)uTemp;
      reg[destHi].I = (u32)(uTemp >> 32);
      Z_FLAG = (uTemp) ? false : true;
      N_FLAG = (reg[destHi].I & 0x80000000) ? true : false;
      clockTicks = 4;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_ADC,  OP_ADC, 0x0a0);
    ARITHMETIC_DATA_OPCODE(OP_ADCS, OP_ADC, 0x0b0);
  case 0x0a9:
    {
      // UMLAL RdLo, RdHi, Rn, Rs
      u32 umult = reg[(opcode & 0x0F)].I;
      u32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u64 uTemp = (u64)reg[destHi].I;
      uTemp <<= 32;
      uTemp |= (u64)reg[destLo].I;
      uTemp += ((u64)umult)*((u64)usource);
      reg[destLo].I = (u32)uTemp;
      reg[destHi].I = (u32)(uTemp >> 32);
      clockTicks = 4;
    }
    break;
  case 0x0b9:
    {
      // UMLALS RdLo, RdHi, Rn, Rs
      u32 umult = reg[(opcode & 0x0F)].I;
      u32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      u64 uTemp = (u64)reg[destHi].I;
      uTemp <<= 32;
      uTemp |= (u64)reg[destLo].I;
      uTemp += ((u64)umult)*((u64)usource);
      reg[destLo].I = (u32)uTemp;
      reg[destHi].I = (u32)(uTemp >> 32);
      Z_FLAG = (uTemp) ? false : true;
      N_FLAG = (reg[destHi].I & 0x80000000) ? true : false;
      clockTicks = 4;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_SBC,  OP_SBC, 0x0c0);
    ARITHMETIC_DATA_OPCODE(OP_SBCS, OP_SBC, 0x0d0);
  case 0x0c9:
    {
      // SMULL RdLo, RdHi, Rm, Rs
      //    u32 umult = reg[(opcode & 0x0F)].I;
      //    u32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      s64 m = (s32)reg[(opcode & 0x0F)].I;
      s64 s = (s32)reg[(opcode >> 8) & 0x0F].I;
      s64 sTemp = m*s;
      reg[destLo].I = (u32)sTemp;
      reg[destHi].I = (u32)(sTemp >> 32);
      clockTicks = 4;
    }
    break;
  case 0x0d9:
    {
      // SMULLS RdLo, RdHi, Rm, Rs
      //      u32 umult = reg[(opcode & 0x0F)].I;
      //      u32 usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      s64 m = (s32)reg[(opcode & 0x0F)].I;
      s64 s = (s32)reg[(opcode >> 8) & 0x0F].I;
      s64 sTemp = m*s;
      reg[destLo].I = (u32)sTemp;
      reg[destHi].I = (u32)(sTemp >> 32);
      Z_FLAG = (sTemp) ? false : true;
      N_FLAG = (sTemp < 0) ? true : false;
      clockTicks = 4;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_RSC,  OP_RSC, 0x0e0);
    ARITHMETIC_DATA_OPCODE(OP_RSCS, OP_RSC, 0x0f0);
  case 0x0e9:
    {
      // SMLAL RdLo, RdHi, Rm, Rs
      //      umult = reg[(opcode & 0x0F)].I;
      //      usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      s64 m = (s32)reg[(opcode & 0x0F)].I;
      s64 s = (s32)reg[(opcode >> 8) & 0x0F].I;
      s64 sTemp = (u64)reg[destHi].I;
      sTemp <<= 32;
      sTemp |= (u64)reg[destLo].I;
      sTemp += m*s;
      reg[destLo].I = (u32)sTemp;
      reg[destHi].I = (u32)(sTemp >> 32);
      clockTicks = 4;
    }
    break;
  case 0x0f9:
    {
      // SMLALS RdLo, RdHi, Rm, Rs
      //      umult = reg[(opcode & 0x0F)].I;
      //      usource = reg[(opcode >> 8) & 0x0F].I;
      int destLo = (opcode >> 12) & 0x0F;         
      int destHi = (opcode >> 16) & 0x0F;
      s64 m = (s32)reg[(opcode & 0x0F)].I;
      s64 s = (s32)reg[(opcode >> 8) & 0x0F].I;
      s64 sTemp = (u64)reg[destHi].I;
      sTemp <<= 32;
      sTemp |= (u64)reg[destLo].I;
      sTemp += m*s;
      reg[destLo].I = (u32)sTemp;
      reg[destHi].I = (u32)(sTemp >> 32);
      Z_FLAG = (sTemp) ? false : true;
      N_FLAG = (sTemp < 0) ? true : false;
      clockTicks = 4;
    }
    break;
    LOGICAL_DATA_OPCODE(OP_TST, OP_TST, 0x110);
  case 0x100:
    // MRS Rd, CPSR
    // TODO: check if right instruction....
    CPUUpdateCPSR();
    reg[(opcode >> 12) & 0x0F].I = reg[16].I;
    break;
  case 0x109:
    {
      // SWP Rd, Rm, [Rn]
      u32 address = reg[(opcode >> 16) & 15].I;
      u32 temp = CPUReadMemory(address);
      CPUWriteMemory(address, reg[opcode&15].I);
      reg[(opcode >> 12) & 15].I = temp;
    }
    break;
    LOGICAL_DATA_OPCODE(OP_TEQ, OP_TEQ, 0x130);
  case 0x120:
    {
      // MSR CPSR_fields, Rm
      CPUUpdateCPSR();
      u32 value = reg[opcode & 15].I;
      u32 newValue = reg[16].I;
      if(armMode > 0x10) {
        if(opcode & 0x00010000)
          newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
      }
      if(opcode & 0x00080000)
        newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);
      CPUSwitchMode(newValue & 0x1f, false);
      reg[16].I = newValue;
      CPUUpdateFlags();
    }
    break;
  case 0x121:
    {
      // BX Rm
      // TODO: check if right instruction...
      clockTicks = 3;
      int base = opcode & 0x0F;
      armState = reg[base].I & 1 ? false : true;
      if(armState) {
        reg[15].I = reg[base].I & 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      } else {
        reg[15].I = reg[base].I & 0xFFFFFFFE;
        armNextPC = reg[15].I;
        reg[15].I += 2;
      }
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_CMP, OP_CMP, 0x150);
  case 0x140:
    // MRS Rd, SPSR
    // TODO: check if right instruction...
    reg[(opcode >> 12) & 0x0F].I = reg[17].I;
    break;
  case 0x149:
    {
      // SWPB Rd, Rm, [Rn]
      u32 address = reg[(opcode >> 16) & 15].I;
      u32 temp = CPUReadByte(address);
      CPUWriteByte(address, reg[opcode&15].B.B0);
      reg[(opcode>>12)&15].I = temp;
    }
    break;
    ARITHMETIC_DATA_OPCODE(OP_CMN, OP_CMN, 0x170);
  case 0x160:
    {
      // MSR SPSR_fields, Rm
      u32 value = reg[opcode & 15].I;
      if(armMode > 0x10 && armMode < 0x1f) {
        if(opcode & 0x00010000)
          reg[17].I = (reg[17].I & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          reg[17].I = (reg[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          reg[17].I = (reg[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
        if(opcode & 0x00080000)
          reg[17].I = (reg[17].I & 0x00FFFFFF) | (value & 0xFF000000);
      }
    }
    break;
    LOGICAL_DATA_OPCODE(OP_ORR,  OP_ORR, 0x180);
    LOGICAL_DATA_OPCODE(OP_ORRS, OP_ORR, 0x190);
    LOGICAL_DATA_OPCODE(OP_MOV,  OP_MOV, 0x1a0);
    LOGICAL_DATA_OPCODE(OP_MOVS, OP_MOV, 0x1b0);
    LOGICAL_DATA_OPCODE(OP_BIC,  OP_BIC, 0x1c0);
    LOGICAL_DATA_OPCODE(OP_BICS, OP_BIC, 0x1d0);
    LOGICAL_DATA_OPCODE(OP_MVN,  OP_MVN, 0x1e0);
    LOGICAL_DATA_OPCODE(OP_MVNS, OP_MVN, 0x1f0);
#ifdef BKPT_SUPPORT
  case 0x127:
  case 0x7ff: // for GDB support
    extern void (*dbgSignal)(int,int);
    reg[15].I -= 4;
    armNextPC -= 4;
    dbgSignal(5, (opcode & 0x0f)|((opcode>>4) & 0xfff0));
    return;
#endif
  case 0x320:
  case 0x321:
  case 0x322:
  case 0x323:
  case 0x324:
  case 0x325:
  case 0x326:
  case 0x327:
  case 0x328:
  case 0x329:
  case 0x32a:
  case 0x32b:
  case 0x32c:
  case 0x32d:
  case 0x32e:
  case 0x32f:
    {
      // MSR CPSR_fields, #
      CPUUpdateCPSR();
      u32 value = opcode & 0xFF;
      int shift = (opcode & 0xF00) >> 7;
      if(shift) {
        ROR_IMM_MSR;
      }
      u32 newValue = reg[16].I;
      if(armMode > 0x10) {
        if(opcode & 0x00010000)
          newValue = (newValue & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          newValue = (newValue & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          newValue = (newValue & 0xFF00FFFF) | (value & 0x00FF0000);
      }
      if(opcode & 0x00080000)
        newValue = (newValue & 0x00FFFFFF) | (value & 0xFF000000);
      CPUSwitchMode(newValue & 0x1f, false);
      reg[16].I = newValue;
      CPUUpdateFlags();
    }
    break;
  case 0x360:
  case 0x361:
  case 0x362:
  case 0x363:
  case 0x364:
  case 0x365:
  case 0x366:
  case 0x367:
  case 0x368:
  case 0x369:
  case 0x36a:
  case 0x36b:
  case 0x36c:
  case 0x36d:
  case 0x36e:
  case 0x36f:
    {
      // MSR SPSR_fields, #
      if(armMode > 0x10 && armMode < 0x1f) {
        u32 value = opcode & 0xFF;
        int shift = (opcode & 0xF00) >> 7;
        if(shift) {
          ROR_IMM_MSR;
        }
        if(opcode & 0x00010000)
          reg[17].I = (reg[17].I & 0xFFFFFF00) | (value & 0x000000FF);
        if(opcode & 0x00020000)
          reg[17].I = (reg[17].I & 0xFFFF00FF) | (value & 0x0000FF00);
        if(opcode & 0x00040000)
          reg[17].I = (reg[17].I & 0xFF00FFFF) | (value & 0x00FF0000);
        if(opcode & 0x00080000)
          reg[17].I = (reg[17].I & 0x00FFFFFF) | (value & 0xFF000000);
      }
    }
  break;
  CASE_16(0x400)
  // T versions shouldn't be different on GBA      
  CASE_16(0x420)
    {
      // STR Rd, [Rn], -#
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - offset;
      clockTicks = 2;
    }
    break;
  CASE_16(0x480)
    // T versions shouldn't be different on GBA
  CASE_16(0x4a0)
    {
      // STR Rd, [Rn], #
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + offset;
      clockTicks = 2;
    }
    break;
  CASE_16(0x500)
    {
      // STR Rd, [Rn, -#]
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  CASE_16(0x520)
    {
      // STR Rd, [Rn, -#]!
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  CASE_16(0x580)
    {
      // STR Rd, [Rn, #]
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  CASE_16(0x5a0)
    {
      // STR Rd, [Rn, #]!
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  CASE_16(0x410)
    {
      // LDR Rd, [Rn], -#
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      reg[dest].I = CPUReadMemory(reg[base].I);
      reg[base].I -= offset;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  CASE_16(0x430)
    {
      // LDRT Rd, [Rn], -#
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      reg[dest].I = CPUReadMemory(reg[base].I);
      reg[base].I -= offset;
      clockTicks = 3;
    }
    break;
  CASE_16(0x490)
    {
      // LDR Rd, [Rn], #
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      reg[dest].I = CPUReadMemory(reg[base].I);
      reg[base].I += offset;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  CASE_16(0x4b0)
    {
      // LDRT Rd, [Rn], #
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      reg[dest].I = CPUReadMemory(reg[base].I);
      reg[base].I += offset;
      clockTicks = 3;
    }
    break;
  CASE_16(0x510)
    {
      // LDR Rd, [Rn, -#]
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  CASE_16(0x530)
    {
      // LDR Rd, [Rn, -#]!
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  CASE_16(0x590)
    {
      // LDR Rd, [Rn, #]
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  CASE_16(0x5b0)
    {
      // LDR Rd, [Rn, #]!
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  CASE_16(0x440)
    // T versions shouldn't be different on GBA      
  CASE_16(0x460)
    {
      // STRB Rd, [Rn], -#
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - offset;
      clockTicks = 2;
    }
    break;
  CASE_16(0x4c0)
    // T versions shouldn't be different on GBA
  CASE_16(0x4e0)
    // STRB Rd, [Rn], #
    {
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + offset;
      clockTicks = 2;
    }
    break;
  CASE_16(0x540)
    {
      // STRB Rd, [Rn, -#]
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  CASE_16(0x560)
    {
      // STRB Rd, [Rn, -#]!
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  CASE_16(0x5c0)
    {
      // STRB Rd, [Rn, #]
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  CASE_16(0x5e0)
    {
      // STRB Rd, [Rn, #]!
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  CASE_16(0x450)
    // T versions shouldn't be different
  CASE_16(0x470)
    {
      // LDRB Rd, [Rn], -#
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      reg[dest].I = CPUReadByte(reg[base].I);
      reg[base].I -= offset;
      clockTicks = 3;
    }
    break;
  CASE_16(0x4d0)
  CASE_16(0x4f0) // T versions should not be different
    {
      // LDRB Rd, [Rn], #
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      reg[dest].I = CPUReadByte(reg[base].I);
      reg[base].I += offset;
      clockTicks = 3;
    }
    break;
  CASE_16(0x550)
    {
      // LDRB Rd, [Rn, -#]
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  CASE_16(0x570)
    {
      // LDRB Rd, [Rn, -#]!
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  CASE_16(0x5d0)
    {
      // LDRB Rd, [Rn, #]
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  CASE_16(0x5f0)
    {
      // LDRB Rd, [Rn, #]!
      int offset = opcode & 0xFFF;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x600:
  case 0x608:
    // T versions are the same
  case 0x620:
  case 0x628:
    {
      // STR Rd, [Rn], -Rm, LSL #
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - offset;
      clockTicks = 2;
    }
    break;
  case 0x602:
  case 0x60a:
    // T versions are the same
  case 0x622:
  case 0x62a:
    {
      // STR Rd, [Rn], -Rm, LSR #
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - offset;
      clockTicks = 2;
    }
    break;
  case 0x604:
  case 0x60c:
    // T versions are the same
  case 0x624:
  case 0x62c:
    {
      // STR Rd, [Rn], -Rm, ASR #
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - offset;
      clockTicks = 2;
    }
    break;
  case 0x606:
  case 0x60e:
    // T versions are the same
  case 0x626:
  case 0x62e:
    {
      // STR Rd, [Rn], -Rm, ROR #
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address - value;
      clockTicks = 2;
    }
    break;
  case 0x680:
  case 0x688:
    // T versions are the same
  case 0x6a0:
  case 0x6a8:
    {
      // STR Rd, [Rn], Rm, LSL #
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + offset;
      clockTicks = 2;
    }
    break;
  case 0x682:
  case 0x68a:
    // T versions are the same
  case 0x6a2:
  case 0x6aa:
    {
      // STR Rd, [Rn], Rm, LSR #
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + offset;
      clockTicks = 2;
    }
    break;
  case 0x684:
  case 0x68c:
    // T versions are the same
  case 0x6a4:
  case 0x6ac:
    {
      // STR Rd, [Rn], Rm, ASR #
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + offset;
      clockTicks = 2;
    }
    break;
  case 0x686:
  case 0x68e:
    // T versions are the same
  case 0x6a6:
  case 0x6ae:
    {
      // STR Rd, [Rn], Rm, ROR #
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteMemory(address, reg[dest].I);
      reg[base].I = address + value;
      clockTicks = 2;
    }
    break;
  case 0x700:
  case 0x708:
    {
      // STR Rd, [Rn, -Rm, LSL #]
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x702:
  case 0x70a:
    {
      // STR Rd, [Rn, -Rm, LSR #]
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x704:
  case 0x70c:
    {
      // STR Rd, [Rn, -Rm, ASR #]
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x706:
  case 0x70e:
    {
      // STR Rd, [Rn, -Rm, ROR #]
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - value;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x720:
  case 0x728:
    {
      // STR Rd, [Rn, -Rm, LSL #]!
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x722:
  case 0x72a:
    {
      // STR Rd, [Rn, -Rm, LSR #]!
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;      
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x724:
  case 0x72c:
    {
      // STR Rd, [Rn, -Rm, ASR #]!
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x726:
  case 0x72e:
    {
      // STR Rd, [Rn, -Rm, ROR #]!
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - value;
      reg[base].I = address;      
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x780:
  case 0x788:
    {
      // STR Rd, [Rn, Rm, LSL #]
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x782:
  case 0x78a:
    {
      // STR Rd, [Rn, Rm, LSR #]
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x784:
  case 0x78c:
    {
      // STR Rd, [Rn, Rm, ASR #]
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x786:
  case 0x78e:
    {
      // STR Rd, [Rn, Rm, ROR #]
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + value;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x7a0:
  case 0x7a8:
    {
      // STR Rd, [Rn, Rm, LSL #]!
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x7a2:
  case 0x7aa:
    {
      // STR Rd, [Rn, Rm, LSR #]!
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;      
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x7a4:
  case 0x7ac:
    {
      // STR Rd, [Rn, Rm, ASR #]!
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x7a6:
  case 0x7ae:
    {
      // STR Rd, [Rn, Rm, ROR #]!
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + value;
      reg[base].I = address;      
      CPUWriteMemory(address, reg[dest].I);
      clockTicks = 2;
    }
    break;
  case 0x610:
  case 0x618:
    // T versions are the same
  case 0x630:
  case 0x638:
    {
      // LDR Rd, [Rn], -Rm, LSL #
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      reg[base].I = address - offset;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x612:
  case 0x61a:
    // T versions are the same
  case 0x632:
  case 0x63a:
    {
      // LDR Rd, [Rn], -Rm, LSR #
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      reg[base].I = address - offset;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x614:
  case 0x61c:
    // T versions are the same
  case 0x634:
  case 0x63c:
    {
      // LDR Rd, [Rn], -Rm, ASR #
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      reg[base].I = address - offset;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x616:
  case 0x61e:
    // T versions are the same
  case 0x636:
  case 0x63e:
    {
      // LDR Rd, [Rn], -Rm, ROR #
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      reg[base].I = address - value;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x690:
  case 0x698:
    // T versions are the same
  case 0x6b0:
  case 0x6b8:
    {
      // LDR Rd, [Rn], Rm, LSL #
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      reg[base].I = address + offset;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x692:
  case 0x69a:
    // T versions are the same
  case 0x6b2:
  case 0x6ba:
    {
      // LDR Rd, [Rn], Rm, LSR #
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      reg[base].I = address + offset;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x694:
  case 0x69c:
    // T versions are the same
  case 0x6b4:
  case 0x6bc:
    {
      // LDR Rd, [Rn], Rm, ASR #
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      reg[base].I = address + offset;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x696:
  case 0x69e:
    // T versions are the same
  case 0x6b6:
  case 0x6be:
    {
      // LDR Rd, [Rn], Rm, ROR #
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadMemory(address);
      reg[base].I = address + value;
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x710:
  case 0x718:
    {
      // LDR Rd, [Rn, -Rm, LSL #]
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x712:
  case 0x71a:
    {
      // LDR Rd, [Rn, -Rm, LSR #]
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x714:
  case 0x71c:
    {
      // LDR Rd, [Rn, -Rm, ASR #]
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x716:
  case 0x71e:
    {
      // LDR Rd, [Rn, -Rm, ROR #]
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - value;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x730:
  case 0x738:
    {
      // LDR Rd, [Rn, -Rm, LSL #]!
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x732:
  case 0x73a:
    {
      // LDR Rd, [Rn, -Rm, LSR #]!
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;      
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x734:
  case 0x73c:
    {
      // LDR Rd, [Rn, -Rm, ASR #]!
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x736:
  case 0x73e:
    {
      // LDR Rd, [Rn, -Rm, ROR #]!
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - value;
      reg[base].I = address;      
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x790:
  case 0x798:
    {
      // LDR Rd, [Rn, Rm, LSL #]
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x792:
  case 0x79a:
    {
      // LDR Rd, [Rn, Rm, LSR #]
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x794:
  case 0x79c:
    {
      // LDR Rd, [Rn, Rm, ASR #]
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x796:
  case 0x79e:
    {
      // LDR Rd, [Rn, Rm, ROR #]
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + value;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x7b0:
  case 0x7b8:
    {
      // LDR Rd, [Rn, Rm, LSL #]!
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x7b2:
  case 0x7ba:
    {
      // LDR Rd, [Rn, Rm, LSR #]!
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;      
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x7b4:
  case 0x7bc:
    {
      // LDR Rd, [Rn, Rm, ASR #]!
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x7b6:
  case 0x7be:
    {
      // LDR Rd, [Rn, Rm, ROR #]!
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + value;
      reg[base].I = address;      
      reg[dest].I = CPUReadMemory(address);
      clockTicks = 3;
      if(dest == 15) {
        clockTicks += 2;
        reg[15].I &= 0xFFFFFFFC;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  case 0x640:
  case 0x648:
    // T versions are the same
  case 0x660:
  case 0x668:
    {
      // STRB Rd, [Rn], -Rm, LSL #
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - offset;
      clockTicks = 2;
    }
    break;
  case 0x642:
  case 0x64a:
    // T versions are the same
  case 0x662:
  case 0x66a:
    {
      // STRB Rd, [Rn], -Rm, LSR #
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - offset;
      clockTicks = 2;
    }
    break;
  case 0x644:
  case 0x64c:
    // T versions are the same
  case 0x664:
  case 0x66c:
    {
      // STRB Rd, [Rn], -Rm, ASR #
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - offset;
      clockTicks = 2;
    }
    break;
  case 0x646:
  case 0x64e:
    // T versions are the same
  case 0x666:
  case 0x66e:
    {
      // STRB Rd, [Rn], -Rm, ROR #
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address - value;
      clockTicks = 2;
    }
    break;
  case 0x6c0:
  case 0x6c8:
    // T versions are the same
  case 0x6e0:
  case 0x6e8:
    {
      // STRB Rd, [Rn], Rm, LSL #
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + offset;
      clockTicks = 2;
    }
    break;
  case 0x6c2:
  case 0x6ca:
    // T versions are the same
  case 0x6e2:
  case 0x6ea:
    {
      // STRB Rd, [Rn], Rm, LSR #
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + offset;
      clockTicks = 2;
    }
    break;
  case 0x6c4:
  case 0x6cc:
    // T versions are the same
  case 0x6e4:
  case 0x6ec:
    {
      // STR Rd, [Rn], Rm, ASR #
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + offset;
      clockTicks = 2;
    }
    break;
  case 0x6c6:
  case 0x6ce:
    // T versions are the same
  case 0x6e6:
  case 0x6ee:
    {
      // STRB Rd, [Rn], Rm, ROR #
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      CPUWriteByte(address, reg[dest].B.B0);
      reg[base].I = address + value;
      clockTicks = 2;
    }
    break;
  case 0x740:
  case 0x748:
    {
      // STRB Rd, [Rn, -Rm, LSL #]
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x742:
  case 0x74a:
    {
      // STRB Rd, [Rn, -Rm, LSR #]
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x744:
  case 0x74c:
    {
      // STRB Rd, [Rn, -Rm, ASR #]
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x746:
  case 0x74e:
    {
      // STRB Rd, [Rn, -Rm, ROR #]
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - value;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x760:
  case 0x768:
    {
      // STRB Rd, [Rn, -Rm, LSL #]!
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x762:
  case 0x76a:
    {
      // STRB Rd, [Rn, -Rm, LSR #]!
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;      
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x764:
  case 0x76c:
    {
      // STRB Rd, [Rn, -Rm, ASR #]!
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x766:
  case 0x76e:
    {
      // STRB Rd, [Rn, -Rm, ROR #]!
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - value;
      reg[base].I = address;      
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x7c0:
  case 0x7c8:
    {
      // STRB Rd, [Rn, Rm, LSL #]
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x7c2:
  case 0x7ca:
    {
      // STRB Rd, [Rn, Rm, LSR #]
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x7c4:
  case 0x7cc:
    {
      // STRB Rd, [Rn, Rm, ASR #]
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x7c6:
  case 0x7ce:
    {
      // STRB Rd, [Rn, Rm, ROR #]
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + value;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x7e0:
  case 0x7e8:
    {
      // STRB Rd, [Rn, Rm, LSL #]!
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x7e2:
  case 0x7ea:
    {
      // STRB Rd, [Rn, Rm, LSR #]!
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;      
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x7e4:
  case 0x7ec:
    {
      // STRB Rd, [Rn, Rm, ASR #]!
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x7e6:
  case 0x7ee:
    {
      // STRB Rd, [Rn, Rm, ROR #]!
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + value;
      reg[base].I = address;      
      CPUWriteByte(address, reg[dest].B.B0);
      clockTicks = 2;
    }
    break;
  case 0x650:
  case 0x658:
    // T versions are the same
  case 0x670:
  case 0x678:
    {
      // LDRB Rd, [Rn], -Rm, LSL #
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      reg[base].I = address - offset;
      clockTicks = 3;
    }
    break;
  case 0x652:
  case 0x65a:
    // T versions are the same
  case 0x672:
  case 0x67a:
    {
      // LDRB Rd, [Rn], -Rm, LSR #
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      reg[base].I = address - offset;
      clockTicks = 3;
    }
    break;
  case 0x654:
  case 0x65c:
    // T versions are the same
  case 0x674:
  case 0x67c:
    {
      // LDRB Rd, [Rn], -Rm, ASR #
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      reg[base].I = address - offset;
      clockTicks = 3;
    }
    break;
  case 0x656:
  case 0x65e:
    // T versions are the same
  case 0x676:
  case 0x67e:
    {
      // LDRB Rd, [Rn], -Rm, ROR #
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      reg[base].I = address - value;
      clockTicks = 3;
    }
    break;
  case 0x6d0:
  case 0x6d8:
    // T versions are the same
  case 0x6f0:
  case 0x6f8:
    {
      // LDRB Rd, [Rn], Rm, LSL #
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      reg[base].I = address + offset;
      clockTicks = 3;
    }
    break;
  case 0x6d2:
  case 0x6da:
    // T versions are the same
  case 0x6f2:
  case 0x6fa:
    {
      // LDRB Rd, [Rn], Rm, LSR #
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      reg[base].I = address + offset;
      clockTicks = 3;
    }
    break;
  case 0x6d4:
  case 0x6dc:
    // T versions are the same
  case 0x6f4:
  case 0x6fc:
    {
      // LDRB Rd, [Rn], Rm, ASR #
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      reg[base].I = address + offset;
      clockTicks = 3;
    }
    break;
  case 0x6d6:
  case 0x6de:
    // T versions are the same
  case 0x6f6:
  case 0x6fe:
    {
      // LDRB Rd, [Rn], Rm, ROR #
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I;
      reg[dest].I = CPUReadByte(address);
      reg[base].I = address + value;
      clockTicks = 3;
    }
    break;
  case 0x750:
  case 0x758:
    {
      // LDRB Rd, [Rn, -Rm, LSL #]
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x752:
  case 0x75a:
    {
      // LDRB Rd, [Rn, -Rm, LSR #]
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x754:
  case 0x75c:
    {
      // LDRB Rd, [Rn, -Rm, ASR #]
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x756:
  case 0x75e:
    {
      // LDRB Rd, [Rn, -Rm, ROR #]
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - value;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x770:
  case 0x778:
    {
      // LDRB Rd, [Rn, -Rm, LSL #]!
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x772:
  case 0x77a:
    {
      // LDRB Rd, [Rn, -Rm, LSR #]!
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;      
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x774:
  case 0x77c:
    {
      // LDRB Rd, [Rn, -Rm, ASR #]!
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - offset;
      reg[base].I = address;
      reg[dest].I = CPUReadByte(address);
    }
    clockTicks = 3;            
    break;
  case 0x776:
  case 0x77e:
    {
      // LDRB Rd, [Rn, -Rm, ROR #]!
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I - value;
      reg[base].I = address;      
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x7d0:
  case 0x7d8:
    {
      // LDRB Rd, [Rn, Rm, LSL #]
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x7d2:
  case 0x7da:
    {
      // LDRB Rd, [Rn, Rm, LSR #]
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x7d4:
  case 0x7dc:
    {
      // LDRB Rd, [Rn, Rm, ASR #]
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x7d6:
  case 0x7de:
    {
      // LDRB Rd, [Rn, Rm, ROR #]
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + value;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x7f0:
  case 0x7f8:
    {
      // LDRB Rd, [Rn, Rm, LSL #]!
      int offset = reg[opcode & 15].I << ((opcode>>7)& 31);
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x7f2:
  case 0x7fa:
    {
      // LDRB Rd, [Rn, Rm, LSR #]!
      int shift = (opcode >> 7) & 31;
      int offset = shift ? reg[opcode & 15].I >> shift : 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;      
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x7f4:
  case 0x7fc:
    {
      // LDRB Rd, [Rn, Rm, ASR #]!
      int shift = (opcode >> 7) & 31;
      int offset;
      if(shift)
        offset = (int)((s32)reg[opcode & 15].I >> shift);
      else if(reg[opcode & 15].I & 0x80000000)
        offset = 0xFFFFFFFF;
      else
        offset = 0;
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + offset;
      reg[base].I = address;
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
  case 0x7f6:
  case 0x7fe:
    {
      // LDRB Rd, [Rn, Rm, ROR #]!
      int shift = (opcode >> 7) & 31;
      u32 value = reg[opcode & 15].I;
      if(shift) {
        ROR_VALUE;
      } else {
        RCR_VALUE;
      }
      int dest = (opcode >> 12) & 15;
      int base = (opcode >> 16) & 15;
      u32 address = reg[base].I + value;
      reg[base].I = address;      
      reg[dest].I = CPUReadByte(address);
      clockTicks = 3;
    }
    break;
#define STMDAW_REG(val,num) \
  if(opcode & (val)) {\
    clockTicks++;\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(num)].I);\
    address -= 4;\
    if(!offset)\
      reg[base].I = temp;\
    offset = 1;\
  }
#define STMDA_REG(val,num) \
  if(opcode & (val)) {\
    clockTicks++;\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(num)].I);\
    address -= 4;\
  }
    
  CASE_16(0x800)
    // STMDA Rn, {Rlist}
    {
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      if(opcode & 32768) {
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        address -= 4;
      }
      STMDA_REG(16384, 14);
      STMDA_REG(8192, 13);
      STMDA_REG(4096, 12);
      STMDA_REG(2048, 11);
      STMDA_REG(1024, 10);
      STMDA_REG(512, 9);
      STMDA_REG(256, 8);
      STMDA_REG(128, 7);
      STMDA_REG(64, 6);
      STMDA_REG(32, 5);
      STMDA_REG(16, 4);
      STMDA_REG(8, 3);
      STMDA_REG(4, 2);
      STMDA_REG(2, 1);
      STMDA_REG(1, 0);
    }
    break;
  CASE_16(0x820)
    {
      // STMDA Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      int offset = 0;
      u32 temp = address - 4*(cpuBitsSet[opcode & 0xFF] +
                              cpuBitsSet[(opcode >> 8) & 255]);
      if(opcode & 32768) {
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        reg[base].I = temp;
        offset = 1;
        address -= 4;
      }
      STMDAW_REG(16384, 14);
      STMDAW_REG(8192, 13);
      STMDAW_REG(4096, 12);
      STMDAW_REG(2048, 11);
      STMDAW_REG(1024, 10);
      STMDAW_REG(512, 9);
      STMDAW_REG(256, 8);
      STMDAW_REG(128, 7);
      STMDAW_REG(64, 6);
      STMDAW_REG(32, 5);
      STMDAW_REG(16, 4);
      STMDAW_REG(8, 3);
      STMDAW_REG(4, 2);
      STMDAW_REG(2, 1);
      STMDAW_REG(1, 0);
    }
  break;
  CASE_16(0x840)
    {
      // STMDA Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      if(opcode & 32768) {
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        address -= 4;
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMDA_REG(16384, R14_USR);
        STMDA_REG(8192, R13_USR);
      } else {
        STMDA_REG(16384, 14);
        STMDA_REG(8192, 13);
      }
      if(armMode == 0x11) {
        STMDA_REG(4096, R12_FIQ);
        STMDA_REG(2048, R11_FIQ);
        STMDA_REG(1024, R10_FIQ);
        STMDA_REG(512, R9_FIQ);
        STMDA_REG(256, R8_FIQ);
      } else {
        STMDA_REG(4096, 12);
        STMDA_REG(2048, 11);
        STMDA_REG(1024, 10);
        STMDA_REG(512, 9);
        STMDA_REG(256, 8);
      }
      STMDA_REG(128, 7);
      STMDA_REG(64, 6);
      STMDA_REG(32, 5);
      STMDA_REG(16, 4);
      STMDA_REG(8, 3);
      STMDA_REG(4, 2);
      STMDA_REG(2, 1);
      STMDA_REG(1, 0);
    }
    break;
  CASE_16(0x860)
    {
      // STMDA Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      int offset = 0;
      u32 temp = address - 4*(cpuBitsSet[opcode & 0xFF] +
                              cpuBitsSet[(opcode >> 8) & 255]);    
      if(opcode & 32768) {
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        address -= 4;
        reg[base].I = temp;
        offset = 1;      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMDAW_REG(16384, R14_USR);
        STMDAW_REG(8192, R13_USR);
      } else {
        STMDAW_REG(16384, 14);
        STMDAW_REG(8192, 13);
      }
      if(armMode == 0x11) {
        STMDAW_REG(4096, R12_FIQ);
        STMDAW_REG(2048, R11_FIQ);
        STMDAW_REG(1024, R10_FIQ);
        STMDAW_REG(512, R9_FIQ);
        STMDAW_REG(256, R8_FIQ);
      } else {
        STMDAW_REG(4096, 12);
        STMDAW_REG(2048, 11);
        STMDAW_REG(1024, 10);
        STMDAW_REG(512, 9);
        STMDAW_REG(256, 8);
      }
      STMDAW_REG(128, 7);
      STMDAW_REG(64, 6);
      STMDAW_REG(32, 5);
      STMDAW_REG(16, 4);
      STMDAW_REG(8, 3);
      STMDAW_REG(4, 2);
      STMDAW_REG(2, 1);
      STMDAW_REG(1, 0);
    }
    break;
  
#define STMIA_REG(val,num) \
  if(opcode & (val)) {\
    clockTicks++;\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(num)].I);\
    address += 4;\
  }
#define STMIAW_REG(val,num) \
  if(opcode & (val)) {\
    clockTicks++;\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(num)].I);\
    address += 4;\
    if(!offset)\
      reg[base].I = temp;\
    offset = 1;\
  }  
  
  CASE_16(0x880)
    {
      // STMIA Rn, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      STMIA_REG(1, 0);
      STMIA_REG(2, 1);
      STMIA_REG(4, 2);
      STMIA_REG(8, 3);
      STMIA_REG(16, 4);
      STMIA_REG(32, 5);
      STMIA_REG(64, 6);
      STMIA_REG(128, 7);
      STMIA_REG(256, 8);
      STMIA_REG(512, 9);
      STMIA_REG(1024, 10);
      STMIA_REG(2048, 11);
      STMIA_REG(4096, 12);
      STMIA_REG(8192, 13);
      STMIA_REG(16384, 14);
      if(opcode & 32768) {
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        address += 4;
      }
    }
  break;
  CASE_16(0x8a0)
    {
      // STMIA Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      int offset = 0;
      u32 temp = address + 4*(cpuBitsSet[opcode & 0xFF] +
                              cpuBitsSet[(opcode >> 8) & 255]);
      STMIAW_REG(1, 0);
      STMIAW_REG(2, 1);
      STMIAW_REG(4, 2);
      STMIAW_REG(8, 3);
      STMIAW_REG(16, 4);
      STMIAW_REG(32, 5);
      STMIAW_REG(64, 6);
      STMIAW_REG(128, 7);
      STMIAW_REG(256, 8);
      STMIAW_REG(512, 9);
      STMIAW_REG(1024, 10);
      STMIAW_REG(2048, 11);
      STMIAW_REG(4096, 12);
      STMIAW_REG(8192, 13);
      STMIAW_REG(16384, 14);
      if(opcode & 32768) {
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        if(!offset)
          reg[base].I = temp;
        address += 4;
      }
    }
  break;
  CASE_16(0x8c0)
    {
      // STMIA Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      STMIA_REG(1, 0);
      STMIA_REG(2, 1);
      STMIA_REG(4, 2);
      STMIA_REG(8, 3);
      STMIA_REG(16, 4);
      STMIA_REG(32, 5);
      STMIA_REG(64, 6);
      STMIA_REG(128, 7);
      if(armMode == 0x11) {
        STMIA_REG(256, R8_FIQ);
        STMIA_REG(512, R9_FIQ);
        STMIA_REG(1024, R10_FIQ);
        STMIA_REG(2048, R11_FIQ);
        STMIA_REG(4096, R12_FIQ);
      } else {
        STMIA_REG(256, 8);
        STMIA_REG(512, 9);
        STMIA_REG(1024, 10);
        STMIA_REG(2048, 11);
        STMIA_REG(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMIA_REG(8192, R13_USR);
        STMIA_REG(16384, R14_USR);
      } else {
        STMIA_REG(8192, 13);
        STMIA_REG(16384, 14);
      }
      if(opcode & 32768) {
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        address += 4;
      }
    }
    break;
  CASE_16(0x8e0)
    {
      // STMIA Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      int offset = 0;
      u32 temp = address + 4*(cpuBitsSet[opcode & 0xFF] +
                              cpuBitsSet[(opcode >> 8) & 255]);    
      STMIAW_REG(1, 0);
      STMIAW_REG(2, 1);
      STMIAW_REG(4, 2);
      STMIAW_REG(8, 3);
      STMIAW_REG(16, 4);
      STMIAW_REG(32, 5);
      STMIAW_REG(64, 6);
      STMIAW_REG(128, 7);
      if(armMode == 0x11) {
        STMIAW_REG(256, R8_FIQ);
        STMIAW_REG(512, R9_FIQ);
        STMIAW_REG(1024, R10_FIQ);
        STMIAW_REG(2048, R11_FIQ);
        STMIAW_REG(4096, R12_FIQ);
      } else {
        STMIAW_REG(256, 8);
        STMIAW_REG(512, 9);
        STMIAW_REG(1024, 10);
        STMIAW_REG(2048, 11);
        STMIAW_REG(4096, 12);      
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMIA_REG(8192, R13_USR);
        STMIA_REG(16384, R14_USR);
      } else {
        STMIA_REG(8192, 13);
        STMIA_REG(16384, 14);
      }
      if(opcode & 32768) {
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        address += 4;
        if(!offset)
          reg[base].I = temp;      
      }
    }
    break;
    
#define STMDB_REG(val,num) \
  if(opcode & (val)) {\
    address -= 4;\
    clockTicks++;\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(num)].I);\
  }
#define STMDBW_REG(val,num) \
  if(opcode & (val)) {\
    address -= 4;\
    clockTicks++;\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(num)].I);\
    if(!offset)\
      reg[base].I = temp;\
    offset = 1;\
  }  
  
  CASE_16(0x900)
    {
      // STMDB Rn, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      if(opcode & 32768) {
        address -= 4;      
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
      }
      STMDB_REG(16384, 14);
      STMDB_REG(8192, 13);
      STMDB_REG(4096, 12);
      STMDB_REG(2048, 11);
      STMDB_REG(1024, 10);
      STMDB_REG(512, 9);
      STMDB_REG(256, 8);
      STMDB_REG(128, 7);
      STMDB_REG(64, 6);
      STMDB_REG(32, 5);
      STMDB_REG(16, 4);
      STMDB_REG(8, 3);
      STMDB_REG(4, 2);
      STMDB_REG(2, 1);
      STMDB_REG(1, 0);
    }
    break;
  CASE_16(0x920)
    {
      // STMDB Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      int offset = 0;
      u32 temp = address - 4*(cpuBitsSet[opcode & 0xFF] +
                              cpuBitsSet[(opcode >> 8) & 255]);
      if(opcode & 32768) {
        address -= 4;      
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        reg[base].I = temp;
        offset = 1;
      }
      STMDBW_REG(16384, 14);
      STMDBW_REG(8192, 13);
      STMDBW_REG(4096, 12);
      STMDBW_REG(2048, 11);
      STMDBW_REG(1024, 10);
      STMDBW_REG(512, 9);
      STMDBW_REG(256, 8);
      STMDBW_REG(128, 7);
      STMDBW_REG(64, 6);
      STMDBW_REG(32, 5);
      STMDBW_REG(16, 4);
      STMDBW_REG(8, 3);
      STMDBW_REG(4, 2);
      STMDBW_REG(2, 1);
      STMDBW_REG(1, 0);
    }
  break;
  CASE_16(0x940)
    {
      // STMDB Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      if(opcode & 32768) {
        address -= 4;      
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMDB_REG(16384, R14_USR);
        STMDB_REG(8192, R13_USR);
      } else {
        STMDB_REG(16384, 14);
        STMDB_REG(8192, 13);
      }
      if(armMode == 0x11) {
        STMDB_REG(4096, R12_FIQ);
        STMDB_REG(2048, R11_FIQ);
        STMDB_REG(1024, R10_FIQ);
        STMDB_REG(512, R9_FIQ);
        STMDB_REG(256, R8_FIQ);
      } else {
        STMDB_REG(4096, 12);
        STMDB_REG(2048, 11);
        STMDB_REG(1024, 10);
        STMDB_REG(512, 9);
        STMDB_REG(256, 8);
      }
      STMDB_REG(128, 7);
      STMDB_REG(64, 6);
      STMDB_REG(32, 5);
      STMDB_REG(16, 4);
      STMDB_REG(8, 3);
      STMDB_REG(4, 2);
      STMDB_REG(2, 1);
      STMDB_REG(1, 0);
    }
    break;
  CASE_16(0x960)
    {
      // STMDB Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      int offset = 0;
      u32 temp = address - 4*(cpuBitsSet[opcode & 0xFF] +
                              cpuBitsSet[(opcode >> 8) & 255]);
      if(opcode & 32768) {
        address -= 4;
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        reg[base].I = temp;
        offset = 1;
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMDBW_REG(16384, R14_USR);
        STMDBW_REG(8192, R13_USR);
      } else {
        STMDBW_REG(16384, 14);
        STMDBW_REG(8192, 13);
      }
      if(armMode == 0x11) {
        STMDBW_REG(4096, R12_FIQ);
        STMDBW_REG(2048, R11_FIQ);
        STMDBW_REG(1024, R10_FIQ);
        STMDBW_REG(512, R9_FIQ);
        STMDBW_REG(256, R8_FIQ);
      } else {
        STMDBW_REG(4096, 12);
        STMDBW_REG(2048, 11);
        STMDBW_REG(1024, 10);
        STMDBW_REG(512, 9);
        STMDBW_REG(256, 8);
      }
      STMDBW_REG(128, 7);
      STMDBW_REG(64, 6);
      STMDBW_REG(32, 5);
      STMDBW_REG(16, 4);
      STMDBW_REG(8, 3);
      STMDBW_REG(4, 2);
      STMDBW_REG(2, 1);
      STMDBW_REG(1, 0);
    }
    break;

#define STMIB_REG(val,num) \
  if(opcode & (val)) {\
    address += 4;\
    clockTicks++;\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(num)].I);\
  }
  
#define STMIBW_REG(val,num) \
  if(opcode & (val)) {\
    address += 4;\
    clockTicks++;\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(num)].I);\
    if(!offset)\
      reg[base].I = temp;\
    offset = 1;\
  }  
  
  CASE_16(0x980)
    // STMIB Rn, {Rlist}
    {
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      STMIB_REG(1, 0);
      STMIB_REG(2, 1);
      STMIB_REG(4, 2);
      STMIB_REG(8, 3);
      STMIB_REG(16, 4);
      STMIB_REG(32, 5);
      STMIB_REG(64, 6);
      STMIB_REG(128, 7);
      STMIB_REG(256, 8);
      STMIB_REG(512, 9);
      STMIB_REG(1024, 10);
      STMIB_REG(2048, 11);
      STMIB_REG(4096, 12);
      STMIB_REG(8192, 13);
      STMIB_REG(16384, 14);
      if(opcode & 32768) {
        address += 4;      
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
      }
    }
    break;
  CASE_16(0x9a0)
    {
      // STMIB Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      int offset = 0;
      u32 temp = address + 4*(cpuBitsSet[opcode & 0xFF] +
                              cpuBitsSet[(opcode >> 8) & 255]);
      STMIBW_REG(1, 0);
      STMIBW_REG(2, 1);
      STMIBW_REG(4, 2);
      STMIBW_REG(8, 3);
      STMIBW_REG(16, 4);
      STMIBW_REG(32, 5);
      STMIBW_REG(64, 6);
      STMIBW_REG(128, 7);
      STMIBW_REG(256, 8);
      STMIBW_REG(512, 9);
      STMIBW_REG(1024, 10);
      STMIBW_REG(2048, 11);
      STMIBW_REG(4096, 12);
      STMIBW_REG(8192, 13);
      STMIBW_REG(16384, 14);
      if(opcode & 32768) {
        address += 4;      
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        if(!offset)
          reg[base].I = temp;
      }
    }
    break;
  CASE_16(0x9c0)
    {
      // STMIB Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      STMIB_REG(1, 0);
      STMIB_REG(2, 1);
      STMIB_REG(4, 2);
      STMIB_REG(8, 3);
      STMIB_REG(16, 4);
      STMIB_REG(32, 5);
      STMIB_REG(64, 6);
      STMIB_REG(128, 7);
      if(armMode == 0x11) {
        STMIB_REG(256, R8_FIQ);
        STMIB_REG(512, R9_FIQ);
        STMIB_REG(1024, R10_FIQ);
        STMIB_REG(2048, R11_FIQ);
        STMIB_REG(4096, R12_FIQ);
      } else {
        STMIB_REG(256, 8);
        STMIB_REG(512, 9);
        STMIB_REG(1024, 10);
        STMIB_REG(2048, 11);
        STMIB_REG(4096, 12);
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMIB_REG(8192, R13_USR);
        STMIB_REG(16384, R14_USR);
      } else {
        STMIB_REG(8192, 13);
        STMIB_REG(16384, 14);
      }
      if(opcode & 32768) {
        address += 4;      
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
      }
    }
    break;
  CASE_16(0x9e0)
    {
      // STMIB Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      int offset = 0;
      u32 temp = address + 4*(cpuBitsSet[opcode & 0xFF] +
                              cpuBitsSet[(opcode >> 8) & 255]);    
      STMIBW_REG(1, 0);
      STMIBW_REG(2, 1);
      STMIBW_REG(4, 2);
      STMIBW_REG(8, 3);
      STMIBW_REG(16, 4);
      STMIBW_REG(32, 5);
      STMIBW_REG(64, 6);
      STMIBW_REG(128, 7);
      if(armMode == 0x11) {
        STMIBW_REG(256, R8_FIQ);
        STMIBW_REG(512, R9_FIQ);
        STMIBW_REG(1024, R10_FIQ);
        STMIBW_REG(2048, R11_FIQ);
        STMIBW_REG(4096, R12_FIQ);
      } else {
        STMIBW_REG(256, 8);
        STMIBW_REG(512, 9);
        STMIBW_REG(1024, 10);
        STMIBW_REG(2048, 11);
        STMIBW_REG(4096, 12);
      }
      if(armMode != 0x10 && armMode != 0x1f) {
        STMIBW_REG(8192, R13_USR);
        STMIBW_REG(16384, R14_USR);
      } else {
        STMIBW_REG(8192, 13);
        STMIBW_REG(16384, 14);
      }
      if(opcode & 32768) {
        address += 4;      
        clockTicks++;
        CPUWriteMemory(address & 0xFFFFFFFC, reg[15].I+4);
        if(!offset)
          reg[base].I = temp;      
      }
    }
    break;
    
#define LDMDA_REG(val,num) \
  if(opcode & (val)) {\
    clockTicks++;\
    reg[(num)].I = CPUReadMemory(address & 0xFFFFFFFC);\
    address -= 4;\
  }
  CASE_16(0x810)
    {
      // LDMDA Rn, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      if(opcode & 32768) {
        clockTicks += 2;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        armNextPC = reg[15].I;
        reg[15].I += 4;
        address -= 4;
      }
      LDMDA_REG(16384, 14);
      LDMDA_REG(8192, 13);
      LDMDA_REG(4096, 12);
      LDMDA_REG(2048, 11);
      LDMDA_REG(1024, 10);
      LDMDA_REG(512, 9);
      LDMDA_REG(256, 8);
      LDMDA_REG(128, 7);
      LDMDA_REG(64, 6);
      LDMDA_REG(32, 5);
      LDMDA_REG(16, 4);
      LDMDA_REG(8, 3);
      LDMDA_REG(4, 2);
      LDMDA_REG(2, 1);
      LDMDA_REG(1, 0);
    }
    break;
  CASE_16(0x830)
    {
      // LDMDA Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      if(opcode & 32768) {
        clockTicks += 2;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        armNextPC = reg[15].I;
        reg[15].I += 4;
        address -= 4;
      }
      LDMDA_REG(16384, 14);
      LDMDA_REG(8192, 13);
      LDMDA_REG(4096, 12);
      LDMDA_REG(2048, 11);
      LDMDA_REG(1024, 10);
      LDMDA_REG(512, 9);
      LDMDA_REG(256, 8);
      LDMDA_REG(128, 7);
      LDMDA_REG(64, 6);
      LDMDA_REG(32, 5);
      LDMDA_REG(16, 4);
      LDMDA_REG(8, 3);
      LDMDA_REG(4, 2);
      LDMDA_REG(2, 1);
      LDMDA_REG(1, 0);
      if(!(opcode & (1 << base)))
        reg[base].I = address;
    }
    break;
  CASE_16(0x850)
    {
      // LDMDA Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      if(opcode & 0x8000) {
        clockTicks = 4;      
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        address -= 4;
        LDMDA_REG(16384, 14);
        LDMDA_REG(8192, 13);
        LDMDA_REG(4096, 12);
        LDMDA_REG(2048, 11);
        LDMDA_REG(1024, 10);
        LDMDA_REG(512, 9);
        LDMDA_REG(256, 8);
        LDMDA_REG(128, 7);
        LDMDA_REG(64, 6);
        LDMDA_REG(32, 5);
        LDMDA_REG(16, 4);
        LDMDA_REG(8, 3);
        LDMDA_REG(4, 2);
        LDMDA_REG(2, 1);
        LDMDA_REG(1, 0);
        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
        }
      } else {
        clockTicks = 2;
        if(armMode != 0x10 && armMode != 0x1f) {
          LDMDA_REG(16384, R14_USR);
          LDMDA_REG(8192, R13_USR);
        } else {
          LDMDA_REG(16384, 14);
          LDMDA_REG(8192, 13);
        }
        if(armMode == 0x11) {
          LDMDA_REG(4096, R12_FIQ);
          LDMDA_REG(2048, R11_FIQ);
          LDMDA_REG(1024, R10_FIQ);
          LDMDA_REG(512, R9_FIQ);
          LDMDA_REG(256, R8_FIQ);
        } else {
          LDMDA_REG(4096, 12);
          LDMDA_REG(2048, 11);
          LDMDA_REG(1024, 10);
          LDMDA_REG(512, 9);
          LDMDA_REG(256, 8);        
        }
        LDMDA_REG(128, 7);
        LDMDA_REG(64, 6);
        LDMDA_REG(32, 5);
        LDMDA_REG(16, 4);
        LDMDA_REG(8, 3);
        LDMDA_REG(4, 2);
        LDMDA_REG(2, 1);
        LDMDA_REG(1, 0);      
      }
    }
    break;
  CASE_16(0x870)
    {
      // LDMDA Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      if(opcode & 0x8000) {
        clockTicks = 4;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        address -= 4;
        LDMDA_REG(16384, 14);
        LDMDA_REG(8192, 13);
        LDMDA_REG(4096, 12);
        LDMDA_REG(2048, 11);
        LDMDA_REG(1024, 10);
        LDMDA_REG(512, 9);
        LDMDA_REG(256, 8);
        LDMDA_REG(128, 7);
        LDMDA_REG(64, 6);
        LDMDA_REG(32, 5);
        LDMDA_REG(16, 4);
        LDMDA_REG(8, 3);
        LDMDA_REG(4, 2);
        LDMDA_REG(2, 1);
        LDMDA_REG(1, 0);
        if(!(opcode & (1 << base)))    
          reg[base].I = address;
        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
        }
      } else {
        clockTicks = 4;
        if(armMode != 0x10 && armMode != 0x1f) {
          LDMDA_REG(16384, R14_USR);
          LDMDA_REG(8192, R13_USR);
        } else {
          LDMDA_REG(16384, 14);
          LDMDA_REG(8192, 13);
        }
        if(armMode == 0x11) {
          LDMDA_REG(4096, R12_FIQ);
          LDMDA_REG(2048, R11_FIQ);
          LDMDA_REG(1024, R10_FIQ);
          LDMDA_REG(512, R9_FIQ);
          LDMDA_REG(256, R8_FIQ);
        } else {
          LDMDA_REG(4096, 12);
          LDMDA_REG(2048, 11);
          LDMDA_REG(1024, 10);
          LDMDA_REG(512, 9);
          LDMDA_REG(256, 8);
        }
        LDMDA_REG(128, 7);
        LDMDA_REG(64, 6);
        LDMDA_REG(32, 5);
        LDMDA_REG(16, 4);
        LDMDA_REG(8, 3);
        LDMDA_REG(4, 2);
        LDMDA_REG(2, 1);
        LDMDA_REG(1, 0);
        if(!(opcode & (1 << base)))    
          reg[base].I = address;
      }
    }
    break;
    
#define LDMIA_REG(val,num) \
  if(opcode & (val)) {\
    reg[(num)].I = CPUReadMemory(address & 0xFFFFFFFC);\
    clockTicks++;\
    address += 4;\
  }
  CASE_16(0x890)
    {
      // LDMIA Rn, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      LDMIA_REG(1, 0);
      LDMIA_REG(2, 1);
      LDMIA_REG(4, 2);
      LDMIA_REG(8, 3);
      LDMIA_REG(16, 4);
      LDMIA_REG(32, 5);
      LDMIA_REG(64, 6);
      LDMIA_REG(128, 7);
      LDMIA_REG(256, 8);
      LDMIA_REG(512, 9);
      LDMIA_REG(1024, 10);
      LDMIA_REG(2048, 11);
      LDMIA_REG(4096, 12);
      LDMIA_REG(8192, 13);
      LDMIA_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        clockTicks += 2;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        address += 4;
      }
    }
  break;
  CASE_16(0x8b0)
    {
      // LDMIA Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      LDMIA_REG(1, 0);
      LDMIA_REG(2, 1);
      LDMIA_REG(4, 2);
      LDMIA_REG(8, 3);
      LDMIA_REG(16, 4);
      LDMIA_REG(32, 5);
      LDMIA_REG(64, 6);
      LDMIA_REG(128, 7);
      LDMIA_REG(256, 8);
      LDMIA_REG(512, 9);
      LDMIA_REG(1024, 10);
      LDMIA_REG(2048, 11);
      LDMIA_REG(4096, 12);
      LDMIA_REG(8192, 13);
      LDMIA_REG(16384, 14);
      if(opcode & 32768) {
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        clockTicks += 2;
        armNextPC = reg[15].I;
        reg[15].I += 4;
        address += 4;
      }
      if(!(opcode & (1 << base)))    
        reg[base].I = address;
    }
    break;
  CASE_16(0x8d0)
    {
      // LDMIA Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      if(opcode & 0x8000) {
        clockTicks = 4;
        LDMIA_REG(1, 0);
        LDMIA_REG(2, 1);
        LDMIA_REG(4, 2);
        LDMIA_REG(8, 3);
        LDMIA_REG(16, 4);
        LDMIA_REG(32, 5);
        LDMIA_REG(64, 6);
        LDMIA_REG(128, 7);
        LDMIA_REG(256, 8);
        LDMIA_REG(512, 9);
        LDMIA_REG(1024, 10);
        LDMIA_REG(2048, 11);
        LDMIA_REG(4096, 12);
        LDMIA_REG(8192, 13);
        LDMIA_REG(16384, 14);
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        address += 4;
        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
        }
      } else {
        clockTicks = 2;
        LDMIA_REG(1, 0);
        LDMIA_REG(2, 1);
        LDMIA_REG(4, 2);
        LDMIA_REG(8, 3);
        LDMIA_REG(16, 4);
        LDMIA_REG(32, 5);
        LDMIA_REG(64, 6);
        LDMIA_REG(128, 7);
        if(armMode == 0x11) {
          LDMIA_REG(256, R8_FIQ);
          LDMIA_REG(512, R9_FIQ);
          LDMIA_REG(1024, R10_FIQ);
          LDMIA_REG(2048, R11_FIQ);
          LDMIA_REG(4096, R12_FIQ);
        } else {
          LDMIA_REG(256, 8);
          LDMIA_REG(512, 9);
          LDMIA_REG(1024, 10);
          LDMIA_REG(2048, 11);
          LDMIA_REG(4096, 12);
        }
        if(armMode != 0x10 && armMode != 0x1f) {
          LDMIA_REG(8192, R13_USR);
          LDMIA_REG(16384, R14_USR);
        } else {
          LDMIA_REG(8192, 13);
          LDMIA_REG(16384, 14);
        }
      }
    }
  break;
  CASE_16(0x8f0)
    {
      // LDMIA Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      if(opcode & 0x8000) {
        clockTicks = 4;
        LDMIA_REG(1, 0);
        LDMIA_REG(2, 1);
        LDMIA_REG(4, 2);
        LDMIA_REG(8, 3);
        LDMIA_REG(16, 4);
        LDMIA_REG(32, 5);
        LDMIA_REG(64, 6);
        LDMIA_REG(128, 7);
        LDMIA_REG(256, 8);
        LDMIA_REG(512, 9);
        LDMIA_REG(1024, 10);
        LDMIA_REG(2048, 11);
        LDMIA_REG(4096, 12);
        LDMIA_REG(8192, 13);
        LDMIA_REG(16384, 14);
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        address += 4;
        if(!(opcode & (1 << base)))    
          reg[base].I = address;    
        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
        }
      } else {
        clockTicks = 4;
        LDMIA_REG(1, 0);
        LDMIA_REG(2, 1);
        LDMIA_REG(4, 2);
        LDMIA_REG(8, 3);
        LDMIA_REG(16, 4);
        LDMIA_REG(32, 5);
        LDMIA_REG(64, 6);
        LDMIA_REG(128, 7);
        if(armMode == 0x11) {
          LDMIA_REG(256, R8_FIQ);
          LDMIA_REG(512, R9_FIQ);
          LDMIA_REG(1024, R10_FIQ);
          LDMIA_REG(2048, R11_FIQ);
          LDMIA_REG(4096, R12_FIQ);
        } else {
          LDMIA_REG(256, 8);
          LDMIA_REG(512, 9);
          LDMIA_REG(1024, 10);
          LDMIA_REG(2048, 11);
          LDMIA_REG(4096, 12);
        }
        if(armMode != 0x10 && armMode != 0x1f) {
          LDMIA_REG(8192, R13_USR);
          LDMIA_REG(16384, R14_USR);
        } else {
          LDMIA_REG(8192, 13);
          LDMIA_REG(16384, 14);
        }
        if(!(opcode & (1 << base)))
          reg[base].I = address;
      }
    }
    break;
    
#define LDMDB_REG(val,num) \
  if(opcode & (val)) {\
    address -= 4;\
    clockTicks++;\
    reg[(num)].I = CPUReadMemory(address & 0xFFFFFFFC);\
  }
  CASE_16(0x910)
    {
      // LDMDB Rn, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      if(opcode & 32768) {
        address -= 4;
        clockTicks += 2;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
      LDMDB_REG(16384, 14);
      LDMDB_REG(8192, 13);
      LDMDB_REG(4096, 12);
      LDMDB_REG(2048, 11);
      LDMDB_REG(1024, 10);
      LDMDB_REG(512, 9);
      LDMDB_REG(256, 8);
      LDMDB_REG(128, 7);
      LDMDB_REG(64, 6);
      LDMDB_REG(32, 5);
      LDMDB_REG(16, 4);
      LDMDB_REG(8, 3);
      LDMDB_REG(4, 2);
      LDMDB_REG(2, 1);
      LDMDB_REG(1, 0);
    }
    break;
  CASE_16(0x930)
    {
      // LDMDB Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      if(opcode & 32768) {
        address -= 4;
        clockTicks += 2;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
      LDMDB_REG(16384, 14);
      LDMDB_REG(8192, 13);
      LDMDB_REG(4096, 12);
      LDMDB_REG(2048, 11);
      LDMDB_REG(1024, 10);
      LDMDB_REG(512, 9);
      LDMDB_REG(256, 8);
      LDMDB_REG(128, 7);
      LDMDB_REG(64, 6);
      LDMDB_REG(32, 5);
      LDMDB_REG(16, 4);
      LDMDB_REG(8, 3);
      LDMDB_REG(4, 2);
      LDMDB_REG(2, 1);
      LDMDB_REG(1, 0);
      if(!(opcode & (1 << base)))    
        reg[base].I = address;
    }
    break;
  CASE_16(0x950)
    {
      // LDMDB Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      if(opcode & 0x8000) {
        clockTicks = 4;
        address -= 4;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        LDMDB_REG(16384, 14);
        LDMDB_REG(8192, 13);
        LDMDB_REG(4096, 12);
        LDMDB_REG(2048, 11);
        LDMDB_REG(1024, 10);
        LDMDB_REG(512, 9);
        LDMDB_REG(256, 8);
        LDMDB_REG(128, 7);
        LDMDB_REG(64, 6);
        LDMDB_REG(32, 5);
        LDMDB_REG(16, 4);
        LDMDB_REG(8, 3);
        LDMDB_REG(4, 2);
        LDMDB_REG(2, 1);
        LDMDB_REG(1, 0);
        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
        }
      } else {
        clockTicks = 2;
        if(armMode != 0x10 && armMode != 0x1f) {
          LDMDB_REG(16384, R14_USR);
          LDMDB_REG(8192, R13_USR);
        } else {
          LDMDB_REG(16384, 14);
          LDMDB_REG(8192, 13);
        }
        if(armMode == 0x11) {
          LDMDB_REG(4096, R12_FIQ);
          LDMDB_REG(2048, R11_FIQ);
          LDMDB_REG(1024, R10_FIQ);
          LDMDB_REG(512, R9_FIQ);
          LDMDB_REG(256, R8_FIQ);        
        } else {
          LDMDB_REG(4096, 12);
          LDMDB_REG(2048, 11);
          LDMDB_REG(1024, 10);
          LDMDB_REG(512, 9);
          LDMDB_REG(256, 8);
        }
        LDMDB_REG(128, 7);
        LDMDB_REG(64, 6);
        LDMDB_REG(32, 5);
        LDMDB_REG(16, 4);
        LDMDB_REG(8, 3);
        LDMDB_REG(4, 2);
        LDMDB_REG(2, 1);
        LDMDB_REG(1, 0);
      }
    }
    break;
  CASE_16(0x970)
    {
      // LDMDB Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      if(opcode & 0x8000) {
        clockTicks = 4;
        address -= 4;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        LDMDB_REG(16384, 14);
        LDMDB_REG(8192, 13);
        LDMDB_REG(4096, 12);
        LDMDB_REG(2048, 11);
        LDMDB_REG(1024, 10);
        LDMDB_REG(512, 9);
        LDMDB_REG(256, 8);
        LDMDB_REG(128, 7);
        LDMDB_REG(64, 6);
        LDMDB_REG(32, 5);
        LDMDB_REG(16, 4);
        LDMDB_REG(8, 3);
        LDMDB_REG(4, 2);
        LDMDB_REG(2, 1);
        LDMDB_REG(1, 0);
        if(!(opcode & (1 << base)))    
          reg[base].I = address;
        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
        }
      } else {
        clockTicks = 4;
        if(armMode != 0x10 && armMode != 0x1f) {
          LDMDB_REG(16384, R14_USR);
          LDMDB_REG(8192, R13_USR);
        } else {
          LDMDB_REG(16384, 14);
          LDMDB_REG(8192, 13);
        }
        if(armMode == 0x11) {
          LDMDB_REG(4096, R12_FIQ);
          LDMDB_REG(2048, R11_FIQ);
          LDMDB_REG(1024, R10_FIQ);
          LDMDB_REG(512, R9_FIQ);
          LDMDB_REG(256, R8_FIQ);
        } else {
          LDMDB_REG(4096, 12);
          LDMDB_REG(2048, 11);
          LDMDB_REG(1024, 10);
          LDMDB_REG(512, 9);
          LDMDB_REG(256, 8);
        }
        LDMDB_REG(128, 7);
        LDMDB_REG(64, 6);
        LDMDB_REG(32, 5);
        LDMDB_REG(16, 4);
        LDMDB_REG(8, 3);
        LDMDB_REG(4, 2);
        LDMDB_REG(2, 1);
        LDMDB_REG(1, 0);
        if(!(opcode & (1 << base)))    
          reg[base].I = address;
      }
    }
    break;
    
#define LDMIB_REG(val,num) \
  if(opcode & (val)) {\
    address += 4;\
    reg[(num)].I = CPUReadMemory(address & 0xFFFFFFFC);\
    clockTicks++;\
  }
  CASE_16(0x990)
    {
      // LDMIB Rn, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      LDMIB_REG(1, 0);
      LDMIB_REG(2, 1);
      LDMIB_REG(4, 2);
      LDMIB_REG(8, 3);
      LDMIB_REG(16, 4);
      LDMIB_REG(32, 5);
      LDMIB_REG(64, 6);
      LDMIB_REG(128, 7);
      LDMIB_REG(256, 8);
      LDMIB_REG(512, 9);
      LDMIB_REG(1024, 10);
      LDMIB_REG(2048, 11);
      LDMIB_REG(4096, 12);
      LDMIB_REG(8192, 13);
      LDMIB_REG(16384, 14);
      if(opcode & 32768) {
        address += 4;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        clockTicks += 2;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
    }
    break;
  CASE_16(0x9b0)
    {
      // LDMIB Rn!, {Rlist}
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      clockTicks = 2;
      LDMIB_REG(1, 0);
      LDMIB_REG(2, 1);
      LDMIB_REG(4, 2);
      LDMIB_REG(8, 3);
      LDMIB_REG(16, 4);
      LDMIB_REG(32, 5);
      LDMIB_REG(64, 6);
      LDMIB_REG(128, 7);
      LDMIB_REG(256, 8);
      LDMIB_REG(512, 9);
      LDMIB_REG(1024, 10);
      LDMIB_REG(2048, 11);
      LDMIB_REG(4096, 12);
      LDMIB_REG(8192, 13);
      LDMIB_REG(16384, 14);
      if(opcode & 32768) {
        address += 4;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        clockTicks += 2;
        armNextPC = reg[15].I;
        reg[15].I += 4;
      }
      if(!(opcode & (1 << base)))    
        reg[base].I = address;
    }
    break;    
  CASE_16(0x9d0)
    {
      // LDMIB Rn, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      if(opcode & 0x8000) {
        clockTicks = 4;
        LDMIB_REG(1, 0);
        LDMIB_REG(2, 1);
        LDMIB_REG(4, 2);
        LDMIB_REG(8, 3);
        LDMIB_REG(16, 4);
        LDMIB_REG(32, 5);
        LDMIB_REG(64, 6);
        LDMIB_REG(128, 7);
        LDMIB_REG(256, 8);
        LDMIB_REG(512, 9);
        LDMIB_REG(1024, 10);
        LDMIB_REG(2048, 11);
        LDMIB_REG(4096, 12);
        LDMIB_REG(8192, 13);
        LDMIB_REG(16384, 14);
        address += 4;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
        }
      } else {
        clockTicks = 2;
        LDMIB_REG(1, 0);
        LDMIB_REG(2, 1);
        LDMIB_REG(4, 2);
        LDMIB_REG(8, 3);
        LDMIB_REG(16, 4);
        LDMIB_REG(32, 5);
        LDMIB_REG(64, 6);
        LDMIB_REG(128, 7);
        if(armMode == 0x11) {
          LDMIB_REG(256, R8_FIQ);
          LDMIB_REG(512, R9_FIQ);
          LDMIB_REG(1024, R10_FIQ);
          LDMIB_REG(2048, R11_FIQ);
          LDMIB_REG(4096, R12_FIQ);
        } else {
          LDMIB_REG(256, 8);
          LDMIB_REG(512, 9);
          LDMIB_REG(1024, 10);
          LDMIB_REG(2048, 11);
          LDMIB_REG(4096, 12);
        }
        LDMIB_REG(8192, R13_USR);
        LDMIB_REG(16384, R14_USR);      
      }
    }
    break;
  CASE_16(0x9f0)
    {
      // LDMIB Rn!, {Rlist}^
      int base = (opcode & 0x000F0000) >> 16;
      u32 address = reg[base].I;
      if(opcode & 0x8000) {
        clockTicks = 4;
        LDMIB_REG(1, 0);
        LDMIB_REG(2, 1);
        LDMIB_REG(4, 2);
        LDMIB_REG(8, 3);
        LDMIB_REG(16, 4);
        LDMIB_REG(32, 5);
        LDMIB_REG(64, 6);
        LDMIB_REG(128, 7);
        LDMIB_REG(256, 8);
        LDMIB_REG(512, 9);
        LDMIB_REG(1024, 10);
        LDMIB_REG(2048, 11);
        LDMIB_REG(4096, 12);
        LDMIB_REG(8192, 13);
        LDMIB_REG(16384, 14);
        address += 4;
        reg[15].I = CPUReadMemory(address & 0xFFFFFFFC);
        if(!(opcode & (1 << base)))    
          reg[base].I = address;
        CPUSwitchMode(reg[17].I & 0x1f, false);
        if(armState) {
          armNextPC = reg[15].I & 0xFFFFFFFC;
          reg[15].I = armNextPC + 4;
        } else {
          armNextPC = reg[15].I & 0xFFFFFFFE;
          reg[15].I = armNextPC + 2;
        }
      } else {
        clockTicks = 4;
        LDMIB_REG(1, 0);
        LDMIB_REG(2, 1);
        LDMIB_REG(4, 2);
        LDMIB_REG(8, 3);
        LDMIB_REG(16, 4);
        LDMIB_REG(32, 5);
        LDMIB_REG(64, 6);
        LDMIB_REG(128, 7);
        if(armMode == 0x11) {
          LDMIB_REG(256, R8_FIQ);
          LDMIB_REG(512, R9_FIQ);
          LDMIB_REG(1024, R10_FIQ);
          LDMIB_REG(2048, R11_FIQ);
          LDMIB_REG(4096, R12_FIQ);
        } else {
          LDMIB_REG(256, 8);
          LDMIB_REG(512, 9);
          LDMIB_REG(1024, 10);
          LDMIB_REG(2048, 11);
          LDMIB_REG(4096, 12);
        }
        if(armMode != 0x10 && armMode != 0x1f) {
          LDMIB_REG(8192, R13_USR);
          LDMIB_REG(16384, R14_USR);
        } else {
          LDMIB_REG(8192, 13);
          LDMIB_REG(16384, 14);
        }
        if(!(opcode & (1 << base)))    
          reg[base].I = address;
      }
    }
    break;    
  CASE_256(0xa00)
    {
      // B <offset>
      clockTicks = 3;
      int offset = opcode & 0x00FFFFFF;
      if(offset & 0x00800000) {
        offset |= 0xFF000000;
      }
      offset <<= 2;
      reg[15].I += offset;
      armNextPC = reg[15].I;
      reg[15].I += 4;
    }
    break;
  CASE_256(0xb00)
    {
      // BL <offset>
      clockTicks = 3;
      int offset = opcode & 0x00FFFFFF;
      if(offset & 0x00800000) {
        offset |= 0xFF000000;
      }
      offset <<= 2;
      reg[14].I = reg[15].I - 4;
      reg[15].I += offset;
      armNextPC = reg[15].I;
      reg[15].I += 4;
    }
    break;
  CASE_256(0xf00)
    // SWI <comment>
    clockTicks = 3;
    CPUSoftwareInterrupt(opcode & 0x00FFFFFF);      
    break;
#ifdef GP_SUPPORT
  case 0xe11:
  case 0xe13:
  case 0xe15:
  case 0xe17:
  case 0xe19:
  case 0xe1b:
  case 0xe1d:
  case 0xe1f:
    // MRC
    break;
  case 0xe01:
  case 0xe03:
  case 0xe05:
  case 0xe07:
  case 0xe09:
  case 0xe0b:
  case 0xe0d:
  case 0xe0f:
    // MRC
    break;    
#endif
  default:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_UNDEFINED)
      log("Undefined ARM instruction %08x at %08x\n", opcode,
          armNextPC-4);
#endif
    CPUUndefinedException();
    break;
    // END
  }
}
