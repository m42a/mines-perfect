// Mines-Perfect: a minesweeper clone
// Copyright (C) 1995-2003  Christian Czepluch
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <string>
#include <vector>
#include <fstream> // visual studio (nur fuer debug-konfiguration)

#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif

//#ifndef LINUX
#ifdef __GNUWIN32__
  #include <io.h> // access()
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//using namespace std;

#include <wx/image.h>
#include <wx/file.h>
#include <wx/dir.h>
#include <wx/string.h>
#include <wx/mimetype.h>

#if (defined __WXMSW__) || wxUSE_WAVE
  #include <wx/wave.h>
#else
  class wxWave
  {
  public:
    wxWave() {}
    void Create(const char* fname) {}
    void Play() {}
    bool IsOk() { return false; }
  };
#endif

#include "../gui/gamectrl.h"
#include "../core/api.h"
#include "app.h"
#include "dialogs.h"

#include "lcd.xpm"
#include "symbols.xpm"
#include "smileys.xpm"
#include "button.xpm"

#include "square.xpm"
#include "triangle.xpm"
#include "hexagon.xpm"
#include "grid3d.xpm"
 
// globale 
extern MainWindow*  main_win;

wxImage  lcd_image;
wxImage  symbols_image;
wxImage  smileys_image;
wxImage  button_image;
wxImage  square_image;
wxImage  triangle_image;
wxImage  hexagon_image;
wxImage  grid3d_image;

wxColour bmp_bg_col; // Bitmap-Background-Colour


//******************************************************************************
void SetBitmapBackgroundColour()
//------------------------------------------------------------------------------
{
  // bmp_bg_col
  wxColour       col = *wxLIGHT_GREY; // (192, 192, 192)
  unsigned char  red, green, blue;

  // Falls keine Truecolour eingeschaltet ist kann es sein, dass die
  // Hintergrundfarbe leicht abweicht. 
  // Deshalb wird eine test-image in eine Bitmap und zurueck gewandelt
  // (GetPalette()-Funktionen funktionieren nicht)

  wxImage img1(1,1);
  img1.SetRGB (0, 0, col.Red(), col.Green(), col.Blue());

  wxImage img2 (img1.ConvertToBitmap());

  red   = img2.GetRed(0,0);  
  green = img2.GetGreen(0,0);
  blue  = img2.GetBlue(0,0); 

  bmp_bg_col.Set(red, green, blue);
}

//******************************************************************************
void PrepareImage (wxImage& image, bool transparent)
//------------------------------------------------------------------------------
//  als Transparentfarbe wird das Pixel links unten genommen
{
  if (transparent)
  {
    int x = 0;
    int y = image.GetHeight() - 1;
  
    unsigned char  red   = image.GetRed (x, y);
    unsigned char  blue  = image.GetBlue (x, y);
    unsigned char  green = image.GetGreen (x, y);
  
    image.SetMaskColour (red, green, blue);
  }
  else
  {
    image.SetMask (false);
  }
}

//******************************************************************************
void LoadImage (wxImage& image, const string& fname, bool transparent)
//------------------------------------------------------------------------------
//  als Transparentfarbe wird das Pixel links unten genommen
{
  ASSERT (image.LoadFile (fname.c_str(), wxBITMAP_TYPE_BMP));

  wxColour win_bg_col = main_win->GetBackgroundColour();
  wxColour bg_col (*wxLIGHT_GREY);

  image.Replace (bg_col.Red(),     bg_col.Green(),     bg_col.Blue(),
                 win_bg_col.Red(), win_bg_col.Green(), win_bg_col.Blue());

  //
  PrepareImage (image, transparent);
}

//******************************************************************************
void SetImage (wxImage& image, char* xpm[], bool transparent)
//------------------------------------------------------------------------------
//  als Transparentfarbe wird das Pixel links unten genommen
{
  ASSERT (xpm != 0);

  image = wxImage(wxBitmap(xpm));

  // replace
  wxColour win_bg_col = main_win->GetBackgroundColour();

  image.Replace (bmp_bg_col.Red(), bmp_bg_col.Green(), bmp_bg_col.Blue(),
                 win_bg_col.Red(), win_bg_col.Green(), win_bg_col.Blue());

  // 
  PrepareImage (image, transparent);
}

