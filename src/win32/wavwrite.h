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
//-----------------------------------------------------------------------------
// File: WavWrite.h
//
// Desc: Support for loading and playing Wave files using DirectSound sound
//       buffers.
//
// Copyright (c) 1999 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef WAVE_WRITE_H
#define WAVE_WRITE_H


#include <mmreg.h>
#include <mmsystem.h>


//-----------------------------------------------------------------------------
// Name: class CWaveSoundWrite
// Desc: A class to write in sound data to a Wave file
//-----------------------------------------------------------------------------
class CWaveSoundWrite
{
public:
    HMMIO    m_hmmioOut;        // MM I/O handle for the WAVE
    MMCKINFO m_ckOut;           // Multimedia RIFF chunk
    MMCKINFO m_ckOutRIFF;       // Use in opening a WAVE file
    MMIOINFO m_mmioinfoOut;

public:
    CWaveSoundWrite();
    ~CWaveSoundWrite();

    HRESULT Open( CHAR* strFilename, WAVEFORMATEX *pwfxDest );
    HRESULT Reset();
    HRESULT Write( UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote );
    HRESULT Close( DWORD dwSamples );

};


#endif WAVE_WRITE_H



