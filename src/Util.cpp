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

/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */
#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <png.h>
}

#if 0
#include "unrarlib.h"
#endif

#include "System.h"
#include "NLS.h"
#include "Util.h"

#ifdef __GNUC__
#define _stricmp strcasecmp
#endif

bool utilWritePNGFile(char *fileName, int w, int h, u8 *pix)
{
  u8 writeBuffer[512 * 3];
  
  FILE *fp = fopen(fileName,"wb");

  if(!fp) {
    systemMessage(MSG_ERROR_CREATING_FILE, "Error creating file %s", fileName);
    return false;
  }
  
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
                                                NULL,
                                                NULL);
  if(!png_ptr) {
    fclose(fp);
    return false;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if(!info_ptr) {
    png_destroy_write_struct(&png_ptr,NULL);
    fclose(fp);
    return false;
  }

  if(setjmp(png_ptr->jmpbuf)) {
    png_destroy_write_struct(&png_ptr,NULL);
    fclose(fp);
    return false;
  }

  png_init_io(png_ptr,fp);

  png_set_IHDR(png_ptr,
               info_ptr,
               w,
               h,
               8,
               PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr,info_ptr);

  u8 *b = writeBuffer;

  int sizeX = w;
  int sizeY = h;

  switch(systemColorDepth) {
  case 16:
    {
      u16 *p = (u16 *)(pix+(w+2)*2); // skip first black line
      for(int y = 0; y < sizeY; y++) {
         for(int x = 0; x < sizeX; x++) {
          u16 v = *p++;
          
          *b++ = ((v >> systemRedShift) & 0x001f) << 3; // R
          *b++ = ((v >> systemGreenShift) & 0x001f) << 3; // G 
          *b++ = ((v >> systemBlueShift) & 0x01f) << 3; // B
        }
        p++; // skip black pixel for filters
        p++; // skip black pixel for filters
        png_write_row(png_ptr,writeBuffer);
        
        b = writeBuffer;
      }
    }
    break;
  case 24:
    {
      u8 *pixU8 = (u8 *)pix;
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          if(systemRedShift < systemBlueShift) {
            *b++ = *pixU8++; // R
            *b++ = *pixU8++; // G
            *b++ = *pixU8++; // B
          } else {
            int blue = *pixU8++;
            int green = *pixU8++;
            int red = *pixU8++;
            
            *b++ = red;
            *b++ = green;
            *b++ = blue;
          }
        }
        png_write_row(png_ptr,writeBuffer);
        
        b = writeBuffer;
      }
    }
    break;
  case 32:
    {
      u32 *pixU32 = (u32 *)(pix+4*(w+1));
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          u32 v = *pixU32++;
          
          *b++ = ((v >> systemRedShift) & 0x001f) << 3; // R
          *b++ = ((v >> systemGreenShift) & 0x001f) << 3; // G
          *b++ = ((v >> systemBlueShift) & 0x001f) << 3; // B
        }
        pixU32++;
        
        png_write_row(png_ptr,writeBuffer);
        
        b = writeBuffer;
      }
    }
    break;
  }
  
  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(fp);

  return true;  
}

void utilPutDword(u8 *p, u32 value)
{
  *p++ = value & 255;
  *p++ = (value >> 8) & 255;
  *p++ = (value >> 16) & 255;
  *p = (value >> 24) & 255;
}

void utilPutWord(u8 *p, u16 value)
{
  *p++ = value & 255;
  *p = (value >> 8) & 255;
}

