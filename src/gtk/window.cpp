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

#include "window.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <SDL.h>

#include "../GBA.h"
#include "../gb/GB.h"
#include "../gb/gbGlobals.h"
#include "../Util.h"

#include "tools.h"
#include "intl.h"

extern bool debugger;
extern int RGB_LOW_BITS_MASK;

#ifdef MMX
extern "C" bool cpu_mmx;
#endif // MMX

namespace VBA
{

using Gnome::Glade::Xml;

Window * Window::m_poInstance = NULL;

// GB/GBA screen sizes
const int iGBScreenWidth   = 160;
const int iGBScreenHeight  = 144;
const int iSGBScreenWidth  = 256;
const int iSGBScreenHeight = 224;
const int iGBAScreenWidth  = 240;
const int iGBAScreenHeight = 160;

// Config limits
const int iFrameskipMin = 0;
const int iFrameskipMax = 9;
const int iThrottleMin  = 5;
const int iThrottleMax  = 1000;
const int iScaleMin     = 1;
const int iScaleMax     = 6;
const int iFilter2xMin  = (int)FirstFilter;
const int iFilter2xMax  = (int)LastFilter;
const int iFilterIBMin  = (int)FirstFilterIB;
const int iFilterIBMax  = (int)LastFilterIB;

Window::Window(GtkWindow * _pstWindow, const Glib::RefPtr<Xml> & _poXml) :
  Gtk::Window(_pstWindow),
  m_poXml(_poXml),
  m_poFileOpenDialog(NULL),
  m_iScreenWidth(iGBAScreenWidth),
  m_iScreenHeight(iGBAScreenHeight),
  m_eCartridge(NO_CARTRIDGE),
  m_uiJoypadState(0)
{
  vInitSystem();
  vInitSDL();

  Gtk::Container * poC;
  poC = dynamic_cast<Gtk::Container *>(_poXml->get_widget("ScreenContainer"));
  m_poScreenArea = Gtk::manage(new ScreenArea(m_iScreenWidth, m_iScreenHeight));
  poC->add(*m_poScreenArea);
  vDrawDefaultScreen();
  m_poScreenArea->show();

  // Get config
  //
  vInitConfig();

  m_sUserDataDir = Glib::get_home_dir() + "/.gvba";
  m_sConfigFile  = m_sUserDataDir + "/config";

  if (! Glib::file_test(m_sUserDataDir, Glib::FILE_TEST_EXISTS))
  {
    mkdir(m_sUserDataDir.c_str(), 0777);
  }
  if (Glib::file_test(m_sConfigFile, Glib::FILE_TEST_EXISTS))
  {
    vLoadConfig(m_sConfigFile);
    vCheckConfig();
  }
  else
  {
    vSaveConfig(m_sConfigFile);
  }

  vLoadKeymap();

  Gtk::MenuItem *      poMI;
  Gtk::CheckMenuItem * poCMI;

  // File menu
  //
  m_poFilePauseItem = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget("FilePause"));
  m_poFilePauseItem->set_active(false);
  m_poFilePauseItem->signal_toggled().connect(SigC::bind<Gtk::CheckMenuItem *>(
                                                SigC::slot(*this, &Window::vOnFilePauseToggled),
                                                m_poFilePauseItem));

  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("FileOpen"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnFileOpen));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("FileReset"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnFileReset));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("FileClose"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnFileClose));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("FileExit"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnFileExit));

  // Frameskip menu
  //
  struct
  {
    const char * m_csName;
    const int    m_iFrameskip;
  }
  astFrameskip[] =
  {
    { "FrameskipAutomatic", -1 },
    { "Frameskip0",          0 },
    { "Frameskip1",          1 },
    { "Frameskip2",          2 },
    { "Frameskip3",          3 },
    { "Frameskip4",          4 },
    { "Frameskip5",          5 },
    { "Frameskip6",          6 },
    { "Frameskip7",          7 },
    { "Frameskip8",          8 },
    { "Frameskip9",          9 }
  };
  int iDefaultFrameskip;
  if (m_poCoreConfig->sGetKey("frameskip") == "auto")
  {
    iDefaultFrameskip = -1;
  }
  else
  {
    iDefaultFrameskip = m_poCoreConfig->oGetKey<int>("frameskip");
  }
  for (guint i = 0; i < sizeof(astFrameskip) / sizeof(astFrameskip[0]); i++)
  {
    poCMI = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget(astFrameskip[i].m_csName));
    if (astFrameskip[i].m_iFrameskip == iDefaultFrameskip)
    {
      poCMI->set_active();
      vOnFrameskipToggled(poCMI, iDefaultFrameskip);
    }
    poCMI->signal_toggled().connect(SigC::bind<Gtk::CheckMenuItem *, int>(
                                      SigC::slot(*this, &Window::vOnFrameskipToggled),
                                      poCMI, astFrameskip[i].m_iFrameskip));
  }

  // Throttle menu
  //
  struct
  {
    const char * m_csName;
    const int    m_iThrottle;
  }
  astThrottle[] =
  {
    { "ThrottleNoThrottle",   0 },
    { "Throttle25",          25 },
    { "Throttle50",          50 },
    { "Throttle100",        100 },
    { "Throttle150",        150 },
    { "Throttle200",        200 }
  };
  poCMI = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget("ThrottleOther"));
  poCMI->set_active();
  poCMI->signal_activate().connect(SigC::bind<Gtk::CheckMenuItem *>(
                                     SigC::slot(*this, &Window::vOnThrottleOther),
                                     poCMI));

  int iDefaultThrottle = m_poCoreConfig->oGetKey<int>("throttle");
  for (guint i = 0; i < sizeof(astThrottle) / sizeof(astThrottle[0]); i++)
  {
    poCMI = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget(astThrottle[i].m_csName));
    if (astThrottle[i].m_iThrottle == iDefaultThrottle)
    {
      poCMI->set_active();
    }
    poCMI->signal_toggled().connect(SigC::bind<Gtk::CheckMenuItem *, int>(
                                      SigC::slot(*this, &Window::vOnThrottleToggled),
                                      poCMI, astThrottle[i].m_iThrottle));
  }
  vSetThrottle(iDefaultThrottle);

  // Video menu
  //
  struct
  {
    const char * m_csName;
    const int    m_iScale;
  }
  astVideoScale[] =
  {
    { "Video1x", 1 },
    { "Video2x", 2 },
    { "Video3x", 3 },
    { "Video4x", 4 },
    { "Video5x", 5 },
    { "Video6x", 6 }
  };
  int iDefaultScale = m_poDisplayConfig->oGetKey<int>("scale");
  for (guint i = 0; i < sizeof(astVideoScale) / sizeof(astVideoScale[0]); i++)
  {
    poCMI = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget(astVideoScale[i].m_csName));
    if (astVideoScale[i].m_iScale == iDefaultScale)
    {
      poCMI->set_active();
      vOnVideoScaleToggled(poCMI, iDefaultScale);
    }
    poCMI->signal_toggled().connect(SigC::bind<Gtk::CheckMenuItem *, int>(
                                      SigC::slot(*this, &Window::vOnVideoScaleToggled),
                                      poCMI, astVideoScale[i].m_iScale));
  }

  // Layers menu
  //
  struct
  {
    const char * m_csName;
    const int    m_iLayer;
    const bool   m_bChecked;
  }
  astLayer[] =
  {
    { "LayersBg0",    0, m_poCoreConfig->oGetKey<bool>("layer_bg0")    },
    { "LayersBg1",    1, m_poCoreConfig->oGetKey<bool>("layer_bg1")    },
    { "LayersBg2",    2, m_poCoreConfig->oGetKey<bool>("layer_bg2")    },
    { "LayersBg3",    3, m_poCoreConfig->oGetKey<bool>("layer_bg3")    },
    { "LayersObj",    4, m_poCoreConfig->oGetKey<bool>("layer_obj")    },
    { "LayersWin0",   5, m_poCoreConfig->oGetKey<bool>("layer_win0")   },
    { "LayersWin1",   6, m_poCoreConfig->oGetKey<bool>("layer_win1")   },
    { "LayersObjWin", 7, m_poCoreConfig->oGetKey<bool>("layer_objwin") }
  };
  for (guint i = 0; i < sizeof(astLayer) / sizeof(astLayer[0]); i++)
  {
    poCMI = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget(astLayer[i].m_csName));
    poCMI->set_active(astLayer[i].m_bChecked);
    vOnLayerToggled(poCMI, astLayer[i].m_iLayer);
    poCMI->signal_toggled().connect(SigC::bind<Gtk::CheckMenuItem *, int>(
                                      SigC::slot(*this, &Window::vOnLayerToggled),
                                      poCMI, astLayer[i].m_iLayer));
  }

  // Filter menu
  //
  struct
  {
    const char *    m_csName;
    const EFilter2x m_eFilter2x;
  }
  astFilter2x[] =
  {
    { "FilterNone",          FilterNone         },
    { "FilterTVMode",        FilterScanlinesTV  },
    { "Filter2xSaI",         Filter2xSaI        },
    { "FilterSuper2xSaI",    FilterSuper2xSaI   },
    { "FilterSuperEagle",    FilterSuperEagle   },
    { "FilterPixelate",      FilterPixelate     },
    { "FilterMotionBlur",    FilterMotionBlur   },
    { "FilterAdvanceMame2x", FilterAdMame2x     },
    { "FilterSimple2x",      FilterSimple2x     },
    { "FilterBilinear",      FilterBilinear     },
    { "FilterBilinearPlus",  FilterBilinearPlus },
    { "FilterScanlines",     FilterScanlines    },
    { "FilterHq2x",          FilterHq2x         },
    { "FilterLq2x",          FilterLq2x         }
  };
  EFilter2x eDefaultFilter2x = (EFilter2x)m_poDisplayConfig->oGetKey<int>("filter2x");
  for (guint i = 0; i < sizeof(astFilter2x) / sizeof(astFilter2x[0]); i++)
  {
    poCMI = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget(astFilter2x[i].m_csName));
    if (astFilter2x[i].m_eFilter2x == eDefaultFilter2x)
    {
      poCMI->set_active();
      vOnFilter2xToggled(poCMI, eDefaultFilter2x);
    }
    poCMI->signal_toggled().connect(SigC::bind<Gtk::CheckMenuItem *, int>(
                                      SigC::slot(*this, &Window::vOnFilter2xToggled),
                                      poCMI, astFilter2x[i].m_eFilter2x));
  }

  poCMI = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget("FilterDisableMmx"));
