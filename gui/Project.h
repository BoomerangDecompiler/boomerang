
/*
 * This file describes the Project class used to store information about the current project.
 * It is responsible for displaying dialogs which can create new Project classes, open existing
 * representations of projects from disk and display settings related to the Project.
 *
 */

class Project 
{
	bool m_unsaved;
	string filename;
public:

	bool hasUnsaved() { return m_unsaved; }
	void saveChanges(wxWindow *parent);
	void displaySettingsDialog(wxWindow *parent);

	static Project *createProjectUsingDialog(wxWindow *parent);
	static Project *openProjectUsingDialog(wxWindow *parent);
};
