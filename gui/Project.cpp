
#include "stdincs.h"

#include "Project.h"
#include "Preferences.h"

void Project::saveChanges(wxWindow *parent)
{	
	ofstream out(filename.c_str());
	out << "# Boomerang project file.  WARNING!  Editing this file may damage it.  WARNING!" << endl << endl;
	out.close();
}

void Project::displaySettingsDialog(wxWindow *parent)
{
}

Project *Project::createProjectUsingDialog(wxWindow *parent)
{
	// TODO: replace this dialog with a custom new project dialog.
	wxFileDialog *d = new wxFileDialog(parent, wxString("New Project"), Preferences::m_ProjectDir, "", "*.bdp", wxSAVE);
	if (d->ShowModal() == wxID_CANCEL)
		return NULL;

	Project *p = new Project();
	p->filename = d->GetFilename();
	p->m_unsaved = true;
	return p;
}

Project *Project::openProjectUsingDialog(wxWindow *parent)
{
	wxFileDialog *d = new wxFileDialog(parent, "Open Project", Preferences::m_ProjectDir, "", "*.bdp", wxOPEN);
	if (d->ShowModal() == wxID_CANCEL)
		return NULL;

	Project *p = new Project();
	p->filename = d->GetFilename();
	p->m_unsaved = false;
	return p;
}