#ifdef MMX
  poCMI->set_active(m_poDisplayConfig->oGetKey<bool>("filter_disable_mmx"));
  vOnDisableMMXToggled(poCMI);
  poCMI->signal_toggled().connect(SigC::bind<Gtk::CheckMenuItem *>(
                                    SigC::slot(*this, &Window::vOnDisableMMXToggled),
                                    poCMI));
#else // ! MMX
  poCMI->set_active();
  poCMI->set_sensitive(false);
#endif // ! MMX

  // Interframe blending menu
  //
  struct
  {
    const char *    m_csName;
    const EFilterIB m_eFilterIB;
  }
  astFilterIB[] =
  {
    { "IFBNone",       FilterIBNone       },
    { "IFBSmart",      FilterIBSmart      },
    { "IFBMotionBlur", FilterIBMotionBlur }
  };
  EFilterIB eDefaultFilterIB = (EFilterIB)m_poDisplayConfig->oGetKey<int>("filterIB");
  for (guint i = 0; i < sizeof(astFilterIB) / sizeof(astFilterIB[0]); i++)
  {
    poCMI = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget(astFilterIB[i].m_csName));
    if (astFilterIB[i].m_eFilterIB == eDefaultFilterIB)
    {
      poCMI->set_active();
      vOnFilterIBToggled(poCMI, eDefaultFilterIB);
    }
    poCMI->signal_toggled().connect(SigC::bind<Gtk::CheckMenuItem *, int>(
                                      SigC::slot(*this, &Window::vOnFilterIBToggled),
                                      poCMI, astFilterIB[i].m_eFilterIB));
  }

  // Help menu
  //
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("HelpAbout"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnHelpAbout));

  if (m_poInstance == NULL)
  {
    m_poInstance = this;
  }
  else
  {
    abort();
  }
}