void utilWriteBMP(char *buf, int w, int h, u8 *pix)
{
  u8 *b = (u8 *)buf;

  int sizeX = w;
  int sizeY = h;

  switch(systemColorDepth) {
  case 16:
    {
      u16 *p = (u16 *)(pix+(w+2)*(h)*2); // skip first black line
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          u16 v = *p++;

          *b++ = ((v >> systemBlueShift) & 0x01f) << 3; // B      
          *b++ = ((v >> systemGreenShift) & 0x001f) << 3; // G 
          *b++ = ((v >> systemRedShift) & 0x001f) << 3; // R
        }
        p++; // skip black pixel for filters
        p++; // skip black pixel for filters
        p -= 2*(w+2);
      }
    }
    break;
  case 24:
    {
      u8 *pixU8 = (u8 *)pix+3*w*(h-1);
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          if(systemRedShift > systemBlueShift) {
            *b++ = *pixU8++; // B
            *b++ = *pixU8++; // G
            *b++ = *pixU8++; // R           
          } else {
            int red = *pixU8++;
            int green = *pixU8++;
            int blue = *pixU8++;
            
            *b++ = blue;
            *b++ = green;
            *b++ = red;
          }
        }
        pixU8 -= 2*3*w;
      }
    }
    break;
  case 32:
    {
      u32 *pixU32 = (u32 *)(pix+4*(w+1)*(h));
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          u32 v = *pixU32++;

          *b++ = ((v >> systemBlueShift) & 0x001f) << 3; // B     
          *b++ = ((v >> systemGreenShift) & 0x001f) << 3; // G
          *b++ = ((v >> systemRedShift) & 0x001f) << 3; // R
        }
        pixU32++;
        pixU32 -= 2*(w+1);
      }
    }
    break;
  }  
}

bool utilWriteBMPFile(char *fileName, int w, int h, u8 *pix)
{
  u8 writeBuffer[512 * 3];
  
  FILE *fp = fopen(fileName,"wb");

  if(!fp) {
    systemMessage(MSG_ERROR_CREATING_FILE, "Error creating file %s", fileName);
    return false;
  }

  struct {
    u8 ident[2];
    u8 filesize[4];
    u8 reserved[4];
    u8 dataoffset[4];
    u8 headersize[4];
    u8 width[4];
    u8 height[4];
    u8 planes[2];
    u8 bitsperpixel[2];
    u8 compression[4];
    u8 datasize[4];
    u8 hres[4];
    u8 vres[4];
    u8 colors[4];
    u8 importantcolors[4];
    //    u8 pad[2];
  } bmpheader;
  memset(&bmpheader, 0, sizeof(bmpheader));

  bmpheader.ident[0] = 'B';
  bmpheader.ident[1] = 'M';

  u32 fsz = sizeof(bmpheader) + w*h*3;
  utilPutDword(bmpheader.filesize, fsz);
  utilPutDword(bmpheader.dataoffset, 0x36);
  utilPutDword(bmpheader.headersize, 0x28);
  utilPutDword(bmpheader.width, w);
  utilPutDword(bmpheader.height, h);
  utilPutDword(bmpheader.planes, 1);
  utilPutDword(bmpheader.bitsperpixel, 24);
  utilPutDword(bmpheader.datasize, 3*w*h);

  fwrite(&bmpheader, 1, sizeof(bmpheader), fp);

  u8 *b = writeBuffer;

  int sizeX = w;
  int sizeY = h;

  switch(systemColorDepth) {
  case 16:
    {
      u16 *p = (u16 *)(pix+(w+2)*(h)*2); // skip first black line
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          u16 v = *p++;

          *b++ = ((v >> systemBlueShift) & 0x01f) << 3; // B      
          *b++ = ((v >> systemGreenShift) & 0x001f) << 3; // G 
          *b++ = ((v >> systemRedShift) & 0x001f) << 3; // R
        }
        p++; // skip black pixel for filters
        p++; // skip black pixel for filters
        p -= 2*(w+2);
        fwrite(writeBuffer, 1, 3*w, fp);
        
        b = writeBuffer;
      }
    }
    break;
  case 24:
    {
      u8 *pixU8 = (u8 *)pix+3*w*(h-1);
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          if(systemRedShift > systemBlueShift) {
            *b++ = *pixU8++; // B
            *b++ = *pixU8++; // G
            *b++ = *pixU8++; // R           
          } else {
            int red = *pixU8++;
            int green = *pixU8++;
            int blue = *pixU8++;
            
            *b++ = blue;
            *b++ = green;
            *b++ = red;
          }
        }
        pixU8 -= 2*3*w;
        fwrite(writeBuffer, 1, 3*w, fp);
        
        b = writeBuffer;
      }
    }
    break;
  case 32:
    {
      u32 *pixU32 = (u32 *)(pix+4*(w+1)*(h));
      for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
          u32 v = *pixU32++;

          *b++ = ((v >> systemBlueShift) & 0x001f) << 3; // B     
          *b++ = ((v >> systemGreenShift) & 0x001f) << 3; // G
          *b++ = ((v >> systemRedShift) & 0x001f) << 3; // R
        }
        pixU32++;
        pixU32 -= 2*(w+1);
        
        fwrite(writeBuffer, 1, 3*w, fp);
        
        b = writeBuffer;
      }
    }
    break;
  }

  fclose(fp);

  return true;
}