//******************************************************************************
void MinesPerfect::InitApi()
//------------------------------------------------------------------------------
{
  SetBitmapBackgroundColour();

/*
  // Bitmaps
  lcd_bmp      = new wxBitmap ("lcd.bmp",      wxBITMAP_TYPE_BMP);
  symbols_bmp  = new wxBitmap ("symbols.bmp",  wxBITMAP_TYPE_BMP);
  square_bmp   = new wxBitmap ("square.bmp",   wxBITMAP_TYPE_BMP);
  triangle_bmp = new wxBitmap ("triangle.bmp", wxBITMAP_TYPE_BMP);
  hexagon_bmp  = new wxBitmap ("hexagon.bmp",  wxBITMAP_TYPE_BMP);
  grid3d_bmp   = new wxBitmap ("grid3d.bmp",   wxBITMAP_TYPE_BMP);
*/  
/*
  // Bitmaps
  LoadImage (lcd_image,      "lcd.bmp",      false);
  LoadImage (symbols_image,  "symbols.bmp",  true);
  LoadImage (smileys_image,  "smileys.bmp",  true);
  LoadImage (button_image,   "button.bmp",   false);
  
  LoadImage (square_image,   "square.bmp",   true);
  LoadImage (triangle_image, "triangle.bmp", true);
  LoadImage (hexagon_image,  "hexagon.bmp",  true);
  LoadImage (grid3d_image,   "3d-grid.bmp",   true);
*/  
  // Bitmaps
  SetImage (lcd_image,      lcd_xpm,      false);
  SetImage (symbols_image,  symbols_xpm,  true);
  SetImage (smileys_image,  smileys_xpm,  true);
  SetImage (button_image,   button_xpm,   false);
  
  SetImage (square_image,   square_xpm,   true);
  SetImage (triangle_image, triangle_xpm, true);
  SetImage (hexagon_image,  hexagon_xpm,  true);
  SetImage (grid3d_image,   grid3d_xpm,   true);
}

/*
const Color Color::BLACK   = clBlack;
const Color Color::WHITE   = clWhite;
const Color Color::DGRAY   = clDkGray;
const Color Color::GRAY    = clGray;
const Color Color::LGRAY   = clLtGray;
const Color Color::RED     = clRed;
const Color Color::BLUE    = clBlue;
const Color Color::GREEN   = clGreen;
const Color Color::YELLOW  = clYellow;
const Color Color::DSHADOW = cl3DDkShadow;
const Color Color::LSHADOW = cl3DLight;
//const Color Color:: = cl;
*/

//******************************************************************************
class BitmapImpl : public MinesPerfect::Bitmap
//------------------------------------------------------------------------------
{
public:
  BitmapImpl (const string& kind, const string& spec);
  BitmapImpl (const MinesPerfect::Bitmap* from, const MinesPerfect::Rect& rect);
  BitmapImpl (const vector<MinesPerfect::Color>& data, int w); // w = width
  ~BitmapImpl() {}

//  void                 draw (const MinesPerfect::Bitmap* bmp, const MinesPerfect::Point& p);
  MinesPerfect::Point  getSize() const ;
  bool                 isTransparent() const { return image.HasMask(); }
  bool                 pointIsTransparent (const MinesPerfect::Point& p) const;

public:
  wxBitmap  bitmap;
  wxImage   image;
};

MinesPerfect::Bitmap* 
MinesPerfect::CreateBitmap (const string& kind, const string& spec)
{
  return new BitmapImpl (kind, spec);
}

MinesPerfect::Bitmap* 
MinesPerfect::CreateBitmap (const MinesPerfect::Bitmap* from, 
                            const MinesPerfect::Rect&   rect)
{
  return new BitmapImpl (from, rect);
}

MinesPerfect::Bitmap* 
MinesPerfect::CreateBitmap (const vector<MinesPerfect::Color>& data, int w)
{
  return new BitmapImpl (data, w);
}

//******************************************************************************
BitmapImpl::BitmapImpl (const vector<MinesPerfect::Color>& data, int w) // w = width
//------------------------------------------------------------------------------
{
  // Funktion noch nicht implementiert!
  ASSERT (false); 

/*
  wxBitmap*  bmp = new wxBitmap;
  rep = bmp;

  bmp->Width  = w;
  bmp->Height = data.size() / w;

  for (int y = 0; y < bmp->Height; y++)
    for (int x = 0; x < bmp->Width; x++)
      bmp->Canvas->Pixels[x][y] = (TColor) data[w * y + x].val;
*/
}