Window::~Window()
{
  vOnFileClose();
  vSaveConfig(m_sConfigFile);

  if (m_poFileOpenDialog != NULL)
  {
    delete m_poFileOpenDialog;
  }

  m_poInstance = NULL;
}

void Window::vInitSystem()
{
  Init_2xSaI(32);

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  systemRedShift    = 3;
  systemGreenShift  = 11;
  systemBlueShift   = 19;
  RGB_LOW_BITS_MASK = 0x00010101;
#else
  systemRedShift    = 27;
  systemGreenShift  = 19;
  systemBlueShift   = 11;
  RGB_LOW_BITS_MASK = 0x01010100;
#endif

  systemColorDepth = 32;
  systemDebug = 0;
  systemVerbose = 0;
  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
  systemFrameSkip = 0;
  systemSoundOn = false;

  emulating = 0;
  debugger = true;

  for (int i = 0; i < 0x10000; i++)
  {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    systemColorMap32[i] = (((i & 0x1f) << systemRedShift)
                           | (((i & 0x3e0) >> 5) << systemGreenShift)
                           | (((i & 0x7c00) >> 10) << systemBlueShift)
                           | 0xff000000);
#else
    systemColorMap32[i] = (((i & 0x1f) << systemRedShift)
                           | (((i & 0x3e0) >> 5) << systemGreenShift)
                           | (((i & 0x7c00) >> 10) << systemBlueShift)
                           | 0xff);
#endif
  }

  gbFrameSkip = 0;
  // TODO : GB init and 16-bit color map (?)
}

