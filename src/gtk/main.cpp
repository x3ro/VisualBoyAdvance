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

#include <libglademm.h>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>

#include "window.h"
#include "intl.h"

using Gnome::Glade::Xml;

int main(int argc, char * argv[])
{
#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
  textdomain(GETTEXT_PACKAGE);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif // ENABLE_NLS

  Gtk::Main oKit(argc, argv);

  Glib::RefPtr<Xml> poXml;
  try
  {
    poXml = Xml::create(PKGDATADIR "/vba.glade", "MainWindow");
  }
  catch (const Xml::Error & e)
  {
    Gtk::MessageDialog oDialog(e.what(),
                               Gtk::MESSAGE_ERROR,
                               Gtk::BUTTONS_CLOSE);
    oDialog.run();
    return 1;
  }

  VBA::Window * poWindow = NULL;
  poXml->get_widget_derived<VBA::Window>("MainWindow", poWindow);

  Gtk::Main::run(*poWindow);
  delete poWindow;

  return 0;
}
