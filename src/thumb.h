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
#define ADD_RD_RS_RN \
   {\
     u32 lhs = reg[source].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADD_RD_RS_O3 \
   {\
     u32 lhs = reg[source].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADD_RN_O8(d) \
   {\
     u32 lhs = reg[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs + rhs;\
     reg[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define CMN_RD_RS \
   {\
     u32 lhs = reg[dest].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define ADC_RD_RS \
   {\
     u32 lhs = reg[dest].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs + (u32)C_FLAG;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RD_RS_RN \
   {\
     u32 lhs = reg[source].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RD_RS_O3 \
   {\
     u32 lhs = reg[source].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SUB_RN_O8(d) \
   {\
     u32 lhs = reg[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs - rhs;\
     reg[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define CMP_RN_O8(d) \
   {\
     u32 lhs = reg[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define SBC_RD_RS \
   {\
     u32 lhs = reg[dest].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs - !((u32)C_FLAG);\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#define LSL_RD_RM_I5 \
   {\
     C_FLAG = (reg[source].I >> (32 - shift)) & 1 ? true : false;\
     value = reg[source].I << shift;\
   }
#define LSL_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (32 - value)) & 1 ? true : false;\
     value = reg[dest].I << value;\
   }
#define LSR_RD_RM_I5 \
   {\
     C_FLAG = (reg[source].I >> (shift - 1)) & 1 ? true : false;\
     value = reg[source].I >> shift;\
   }
#define LSR_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false;\
     value = reg[dest].I >> value;\
   }
#define ASR_RD_RM_I5 \
   {\
     C_FLAG = ((s32)reg[source].I >> (int)(shift - 1)) & 1 ? true : false;\
     value = (s32)reg[source].I >> (int)shift;\
   }
#define ASR_RD_RS \
   {\
     C_FLAG = ((s32)reg[dest].I >> (int)(value - 1)) & 1 ? true : false;\
     value = (s32)reg[dest].I >> (int)value;\
   }
#define ROR_RD_RS \
   {\
     C_FLAG = (reg[dest].I >> (value - 1)) & 1 ? true : false;\
     value = ((reg[dest].I << (32 - value)) |\
              (reg[dest].I >> value));\
   }
#define NEG_RD_RS \
   {\
     u32 lhs = reg[source].I;\
     u32 rhs = 0;\
     u32 res = rhs - lhs;\
     reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#define CMP_RD_RS \
   {\
     u32 lhs = reg[dest].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#else
#ifdef __GNUC__
#define ADD_RD_RS_RN \
     asm ("add %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (reg[dest].I)\
          : "r" (value), "b" (reg[source].I));
#define ADD_RD_RS_O3 \
     asm ("add %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (reg[dest].I)\
          : "r" (value), "b" (reg[source].I));
#define ADD_RN_O8(d) \
     asm ("add %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (reg[(d)].I)\
          : "r" (opcode & 255), "b" (reg[(d)].I));
#define CMN_RD_RS \
     asm ("add %0, %1;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : \
          : "r" (value), "r" (reg[dest].I):"1");
#define ADC_RD_RS \
     asm ("bt $0, C_FLAG;"\
          "adc %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setcb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (reg[dest].I)\
          : "r" (value), "b" (reg[dest].I));
#define SUB_RD_RS_RN \
     asm ("sub %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (reg[dest].I)\
          : "r" (value), "b" (reg[source].I));
#define SUB_RD_RS_O3 \
     asm ("sub %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (reg[dest].I)\
          : "r" (value), "b" (reg[source].I));
#define SUB_RN_O8(d) \
     asm ("sub %1, %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (reg[(d)].I)\
          : "r" (opcode & 255), "b" (reg[(d)].I));
#define CMP_RN_O8(d) \
     asm ("sub %0, %1;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : \
          : "r" (opcode & 255), "r" (reg[(d)].I) : "1");
#define SBC_RD_RS \
     asm volatile ("bt $0, C_FLAG;"\
                   "cmc;"\
                   "sbb %1, %%ebx;"\
                   "setsb N_FLAG;"\
                   "setzb Z_FLAG;"\
                   "setncb C_FLAG;"\
                   "setob V_FLAG;"\
                   : "=b" (reg[dest].I)\
                   : "r" (value), "b" (reg[dest].I) : "cc", "memory");
#define LSL_RD_RM_I5 \
       asm ("shl %%cl, %%eax;"\
            "setcb C_FLAG;"\
            : "=a" (value)\
            : "a" (reg[source].I), "c" (shift));
#define LSL_RD_RS \
         asm ("shl %%cl, %%eax;"\
              "setcb C_FLAG;"\
              : "=a" (value)\
              : "a" (reg[dest].I), "c" (value));
#define LSR_RD_RM_I5 \
       asm ("shr %%cl, %%eax;"\
            "setcb C_FLAG;"\
            : "=a" (value)\
            : "a" (reg[source].I), "c" (shift));
#define LSR_RD_RS \
         asm ("shr %%cl, %%eax;"\
              "setcb C_FLAG;"\
              : "=a" (value)\
              : "a" (reg[dest].I), "c" (value));
#define ASR_RD_RM_I5 \
     asm ("sar %%cl, %%eax;"\
          "setcb C_FLAG;"\
          : "=a" (value)\
          : "a" (reg[source].I), "c" (shift));
#define ASR_RD_RS \
         asm ("sar %%cl, %%eax;"\
              "setcb C_FLAG;"\
              : "=a" (value)\
              : "a" (reg[dest].I), "c" (value));
#define ROR_RD_RS \
         asm ("ror %%cl, %%eax;"\
              "setcb C_FLAG;"\
              : "=a" (value)\
              : "a" (reg[dest].I), "c" (value));
#define NEG_RD_RS \
     asm ("neg %%ebx;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : "=b" (reg[dest].I)\
          : "b" (reg[source].I));
#define CMP_RD_RS \
     asm ("sub %0, %1;"\
          "setsb N_FLAG;"\
          "setzb Z_FLAG;"\
          "setncb C_FLAG;"\
          "setob V_FLAG;"\
          : \
          : "r" (value), "r" (reg[dest].I):"1");
#else
#define ADD_RD_RS_RN \
   {\
     __asm mov eax, source\
     __asm mov ebx, dword ptr [OFFSET reg+4*eax]\
     __asm add ebx, value\
     __asm mov eax, dest\
     __asm mov dword ptr [OFFSET reg+4*eax], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define ADD_RD_RS_O3 \
   {\
     __asm mov eax, source\
     __asm mov ebx, dword ptr [OFFSET reg+4*eax]\
     __asm add ebx, value\
     __asm mov eax, dest\
     __asm mov dword ptr [OFFSET reg+4*eax], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define ADD_RN_O8(d) \
   {\
     __asm mov ebx, opcode\
     __asm and ebx, 255\
     __asm add dword ptr [OFFSET reg+4*(d)], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define CMN_RD_RS \
     {\
       __asm mov eax, dest\
       __asm mov ebx, dword ptr [OFFSET reg+4*eax]\
       __asm add ebx, value\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#define ADC_RD_RS \
     {\
       __asm mov ebx, dest\
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
#define SUB_RD_RS_RN \
   {\
     __asm mov eax, source\
     __asm mov ebx, dword ptr [OFFSET reg+4*eax]\
     __asm sub ebx, value\
     __asm mov eax, dest\
     __asm mov dword ptr [OFFSET reg+4*eax], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setnc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define SUB_RD_RS_O3 \
   {\
     __asm mov eax, source\
     __asm mov ebx, dword ptr [OFFSET reg+4*eax]\
     __asm sub ebx, value\
     __asm mov eax, dest\
     __asm mov dword ptr [OFFSET reg+4*eax], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setnc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define SUB_RN_O8(d) \
   {\
     __asm mov ebx, opcode\
     __asm and ebx, 255\
     __asm sub dword ptr [OFFSET reg + 4*(d)], ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setnc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define CMP_RN_O8(d) \
   {\
     __asm mov eax, dword ptr [OFFSET reg+4*(d)]\
     __asm mov ebx, opcode\
     __asm and ebx, 255\
     __asm sub eax, ebx\
     __asm sets byte ptr N_FLAG\
     __asm setz byte ptr Z_FLAG\
     __asm setnc byte ptr C_FLAG\
     __asm seto byte ptr V_FLAG\
   }
#define SBC_RD_RS \
     {\
       __asm mov ebx, dest\
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
#define LSL_RD_RM_I5 \
     {\
       __asm mov eax, source\
       __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
       __asm mov cl, byte ptr shift\
       __asm shl eax, cl\
       __asm mov value, eax\
       __asm setc byte ptr C_FLAG\
     }
#define LSL_RD_RS \
         {\
           __asm mov eax, dest\
           __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
           __asm mov cl, byte ptr value\
           __asm shl eax, cl\
           __asm mov value, eax\
           __asm setc byte ptr C_FLAG\
         }
#define LSR_RD_RM_I5 \
     {\
       __asm mov eax, source\
       __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
       __asm mov cl, byte ptr shift\
       __asm shr eax, cl\
       __asm mov value, eax\
       __asm setc byte ptr C_FLAG\
     }
#define LSR_RD_RS \
         {\
           __asm mov eax, dest\
           __asm mov eax, dword ptr [OFFSET reg + 4 * eax]\
           __asm mov cl, byte ptr value\
           __asm shr eax, cl\
           __asm mov value, eax\
           __asm setc byte ptr C_FLAG\
         }
#define ASR_RD_RM_I5 \
     {\
       __asm mov eax, source\
       __asm mov eax, dword ptr [OFFSET reg + 4*eax]\
       __asm mov cl, byte ptr shift\
       __asm sar eax, cl\
       __asm mov value, eax\
       __asm setc byte ptr C_FLAG\
     }
#define ASR_RD_RS \
         {\
           __asm mov eax, dest\
           __asm mov eax, dword ptr [OFFSET reg + 4*eax]\
           __asm mov cl, byte ptr value\
           __asm sar eax, cl\
           __asm mov value, eax\
           __asm setc byte ptr C_FLAG\
         }
#define ROR_RD_RS \
         {\
           __asm mov eax, dest\
           __asm mov eax, dword ptr [OFFSET reg + 4*eax]\
           __asm mov cl, byte ptr value\
           __asm ror eax, cl\
           __asm mov value, eax\
           __asm setc byte ptr C_FLAG\
         }
#define NEG_RD_RS \
     {\
       __asm mov ebx, source\
       __asm mov ebx, dword ptr [OFFSET reg+4*ebx]\
       __asm neg ebx\
       __asm mov eax, dest\
       __asm mov dword ptr [OFFSET reg+4*eax],ebx\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setnc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#define CMP_RD_RS \
     {\
       __asm mov eax, dest\
       __asm mov ebx, dword ptr [OFFSET reg+4*eax]\
       __asm sub ebx, value\
       __asm sets byte ptr N_FLAG\
       __asm setz byte ptr Z_FLAG\
       __asm setnc byte ptr C_FLAG\
       __asm seto byte ptr V_FLAG\
     }
#endif
#endif

u32 opcode = CPUReadHalfWordQuick(armNextPC);

#ifndef FINAL_VERSION
if(armNextPC == stop) {
  armNextPC = armNextPC++;
}
#endif

armNextPC = reg[15].I;
reg[15].I += 2;

clockTicks = thumbCycles[opcode >> 8];

switch(opcode >> 8) {
 case 0x00:
 case 0x01:
 case 0x02:
 case 0x03:
 case 0x04:
 case 0x05:
 case 0x06:
 case 0x07:
   {
     // LSL Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     u32 value;
     
     if(shift) {
       LSL_RD_RM_I5;
     } else {
       value = reg[source].I;
     }
     reg[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false : true);
   }
   break;
 case 0x08:
 case 0x09:
 case 0x0a:
 case 0x0b:
 case 0x0c:
 case 0x0d:
 case 0x0e:
 case 0x0f:
   {
     // LSR Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     u32 value;
     
     if(shift) {
       LSR_RD_RM_I5;
     } else {
       C_FLAG = reg[source].I & 0x80000000 ? true : false;
       value = 0;
     }
     reg[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false : true);
   }
   break;
 case 0x10:
 case 0x11:
 case 0x12:
 case 0x13:
 case 0x14:
 case 0x15:
 case 0x16:
 case 0x17:
   {     
     // ASR Rd, Rm, #Imm 5
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     int shift = (opcode >> 6) & 0x1f;
     u32 value;
     
     if(shift) {
       ASR_RD_RM_I5;
     } else {
       if(reg[source].I & 0x80000000) {
         value = 0xFFFFFFFF;
         C_FLAG = true;
       } else {
         value = 0;
         C_FLAG = false;
       }
     }
     reg[dest].I = value;
     // C_FLAG set above
     N_FLAG = (value & 0x80000000 ? true : false);
     Z_FLAG = (value ? false :true);
   }
   break;
 case 0x18:
 case 0x19:
   {
     // ADD Rd, Rs, Rn
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     u32 value = reg[(opcode>>6)& 0x07].I;
     ADD_RD_RS_RN;
   }
   break;
 case 0x1a:
 case 0x1b:
   {
     // SUB Rd, Rs, Rn
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     u32 value = reg[(opcode>>6)& 0x07].I;
     SUB_RD_RS_RN;
   }
   break;
 case 0x1c:
 case 0x1d:
   {
     // ADD Rd, Rs, #Offset3
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     u32 value = (opcode >> 6) & 7;
     ADD_RD_RS_O3;
   }
   break;
 case 0x1e:
 case 0x1f:
   {
     // SUB Rd, Rs, #Offset3
     int dest = opcode & 0x07;
     int source = (opcode >> 3) & 0x07;
     u32 value = (opcode >> 6) & 7;
     SUB_RD_RS_O3;
   }
   break;
 case 0x20:
   // MOV R0, #Offset8
   reg[0].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[0].I ? false : true);
   break;
 case 0x21:
   // MOV R1, #Offset8
   reg[1].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[1].I ? false : true);
   break;   
 case 0x22:
   // MOV R2, #Offset8
   reg[2].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[2].I ? false : true);
   break;   
 case 0x23:
   // MOV R3, #Offset8
   reg[3].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[3].I ? false : true);
   break;   
 case 0x24:
   // MOV R4, #Offset8
   reg[4].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[4].I ? false : true);
   break;   
 case 0x25:
   // MOV R5, #Offset8
   reg[5].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[5].I ? false : true);
   break;   
 case 0x26:
   // MOV R6, #Offset8
   reg[6].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[6].I ? false : true);
   break;   
 case 0x27:
   // MOV R7, #Offset8
   reg[7].I = opcode & 255;
   N_FLAG = false;
   Z_FLAG = (reg[7].I ? false : true);
   break;
 case 0x28:
   // CMP R0, #Offset8
   CMP_RN_O8(0);
   break;
 case 0x29:
   // CMP R1, #Offset8
   CMP_RN_O8(1);
   break;
 case 0x2a:
   // CMP R2, #Offset8
   CMP_RN_O8(2);
   break;
 case 0x2b:
   // CMP R3, #Offset8
   CMP_RN_O8(3);
   break;
 case 0x2c:
   // CMP R4, #Offset8
   CMP_RN_O8(4);
   break;
 case 0x2d:
   // CMP R5, #Offset8
   CMP_RN_O8(5);
   break;
 case 0x2e:
   // CMP R6, #Offset8
   CMP_RN_O8(6);
   break;
 case 0x2f:
   // CMP R7, #Offset8
   CMP_RN_O8(7);
   break;
 case 0x30:
   // ADD R0,#Offset8
   ADD_RN_O8(0);
   break;   
 case 0x31:
   // ADD R1,#Offset8
   ADD_RN_O8(1);
   break;   
 case 0x32:
   // ADD R2,#Offset8
   ADD_RN_O8(2);
   break;   
 case 0x33:
   // ADD R3,#Offset8
   ADD_RN_O8(3);
   break;   
 case 0x34:
   // ADD R4,#Offset8
   ADD_RN_O8(4);
   break;   
 case 0x35:
   // ADD R5,#Offset8
   ADD_RN_O8(5);
   break;   
 case 0x36:
   // ADD R6,#Offset8
   ADD_RN_O8(6);
   break;   
 case 0x37:
   // ADD R7,#Offset8
   ADD_RN_O8(7);
   break;
 case 0x38:
   // SUB R0,#Offset8
   SUB_RN_O8(0);
   break;
 case 0x39:
   // SUB R1,#Offset8
   SUB_RN_O8(1);
   break;
 case 0x3a:
   // SUB R2,#Offset8
   SUB_RN_O8(2);
   break;
 case 0x3b:
   // SUB R3,#Offset8
   SUB_RN_O8(3);
   break;
 case 0x3c:
   // SUB R4,#Offset8
   SUB_RN_O8(4);
   break;
 case 0x3d:
   // SUB R5,#Offset8
   SUB_RN_O8(5);
   break;
 case 0x3e:
   // SUB R6,#Offset8
   SUB_RN_O8(6);
   break;
 case 0x3f:
   // SUB R7,#Offset8
   SUB_RN_O8(7);
   break;
 case 0x40:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // AND Rd, Rs
       int dest = opcode & 7;
       reg[dest].I &= reg[(opcode >> 3)&7].I;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
#ifdef BKPT_SUPPORT     
#define THUMB_CONSOLE_OUTPUT(a,b) \
  if((opcode == 0x4000) && (reg[0].I == 0xC0DED00D)) {\
    extern void (*dbgOutput)(char *, u32);\
    dbgOutput((a), (b));\
  }
#else
#define THUMB_CONSOLE_OUTPUT(a,b)
#endif
       THUMB_CONSOLE_OUTPUT(NULL, reg[2].I);
     }
     break;
   case 0x01:
     // EOR Rd, Rs
     {
       int dest = opcode & 7;
       reg[dest].I ^= reg[(opcode >> 3)&7].I;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
     }
     break;
   case 0x02:
     // LSL Rd, Rs
     {
       int dest = opcode & 7;
       u32 value = reg[(opcode >> 3)&7].B.B0;
       if(value) {
         if(value == 32) {
           value = 0;
           C_FLAG = (reg[dest].I & 1 ? true : false);
         } else if(value < 32) {
           LSL_RD_RS;
         } else {
           value = 0;
           C_FLAG = false;
         }
         reg[dest].I = value;        
       }
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
       clockTicks++;
     }
     break;
   case 0x03:
     {
       // LSR Rd, Rs
       int dest = opcode & 7;
       u32 value = reg[(opcode >> 3)&7].B.B0;
       if(value) {
         if(value == 32) {
           value = 0;
           C_FLAG = (reg[dest].I & 0x80000000 ? true : false);
         } else if(value < 32) {
           LSR_RD_RS;
         } else {
           value = 0;
           C_FLAG = false;
         }
         reg[dest].I = value;        
       }
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
       clockTicks++;
     }
     break;
   }
   break;
 case 0x41:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // ASR Rd, Rs
       int dest = opcode & 7;
       u32 value = reg[(opcode >> 3)&7].B.B0;
       // ASR
       if(value) {
         if(value < 32) {
           ASR_RD_RS;
           reg[dest].I = value;        
         } else {
           if(reg[dest].I & 0x80000000){
             reg[dest].I = 0xFFFFFFFF;
             C_FLAG = true;
           } else {
             reg[dest].I = 0x00000000;
             C_FLAG = false;
           }
         }
       }
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
       clockTicks++;
     }
     break;
   case 0x01:
     {
       // ADC Rd, Rs
       int dest = opcode & 0x07;
       u32 value = reg[(opcode >> 3)&7].I;
       // ADC
       ADC_RD_RS;
     }
     break;
   case 0x02:
     {
       // SBC Rd, Rs
       int dest = opcode & 0x07;
       u32 value = reg[(opcode >> 3)&7].I;
       
       // SBC
       SBC_RD_RS;
     }
     break;
   case 0x03:
     // ROR Rd, Rs
     {
       int dest = opcode & 7;
       u32 value = reg[(opcode >> 3)&7].B.B0;
       
       if(value) {
         value = value & 0x1f;
         if(value == 0) {
           C_FLAG = (reg[dest].I & 0x80000000 ? true : false);
         } else {
           ROR_RD_RS;
           reg[dest].I = value;
         }
       }
       clockTicks++;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
       Z_FLAG = reg[dest].I ? false : true;
     }
     break;
   }
   break;
 case 0x42:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // TST Rd, Rs
       u32 value = reg[opcode & 7].I & reg[(opcode >> 3) & 7].I;
       N_FLAG = value & 0x80000000 ? true : false;
       Z_FLAG = value ? false : true;
     }
     break;
   case 0x01:
     {
       // NEG Rd, Rs
       int dest = opcode & 7;
       int source = (opcode >> 3) & 7;
       NEG_RD_RS;
     }
     break;
   case 0x02:
     {
       // CMP Rd, Rs
       int dest = opcode & 7;
       u32 value = reg[(opcode >> 3)&7].I;
       CMP_RD_RS;
     }
     break;
   case 0x03:
     {
       // CMN Rd, Rs
       int dest = opcode & 7;
       u32 value = reg[(opcode >> 3)&7].I;
       // CMN
       CMN_RD_RS;
     }
     break;
   }
   break;
 case 0x43:
   switch((opcode >> 6) & 3) {
   case 0x00:
     {
       // ORR Rd, Rs       
       int dest = opcode & 7;
       reg[dest].I |= reg[(opcode >> 3) & 7].I;
       Z_FLAG = reg[dest].I ? false : true;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x01:
     {
       // MUL Rd, Rs
       int dest = opcode & 7;
       reg[dest].I = reg[(opcode >> 3) & 7].I * reg[dest].I;
       Z_FLAG = reg[dest].I ? false : true;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x02:
     {
       // BIC Rd, Rs
       int dest = opcode & 7;
       reg[dest].I &= (~reg[(opcode >> 3) & 7].I);
       Z_FLAG = reg[dest].I ? false : true;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
     }
     break;
   case 0x03:
     {
       // MVN Rd, Rs
       int dest = opcode & 7;
       reg[dest].I = ~reg[(opcode >> 3) & 7].I;
       Z_FLAG = reg[dest].I ? false : true;
       N_FLAG = reg[dest].I & 0x80000000 ? true : false;
     }
     break;
   }
   break;
 case 0x44:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     switch((opcode >> 6)& 3) {
     default:
       goto unknown_thumb;
     case 1:
       // ADD Rd, Hs
       reg[dest].I += reg[base+8].I;
       break;
     case 2:
       // ADD Hd, Rs
       reg[dest+8].I += reg[base].I;
       if(dest == 7) {
         reg[15].I &= 0xFFFFFFFE;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         clockTicks++;
       }       
       break;
     case 3:
       // ADD Hd, Hs
       reg[dest+8].I += reg[base+8].I;
       if(dest == 7) {
         reg[15].I &= 0xFFFFFFFE;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         clockTicks++;     
       }
       break;
     }
   }
   break;
 case 0x45:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     u32 value;
     switch((opcode >> 6) & 3) {
     case 0:
       // CMP Rd, Hs
       value = reg[base].I;
       CMP_RD_RS;
       break;
     case 1:
       // CMP Rd, Hs
       value = reg[base+8].I;
       CMP_RD_RS;
       break;
     case 2:
       // CMP Hd, Rs
       value = reg[base].I;
       dest += 8;
       CMP_RD_RS;
       break;
     case 3:
       // CMP Hd, Hs
       value = reg[base+8].I;
       dest += 8;
       CMP_RD_RS;
       break;
     }
   }
   break;
 case 0x46:
   {
     int dest = opcode & 7;
     int base = (opcode >> 3) & 7;
     switch((opcode >> 6) & 3) {
     case 0:
       // this form should not be used...
       // MOV Rd, Rs
       reg[dest].I = reg[base].I;
       break;
     case 1:
       // MOV Rd, Hs
       reg[dest].I = reg[base+8].I;
       break;
     case 2:
       // MOV Hd, Rs
       reg[dest+8].I = reg[base].I;
       if(dest == 7) {
         reg[15].I &= 0xFFFFFFFE;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         clockTicks++;
       }
       break;
     case 3:
       // MOV Hd, Hs
       reg[dest+8].I = reg[base+8].I;
       if(dest == 7) {
         reg[15].I &= 0xFFFFFFFE;
         armNextPC = reg[15].I;
         reg[15].I += 2;
         clockTicks++;
       }   
       break;
     }
   }
   break;
 case 0x47:
   {
     int base = (opcode >> 3) & 7;
     switch((opcode >>6) & 3) {
     case 0:
       // BX Rs
       reg[15].I = (reg[base].I) & 0xFFFFFFFE;
       if(reg[base].I & 1) {
         armState = false;
         armNextPC = reg[15].I;
         reg[15].I += 2;
       } else {
         armState = true;
         reg[15].I &= 0xFFFFFFFC;
         armNextPC = reg[15].I;
         reg[15].I += 4;
       }
       break;
     case 1:
       // BX Hs
       reg[15].I = (reg[8+base].I) & 0xFFFFFFFE;
       if(reg[8+base].I & 1) {
         armState = false;
         armNextPC = reg[15].I;
         reg[15].I += 2;
       } else {
         armState = true;
         reg[15].I &= 0xFFFFFFFC;       
         armNextPC = reg[15].I;
         reg[15].I += 4;
       }
       break;
     default:
       goto unknown_thumb;
     }
   }
   break;
 case 0x48:
   // LDR R0,[PC, #Imm]
   reg[0].I = CPUReadMemoryQuick((reg[15].I & 0xFFFFFFFC) +
                                 ((opcode & 0xFF) << 2));
   break;
 case 0x49:
   // LDR R1,[PC, #Imm]
   reg[1].I = CPUReadMemoryQuick((reg[15].I & 0xFFFFFFFC) +
                                 ((opcode & 0xFF) << 2));
   break;
 case 0x4a:
   // LDR R2,[PC, #Imm]
   reg[2].I = CPUReadMemoryQuick((reg[15].I & 0xFFFFFFFC) +
                                 ((opcode & 0xFF) << 2));
   break;
 case 0x4b:
   // LDR R3,[PC, #Imm]
   reg[3].I = CPUReadMemoryQuick((reg[15].I & 0xFFFFFFFC) +
                                 ((opcode & 0xFF) << 2));
   break;
 case 0x4c:
   // LDR R4,[PC, #Imm]
   reg[4].I = CPUReadMemoryQuick((reg[15].I & 0xFFFFFFFC) +
                                 ((opcode & 0xFF) << 2));
   break;
 case 0x4d:
   // LDR R5,[PC, #Imm]
   reg[5].I = CPUReadMemoryQuick((reg[15].I & 0xFFFFFFFC) +
                                 ((opcode & 0xFF) << 2));
   break;
 case 0x4e:
   // LDR R6,[PC, #Imm]
   reg[6].I = CPUReadMemoryQuick((reg[15].I & 0xFFFFFFFC) +
                                 ((opcode & 0xFF) << 2));
   break;
 case 0x4f:
   // LDR R7,[PC, #Imm]
   reg[7].I = CPUReadMemoryQuick((reg[15].I & 0xFFFFFFFC) +
                                 ((opcode & 0xFF) << 2));
   break;
 case 0x50:
 case 0x51:
   CPUWriteMemory(reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I,
                  reg[opcode & 7].I);
   break;
 case 0x52:
 case 0x53:
   CPUWriteHalfWord(reg[(opcode>>3)&7].I + reg[(opcode>>6)&7].I,
                    reg[opcode&7].W.W0);
   break;
 case 0x54:
 case 0x55:
   CPUWriteByte(reg[(opcode>>3)&7].I + reg[(opcode >>6)&7].I,
                reg[opcode & 7].B.B0);
   break;
 case 0x56:
 case 0x57:
   reg[opcode&7].I = (s8)CPUReadByte(reg[(opcode>>3)&7].I +
                                     reg[(opcode>>6)&7].I);     
   break;
 case 0x58:
 case 0x59:
   reg[opcode&7].I = CPUReadMemory(reg[(opcode>>3)&7].I +
                                   reg[(opcode>>6)&7].I);
   break;
 case 0x5a:
 case 0x5b:
   reg[opcode&7].I = CPUReadHalfWord(reg[(opcode>>3)&7].I +
                                     reg[(opcode>>6)&7].I);
   break;
 case 0x5c:
 case 0x5d:
   reg[opcode&7].I = CPUReadByte(reg[(opcode>>3)&7].I +
                                 reg[(opcode>>6)&7].I);
   break;
 case 0x5e:
 case 0x5f:
   reg[opcode&7].I = (s16)CPUReadHalfWordSigned(reg[(opcode>>3)&7].I +
                                                reg[(opcode>>6)&7].I);
   break;
 case 0x60:
 case 0x61:
 case 0x62:
 case 0x63:
 case 0x64:
 case 0x65:
 case 0x66:
 case 0x67:
   CPUWriteMemory(reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<2),
                  reg[opcode&7].I);
   break;
 case 0x68:
 case 0x69:
 case 0x6a:
 case 0x6b:
 case 0x6c:
 case 0x6d:
 case 0x6e:
 case 0x6f:
   reg[opcode&7].I = CPUReadMemory(reg[(opcode>>3)&7].I +
                                   (((opcode>>6)&31)<<2));
   break;
 case 0x70:
 case 0x71:
 case 0x72:
 case 0x73:
 case 0x74:
 case 0x75:
 case 0x76:
 case 0x77:
   CPUWriteByte(reg[(opcode>>3)&7].I + (((opcode>>6)&31)),
                reg[opcode&7].B.B0);
   break;
 case 0x78:
 case 0x79:
 case 0x7a:
 case 0x7b:
 case 0x7c:
 case 0x7d:
 case 0x7e:
 case 0x7f:
   reg[opcode&7].I = CPUReadByte(reg[(opcode>>3)&7].I +
                                 (((opcode>>6)&31)));
   break;
 case 0x80:
 case 0x81:
 case 0x82:
 case 0x83:
 case 0x84:
 case 0x85:
 case 0x86:
 case 0x87:
   CPUWriteHalfWord(reg[(opcode>>3)&7].I + (((opcode >> 6) & 0x1F) << 1),
                    reg[opcode&7].W.W0);
   break;   
 case 0x88:
 case 0x89:
 case 0x8a:
 case 0x8b:
 case 0x8c:
 case 0x8d:
 case 0x8e:
 case 0x8f:
   reg[opcode&7].I = CPUReadHalfWord(reg[(opcode>>3)&7].I +
                                     (((opcode >> 6) & 0x1F) << 1));
   break;
 case 0x90:
   CPUWriteMemory(reg[13].I + ((opcode&255)<<2), reg[0].I);
   break;      
 case 0x91:
   CPUWriteMemory(reg[13].I + ((opcode&255)<<2), reg[1].I);
   break;      
 case 0x92:
   CPUWriteMemory(reg[13].I + ((opcode&255)<<2), reg[2].I);
   break;      
 case 0x93:
   CPUWriteMemory(reg[13].I + ((opcode&255)<<2), reg[3].I);
   break;      
 case 0x94:
   CPUWriteMemory(reg[13].I + ((opcode&255)<<2), reg[4].I);
   break;      
 case 0x95:
   CPUWriteMemory(reg[13].I + ((opcode&255)<<2), reg[5].I);
   break;      
 case 0x96:
   CPUWriteMemory(reg[13].I + ((opcode&255)<<2), reg[6].I);
   break;      
 case 0x97:
   CPUWriteMemory(reg[13].I + ((opcode&255)<<2), reg[7].I);
   break;   
 case 0x98:
   reg[0].I = CPUReadMemoryQuick(reg[13].I + ((opcode&255)<<2));
   break;
 case 0x99:
   reg[1].I = CPUReadMemoryQuick(reg[13].I + ((opcode&255)<<2));
   break;
 case 0x9a:
   reg[2].I = CPUReadMemoryQuick(reg[13].I + ((opcode&255)<<2));
   break;
 case 0x9b:
   reg[3].I = CPUReadMemoryQuick(reg[13].I + ((opcode&255)<<2));
   break;
 case 0x9c:
   reg[4].I = CPUReadMemoryQuick(reg[13].I + ((opcode&255)<<2));
   break;
 case 0x9d:
   reg[5].I = CPUReadMemoryQuick(reg[13].I + ((opcode&255)<<2));
   break;
 case 0x9e:
   reg[6].I = CPUReadMemoryQuick(reg[13].I + ((opcode&255)<<2));
   break;
 case 0x9f:
   reg[7].I = CPUReadMemoryQuick(reg[13].I + ((opcode&255)<<2));
   break;
 case 0xa0:
   reg[0].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa1:
   reg[1].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa2:
   reg[2].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa3:
   reg[3].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa4:
   reg[4].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa5:
   reg[5].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa6:
   reg[6].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa7:
   reg[7].I = (reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
   break;   
 case 0xa8:
   reg[0].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xa9:
   reg[1].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xaa:
   reg[2].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xab:
   reg[3].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xac:
   reg[4].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xad:
   reg[5].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xae:
   reg[6].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xaf:
   reg[7].I = reg[13].I + ((opcode&255)<<2);
   break;   
 case 0xb0:
   {
     // add offset to SP
     int offset = (opcode & 127) << 2;
     if(opcode & 0x80)
       offset = -offset;
     reg[13].I += offset;
   }
   break;
#define PUSH_REG(val, r) \
  if(opcode & (val)) {\
    reg[13].I -= 4;\
    CPUWriteMemory(reg[13].I & 0xFFFFFFFC, reg[(r)].I);\
    clockTicks++;\
  }
 case 0xb4:
   // PUSH
   PUSH_REG(128, 7);
   PUSH_REG(64, 6);
   PUSH_REG(32, 5);
   PUSH_REG(16, 4);
   PUSH_REG(8, 3);
   PUSH_REG(4, 2);
   PUSH_REG(2, 1);
   PUSH_REG(1, 0);
   break;
 case 0xb5:
   reg[13].I -= 4;
   CPUWriteMemory(reg[13].I & 0xFFFFFFFC, reg[14].I);
   PUSH_REG(128, 7);
   PUSH_REG(64, 6);
   PUSH_REG(32, 5);
   PUSH_REG(16, 4);
   PUSH_REG(8, 3);
   PUSH_REG(4, 2);
   PUSH_REG(2, 1);
   PUSH_REG(1, 0);
   break;
#define POP_REG(val, r) \
  if(opcode & (val)) {\
    reg[(r)].I = CPUReadMemory(reg[13].I & 0xFFFFFFFC);\
    reg[13].I += 4;\
    clockTicks += 2;\
  }
 case 0xbc:
   // POP
   POP_REG(1, 0);
   POP_REG(2, 1);
   POP_REG(4, 2);
   POP_REG(8, 3);
   POP_REG(16, 4);
   POP_REG(32, 5);
   POP_REG(64, 6);
   POP_REG(128, 7);
   break;   
 case 0xbd:
   // POP
   POP_REG(1, 0);
   POP_REG(2, 1);
   POP_REG(4, 2);
   POP_REG(8, 3);
   POP_REG(16, 4);
   POP_REG(32, 5);
   POP_REG(64, 6);
   POP_REG(128, 7);
   reg[15].I = (CPUReadMemory(reg[13].I & 0xFFFFFFFC) & 0xFFFFFFFE);
   armNextPC = reg[15].I;
   reg[15].I += 2;
   reg[13].I += 4;
   break;      
#define STM_REG(val,r,b) \
  if(opcode & (val)) {\
    CPUWriteMemory(address & 0xFFFFFFFC, reg[(r)].I);\
    address += 4;\
    clockTicks++;\
    if(!offset)\
      reg[(b)].I = temp;\
    offset = 1;\
  }
 case 0xc0:
   {
     u32 address = reg[0].I;
     u32 temp = address + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     STM_REG(1, 0, 0);
     STM_REG(2, 1, 0);
     STM_REG(4, 2, 0);
     STM_REG(8, 3, 0);
     STM_REG(16, 4, 0);
     STM_REG(32, 5, 0);
     STM_REG(64, 6, 0);
     STM_REG(128, 7, 0);
   }
   break;   
 case 0xc1:
   {
     u32 address = reg[1].I;
     u32 temp = address + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     STM_REG(1, 0, 1);
     STM_REG(2, 1, 1);
     STM_REG(4, 2, 1);
     STM_REG(8, 3, 1);
     STM_REG(16, 4, 1);
     STM_REG(32, 5, 1);
     STM_REG(64, 6, 1);
     STM_REG(128, 7, 1);
   }
   break;      
 case 0xc2:
   {
     u32 address = reg[2].I;
     u32 temp = address + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     STM_REG(1, 0, 2);
     STM_REG(2, 1, 2);
     STM_REG(4, 2, 2);
     STM_REG(8, 3, 2);
     STM_REG(16, 4, 2);
     STM_REG(32, 5, 2);
     STM_REG(64, 6, 2);
     STM_REG(128, 7, 2);
   }
   break;      
 case 0xc3:
   {
     u32 address = reg[3].I;
     u32 temp = address + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     STM_REG(1, 0, 3);
     STM_REG(2, 1, 3);
     STM_REG(4, 2, 3);
     STM_REG(8, 3, 3);
     STM_REG(16, 4, 3);
     STM_REG(32, 5, 3);
     STM_REG(64, 6, 3);
     STM_REG(128, 7, 3);
   }
   break;   
 case 0xc4:
   {
     u32 address = reg[4].I;
     u32 temp = address + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     STM_REG(1, 0, 4);
     STM_REG(2, 1, 4);
     STM_REG(4, 2, 4);
     STM_REG(8, 3, 4);
     STM_REG(16, 4, 4);
     STM_REG(32, 5, 4);
     STM_REG(64, 6, 4);
     STM_REG(128, 7, 4);
   }
   break;   
 case 0xc5:
   {
     u32 address = reg[5].I;
     u32 temp = address + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     STM_REG(1, 0, 5);
     STM_REG(2, 1, 5);
     STM_REG(4, 2, 5);
     STM_REG(8, 3, 5);
     STM_REG(16, 4, 5);
     STM_REG(32, 5, 5);
     STM_REG(64, 6, 5);
     STM_REG(128, 7, 5);
   }
   break;   
 case 0xc6:
   {
     u32 address = reg[6].I;
     u32 temp = address + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     STM_REG(1, 0, 6);
     STM_REG(2, 1, 6);
     STM_REG(4, 2, 6);
     STM_REG(8, 3, 6);
     STM_REG(16, 4, 6);
     STM_REG(32, 5, 6);
     STM_REG(64, 6, 6);
     STM_REG(128, 7, 6);
   }
   break;   
 case 0xc7:
   {
     u32 address = reg[7].I;
     u32 temp = address + 4*cpuBitsSet[opcode & 0xff];
     int offset = 0;
     // store
     STM_REG(1, 0, 7);
     STM_REG(2, 1, 7);
     STM_REG(4, 2, 7);
     STM_REG(8, 3, 7);
     STM_REG(16, 4, 7);
     STM_REG(32, 5, 7);
     STM_REG(64, 6, 7);
     STM_REG(128, 7, 7);
   }
   break;
#define LDM_REG(val,r) \
  if(opcode & (val)) {\
    reg[(r)].I = CPUReadMemory(address & 0xFFFFFFFC);\
    address += 4;\
    clockTicks += 2;\
  }
 case 0xc8:
   {
     u32 address = reg[0].I;
     // load
     LDM_REG(1, 0);
     LDM_REG(2, 1);
     LDM_REG(4, 2);
     LDM_REG(8, 3);
     LDM_REG(16, 4);
     LDM_REG(32, 5);
     LDM_REG(64, 6);
     LDM_REG(128, 7);
     if(!(opcode & 1))
       reg[0].I = address;
   }
   break;
 case 0xc9:
   {
     u32 address = reg[1].I;
     // load
     LDM_REG(1, 0);
     LDM_REG(2, 1);
     LDM_REG(4, 2);
     LDM_REG(8, 3);
     LDM_REG(16, 4);
     LDM_REG(32, 5);
     LDM_REG(64, 6);
     LDM_REG(128, 7);
     if(!(opcode & 2))
       reg[1].I = address;
   }
   break;
 case 0xca:
   {
     u32 address = reg[2].I;
     // load
     LDM_REG(1, 0);
     LDM_REG(2, 1);
     LDM_REG(4, 2);
     LDM_REG(8, 3);
     LDM_REG(16, 4);
     LDM_REG(32, 5);
     LDM_REG(64, 6);
     LDM_REG(128, 7);
     if(!(opcode & 4))
       reg[2].I = address;
   }
   break;
 case 0xcb:
   {
     u32 address = reg[3].I;
     // load
     LDM_REG(1, 0);
     LDM_REG(2, 1);
     LDM_REG(4, 2);
     LDM_REG(8, 3);
     LDM_REG(16, 4);
     LDM_REG(32, 5);
     LDM_REG(64, 6);
     LDM_REG(128, 7);
     if(!(opcode & 8))
       reg[3].I = address;
   }
   break;
 case 0xcc:
   {
     u32 address = reg[4].I;
     // load
     LDM_REG(1, 0);
     LDM_REG(2, 1);
     LDM_REG(4, 2);
     LDM_REG(8, 3);
     LDM_REG(16, 4);
     LDM_REG(32, 5);
     LDM_REG(64, 6);
     LDM_REG(128, 7);   
     if(!(opcode & 16))
       reg[4].I = address;
   }
   break;
 case 0xcd:
   {
     u32 address = reg[5].I;
     // load
     LDM_REG(1, 0);
     LDM_REG(2, 1);
     LDM_REG(4, 2);
     LDM_REG(8, 3);
     LDM_REG(16, 4);
     LDM_REG(32, 5);
     LDM_REG(64, 6);
     LDM_REG(128, 7);   
     if(!(opcode & 32))
       reg[5].I = address;
   }
   break;
 case 0xce:
   {
     u32 address = reg[6].I;
     // load
     LDM_REG(1, 0);
     LDM_REG(2, 1);
     LDM_REG(4, 2);
     LDM_REG(8, 3);
     LDM_REG(16, 4);
     LDM_REG(32, 5);
     LDM_REG(64, 6);
     LDM_REG(128, 7);   
     if(!(opcode & 64))
       reg[6].I = address;
   }
   break;
 case 0xcf:
   {
     u32 address = reg[7].I;
     // load
     LDM_REG(1, 0);
     LDM_REG(2, 1);
     LDM_REG(4, 2);
     LDM_REG(8, 3);
     LDM_REG(16, 4);
     LDM_REG(32, 5);
     LDM_REG(64, 6);
     LDM_REG(128, 7);   
     if(!(opcode & 128))
       reg[7].I = address;
   }
   break;
 case 0xd0:
   if(Z_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;
   }
   break;      
 case 0xd1:
   if(!Z_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xd2:
   if(C_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xd3:
   if(!C_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xd4:
   if(N_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xd5:
   if(!N_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xd6:
   if(V_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xd7:
   if(!V_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xd8:
   if(C_FLAG && !Z_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xd9:
   if(!C_FLAG || Z_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xda:
   if(N_FLAG == V_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xdb:
   if(N_FLAG != V_FLAG) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xdc:
   if(!Z_FLAG && (N_FLAG == V_FLAG)) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xdd:
   if(Z_FLAG || (N_FLAG != V_FLAG)) {
     reg[15].I += ((s8)(opcode & 0xFF)) << 1;       
     armNextPC = reg[15].I;
     reg[15].I += 2;
     clockTicks = 3;       
   }
   break;   
 case 0xdf:
   CPUSoftwareInterrupt(opcode & 0xFF);
   break;
 case 0xe0:
 case 0xe1:
 case 0xe2:
 case 0xe3:
 case 0xe4:
 case 0xe5:
 case 0xe6:
 case 0xe7:
 case 0xe8:
 case 0xe9:
 case 0xea:
 case 0xeb:
 case 0xec:
 case 0xed:
 case 0xee:
 case 0xef:
   {
     // unconditional branch
     int offset = (opcode & 0x3FF) << 1;
     if(opcode & 0x0400)
       offset |= 0xFFFFF800;
     reg[15].I += offset;
     armNextPC = reg[15].I;
     reg[15].I += 2;
   }
   break;
 case 0xf0:
 case 0xf1:
 case 0xf2:
 case 0xf3:
   {
     // long branch with link
     int offset = (opcode & 0x7FF);
     reg[14].I = reg[15].I + (offset << 12);
   }
   break;      
 case 0xf4:
 case 0xf5:
 case 0xf6:
 case 0xf7:
   {
     // long branch with link
     int offset = (opcode & 0x7FF);
     reg[14].I = reg[15].I + ((offset << 12) | 0xFF800000);
   }
   break;   
 case 0xf8:
 case 0xf9:
 case 0xfa:
 case 0xfb:
 case 0xfc:
 case 0xfd:
 case 0xfe:
 case 0xff:
   {
     // long branch with link
     int offset = (opcode & 0x7FF);
     u32 temp = reg[15].I-2;
     reg[15].I = (reg[14].I + (offset<<1))&0xFFFFFFFE;
     armNextPC = reg[15].I;
     reg[15].I += 2;
     reg[14].I = temp|1;
   }
   break;
#ifdef BKPT_SUPPORT
 case 0xbe:
   extern void (*dbgSignal)(int,int);
   reg[15].I -= 2;
   armNextPC -= 2;   
   dbgSignal(5, opcode & 255);
   return;
#endif
 case 0xb1:
 case 0xb2:
 case 0xb3:
 case 0xb6:
 case 0xb7:
 case 0xb8:
 case 0xb9:
 case 0xba:
 case 0xbb:
#ifndef BKPT_SUPPORT
 case 0xbe:
#endif
 case 0xbf:
 case 0xde:
 default:
 unknown_thumb:
#ifdef BKPT_SUPPORT
    extern void (*dbgSignal)(int,int);
    reg[15].I -= 2;
    armNextPC -= 2;    
    dbgSignal(4, 0);
    return;   
#else
   // NOT DEFINED
   systemMessage(MSG_UNKNOWN_THUMB_OPCODE,
                 "Unknown opcode %04x from %08x", opcode, armNextPC);
   break;
#endif
}