void Window::vInitSDL()
{
  static bool bDone = false;

  if (bDone)
    return;

  int iFlags = (SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE);

  if (SDL_Init(iFlags) < 0)
  {
    fprintf(stderr, "Failed to init SDL: %s", SDL_GetError());
    abort();
  }

  bDone = true;
}

void Window::vInitConfig()
{
  m_oConfig.vClear();

  // Core section
  //
  m_poCoreConfig = m_oConfig.poAddSection("Core");
  m_poCoreConfig->vSetKey("frameskip",    "auto" );
  m_poCoreConfig->vSetKey("throttle",     0      );
  m_poCoreConfig->vSetKey("layer_bg0",    true   );
  m_poCoreConfig->vSetKey("layer_bg1",    true   );
  m_poCoreConfig->vSetKey("layer_bg2",    true   );
  m_poCoreConfig->vSetKey("layer_bg3",    true   );
  m_poCoreConfig->vSetKey("layer_obj",    true   );
  m_poCoreConfig->vSetKey("layer_win0",   true   );
  m_poCoreConfig->vSetKey("layer_win1",   true   );
  m_poCoreConfig->vSetKey("layer_objwin", true   );

  // Display section
  //
  m_poDisplayConfig = m_oConfig.poAddSection("Display");
  m_poDisplayConfig->vSetKey     ("scale",               1            );
  m_poDisplayConfig->vSetKey<int>("filter2x",            FilterNone   );
  m_poDisplayConfig->vSetKey<int>("filterIB",            FilterIBNone );
#ifdef MMX
  m_poDisplayConfig->vSetKey     ("filter_disable_mmx",  false        );
#endif // MMX
}

void Window::vCheckConfig()
{
  int iValue;
  int iAdjusted;

  if (m_poCoreConfig->sGetKey("frameskip") != "auto")
  {
    iValue = m_poCoreConfig->oGetKey<int>("frameskip");
    iAdjusted = CLAMP(iValue, iFrameskipMin, iFrameskipMax);
    if (iValue != iAdjusted)
    {
      m_poCoreConfig->vSetKey("frameskip", iAdjusted);
    }
  }

  iValue = m_poCoreConfig->oGetKey<int>("throttle");
  if (iValue != 0)
  {
    iAdjusted = CLAMP(iValue, iThrottleMin, iThrottleMax);
    if (iValue != iAdjusted)
    {
      m_poCoreConfig->vSetKey("throttle", iAdjusted);
    }
  }

  iValue = m_poDisplayConfig->oGetKey<int>("scale");
  iAdjusted = CLAMP(iValue, iScaleMin, iScaleMax);
  if (iValue != iAdjusted)
  {
    m_poDisplayConfig->vSetKey("scale", iAdjusted);
  }

  iValue = m_poDisplayConfig->oGetKey<int>("filter2x");
  iAdjusted = CLAMP(iValue, iFilter2xMin, iFilter2xMax);
  if (iValue != iAdjusted)
  {
    m_poDisplayConfig->vSetKey("filter2x", iAdjusted);
  }

  iValue = m_poDisplayConfig->oGetKey<int>("filterIB");
  iAdjusted = CLAMP(iValue, iFilterIBMin, iFilterIBMax);
  if (iValue != iAdjusted)
  {
    m_poDisplayConfig->vSetKey("filterIB", iAdjusted);
  }
}

void Window::vLoadConfig(const std::string & _sFilename)
{
  try
  {
    m_oConfig.vLoad(_sFilename, false, false);
  }
  catch (const Glib::Error & e)
  {
    Gtk::MessageDialog oDialog(*this,
                               e.what(),
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_CLOSE);
    oDialog.run();
  }
}

void Window::vSaveConfig(const std::string & _sFilename)
{
  try
  {
    m_oConfig.vSave(_sFilename);
  }
  catch (const Glib::Error & e)
  {
    Gtk::MessageDialog oDialog(*this,
                               e.what(),
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_CLOSE);
    oDialog.run();
  }
}