// IPS patching adapted from Snes9x memmap.cpp file

// Read variable size MSB int from a file
static int utilReadInt(FILE *f, int nbytes)
{
  long v = 0;
  while (nbytes--) {
    int c = fgetc(f);
    if (c == EOF) 
      return -1;
    v = (v << 8) | (c & 0xFF);
  }
  return (v);
}

void utilApplyIPS(char *ips, u8 **r, int *s)
{
  char buffer[10];
  
  FILE *patch = NULL;
  int offset = 0;

  u8 *rom = *r;
  int size = *s;

  if(!(patch = fopen (ips, "rb"))) {
    return;
  }

  if(fread (buffer, 1, 5, patch) != 5 ||
     strncmp (buffer, "PATCH", 5) != 0) {
    fclose (patch);
    return;
  }
  
  for(;;) {
    int len;
    int c;

    offset = utilReadInt(patch, 3);
    if(offset == -1)
      goto err;

    // IPS end
    if (offset == 0x454f46)
      break;

    len = utilReadInt(patch, 2);
    if(len == -1)
      goto err;

    // if not zero, then it is a patch block
    if (len) {
      while(len--) {
        if(offset >= size) {
          rom = (u8 *)realloc(rom, (size<<1));
          *r = rom;
          *s = size = (size << 1);
        }
        c = fgetc(patch);
        if(c == EOF) 
          goto err;
        rom[offset++] = (u8)c;
      }
    } else {
      // RLE block
      len = utilReadInt(patch, 2);
      if(len == -1)
        goto err;
      c = fgetc(patch);
      
      if(c == EOF) 
        goto err;

      if((offset + len) >= size) {
        rom = (u8 *)realloc(rom, (size<<1));
        *r = rom;
        *s = size = (size << 1);        
      }
      
      while(len--) 
        rom[offset++] = (u8)c;
    }
  }

 err:
  if(patch)
    fclose(patch);
}

extern bool cpuIsMultiBoot;

bool utilIsGBAImage(const char * file)
{
  cpuIsMultiBoot = false;
  if(strlen(file) > 4) {
    char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".gba") == 0)
        return true;
      if(_stricmp(p, ".agb") == 0)
        return true;
      if(_stricmp(p, ".bin") == 0)
        return true;
      if(_stricmp(p, ".elf") == 0)
        return true;
      if(_stricmp(p, ".mb") == 0) {
        cpuIsMultiBoot = true;
        return true;
      }
    }
  }

  return false;
}

bool utilIsGBImage(const char * file)
{
  if(strlen(file) > 4) {
    char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".gb") == 0)
        return true;
      if(_stricmp(p, ".gbc") == 0)
        return true;
      if(_stricmp(p, ".cgb") == 0)
        return true;
      if(_stricmp(p, ".sgb") == 0)
        return true;      
    }
  }

  return false;
}

bool utilIsZipFile(const char *file)
{
  if(strlen(file) > 4) {
    char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".zip") == 0)
        return true;
    }
  }

  return false;  
}

#if 0
bool utilIsRarFile(const char *file)
{
  if(strlen(file) > 4) {
    char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".rar") == 0)
        return true;
    }
  }

  return false;  
}
#endif

bool utilIsGzipFile(const char *file)
{
  if(strlen(file) > 3) {
    char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".gz") == 0)
        return true;
      if(_stricmp(p, ".z") == 0)
        return true;
    }
  }

  return false;  
}

void utilGetBaseName(const char *file, char *buffer)
{
  strcpy(buffer, file);

  if(utilIsGzipFile(file)) {
    char *p = strrchr(buffer, '.');

    if(p)
      *p = 0;
  }
}

