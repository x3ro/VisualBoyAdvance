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
#include <windows.h>

static char buffer[2048];
static HKEY vbKey = NULL;

void regInit()
{
  DWORD disp = 0;
  LONG res = RegCreateKeyEx(HKEY_CURRENT_USER,
                            "Software\\Emulators\\VisualBoyAdvance",
                            0,
                            "",
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &vbKey,
                            &disp);
}

void regShutdown()
{
  LONG res = RegCloseKey(vbKey);
}

char *regQueryStringValue(char * key, char *def)
{
  DWORD type = 0;
  DWORD size = 2048;
  
  LONG res = RegQueryValueEx(vbKey,
                             key,
                             NULL,
                             &type,
                             (UCHAR *)buffer,
                             &size);

  if(res == ERROR_SUCCESS && type == REG_SZ)
    return buffer;

  return def;
}

DWORD regQueryDwordValue(char * key, DWORD def)
{
  DWORD type = 0;
  DWORD size = sizeof(DWORD);
  DWORD result = 0;
  
  LONG res = RegQueryValueEx(vbKey,
                             key,
                             NULL,
                             &type,
                             (UCHAR *)&result,
                             &size);
  
  if(res == ERROR_SUCCESS && type == REG_DWORD)
    return result;

  return def;
}

BOOL regQueryBinaryValue(char * key, char *value, int count)
{
  DWORD type = 0;
  DWORD size = count;
  DWORD result = 0;
  
  LONG res = RegQueryValueEx(vbKey,
                             key,
                             NULL,
                             &type,
                             (UCHAR *)value,
                             &size);
  
  if(res == ERROR_SUCCESS && type == REG_BINARY)
    return TRUE;

  return FALSE;
}

void regSetStringValue(char * key, char * value)
{
  LONG res = RegSetValueEx(vbKey,
                           key,
                           NULL,
                           REG_SZ,
                           (const UCHAR *)value,
                           strlen(value)+1);
  
}

void regSetDwordValue(char * key, DWORD value)
{
  LONG res = RegSetValueEx(vbKey,
                           key,
                           NULL,
                           REG_DWORD,
                           (const UCHAR *)&value,
                           sizeof(DWORD));
}

void regSetBinaryValue(char *key, char *value, int count)
{
  LONG res = RegSetValueEx(vbKey,
                           key,
                           NULL,
                           REG_BINARY,
                           (const UCHAR *)value,
                           count);  
}

void regDeleteValue(char *key)
{
  LONG res = RegDeleteValue(vbKey,
                            key);
}

bool regCreateFileType(char *ext, char *type)
{
  DWORD disp = 0;
  HKEY key;
  LONG res = RegCreateKeyEx(HKEY_CLASSES_ROOT,
                            ext,
                            0,
                            "",
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &key,
                            &disp);
  if(res == ERROR_SUCCESS) {
    res = RegSetValueEx(key,
                        "",
                        0,
                        REG_SZ,
                        (const UCHAR *)type,
                        strlen(type)+1);
    RegCloseKey(key);
    return true;
  }
  return false;
}

bool regAssociateType(char *type, char *desc, char *application)
{
  DWORD disp = 0;
  HKEY key;
  LONG res = RegCreateKeyEx(HKEY_CLASSES_ROOT,
                            type,
                            0,
                            "",
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &key,
                            &disp);
  if(res == ERROR_SUCCESS) {
    res = RegSetValueEx(key,
                        "",
                        0,
                        REG_SZ,
                        (const UCHAR *)desc,
                        strlen(desc)+1);
    HKEY key2;
    res = RegCreateKeyEx(key,
                         "Shell\\Open\\Command",
                         0,
                         "",
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &key2,
                         &disp);
    if(res == ERROR_SUCCESS) {
      res = RegSetValueEx(key2,
                          "",
                          0,
                          REG_SZ,
                          (const UCHAR *)application,
                          strlen(application)+1);
      RegCloseKey(key2);
      RegCloseKey(key);
      return true;
    }
    
    RegCloseKey(key);
  }
  return false;
}
