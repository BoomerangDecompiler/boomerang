
#include "stdincs.h"
#include "Preferences.h"

wxString Preferences::m_ProjectDir = "";
wxString Preferences::m_PluginDir = "";

BEGIN_EVENT_TABLE(Preferences, wxDialog)
    EVT_BUTTON(-1, Preferences::OnButton)
END_EVENT_TABLE()

void Preferences::LoadPreferences(void)
{
	config->Read("ProjectDir", &m_ProjectDir);
	config->Read("PluginDir", &m_PluginDir);
}

void Preferences::SavePreferences(void)
{
	config->Write("ProjectDir", m_ProjectDir);
	config->Write("PluginDir", m_PluginDir);
}

void Preferences::GetPrefsFromDialog(void)
{
	m_ProjectDir = m_textProjectDir->GetValue();
	m_PluginDir = m_textPluginDir->GetValue();
}

Preferences::Preferences(wxWindow* parent) : 
	wxDialog(parent, -1, wxString("Preferences"))
{
	config = wxConfigBase::Get();

	LoadPreferences();

	wxStaticText *st = new wxStaticText(this, -1, "Projects directory", wxPoint(0, 0));	
	m_textProjectDir = new wxTextCtrl(this, -1, m_ProjectDir, wxPoint(st->GetRect().GetRight(), 0));
	m_btnBrowse = new wxButton(this, -1, "Browse", wxPoint(m_textProjectDir->GetRect().GetRight(), 0));
	wxStaticText *st1 = new wxStaticText(this, -1, "Plugin directory", wxPoint(m_btnBrowse->GetRect().GetRight(), 0));	
	m_textPluginDir = new wxTextCtrl(this, -1, m_PluginDir, wxPoint(st1->GetRect().GetRight(), 0));
	m_btnBrowse1 = new wxButton(this, -1, "Browse", wxPoint(m_textPluginDir->GetRect().GetRight(), 0));
	m_btnApply = new wxButton(this, -1, "&Apply", wxPoint(0, GetSize().GetHeight() - 100));
    m_btnSave = new wxButton(this, -1, "&Save", wxPoint(GetSize().GetWidth()/2 - 100, GetSize().GetHeight() - 100));
    m_btnCancel = new wxButton(this, -1, "&Cancel", wxPoint(GetSize().GetWidth() - 200, GetSize().GetHeight() - 100));

    m_btnSave->SetFocus();
    m_btnSave->SetDefault();
}

void Preferences::OnButton(wxCommandEvent& event)
{
	if ( event.GetEventObject() == m_btnApply )
	{
		GetPrefsFromDialog();
		Show(false);
	} else if ( event.GetEventObject() == m_btnBrowse )
	{
		wxDirDialog *d = new wxDirDialog(this);
		if (d->ShowModal() == wxID_OK)
		{
			m_textProjectDir->SetValue(d->GetPath());
		}
	} else if ( event.GetEventObject() == m_btnBrowse1 )
	{
		wxDirDialog *d = new wxDirDialog(this);
		if (d->ShowModal() == wxID_OK)
		{
			m_textPluginDir->SetValue(d->GetPath());
		}
	} else if ( event.GetEventObject() == m_btnCancel )
	{
		Show(false);
	} else if ( event.GetEventObject() == m_btnSave )
	{
		GetPrefsFromDialog();
		SavePreferences();
		Show(false);
	} else {
		event.Skip();
	}
}