
class TestPluginFrame : public wxMDIChildFrame
{
public:
	TestPluginFrame(wxMDIParentFrame *parent);
};

class TestPlugin : public IPlugin
{
	TestPluginFrame *m_window;
	wxString name;
public:
	virtual wxMDIChildFrame *GetWindow(wxMDIParentFrame *parent);
	virtual wxString &GetName(void) { return name; }
	TestPlugin() : m_window(NULL), name("Test Plugin") { }
};