IMAGE_TYPE utilFindType(const char *file)
{
  char buffer[2048];
  
  if(utilIsZipFile(file)) {
    unzFile unz = unzOpen(file);
    
    if(unz == NULL) {
      systemMessage(MSG_CANNOT_OPEN_FILE, "Cannot open file %s", file);
      return IMAGE_UNKNOWN;
    }
    
    int r = unzGoToFirstFile(unz);
    
    if(r != UNZ_OK) {
      unzClose(unz);
      systemMessage(MSG_BAD_ZIP_FILE, "Bad ZIP file %s", file);
      return IMAGE_UNKNOWN;
    }
    
    IMAGE_TYPE found = IMAGE_UNKNOWN;
    
    unz_file_info info;
    
    while(true) {
      r = unzGetCurrentFileInfo(unz,
                                &info,
                                buffer,
                                sizeof(buffer),
                                NULL,
                                0,
                                NULL,
                                0);
      
      if(r != UNZ_OK) {
        unzClose(unz);
        systemMessage(MSG_BAD_ZIP_FILE,"Bad ZIP file %s", file);
        return IMAGE_UNKNOWN;
      }
      
      if(utilIsGBAImage(buffer)) {
        found = IMAGE_GBA;
        break;
      }

      if(utilIsGBImage(buffer)) {
        found = IMAGE_GB;
        break;
      }
        
      r = unzGoToNextFile(unz);
      
      if(r != UNZ_OK)
        break;
    }
    unzClose(unz);
    
    if(found == IMAGE_UNKNOWN) {
      systemMessage(MSG_NO_IMAGE_ON_ZIP,
                    "No image found on ZIP file %s", file);
      return found;
    }
    return found;
#if 0
  } else if(utilIsRarFile(file)) {
    IMAGE_TYPE found = IMAGE_UNKNOWN;
    
    ArchiveList_struct *rarList = NULL;
    if(urarlib_list((void *)file, (ArchiveList_struct *)&rarList)) {
      ArchiveList_struct *p = rarList;

      while(p) {
        if(utilIsGBAImage(p->item.Name)) {
          found = IMAGE_GBA;
          break;
        }

        if(utilIsGBImage(p->item.Name)) {
          found = IMAGE_GB;
          break;
        }
        p = p->next;
      }
      
      urarlib_freelist(rarList);
    }
    return found;
#endif
  } else {
    if(utilIsGzipFile(file))
      utilGetBaseName(file, buffer);
    else
      strcpy(buffer, file);
    
    if(utilIsGBAImage(buffer))
      return IMAGE_GBA;
    if(utilIsGBImage(buffer))
      return IMAGE_GB;
  }
  return IMAGE_UNKNOWN;  
}

static u8 *utilLoadFromZip(const char *file,
                           bool (*accept)(const char *),
                           u8 *data,
                           int &size)
{
  char buffer[2048];
  
  unzFile unz = unzOpen(file);
    
  if(unz == NULL) {
    systemMessage(MSG_CANNOT_OPEN_FILE, "Cannot open file %s", file);
    return NULL;
  }
  int r = unzGoToFirstFile(unz);
    
  if(r != UNZ_OK) {
    unzClose(unz);
    systemMessage(MSG_BAD_ZIP_FILE, "Bad ZIP file %s", file);
    return NULL;
  }
    
  bool found = false;
    
  unz_file_info info;
  
  while(true) {
    r = unzGetCurrentFileInfo(unz,
                              &info,
                              buffer,
                              sizeof(buffer),
                              NULL,
                              0,
                              NULL,
                              0);
      
    if(r != UNZ_OK) {
      unzClose(unz);
      systemMessage(MSG_BAD_ZIP_FILE,"Bad ZIP file %s", file);
      return NULL;
    }

    if(accept(buffer)) {
      found = true;
      break;
    }
    
    r = unzGoToNextFile(unz);
      
    if(r != UNZ_OK)
      break;
  }

  if(!found) {
    unzClose(unz);
    systemMessage(MSG_NO_IMAGE_ON_ZIP,
                  "No image found on ZIP file %s", file);
    return NULL;
  }
  
  size = info.uncompressed_size;
    
  r = unzOpenCurrentFile(unz);

  if(r != UNZ_OK) {
    unzClose(unz);
    systemMessage(MSG_ERROR_OPENING_IMAGE,"Error opening image %s", buffer);
    return NULL;
  }

  u8 *image = data;
  
  if(image == NULL) {
    image = (u8 *)malloc(size);
    if(image == NULL) {
      unzCloseCurrentFile(unz);
      unzClose(unz);
      systemMessage(MSG_OUT_OF_MEMORY, "Failed to allocate memory for %s",
                    "data");
      return NULL;
    }
  }
  
  r = unzReadCurrentFile(unz,
                         image,
                         size);

  unzCloseCurrentFile(unz);
  unzClose(unz);
  
  if(r != (int)size) {
    systemMessage(MSG_ERROR_READING_IMAGE,
                  "Error reading image %s", buffer);
    if(data == NULL)
      free(image);
    return NULL;
  }

  return image;
}

