

class Preferences : public wxDialog 
{
public:
	Preferences(wxWindow* parent);

	void LoadPreferences(void);
	void SavePreferences(void);
	void OnButton(wxCommandEvent& event);

	// the actual preferences
	static wxString m_ProjectDir;
	static wxString m_PluginDir;

private:

	void GetPrefsFromDialog();

	wxButton *m_btnApply, *m_btnSave, *m_btnCancel, *m_btnBrowse, *m_btnBrowse1;
	wxTextCtrl *m_textProjectDir;
	wxTextCtrl *m_textPluginDir;

	wxConfigBase *config;

	DECLARE_EVENT_TABLE()
};