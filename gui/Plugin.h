
class IPlugin
{
public:
	virtual wxMDIChildFrame *GetWindow(wxMDIParentFrame *parent) = 0;
	virtual wxString &GetName(void) = 0;

	static IPlugin *load(wxString filename);
};