static u8 *utilLoadGzipFile(const char *file,
                            bool (*accept)(const char *),
                            u8 *data,
                            int &size)
{
  FILE *f = fopen(file, "rb");

  if(f == NULL) {
    systemMessage(MSG_ERROR_OPENING_IMAGE, "Error opening image %s", file);
    return NULL;
  }

  fseek(f, -4, SEEK_END);
  size = fgetc(f) | (fgetc(f) << 8) | (fgetc(f) << 16) | (fgetc(f) << 24);
  fclose(f);

  gzFile gz = gzopen(file, "rb");

  if(gz == NULL) {
    // should not happen, but who knows?
    systemMessage(MSG_ERROR_OPENING_IMAGE, "Error opening image %s", file);
    return NULL;
  }

  u8 *image = data;

  if(image == NULL) {
    image = (u8 *)malloc(size);
    if(image == NULL) {
      systemMessage(MSG_OUT_OF_MEMORY, "Failed to allocate memory for %s",
                    "data");
      fclose(f);
      return NULL;
    }
  }

  int r = gzread(gz, image, size);
  gzclose(gz);

  if(r != (int)size) {
    systemMessage(MSG_ERROR_READING_IMAGE,
                  "Error reading image %s", file);
    if(data == NULL)
      free(image);
    return NULL;
  }  

  return image;  
}

#if 0
static u8 *utilLoadRarFile(const char *file,
                           bool (*accept)(const char *),
                           u8 *data,
                           int &size)
{
  char buffer[2048];

  ArchiveList_struct *rarList = NULL;
  if(urarlib_list((void *)file, (ArchiveList_struct *)&rarList)) {
    ArchiveList_struct *p = rarList;
    
    bool found = false;
    while(p) {
      if(accept(p->item.Name)) {
        strcpy(buffer, p->item.Name);
        found = true;
        break;
      }
      p = p->next;
    }
    if(found) {
      void *memory = NULL;
      unsigned long lsize = 0;
      size = p->item.UnpSize;
      int r = urarlib_get((void *)&memory, &lsize, buffer, (void *)file, "");
      if(!r) {
        systemMessage(MSG_ERROR_READING_IMAGE,
                      "Error reading image %s", buffer);
        urarlib_freelist(rarList);
        return NULL;
      }
      u8 *image = (u8 *)memory;
      if(data != NULL) {
        memcpy(image, data, size);
      }
      urarlib_freelist(rarList);
      return image;
    }
    systemMessage(MSG_NO_IMAGE_ON_ZIP,
                  "No image found on RAR file %s", file);
    urarlib_freelist(rarList);
    return NULL;
  }
  // nothing found
  return NULL;
}
#endif

u8 *utilLoad(const char *file,
             bool (*accept)(const char *),
             u8 *data,
             int &size)
{
  if(utilIsZipFile(file)) {
    return utilLoadFromZip(file, accept, data, size);
  }
  if(utilIsGzipFile(file)) {
    return utilLoadGzipFile(file, accept, data, size);
  }
#if 0
  if(utilIsRarFile(file)) {
    return utilLoadRarFile(file, accept, data, size);
  }
#endif
  
  u8 *image = data;

  FILE *f = fopen(file, "rb");

  if(!f) {
    systemMessage(MSG_ERROR_OPENING_IMAGE, "Error opening image %s", file);
    return NULL;
  }

  fseek(f,0,SEEK_END);
  size = ftell(f);
  fseek(f,0,SEEK_SET);

  if(image == NULL) {
    image = (u8 *)malloc(size);
    if(image == NULL) {
      systemMessage(MSG_OUT_OF_MEMORY, "Failed to allocate memory for %s",
                    "data");
      fclose(f);
      return NULL;
    }
  }

  int r = fread(image, 1, size, f);
  fclose(f);

  if(r != (int)size) {
    systemMessage(MSG_ERROR_READING_IMAGE,
                  "Error reading image %s", file);
    if(data == NULL)
      free(image);
    return NULL;
  }  

  return image;
}
