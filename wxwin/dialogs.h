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

#ifndef DLGABOUT_H
#define DLGABOUT_H

#include <wx/valgen.h>

class AboutDialog : public wxDialog
{
  public:
    AboutDialog(wxWindow *parent);

  private:
    wxBoxSizer*      m_topsizer;
    wxBoxSizer*      m_hsizer1;
    wxStaticBitmap*  m_bitmap;
    wxStaticText*    m_text1;
    wxStaticText*    m_text2;
    wxBoxSizer*      m_hsizer3;
    wxStaticText*    m_text3;
    wxButton*        m_button;
};

class ShowBestTimesDialog : public wxDialog
{
  public:
    ShowBestTimesDialog(wxWindow *parent, MinesPerfect::Options* options);
    void OnButton(wxCommandEvent& event);
    void OnChoice(wxCommandEvent& event);

  private:
    MinesPerfect::Options*  m_options;
  
    wxBoxSizer*       m_topsizer;
    wxFlexGridSizer*  m_score_sizer;
    wxBoxSizer*       m_button_sizer1;
    wxBoxSizer*       m_button_sizer2;
    wxBoxSizer*       m_nobody_sizer;

    wxChoice*         m_choice;

    wxStaticText*     m_level_text[3];
    wxStaticText*     m_time_text[3];
    wxStaticText*     m_name_text[3];
    wxRadioButton*    m_select_buttons[3];
    int               m_sel_board;
    int               m_sel_level;

    wxButton*         m_ok_button;
    wxButton*         m_reset_button;
    wxButton*         m_send_button;
    wxButton*         m_show_button;
    
    wxStaticText*     m_nobody_text;

    void Show();
    void Send();
    
    DECLARE_EVENT_TABLE()
};

class SelfdefinedDialog : public wxDialog
{
  public:
    SelfdefinedDialog(wxWindow *parent, MinesPerfect::Level* lvl);
    void OnButton(wxCommandEvent& event);
    
  private:
  
    MinesPerfect::Level*  m_level;
  
    wxBoxSizer*        m_topsizer;
    wxFlexGridSizer*   m_inputsizer;
    wxBoxSizer*        m_buttonsizer;
    
    wxTextCtrl*        m_rowsctrl;
    wxTextCtrl*        m_colsctrl;
    wxTextCtrl*        m_pagesctrl;
    wxTextCtrl*        m_minesctrl;
    wxTextCtrl*        m_wholesctrl;
    
    DECLARE_EVENT_TABLE()
};

class GenValidator : public wxValidator
{
  public:

    GenValidator(wxString* text, MinesPerfect::StringChecker* checker)
       : m_text(text), m_checker(checker) {}

    GenValidator(const GenValidator& val) 
       : m_text(val.m_text), m_checker(val.m_checker) {}

    ~GenValidator(){}

    wxObject *Clone(void) const { return new GenValidator(*this); }

    bool TransferToWindow(void);
    bool TransferFromWindow(void);

    void OnChar(wxKeyEvent& event);
    bool Validate(wxWindow* parent);

  private:

    wxString*                    m_text; 
    MinesPerfect::StringChecker* m_checker;

    DECLARE_EVENT_TABLE()
};

class ChooseComboDialog : public wxDialog
{
  public:

    enum ValidatorType { USERNAME_VALIDATOR, USERADD_VALIDATOR };

    ChooseComboDialog (wxWindow *parent, const wxString& msg, 
                       const wxString& title, int n, const wxString* list,
                       const wxValidator& validator);

    wxComboBox*        m_combo;
    wxStaticText*      m_msg_text;

  private:
  
    int                m_n;
    const wxString*    m_list;
  
    wxBoxSizer*        m_topsizer;
    wxBoxSizer*        m_buttonsizer;
    
    wxButton*          m_ok_button;
    wxButton*          m_cancel_button;
    
    DECLARE_EVENT_TABLE()
};

class ExceptionDialog : public wxDialog
{
  public:
    ExceptionDialog(wxWindow *parent, const wxString& exc_text);
    void OnButton(wxCommandEvent& event);

  private:
    wxString         m_exc_text;

    wxBoxSizer*      m_topsizer;
    wxStaticText*    m_static_text;
    wxBoxSizer*      m_buttonsizer;
    wxButton*        m_send_button;
    wxButton*        m_save_button;
    wxButton*        m_cancel_button;

    DECLARE_EVENT_TABLE()
};

#endif
