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
#include "Wnd.h"
#include "../System.h"
#include "WinResUtil.h"
#include "resource.h"

extern void winCenterWindow(HWND);

extern HWND hWindow;
extern int gbRomSize;

class RomInfoGBA : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  RomInfoGBA();

  virtual BOOL OnInitDialog(LPARAM);
  void OnOk();
};

class RomInfoGB : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  RomInfoGB();

  virtual BOOL OnInitDialog(LPARAM);
  void OnOk();
};

struct WinGBACompanyName {
  char *code;
  char *name;
};

static WinGBACompanyName winGBARomInfoCompanies[] = {
  { "01", "Nintendo" },
  { "02", "Rocket Games" },
  { "08", "Capcom" },
  { "09", "Hot B Co." },
  { "0A", "Jaleco" },
  { "0B", "Coconuts" },
  { "0C", "Coconuts Japan/Elite" },
  { "0H", "Starfish" },
  { "0L", "Warashi Inc." },
  { "13", "Electronic Arts Japan" },
  { "18", "Hudson Soft Japan" },
  { "1A", "Yonoman/Japan Art Media" },
  { "1P", "Creatures Inc." },
  { "20", "Destination Software" },
  { "22", "VR 1 Japan" },
  { "25", "San-X" },
  { "28", "Kemco Japan" },
  { "29", "Seta" },
  { "2K", "NEC InterChannel" },
  { "2L", "Tam" },
  { "2M", "GU Inc/Gajin/Jordan" },
  { "2N", "Smilesoft" },
  { "2Q", "Mediakite/Systemsoft Alpha Corp" },
  { "36", "Codemasters" },
  { "37", "GAGA Communications" },
  { "38", "Laguna" },
  { "39", "Telstar Fun and Games" },
  { "41", "Ubi Soft Entertainment" },
  { "47", "Spectrum Holobyte" },
  { "49", "IREM" },
  { "4D", "Malibu Games" },
  { "4F", "U.S. Gold" },
  { "4J", "Fox Interactive" },
  { "4K", "Time Warner Interactive" },
  { "4Q", "Disney" },
  { "4S", "EA SPORTS/THQ" },
  { "4X", "GT Interactive" },
  { "4Y", "RARE" },
  { "4Z", "Crave Entertainment" },
  { "50", "Absolute Entertainment" },
  { "51", "Acclaim" },
  { "52", "Activision" },
  { "53", "American Sammy Corp." },
  { "54", "Take 2 Interactive" },
  { "55", "Hi Tech" },
  { "56", "LJN LTD." },
  { "58", "Mattel" },
  { "5A", "Red Orb Entertainment" },
  { "5C", "Taxan" },
  { "5D", "Midway" },
  { "5G", "Majesco Sales Inc" },
  { "5H", "3DO" },
  { "5K", "Hasbro" },
  { "5L", "NewKidCo" },
  { "5M", "Telegames" },
  { "5N", "Metro3D" },
  { "5P", "Vatical Entertainment" },
  { "5Q", "LEGO Media" },
  { "5S", "Xicat Interactive" },
  { "5T", "Cryo Interactive" },
  { "5X", "Microids" },
  { "5W", "BKN Ent./Red Storm Ent." },
  { "5Z", "Conspiracy Entertainment Corp." },
  { "60", "Titus Interactive Studios" },
  { "61", "Virgin Interactive" },
  { "64", "LucasArts Entertainment" },
  { "67", "Ocean" },
  { "69", "Electronic Arts" },
  { "6E", "Elite Systems Ltd." },
  { "6F", "Electro Brain" },
  { "6H", "Crawfish" },
  { "6L", "BAM! Entertainment" },
  { "6M", "Studio 3" },
  { "6Q", "Classified Games" },
  { "6S", "TDK Mediactive" },
  { "6U", "DreamCatcher" },
  { "6V", "JoWood Productions" },
  { "6W", "SEGA" },
  { "6Y", "LSP" },
  { "70", "Infogrames" },
  { "71", "Interplay" },
  { "72", "JVC Musical Industries Inc" },
  { "75", "SCI" },
  { "78", "THQ" },
  { "79", "Accolade" },
  { "7A", "Triffix Ent. Inc." },
  { "7C", "Microprose Software" },
  { "7D", "Universal Interactive Studios" },
  { "7F", "Kemco" },
  { "7G", "Rage Software" },
  { "7M", "Asmik Ace Entertainment Inc./AIA" },
  { "83", "LOZC/G.Amusements" },
  { "8B", "Bulletproof Software" },
  { "8C", "Vic Tokai Inc." },
  { "8J", "General Entertainment" },
  { "8N", "Success" },
  { "8P", "SEGA Japan" },
  { "91", "Chun Soft" },
  { "93", "BEC" },
  { "97", "Kaneko" },
  { "99", "Victor Interactive Software" },
  { "9B", "Tecmo" },
  { "9C", "Imagineer" },
  { "9H", "Bottom Up" },
  { "9N", "Marvelous Entertainment" },
  { "9P", "Keynet Inc." },
  { "9Q", "Hands-On Entertainment" },
  { "A0", "Telenet/Olympia" },
  { "A1", "Hori" },
  { "A4", "Konami" },
  { "A6", "Kawada" },
  { "A7", "Takara" },
  { "A9", "Technos Japan Corp." },
  { "AC", "Toei Animation" },
  { "AD", "Toho" },
  { "AF", "Namco" },
  { "AG", "Media Rings Corporation/Amedio/Playmates" },
  { "AH", "J-Wing" },
  { "AK", "KID" },
  { "AL", "MediaFactory" },
  { "B0", "Acclaim Japan" },
  { "B1", "Nexoft" },
  { "B2", "Bandai" },
  { "B4", "Enix" },
  { "B6", "HAL Laboratory" },
  { "B7", "SNK" },
  { "B9", "Pony Canyon" },
  { "BA", "Culture Brain" },
  { "BB", "Sunsoft" },
  { "BD", "Sony Imagesoft" },
  { "BF", "Sammy" },
  { "BG", "Magical" },
  { "BJ", "Compile" },
  { "BL", "MTO Inc." },
  { "BN", "Sunrise Interactive" },
  { "BP", "Global A Entertainment" },
  { "BQ", "Fuuki" },
  { "C0", "Taito" },
  { "C2", "Kemco" },
  { "C3", "Square Soft" },
  { "C5", "Data East" },
  { "C6", "Broderbund Japan" },
  { "C8", "Koei" },
  { "CA", "Ultra Games" },
  { "CB", "Vapinc/NTVIC" },
  { "CC", "Use Co.,Ltd." },
  { "CE", "FCI" },
  { "CF", "Angel" },
  { "CM", "Konami Computer Enterteinment Osaka" },
  { "D1", "Sofel" },
  { "D3", "Sigma Enterprises" },
  { "D4", "Ask Kodansa/Lenar" },
  { "D7", "Copya System" },
  { "D9", "Banpresto" },
  { "DA", "TOMY" },
  { "DD", "NCS" },
  { "DF", "Altron Corporation" },
  { "E2", "Lad/Shogakukan.Nas/Yutaka" },
  { "E3", "Varie" },
  { "E5", "Epoch" },
  { "E7", "Athena" },
  { "E8", "Asmik Ace Entertainment Inc." },
  { "E9", "Natsume" },
  { "EA", "King Records" },
  { "EB", "Atlus" },
  { "EC", "Epic/Sony/Ocean" },
  { "EE", "IGS" },
  { "EL", "Vaill" },
  { "EM", "Konami Computer Entertainment Tokyo" },
  { "EN", "Alphadream Corporation" },
  { "F0", "A Wave" },
  { "HY", "Sachen" },
  { NULL, NULL }
};

