void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glEnable(GLenum cap);
void glClear(GLbitfield mask);
void glLoadIdentity();
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glFlush();
void glDisable(GLenum cap);
void glMatrixMode(GLenum mode);
void glPushMatrix();
void glPopMatrix();
void glBegin(GLenum mode);
void glEnd();
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
             GLdouble near_val, GLdouble far_val);
void glColor3f(GLfloat red, GLfloat green, GLfloat blue);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
GLenum glGetError();
void glRasterPos2f(GLfloat x, GLfloat y);


void glGenTextures(GLsizei n, GLuint *textures);
void glBindTexture(GLenum target, GLuint texture);
__stdcall void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
__stdcall void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