void Window::vLoadKeymap()
{
  // TODO : load from prefs

  m_oKeymap.vRegister(GDK_z,            KEY_A);
  m_oKeymap.vRegister(GDK_Z,            KEY_A);
  m_oKeymap.vRegister(GDK_x,            KEY_B);
  m_oKeymap.vRegister(GDK_X,            KEY_B);
  m_oKeymap.vRegister(GDK_BackSpace,    KEY_SELECT);
  m_oKeymap.vRegister(GDK_Return,       KEY_START);
  m_oKeymap.vRegister(GDK_Right,        KEY_RIGHT);
  m_oKeymap.vRegister(GDK_Left,         KEY_LEFT);
  m_oKeymap.vRegister(GDK_Up,           KEY_UP);
  m_oKeymap.vRegister(GDK_Down,         KEY_DOWN);
  m_oKeymap.vRegister(GDK_s,            KEY_R);
  m_oKeymap.vRegister(GDK_S,            KEY_R);
  m_oKeymap.vRegister(GDK_a,            KEY_L);
  m_oKeymap.vRegister(GDK_A,            KEY_L);
  m_oKeymap.vRegister(GDK_space,        KEY_SPEED);
  m_oKeymap.vRegister(GDK_F12,          KEY_CAPTURE);
}

void Window::vUpdateScreen()
{
  g_return_if_fail(m_iScreenWidth >= 1 && m_iScreenHeight >= 1);

  m_poScreenArea->vSetSize(m_iScreenWidth, m_iScreenHeight);
  m_poScreenArea->vSetScale(m_poDisplayConfig->oGetKey<int>("scale"));

  resize(1, 1);

  if (emulating)
  {
    vDrawScreen();
  }
  else
  {
    vDrawDefaultScreen();
  }
}

void Window::vDrawScreen()
{
  m_poScreenArea->vDrawPixels(pix);
}

void Window::vDrawDefaultScreen()
{
  m_poScreenArea->vDrawColor(0x000000); // Black
}

void Window::vComputeFrameskip(int _iRate)
{
  static u32 uiLastTime = 0;
  static int iFrameskipAdjust = 0;

  u32 uiTime = SDL_GetTicks();

  if (m_bWasEmulating)
  {
    int iWantedSpeed = 100;

    if (m_iThrottle > 0)
    {
      if (! speedup)
      {
        u32 uiDiff  = uiTime - m_uiThrottleLastTime;
        int iTarget = 1000000 / (_iRate * m_iThrottle);
        int iDelay  = iTarget - uiDiff;
        if (iDelay > 0)
        {
          m_uiThrottleDelay = iDelay;
        }
      }
      iWantedSpeed = m_iThrottle;
    }

    if (m_bAutoFrameskip)
    {
      u32 uiDiff = uiTime - uiLastTime;
      int iSpeed = iWantedSpeed;

      if (uiDiff != 0)
      {
        iSpeed = (1000000 / _iRate) / uiDiff;
      }

      if (iSpeed >= iWantedSpeed - 2)
      {
        iFrameskipAdjust++;
        if (iFrameskipAdjust >= 3)
        {
          iFrameskipAdjust = 0;
          if (systemFrameSkip > 0)
          {
            systemFrameSkip--;
          }
        }
      }
      else
      {
        if (iSpeed < iWantedSpeed - 20)
        {
          iFrameskipAdjust -= ((iWantedSpeed - 10) - iSpeed) / 5;
        }
        else if (systemFrameSkip < 9)
        {
          iFrameskipAdjust--;
        }

        if (iFrameskipAdjust <= -2)
        {
          iFrameskipAdjust += 2;
          if (systemFrameSkip < 9)
          {
            systemFrameSkip++;
          }
        }
      }
    }
  }
  else
  {
    m_bWasEmulating = true;
  }

  uiLastTime = uiTime;
  m_uiThrottleLastTime = uiTime;
}