//******************************************************************************
BitmapImpl::BitmapImpl (const string& kind, const string& spec)
//------------------------------------------------------------------------------
{
  int      nr       = -1;
  int      num_cols = -1;
  int      num_rows = -1;

  if (kind == "board")
  {
    string  fname = string("./boards/") + spec + ".bmp";

    if      (wxFile::Exists (fname.c_str()))  LoadImage (image, fname, true);
    else if (spec == "Square")                image = square_image;
    else if (spec == "Triangle")              image = triangle_image;
    else if (spec == "Hexagon")               image = hexagon_image;
    else if (spec == "3d-Grid")               image = grid3d_image;
    else      
      ASSERT (false);

    bitmap = image.ConvertToBitmap();
    return;
  }

  if (kind == "smiley")
  {
    if      (spec == "happy")    nr = 0;
    else if (spec == "1eye")     nr = 1;
    else if (spec == "2eyes")    nr = 2;
    else if (spec == "3eyes")    nr = 3;
    else if (spec == "4eyes")    nr = 4;
    else if (spec == "norm")     nr = 5;
    else if (spec == "ooh")      nr = 6;
    else if (spec == "worry")    nr = 7;
    else if (spec == "sunglass") nr = 8;
    else if (spec == "tmp")      nr = 9;

    num_cols = 5;
    num_rows = 2;     
    image    = smileys_image;
  }
  else if (kind == "lcd")
  {
    nr = atoi (spec.c_str());
    
    if (nr <= 0)
    {
      if      (spec == "0")    nr =  0;
      else if (spec == "-")    nr = 10;
      else                     nr = -1;
    }

    num_cols = 11;
    num_rows =  1;    
    image    = lcd_image;
  }
  else if (kind == "symbol")
  {
    nr = atoi (spec.c_str());

    // enthaelt spec eine Zahl?
    if (nr > 0)
    {
      nr += 4; // Zahlen beginnen in der 2. Zeile
    }
    else // keine Zahl (oder 0)
    {
      if      (spec == "flag")   nr =  0;
      else if (spec == "quest")  nr =  1;
      else if (spec == "mine")   nr =  2;
      else if (spec == "cross")  nr =  3;
      else if (spec == "0")      nr =  4;
      else                       nr = -1; // fehler
    }

    num_cols = 4;
    num_rows = 8;    
    image    = symbols_image;
  }
  else if (kind == "button")
  {
    if      (spec == "up")   nr =  0;
    else if (spec == "down") nr =  1;

    num_cols = 2;
    num_rows = 1;    
    image    = button_image;
  }
  else
  {
    ASSERT (false);
  }

  // check
  ASSERT (nr >= 0 && nr < num_rows * num_cols);
  ASSERT (num_rows > 0 && num_cols > 0 && image != 0);

  // bmp
  int i = nr / num_cols;
  int j = nr % num_cols;
  int w = image.GetWidth()  / num_cols;
  int h = image.GetHeight() / num_rows;

  wxRect  rect (j * w, i * h, w, h);

  image  = image.GetSubImage (rect);
  bitmap = image.ConvertToBitmap();
}

//******************************************************************************
BitmapImpl::BitmapImpl (const MinesPerfect::Bitmap* from, const MinesPerfect::Rect& rect)
//------------------------------------------------------------------------------
{
  wxRect   rect2 (rect.getLeft(), rect.getTop(), rect.getWidth(), rect.getHeight());
  
  const BitmapImpl* from2 = dynamic_cast<const BitmapImpl*>(from);
  
  ASSERT (from2 != 0);
  
  image  = from2->image.GetSubImage(rect2);
  bitmap = image.ConvertToBitmap();
}

//******************************************************************************
MinesPerfect::Point BitmapImpl::getSize() const
//------------------------------------------------------------------------------
{
  return MinesPerfect::Point (image.GetWidth(), image.GetHeight());
}

//******************************************************************************
bool BitmapImpl::pointIsTransparent (const MinesPerfect::Point& p) const
//------------------------------------------------------------------------------
{
  return   image.HasMask()
        && image.GetMaskRed()   == image.GetRed   (p.x, p.y)
        && image.GetMaskGreen() == image.GetGreen (p.x, p.y)
        && image.GetMaskBlue()  == image.GetBlue  (p.x, p.y);
}

