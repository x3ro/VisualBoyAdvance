/**************************************************************************
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 *  Copyright (C) 1992 - 1996 Microsoft Corporation.  All Rights Reserved.
 *
 **************************************************************************/
/****************************************************************************
 *
 *  WRITEAVI.C
 *
 *  Creates the file OUTPUT.AVI, an AVI file consisting of a rotating clock
 *  face.  This program demonstrates using the functions in AVIFILE.DLL
 *  to make writing AVI files simple.
 *
 *  This is a stripped-down example; a real application would have a user
 *  interface and check for errors.
 *
 ***************************************************************************/
#include "stdafx.h"
#include <windowsx.h>
#include <memory.h>
#include <mmsystem.h>
#include <vfw.h>

#include "WriteAVI.h"

extern HWND hWindow;

CAVIFile::CAVIFile()
  : bOK(true), nFrames(0), sFrames(0)
{
  soundAdded = false;
  pfile = NULL;
  ps = NULL;
  psCompressed = NULL;
  psound = NULL;
  aopts[0] = &opts;
  WORD wVer = HIWORD(VideoForWindowsVersion());
  if (wVer < 0x010A) {
    // oops, we are too old, blow out of here
    bOK = false;
  } else {
    AVIFileInit();
  }
}

CAVIFile::~CAVIFile()
{
  if (ps)
    AVIStreamClose(ps);
  
  if (psCompressed)
    AVIStreamClose(psCompressed);

  if(psound)
    AVIStreamClose(psound);

  if (pfile)
    AVIFileClose(pfile);

  WORD wVer = HIWORD(VideoForWindowsVersion());
  if (wVer >= 0x010A) {
    AVIFileExit();
  }
}

void CAVIFile::SetFormat(BITMAPINFOHEADER *bh)
{
  memcpy(&bitmap, bh, 0x28);
}

void CAVIFile::SetSoundFormat(WAVEFORMATEX *f)
{
  memcpy(&soundFormat, f, sizeof(soundFormat));
  memset(&soundhdr, 0, sizeof(soundhdr));
  
  soundhdr.fccType                = streamtypeAUDIO;// stream type
  soundhdr.dwQuality              = (DWORD)-1;
  soundhdr.dwScale                = f->nBlockAlign;
  soundhdr.dwInitialFrames        = 1;
  soundhdr.dwRate                 = f->nAvgBytesPerSec;
  soundhdr.dwSampleSize           = f->nBlockAlign;

  HRESULT hr = AVIFileCreateStream(pfile, &psound, &soundhdr);

  if(hr != AVIERR_OK) {
    bOK = false;
    return;
  }

  hr = AVIStreamSetFormat(psound,
                          0,
                          (void *)&soundFormat,
                          sizeof(soundFormat));

  if(hr != AVIERR_OK) {
    bOK = false;
    return;
  }

  soundAdded = true;
}

bool CAVIFile::Open(const char *filename)
{
  HRESULT hr = AVIFileOpen(&pfile,		    // returned file pointer
                           filename,							// file name
                           OF_WRITE | OF_CREATE,		    // mode to open file with
                           NULL);							// use handler determined
  // from file extension....
  if (hr != AVIERR_OK) {
    bOK = false;
    return false;
  }
  memset(&strhdr, 0, sizeof(strhdr));
  strhdr.fccType                = streamtypeVIDEO;// stream type
  strhdr.fccHandler             = 0;
  strhdr.dwScale                = 1;
  strhdr.dwRate                 = rate;
  strhdr.dwSuggestedBufferSize  = bitmap.biSizeImage;
  SetRect(&strhdr.rcFrame,
          0,
          0,		    // rectangle for stream
          (int) bitmap.biWidth,
          (int) bitmap.biHeight);
      
  // And create the stream;
  hr = AVIFileCreateStream(pfile,		    // file pointer
                           &ps,		    // returned stream pointer
                           &strhdr);	    // stream header
  if (hr != AVIERR_OK) {
    bOK = false;
    return false;
  }
      
  memset(&opts, 0, sizeof(opts));
  
  if(!AVISaveOptions(hWindow, 0, 1, &ps, aopts)) {
    bOK = false;
    return false;
  }
      
  hr = AVIMakeCompressedStream(&psCompressed, ps, &opts, NULL);
  if (hr != AVIERR_OK) {
    bOK = false;
    return false;
  }
      
  hr = AVIStreamSetFormat(psCompressed, 0,
                          &bitmap,	    // stream format
                          bitmap.biSize +   // format size
                          bitmap.biClrUsed * sizeof(RGBQUAD));
  if (hr != AVIERR_OK) {
    bOK = false;
    return false;
  }

  return true;
}

bool CAVIFile::AddSound(const int frame, const char *sound, int len)
{
  if(!bOK)
    return false;

  int samples = len / soundFormat.nBlockAlign;
  ULONG nTotalBytesWritten = 0;
  HRESULT hr = S_OK;
  while((hr == S_OK) && (nTotalBytesWritten < len)) {
    LONG nBytesWritten = 0;
    LONG nSamplesWritten = 0;
    
    hr = AVIStreamWrite(psound,
                        sFrames,
                        samples,
                        (LPVOID)sound,
                        len,
                        0,
                        &nSamplesWritten,
                        &nBytesWritten);
    nTotalBytesWritten += nBytesWritten;
    sFrames += nSamplesWritten;
  }
  if(hr != AVIERR_OK) {
    bOK = false;
    return false;
  }

  return true;
}

bool CAVIFile::AddFrame(const int frame, const char *bmp)
{
  HRESULT hr;
  
  if (!bOK)
    return false;
  
  hr = AVIStreamWrite(psCompressed,	// stream pointer
                      frame,				// time of this frame
                      1,				// number to write
                      (LPVOID)bmp,
                      bitmap.biSizeImage,	// size of this frame
                      AVIIF_KEYFRAME,
                      NULL,
                      NULL);
  if (hr != AVIERR_OK) {
    bOK = false;
    return false;
  }

  nFrames++;
  return true;
}