bool Window::bLoadROM(const std::string & _rsFilename)
{
  vOnFileClose();

  m_sRomFile = _rsFilename;
  const char * csFilename = _rsFilename.c_str();

  IMAGE_TYPE eType = utilFindType(csFilename);
  if (eType == IMAGE_UNKNOWN)
  {
    systemMessage(0, _("Unknown file type %s"), csFilename);
    return false;
  }

  bool bLoaded = false;
  if (eType == IMAGE_GB)
  {
    bLoaded = gbLoadRom(csFilename);
    if (bLoaded)
    {
      m_eCartridge = GB_CARTRIDGE;
      m_stEmulator = GBSystem;

      //if(sdlAutoIPS) {
      //  int size = gbRomSize;
      //  utilApplyIPS(ipsname, &gbRom, &size);
      //  if(size != gbRomSize) {
      //    extern bool gbUpdateSizes();
      //    gbUpdateSizes();
      //    gbReset();
      //  }
      //}

      if (gbBorderOn)
      {
        m_iScreenWidth     = iSGBScreenWidth;
        m_iScreenHeight    = iSGBScreenHeight;
        gbBorderLineSkip   = iSGBScreenWidth;
        gbBorderColumnSkip = (iSGBScreenWidth - iGBScreenWidth) / 2;
        gbBorderRowSkip    = (iSGBScreenHeight - iGBScreenHeight) / 2;
      }
      else
      {
        m_iScreenWidth     = iGBScreenWidth;
        m_iScreenHeight    = iGBScreenHeight;
        gbBorderLineSkip   = iGBScreenWidth;
        gbBorderColumnSkip = 0;
        gbBorderRowSkip    = 0;
      }
    }
  }
  else if (eType == IMAGE_GBA)
  {
    int iSize = CPULoadRom(csFilename);
    bLoaded = (iSize > 0);
    if (bLoaded)
    {
      //sdlApplyPerImagePreferences();

      m_eCartridge = GBA_CARTRIDGE;
      m_stEmulator = GBASystem;

      // TODO
      //CPUInit(biosFileName, useBios);
      useBios = false;
      CPUInit(NULL, useBios);
      CPUReset();

      //if(sdlAutoIPS) {
      //  int size = 0x2000000;
      //  utilApplyIPS(ipsname, &rom, &size);
      //  if(size != 0x2000000) {
      //    CPUReset();
      //  }
      //}

      m_iScreenWidth  = iGBAScreenWidth;
      m_iScreenHeight = iGBAScreenHeight;
    }
  }

  if (! bLoaded)
  {
    systemMessage(0, _("Failed to load file %s"), csFilename);
    return false;
  }

  vLoadBattery();
  vUpdateScreen();

  emulating = 1;
  m_bWasEmulating = false;
  m_uiThrottleDelay = 0;

  if (m_poFilePauseItem->get_active())
  {
    m_poFilePauseItem->set_active(false);
  }
  else
  {
    vStartEmu();
  }

  return true;
}

void Window::vLoadBattery()
{
  // TODO : from battery dir

  std::string sBattery = sCutSuffix(m_sRomFile) + ".sav";
  if (m_stEmulator.emuReadBattery(sBattery.c_str()))
  {
    systemScreenMessage(_("Loaded battery"));
  }
}

void Window::vSaveBattery()
{
  // TODO : from battery dir

  std::string sBattery = sCutSuffix(m_sRomFile) + ".sav";
  if (m_stEmulator.emuWriteBattery(sBattery.c_str()))
  {
    systemScreenMessage(_("Saved battery"));
  }
}

void Window::vLoadState(int _iNum)
{
  // TODO
}

void Window::vSaveState(int _iNum)
{
  // TODO
}

void Window::vStartEmu()
{
  if (m_oEmuSig.connected())
  {
    return;
  }

  m_oEmuSig = Glib::signal_idle().connect(SigC::slot(*this, &Window::bOnEmuIdle),
                                          Glib::PRIORITY_DEFAULT_IDLE);
}

void Window::vStopEmu()
{
  m_oEmuSig.disconnect();
  m_bWasEmulating = false;
}

void Window::vSetThrottle(int _iPercent)
{
  m_iThrottle = _iPercent;
  m_poCoreConfig->vSetKey("throttle", _iPercent);
}

void Window::vSelectBestThrottleItem()
{
  struct
  {
    const char * m_csName;
    const int    m_iThrottle;
  }
  astThrottle[] =
  {
    { "ThrottleNoThrottle",   0 },
    { "Throttle25",          25 },
    { "Throttle50",          50 },
    { "Throttle100",        100 },
    { "Throttle150",        150 },
    { "Throttle200",        200 }
  };
  for (guint i = 0; i < sizeof(astThrottle) / sizeof(astThrottle[0]); i++)
  {
    Gtk::CheckMenuItem * poCMI;
    poCMI = dynamic_cast<Gtk::CheckMenuItem *>(m_poXml->get_widget(astThrottle[i].m_csName));
    if (astThrottle[i].m_iThrottle == m_iThrottle)
    {
      poCMI->set_active();
    }
  }
}

void Window::vOnFileOpen()
{
  if (m_poFileOpenDialog == NULL)
  {
    m_poFileOpenDialog = new Gtk::FileSelection(_("Open a ROM"));
    m_poFileOpenDialog->set_transient_for(*this);
  }

  while (m_poFileOpenDialog->run() == Gtk::RESPONSE_OK)
  {
    if (bLoadROM(m_poFileOpenDialog->get_filename()))
    {
      break;
    }
  }
  m_poFileOpenDialog->hide();
}