//******************************************************************************
class SoundImpl : public MinesPerfect::Sound, public wxWave
//------------------------------------------------------------------------------
{
public:
  SoundImpl (const string& name); 
  bool isOk(); // Methoden muessen anscheinend spaeter definiert
  void play(); // werden. (sonst meckert der Compiler)
};

MinesPerfect::Sound*
MinesPerfect::CreateSound (const string& name)
{
  return new SoundImpl(name);
}

//******************************************************************************
SoundImpl::SoundImpl (const string& name) : wxWave()
//------------------------------------------------------------------------------
{
  string fname = "./sound/" + name + ".wav";

  if (MinesPerfect::FileExist(fname))
    wxWave::Create(fname.c_str());
}

//******************************************************************************
bool SoundImpl::isOk()
//------------------------------------------------------------------------------
{
  return wxWave::IsOk();
}

//******************************************************************************
void SoundImpl::play()
//------------------------------------------------------------------------------
{
  if (wxWave::IsOk())
    wxWave::Play();
}

//******************************************************************************
class TimerImpl : public MinesPerfect::Timer, public wxTimer
//------------------------------------------------------------------------------
{
public:
  TimerImpl() : wxTimer() { num_notifies = 0; clock0 = clock1 = 0; }
    
  bool isRunning() const { return wxTimer::IsRunning(); }

  int getMSecs() const
  { 
    clock_t c1 = isRunning() ? clock() : clock1;

    return 1000 * (c1 - clock0) / CLK_TCK; 
  }

  clock_t start()
  {
    reset();
    clock0 = clock();
    wxTimer::Start(100); // alle 100 ms
                // muss kuerzer als eine Sekunde sein, da wxTimer recht ungenau 
                // ist. Bei 900 ms laeuft die Uhr sehr unruhig.
    return clock0;
  }
  
  void stop()
  {
    clock1 = clock();

    if (wxTimer::IsRunning())
      wxTimer::Stop();
  }
  
  void reset()
  {
    stop();
    num_notifies = 0;
  }  
  
  void Notify()          
  { 
    int num2 = (clock() - clock0) / CLK_TCK;

    if (num_notifies != num2)
    {
      num_notifies = num2;
      main_win->game->showTime(num_notifies);  
    }
  }

private:  
  clock_t  clock0;
  clock_t  clock1;
  int      num_notifies;
};

MinesPerfect::Timer*  MinesPerfect::CreateTimer()
{
  return new TimerImpl();
}

//******************************************************************************
void MinesPerfect::DlgNewRecord (Options* options, int num_msecs, bool certified_board)
//------------------------------------------------------------------------------
{
  // board_text
  wxString  board_text("Board:  ");
  board_text += options->getBoardName().c_str();
  
  // level_text
  wxString  level_text("Level:  ");

  if (options->getLevelNr() == BEGINNER)
    level_text += "Beginner";
  else if (options->getLevelNr() == INTERMEDIATE)
    level_text += "Intermediate";
  else
    level_text += "Expert";

  // msg_text
  wxString msg_text("You have a new record.\n\n");;
  msg_text += board_text + "\n" + level_text;
  
  // rec_nr, old_name
  int       rec_nr = options->getLevelNr();
//  wxString  old_name(options.getRecord(rec_nr).name.c_str());

  // user_vec
  vector<string> user_vec;
  options->getUserlist(user_vec);

  // user_list
  wxString* user_list = new wxString[user_vec.size()];

  for (unsigned k = 0; k < user_vec.size(); ++k)
    user_list[k] = wxString(user_vec[k].c_str());

  // new_name (Dialog)
  wxString                        new_name = (user_vec.size() > 0) ? user_list[0] : wxString("");
  MinesPerfect::User::NameChecker checker;

  GenValidator       validator (&new_name, &checker);
  ChooseComboDialog  dlg (main_win, msg_text, "Congratulations", 
                          user_vec.size(), user_list, validator);

  if (dlg.ShowModal() == wxID_OK)
  {
    options->setRecord (rec_nr, new_name.c_str(), num_msecs, certified_board);
  
    ShowBestTimesDialog dlg3(main_win, main_win->game->m_options);
    dlg3.ShowModal();
//    dlg3.Destroy();
  }

  // tidy up
  delete[] user_list;
}