static char *winGBARomInfoFindMakerCode(char *code)
{
  int i = 0;
  while(winGBARomInfoCompanies[i].code) {
    if(!strcmp(winGBARomInfoCompanies[i].code, code))
      return winGBARomInfoCompanies[i].name;
    i++;
  }
  return (char *)winResLoadString(IDS_UNKNOWN);
}

BEGIN_MESSAGE_MAP(RomInfoGBA, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
END_MESSAGE_MAP()

RomInfoGBA::RomInfoGBA()
  : Dlg()
{
}

BOOL RomInfoGBA::OnInitDialog(LPARAM l)
{
  char buffer[13];
  u8 *rom = (u8 *)l;
  strncpy(buffer, (const char *)&rom[0xa0], 12);
  buffer[12] = 0;
  ::SetWindowText(GetDlgItem(IDC_ROM_TITLE), buffer);

  strncpy(buffer, (const char *)&rom[0xac], 4);
  buffer[4] = 0;
  ::SetWindowText(GetDlgItem(IDC_ROM_GAME_CODE), buffer);

  strncpy(buffer, (const char *)&rom[0xb0],2);
  buffer[2] = 0;
  ::SetWindowText(GetDlgItem(IDC_ROM_MAKER_CODE), buffer);

  ::SetWindowText(GetDlgItem(IDC_ROM_MAKER_NAME),
                winGBARomInfoFindMakerCode(buffer));
  
  sprintf(buffer, "%02x", rom[0xb3]);
  ::SetWindowText(GetDlgItem(IDC_ROM_UNIT_CODE), buffer);

  sprintf(buffer, "%02x", rom[0xb4]);
  ::SetWindowText(GetDlgItem(IDC_ROM_DEVICE_TYPE), buffer);

  sprintf(buffer, "%02x", rom[0xbc]);
  ::SetWindowText(GetDlgItem(IDC_ROM_VERSION), buffer);

  u8 crc = 0x19;
  for(int i = 0xa0; i < 0xbd; i++) {
    crc += rom[i];
  }

  crc = (-crc) & 255;

  sprintf(buffer, "%02x (%02x)", crc, rom[0xbd]);
  ::SetWindowText(GetDlgItem(IDC_ROM_CRC), buffer);
  winCenterWindow(getHandle());
  return TRUE;
}

void RomInfoGBA::OnOk()
{
  EndDialog(TRUE);
}

BEGIN_MESSAGE_MAP(RomInfoGB, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
END_MESSAGE_MAP()

RomInfoGB::RomInfoGB()
  : Dlg()
{
}

BOOL RomInfoGB::OnInitDialog(LPARAM l)
{
  char buffer[128];
  u8 *rom = (u8 *)l;
  
  strncpy(buffer, (const char *)&rom[0x134], 15);
  buffer[15] = 0;
  ::SetWindowText(GetDlgItem(IDC_ROM_TITLE), buffer);

  sprintf(buffer, "%02x", rom[0x143]);
  ::SetWindowText(GetDlgItem(IDC_ROM_COLOR), buffer);
  
  strncpy(buffer, (const char *)&rom[0x144],2);
  buffer[2] = 0;
  ::SetWindowText(GetDlgItem(IDC_ROM_MAKER_CODE), buffer);

  if(rom[0x14b] != 0x33)
    sprintf(buffer, "%02x", rom[0x14b]);
  ::SetWindowText(GetDlgItem(IDC_ROM_MAKER_NAME2),
                winGBARomInfoFindMakerCode(buffer));
  
  sprintf(buffer, "%02x", rom[0x146]);
  ::SetWindowText(GetDlgItem(IDC_ROM_UNIT_CODE), buffer);

  char *type = (char *)winResLoadString(IDS_UNKNOWN);
  switch(rom[0x147]) {
  case 0x00:
    type = "ROM";
    break;
  case 0x01:
    type = "ROM+MBC1";
    break;
  case 0x02:
    type = "ROM+MBC1+RAM";
    break;
  case 0x03:
    type = "ROM+MBC1+RAM+BATT";
    break;
  case 0x05:
    type = "ROM+MBC2";
    break;
  case 0x06:
    type = "ROM+MBC2+BATT";
    break;
  case 0x0f:
    type = "ROM+MBC3+TIMER+BATT";
    break;
  case 0x10:
    type = "ROM+MBC3+TIMER+RAM+BATT";
    break;
  case 0x11:
    type = "ROM+MBC3";
    break;
  case 0x12:
    type = "ROM+MBC3+RAM";
    break;
  case 0x13:
    type = "ROM+MBC3+RAM+BATT";
    break;
  case 0x19:
    type = "ROM+MBC5";
    break;
  case 0x1a:
    type = "ROM+MBC5+RAM";
    break;
  case 0x1b:
    type = "ROM+MBC5+RAM+BATT";
    break;
  case 0x1c:
    type = "ROM+MBC5+RUMBLE";
    break;
  case 0x1d:
    type = "ROM+MBC5+RUMBLE+RAM";
    break;
  case 0x1e:
    type = "ROM+MBC5+RUMBLE+RAM+BATT";
    break;
  case 0x22:
    type = "ROM+MBC7+BATT";
    break;
  case 0xfe:
    type = "ROM+HuC-3";
    break;
  case 0xff:
    type = "ROM+HuC-1";
    break;
  }
  sprintf(buffer, "%02x (%s)", rom[0x147], type);
  ::SetWindowText(GetDlgItem(IDC_ROM_DEVICE_TYPE), buffer);

  type = (char *)winResLoadString(IDS_UNKNOWN);
  switch(rom[0x148]) {
  case 0:
    type = "32K";
    break;
  case 1:
    type = "64K";
    break;
  case 2:
    type = "128K";
    break;
  case 3:
    type = "256K";
    break;
  case 4:
    type = "512K";
    break;
  case 5:
    type = "1M";
    break;
  case 6:
    type = "2M";
    break;
  case 7:
    type = "4M";
    break;
  }

  sprintf(buffer, "%02x (%s)", rom[0x148], type);
  ::SetWindowText(GetDlgItem(IDC_ROM_SIZE), buffer);

  type = (char *)winResLoadString(IDS_UNKNOWN);
  switch(rom[0x149]) {
  case 0:
    type = (char *)winResLoadString(IDS_NONE);
    break;
  case 1:
    type = "2K";
    break;
  case 2:
    type = "8K";
    break;
  case 3:
    type = "32K";
    break;
  case 4:
    type = "128K";
    break;
  case 5:
    type = "64K";
    break;
  }

  sprintf(buffer, "%02x (%s)", rom[0x149], type);
  ::SetWindowText(GetDlgItem(IDC_ROM_RAM_SIZE), buffer);

  sprintf(buffer, "%02x", rom[0x14a]);
  ::SetWindowText(GetDlgItem(IDC_ROM_DEST_CODE), buffer);

  sprintf(buffer, "%02x", rom[0x14b]);
  ::SetWindowText(GetDlgItem(IDC_ROM_LIC_CODE), buffer);
  
  sprintf(buffer, "%02x", rom[0x14c]);
  ::SetWindowText(GetDlgItem(IDC_ROM_VERSION), buffer);

  u8 crc = 25;
  int i;
  for(i = 0x134; i < 0x14d; i++) {
    crc += rom[i];
  }

  crc = 256 - crc;
  
  sprintf(buffer, "%02x (%02x)", crc, rom[0x14d]);
  ::SetWindowText(GetDlgItem(IDC_ROM_CRC), buffer);

  u16 crc16 = 0;
  for(i = 0; i < gbRomSize; i++) {
    crc16 += rom[i];
  }

  crc16 -= rom[0x14e];
  crc16 -= rom[0x14f];
  sprintf(buffer, "%04x (%04x)", crc16, (rom[0x14e]<<8)|rom[0x14f]);
  ::SetWindowText(GetDlgItem(IDC_ROM_CHECKSUM), buffer);
  winCenterWindow(getHandle());
  return TRUE;
}

void RomInfoGB::OnOk()
{
  EndDialog(TRUE);
}

void winGBARomInfo(u8 *rom)
{
  RomInfoGBA gba;
  gba.DoModal(IDD_GBA_ROM_INFO,
              hWindow,
              (LPARAM)rom);
}

void winGBRomInfo(u8 *rom)
{
  RomInfoGB gb;
  gb.DoModal(IDD_GB_ROM_INFO,
             hWindow,
             (LPARAM)rom);
}