void Window::vOnFilePauseToggled(Gtk::CheckMenuItem * _poCMI)
{
  if (emulating)
  {
    if (_poCMI->get_active())
    {
      vStopEmu();
    }
    else
    {
      vStartEmu();
    }
  }
}

void Window::vOnFileReset()
{
  if (emulating)
  {
    m_stEmulator.emuReset();
  }
}

void Window::vOnFileClose()
{
  if (emulating)
  {
    vStopEmu();
    vDrawDefaultScreen();
    vSaveBattery();
    m_stEmulator.emuCleanUp();
    m_eCartridge = NO_CARTRIDGE;
    emulating = 0;
  }
}

void Window::vOnFileExit()
{
  hide();
}

void Window::vOnFrameskipToggled(Gtk::CheckMenuItem * _poCMI, int _iValue)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  if (_iValue >= 0 && _iValue <= 9)
  {
    m_poCoreConfig->vSetKey("frameskip", _iValue);
    gbFrameSkip      = _iValue;
    systemFrameSkip  = _iValue;
    m_bAutoFrameskip = false;
  }
  else
  {
    m_poCoreConfig->vSetKey("frameskip", "auto");
    systemFrameSkip  = 0;
    m_bAutoFrameskip = true;
  }
}

void Window::vOnThrottleToggled(Gtk::CheckMenuItem * _poCMI, int _iPercent)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  vSetThrottle(_iPercent);

  // Initialize the frameskip adjustment each time throttle is changed
  if (m_bAutoFrameskip)
  {
    systemFrameSkip = 0;
  }
}

void Window::vOnThrottleOther(Gtk::CheckMenuItem * _poCMI)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  Glib::RefPtr<Xml> poXml;
  poXml = Xml::create(PKGDATADIR "/vba.glade", "ThrottleDialog");

  Gtk::Dialog * poDialog = dynamic_cast<Gtk::Dialog *>(poXml->get_widget("ThrottleDialog"));
  Gtk::SpinButton * poSpin = dynamic_cast<Gtk::SpinButton *>(poXml->get_widget("ThrottleSpin"));

  poDialog->set_transient_for(*this);

  if (m_iThrottle != 0)
  {
    poSpin->set_value(m_iThrottle);
  }
  else
  {
    poSpin->set_value(100);
  }

  if (poDialog->run() == Gtk::RESPONSE_OK)
  {
    vSetThrottle(poSpin->get_value_as_int());
  }

  delete poDialog;
  vSelectBestThrottleItem();
}

void Window::vOnVideoScaleToggled(Gtk::CheckMenuItem * _poCMI, int _iScale)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  m_poDisplayConfig->vSetKey("scale", _iScale);
  vUpdateScreen();
}

void Window::vOnLayerToggled(Gtk::CheckMenuItem * _poCMI, int _iLayer)
{
  int iMask = (0x0100 << _iLayer);
  if (_poCMI->get_active())
  {
    layerSettings |= iMask;
  }
  else
  {
    layerSettings &= ~iMask;
  }
  layerEnable = DISPCNT & layerSettings;

  const char * acsLayers[] =
  {
    "layer_bg0",
    "layer_bg1",
    "layer_bg2",
    "layer_bg3",
    "layer_obj",
    "layer_win0",
    "layer_win1",
    "layer_objwin"
  };
  m_poCoreConfig->vSetKey(acsLayers[_iLayer], _poCMI->get_active());
}

void Window::vOnFilter2xToggled(Gtk::CheckMenuItem * _poCMI, int _iFilter2x)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  m_poScreenArea->vSetFilter2x((EFilter2x)_iFilter2x);
  if (emulating)
  {
    vDrawScreen();
  }
  m_poDisplayConfig->vSetKey("filter2x", _iFilter2x);
}

void Window::vOnFilterIBToggled(Gtk::CheckMenuItem * _poCMI, int _iFilterIB)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  m_poScreenArea->vSetFilterIB((EFilterIB)_iFilterIB);
  if (emulating)
  {
    vDrawScreen();
  }
  m_poDisplayConfig->vSetKey("filterIB", _iFilterIB);
}

#ifdef MMX
void Window::vOnDisableMMXToggled(Gtk::CheckMenuItem * _poCMI)
{
  cpu_mmx = ! _poCMI->get_active();
  m_poDisplayConfig->vSetKey("filter_disable_mmx", _poCMI->get_active());
}
#endif // MMX