//******************************************************************************
void MinesPerfect::WinSetSize (const MinesPerfect::Point& sz)
//------------------------------------------------------------------------------
{
  main_win->SetClientSize (sz.x, sz.y);
  main_win->Show(TRUE);
}

//******************************************************************************
void MinesPerfect::WinDrawBitmap (const MinesPerfect::Bitmap* bmp, 
                                  const MinesPerfect::Point& p)
//------------------------------------------------------------------------------
{
  if (bmp == 0)
    return;

  const BitmapImpl*  bmp2 = dynamic_cast<const BitmapImpl*>(bmp);
  ASSERT (bmp2 != 0);
                                    
  wxClientDC  dc (main_win);
  dc.DrawBitmap (bmp2->bitmap, p.x, p.y, true);
}

//******************************************************************************
void MinesPerfect::WinDrawBevel (const MinesPerfect::Rect& rect, int thickness, bool raised)
//------------------------------------------------------------------------------
{
  if (rect.getWidth()  < 2 * thickness
  ||  rect.getHeight() < 2 * thickness)
    return;

  wxClientDC  dc (main_win);
  const int   l = rect.getLeft();
  const int   t = rect.getTop();
  const int   r = rect.getLeft() + rect.getWidth()  - 1;
  const int   b = rect.getTop()  + rect.getHeight() - 1;

  dc.SetPen (raised ? *wxGREY_PEN : *wxWHITE_PEN);


  int i; // visual studio

  for (i = 0; i < thickness; i++)
  {
    dc.DrawLine (r - i, t + i, r - i,     b - i); // rechts
    dc.DrawLine (r - i, b - i, l + i - 1, b - i); // unten 
  }

  dc.SetPen (raised ? *wxWHITE_PEN : *wxGREY_PEN);

  for (i = 0; i < thickness; i++)
  {
    dc.DrawLine (l + i, b - i - 1, l + i, t + i); // links
    dc.DrawLine (l + i, t + i,     r - i, t + i); // oben
  }
}

//******************************************************************************
void MinesPerfect::FindFiles (vector<string>& files, const string& pattern)
//------------------------------------------------------------------------------
{
  wxString  fname;
  wxDir     dir (".");

  // Aufruf von dir.GetFirst oder dir.GetNext liefert einen Fehler 
  // (aber erst spaeter), falls das Verzeichnis nicht existiert.

  // Ende, falls Verzeichnis nicht exist.
  unsigned i = pattern.rfind('/');

  if (i != string::npos
  &&  !wxDir::Exists (pattern.substr(0, i).c_str()))
      return;

  // files
  if (dir.GetFirst (&fname, wxString (pattern.c_str())))
  {
    files.push_back (fname.c_str());
    
    while (dir.GetNext (&fname))
      files.push_back (fname.c_str());
  }
}

//******************************************************************************
bool MinesPerfect::FileExist (const string& fname)
//------------------------------------------------------------------------------
{
  return wxFile::Exists (fname.c_str());
}

//******************************************************************************
void MinesPerfect::ShowMessageDlg (const string& text, const string& title)
//------------------------------------------------------------------------------
{
  wxMessageDialog  dlg (main_win, text.c_str(), title.c_str(), wxOK);
  dlg.ShowModal();
}

//******************************************************************************
bool MinesPerfect::StartBrowser (const string& protokoll, const string& fname)
//------------------------------------------------------------------------------
{
  // ftype
  wxMimeTypesManager mime;
  wxFileType*        ftype = mime.GetFileTypeFromExtension("html");
  
  if (ftype == 0)
  {
    wxMessageDialog  dlg (main_win, "There's no browser for html-files!", 
                          "Error", wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  }

  // mime_type
  wxString  mime_type;
  ftype->GetMimeType (&mime_type);
  if (mime_type != "text/html")
  {
    wxMessageDialog  dlg (main_win, "Mimetype is wrong!", 
                          "Error", wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  }
 
  // command, params
  wxFileType::MessageParameters  params (fname.c_str(), mime_type);
  wxString                       command;

  if (!ftype->GetOpenCommand(&command, params))
  {
    wxMessageDialog  dlg (main_win, "Error in 'GetOpenCommand'", 
                          "Error", wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return false;
  }

  if (protokoll != "file")
    command.Replace("file://", wxString(protokoll.c_str()) + "://", false);

  // execute
  wxExecute (command);
  return true;
}


