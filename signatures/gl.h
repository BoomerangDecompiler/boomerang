void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glEnable(GLenum cap);
void glClear(GLbitfield mask);
void glLoadIdentity(void);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glFlush(void);
void glDisable(GLenum cap);
void glMatrixMode(GLenum mode);
void glPushMatrix(void);
void glPopMatrix(void);
void glBegin(GLenum mode);
void glEnd(void);
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
			 GLdouble near_val, GLdouble far_val);
void glColor3f(GLfloat red, GLfloat green, GLfloat blue);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
GLenum glGetError(void);
void glRasterPos2f(GLfloat x, GLfloat y);