void Window::vOnHelpAbout()
{
  Glib::RefPtr<Xml> poXml;
  poXml = Xml::create(PKGDATADIR "/vba.glade", "AboutDialog");

  Gtk::Dialog * poDialog = dynamic_cast<Gtk::Dialog *>(poXml->get_widget("AboutDialog"));
  Gtk::Label *  poLabel  = dynamic_cast<Gtk::Label *>(poXml->get_widget("VersionLabel"));

  poDialog->set_transient_for(*this);
  poLabel->set_markup("<b><big>" PACKAGE " " VERSION "</big></b>");
  poDialog->run();
  delete poDialog;
}

bool Window::bOnEmuIdle()
{
  if (m_uiThrottleDelay != 0)
  {
    u32 uiTime = SDL_GetTicks();
    if (uiTime - m_uiThrottleLastTime >= m_uiThrottleDelay)
    {
      m_uiThrottleDelay = 0;
      m_uiThrottleLastTime = uiTime;
    }
    else
    {
      return true;
    }
  }

  m_stEmulator.emuMain(m_stEmulator.emuCount);
  return true;
}

bool Window::on_key_press_event(GdkEventKey * _pstEvent)
{
  EKey eKey;

  if ((_pstEvent->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK))
      || (eKey = m_oKeymap.eGetKey(_pstEvent->keyval)) == KEY_NONE)
  {
    return Gtk::Window::on_key_press_event(_pstEvent);
  }

  switch (eKey)
  {
  case KEY_A:
    m_uiJoypadState |= KEYFLAG_A;
    break;
  case KEY_B:
    m_uiJoypadState |= KEYFLAG_B;
    break;
  case KEY_SELECT:
    m_uiJoypadState |= KEYFLAG_SELECT;
    break;
  case KEY_START:
    m_uiJoypadState |= KEYFLAG_START;
    break;
  case KEY_RIGHT:
    m_uiJoypadState |= KEYFLAG_RIGHT;
    m_uiJoypadState &= ~KEYFLAG_LEFT;
    break;
  case KEY_LEFT:
    m_uiJoypadState |= KEYFLAG_LEFT;
    m_uiJoypadState &= ~KEYFLAG_RIGHT;
    break;
  case KEY_UP:
    m_uiJoypadState |= KEYFLAG_UP;
    m_uiJoypadState &= ~KEYFLAG_DOWN;
    break;
  case KEY_DOWN:
    m_uiJoypadState |= KEYFLAG_DOWN;
    m_uiJoypadState &= ~KEYFLAG_UP;
    break;
  case KEY_R:
    m_uiJoypadState |= KEYFLAG_R;
    break;
  case KEY_L:
    m_uiJoypadState |= KEYFLAG_L;
    break;
  case KEY_SPEED:
    m_uiJoypadState |= KEYFLAG_SPEED;
    break;
  case KEY_CAPTURE:
    m_uiJoypadState |= KEYFLAG_CAPTURE;
    break;
  case KEY_NONE:
    break;
  }
  return true;
}

bool Window::on_key_release_event(GdkEventKey * _pstEvent)
{
  EKey eKey;

  if ((_pstEvent->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK))
      || (eKey = m_oKeymap.eGetKey(_pstEvent->keyval)) == KEY_NONE)
  {
    return Gtk::Window::on_key_release_event(_pstEvent);
  }

  switch (eKey)
  {
  case KEY_A:
    m_uiJoypadState &= ~KEYFLAG_A;
    break;
  case KEY_B:
    m_uiJoypadState &= ~KEYFLAG_B;
    break;
  case KEY_SELECT:
    m_uiJoypadState &= ~KEYFLAG_SELECT;
    break;
  case KEY_START:
    m_uiJoypadState &= ~KEYFLAG_START;
    break;
  case KEY_RIGHT:
    m_uiJoypadState &= ~KEYFLAG_RIGHT;
    break;
  case KEY_LEFT:
    m_uiJoypadState &= ~KEYFLAG_LEFT;
    break;
  case KEY_UP:
    m_uiJoypadState &= ~KEYFLAG_UP;
    break;
  case KEY_DOWN:
    m_uiJoypadState &= ~KEYFLAG_DOWN;
    break;
  case KEY_R:
    m_uiJoypadState &= ~KEYFLAG_R;
    break;
  case KEY_L:
    m_uiJoypadState &= ~KEYFLAG_L;
    break;
  case KEY_SPEED:
    m_uiJoypadState &= ~KEYFLAG_SPEED;
    break;
  case KEY_CAPTURE:
    m_uiJoypadState &= ~KEYFLAG_CAPTURE;
    break;
  case KEY_NONE:
    break;
  }
  return true;
}

} // VBA namespace
