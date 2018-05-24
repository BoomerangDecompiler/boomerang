union { int[]; __size32 *; } global2_804c280;
int global21_804b4e0 = 100;
unsigned char global120_804c05a = 1;
int global112_804c05c = 1;
union { GLfloat x812; float; __size32; } global147_804c068;
union { float; int; } global145_804c060;
union { GLfloat x813; float; __size32; } global149_804c064;
double global189_804b798 = 90.;
double global188_804b790 = 2.;
union { float x811; __size8; int * x809; __size8 * x810; int[]; } global175_804b7a0;
double global222_804b498 = 6.;
double global238_804b490 = 2.;
union { int[]; __size32 *; } global2_804c280;
int global21_804b4e0 = 100;
unsigned char global120_804c05a = 1;
int global112_804c05c = 1;
union { GLfloat x812; float; __size32; } global147_804c068;
union { float; int; } global145_804c060;
union { GLfloat x813; float; __size32; } global149_804c064;
double global189_804b798 = 90.;
double global188_804b790 = 2.;
union { float x811; __size8; int * x809; __size8 * x810; int[]; } global175_804b7a0;
double global222_804b498 = 6.;
double global238_804b490 = 2.;
int main(int argc, char *argv[]);
void proc_0x0804a550();
void proc_0x0804ac50();
void proc_0x0804aeec(char key, int x, int y);
void proc_0x0804b098(int key, int x, int y);
void proc_0x0804ad04(int width, int height);
void proc_0x0804ab20();
void proc_0x0804aaa0();
void proc_0x0804a940(char param1[]);
void proc_0x0804ac08(union { char[] *; __size32; } param1);
__size32 proc_0x0804a29c(union { __size80; double; } param1);
__size32 proc_0x0804a4e0(int param1);
__size32 proc_0x0804a228(union { __size80; double; } param1);
__size32 proc_0x0804a330(union { __size80; double; } param1);
__size32 proc_0x0804a3c4(union { __size80; double; } param1);
__size32 proc_0x0804a438(union { __size32 *; __size32; } param1);
void proc_0x0804a5ac(__size32 param1, __size32 param2, int param3);
void proc_0x0804a610();
void proc_0x0804a1f4();
void proc_0x0804a17c(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5);
void proc_0x0804a154(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5);
void proc_0x0804a1a4(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5);
void proc_0x0804a1cc(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5);
void proc_0x08048ffc(union { __size32 *; __size32; } param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6);

/** address: 0x0804b320 */
int main(int argc, char *argv[])
{
    int *local0; 		// m[esp - 28]

    glutInit(&argc, argv);
    glutInitDisplayMode(16);
    local0 = *argv;
    glutCreateWindow(local0);
    proc_0x0804a550();
    glutDisplayFunc(proc_0x0804ac50);
    glutKeyboardFunc(proc_0x0804aeec);
    glutSpecialFunc(proc_0x0804b098);
    glutReshapeFunc(proc_0x0804ad04);
    glutMainLoop();
    return 0;
}

/** address: 0x0804a550 */
void proc_0x0804a550()
{
    glClearColor(0, 0x3f000000, 0x3f333333, 0x3f800000);
    glEnable(0xb71);
    memset(0x804c280, 0, 500);
    memset(0x804ca50, 0, 4);
    return;
}

/** address: 0x0804ac50 */
void proc_0x0804ac50()
{
    int esp; 		// r28
    union { int; GLfloat; } local4; 		// m[esp - 16]

    glClear(0x4100);
    glLoadIdentity();
    if (*0x804c05c == 1) {
        glTranslatef(0, 0, (float)(0 - global21_804b4e0));
        glTranslatef(0, 0, global145_804c060 ^ 0x80000000);
    }
    glRotatef(global147_804c068, 0x3f800000, 0, 0);
    glRotatef(global149_804c064, 0, 0x3f800000, 0);
    proc_0x0804ab20();
    esp = proc_0x0804aaa0();
    proc_0x0804a940(*(esp - 80));
    glFlush();
    local4 = 0x804b76b;
    proc_0x0804ac08(*(esp - 16));
    return;
}

/** address: 0x0804aeec */
void proc_0x0804aeec(char key, int x, int y)
{
    unsigned int al; 		// r8
    __size32 eax; 		// r24
    int eax_1; 		// r24{0}

    eax = ((unsigned char) key);
    if ((unsigned int)(((unsigned char) key) - 27) <= (unsigned int)95) {
bb0x804af0c:
        switch(((unsigned char) key)) {
        case 27:
            exit(0);
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
            al =  (*0x804c058 == 0) ? 1 : 0;
            *(unsigned int*)(&global175_804b7a0 + 0x8b8) = al;
            if (*0x804c058 == 0) {
                glShadeModel();
bb0x804af59:
                goto bb0x804af59;
            }
            else {
                glShadeModel();
bb0x804af42:
                goto bb0x804af42;
            }
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
        case 58:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
            *(union { float; __size32; }*)0x804c068 = global147_804c068 + global188_804b790;
            if (global147_804c068 > global189_804b798) {
bb0x804aff4:
                global147_804c068 = 0x42b40000;
                goto bb0x804aff4;
            }
            else {
bb0x804afef:
                goto bb0x804afef;
            }
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 79:
        case 80:
            eax = glutGet();
            eax_1 = glutGet();
            if (*0x804c05c != 1) {
                global112_804c05c = 1;
            }
            else {
                global112_804c05c = 0;
            }
            proc_0x0804ad04(eax, eax_1);
bb0x804b08c:
            goto bb0x804b08c;
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
            al =  (*0x804c059 == 0) ? 1 : 0;
            *(unsigned int*)(&global175_804b7a0 + 0x8b9) = al;
            if (*0x804c059 == 0) {
                glPolygonMode();
bb0x804afae:
                goto bb0x804afae;
            }
            else {
                glPolygonMode();
bb0x804af93:
                goto bb0x804af93;
            }
        case 88:
        case 89:
        case 90:
            *(union { float; __size32; }*)0x804c068 = global147_804c068 - global188_804b790;
            if (global147_804c068 < 0.) {
bb0x804b030:
                global147_804c068 = 0;
                goto bb0x804b030;
            }
            else {
bb0x804b02e:
                goto bb0x804b02e;
            }
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
            goto bb0x804af0c;
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:
        case 111:
        case 112:
            goto bb0x804af0c;
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 119:
            goto bb0x804af0c;
        case 120:
        case 121:
        case 122:
            goto bb0x804af0c;
        }
    }
    glutPostRedisplay();
    return;
}

/** address: 0x0804b098 */
void proc_0x0804b098(int key, int x, int y)
{
    unsigned char al; 		// r8
    __size32 eax; 		// r24
    int eax_1; 		// r24{0}
    int eax_2; 		// r24{0}
    int eax_3; 		// r24{0}
    int eax_4; 		// r24{0}
    int eax_5; 		// r24{0}
    int eax_6; 		// r24{0}
    __size80 st; 		// r32
    double st6; 		// r38

    if (key == 5) {
        eax_1 = proc_0x0804a29c(st);
        eax = proc_0x0804a4e0(eax_1);
        if (eax != -1) {
            *(__size32*)(&global2_804c280 + 2012)++;
        }
        else {
            free(eax_1);
        }
    }
    else {
        if (key > 5) {
            if (key == 101) {
                st6 = *((float *)&*(&global175_804b7a0 + 384));
                *(union { float; __size32; }*)0x804c060 = global145_804c060 - st6;
                if (*0x804c05c == 0) {
                    eax_3 = glutGet();
                    eax = glutGet();
                    proc_0x0804ad04(eax_3, eax);
                }
            }
            else {
                if (key > 101) {
                    if (key == 102) {
                        st6 = *((float *)&*(&global175_804b7a0 + 384));
                        *(union { float; __size32; }*)0x804c064 = global149_804c064 - st6;
                    }
                    else {
                        if (key == 103) {
                            st6 = *((float *)&*(&global175_804b7a0 + 384));
                            *(union { float; __size32; }*)0x804c060 = global145_804c060 + st6;
                            if (*0x804c05c == 0) {
                                eax_6 = glutGet();
                                eax = glutGet();
                                proc_0x0804ad04(eax_6, eax);
                            }
                        }
                        else {
                        }
                    }
                }
                else {
                    if (key == 100) {
                        st6 = *((float *)&*(&global175_804b7a0 + 384));
                        *(union { float; __size32; }*)0x804c064 = global149_804c064 + st6;
                    }
                    else {
                    }
                }
            }
        }
        else {
            if (key == 2) {
                eax_2 = proc_0x0804a228(st);
                eax = proc_0x0804a4e0(eax_2);
                if (eax != -1) {
                    *(__size32*)(&global2_804c280 + 2000)++;
                }
                else {
                    free(eax_2);
                }
            }
            else {
                if (key > 2) {
                    if (key == 3) {
                        eax_4 = proc_0x0804a330(st);
                        eax = proc_0x0804a4e0(eax_4);
                        if (eax != -1) {
                            *(__size32*)(&global2_804c280 + 2004)++;
                        }
                        else {
                            free(eax_4);
                        }
                    }
                    else {
                        if (key == 4) {
                            eax_5 = proc_0x0804a3c4(st);
                            eax = proc_0x0804a4e0(eax_5);
                            if (eax != -1) {
                                *(__size32*)(&global2_804c280 + 2008)++;
                            }
                            else {
                                free(eax_5);
                            }
                        }
                        else {
                        }
                    }
                }
                else {
                    if (key == 1) {
                        al =  (*0x804c05a == 0) ? 1 : 0;
                        global120_804c05a = al;
                    }
                    else {
                    }
                }
            }
        }
    }
    glutPostRedisplay();
    return;
}

/** address: 0x0804ad04 */
void proc_0x0804ad04(int width, int height)
{
    GLdouble local0; 		// m[esp - 64]
    double st6; 		// r38
    double st7; 		// r39
    double st7_1; 		// r39{0}
    double st7_4; 		// r39{0}
    double st7_5; 		// r39{0}

    st7_1 = (float)width;
    st6 = (float)height;
    glViewport();
    glMatrixMode(0x1701);
    glLoadIdentity();
    if (global112_804c05c == 0) {
        st7 = st7_1 / st6;
        if (st7_1 / st6 > 1.) {
            st7_5 = (float)*0x804b4e0;
            st7 = (float)(0 - global21_804b4e0) * st7_1 / st6;
            glOrtho((float)(0 - global21_804b4e0) * st7_1 / st6, 0 - global21_804b4e0, st7_5 * st7_1 / st6, local0, (float)(0 - global21_804b4e0), 0 - global21_804b4e0);
        }
        else {
            st7_4 = (float)*0x804b4e0;
            glOrtho((float)(0 - global21_804b4e0), 0 - global21_804b4e0, st7_4, local0, (float)(0 - global21_804b4e0) / st7_1 / st6, 0 - global21_804b4e0);
        }
    }
    else {
        if (global112_804c05c == 1) {
            gluPerspective();
        }
        else {
            global112_804c05c = 1;
            gluPerspective();
        }
    }
    glMatrixMode(0x1700);
    return;
}

/** address: 0x0804ab20 */
void proc_0x0804ab20()
{
    double st7; 		// r39
    double st7_1; 		// r39{0}

    glColor3f(0x3f333333, 0x3f000000, 0);
    glBegin(7);
    glVertex3f((float)(0 - global21_804b4e0), 0, (float)(0 - global21_804b4e0));
    st7 = (float)*0x804b4e0;
    glVertex3f((float)(0 - global21_804b4e0), 0, st7);
    st7_1 = (float)*0x804b4e0;
    st7 = (float)*0x804b4e0;
    glVertex3f(st7, 0, st7_1);
    st7 = (float)*0x804b4e0;
    glVertex3f(st7, 0, (float)(0 - global21_804b4e0));
    glEnd();
    return;
}

/** address: 0x0804aaa0 */
void proc_0x0804aaa0()
{
    __size32 eax; 		// r24
    __size32 ebp; 		// r29
    int esp; 		// r28
    int local0; 		// m[esp - 4]

    ebp = (esp - 4);
    esp = glPushMatrix();
    while (*(ebp - 8) != 0) {
        *(__size32*)(esp - 16) = *(ebp - 8);
        esp = proc_0x0804a438(*(esp - 16)); /* Warning: also results in ebp */
        esp += 16;
        *(__size32*)(ebp - 4)++;
        if (*(ebp - 4) <= 499) {
bb0x804aaf0:
            eax = *(ebp - 4);
            *(__size32*)(ebp - 8) = global2_804c280[eax];
            goto bb0x804aaf0;
        }
        *(__size32*)(ebp - 8) = 0;
    }
    glPopMatrix();
    return;
}

/** address: 0x0804a940 */
void proc_0x0804a940(char param1[])
{
    int esp; 		// r28

    glDisable(0xb71);
    sprintf(&param1, 0x804b700);
    glMatrixMode(0x1701);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 0, 0, 0x40240000, 0, 0);
    glMatrixMode(0x1700);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(0, 0x3f800000, 0);
    proc_0x0804a5ac(esp - 76, 0, 0x411ccccd);
    if (*0x804c05c != 1) {
        sprintf(&param1, 0x804b74a);
    }
    else {
        sprintf(&param1, 0x804b732);
    }
    proc_0x0804a5ac(esp - 76, 0, 0x41180000);
    if (*0x804c05a != 0) {
        proc_0x0804a610();
    }
    glPopMatrix();
    glMatrixMode(0x1701);
    glPopMatrix();
    glMatrixMode(0x1700);
    glEnable(0xb71);
    return;
}

/** address: 0x0804ac08 */
void proc_0x0804ac08(union { char[] *; __size32; } param1)
{
    GLenum eax_2; 		// r24{0}
    char *eax_3; 		// r24{0}

    for(;;) {
        eax_2 = glGetError();
        if (eax_2 != 0) {
            eax_3 = gluErrorString(eax_2);
            printf("%s: %s\n", param1, eax_3);
        }
    }
    return;
}

/** address: 0x0804a29c */
__size32 proc_0x0804a29c(union { __size80; double; } param1)
{
    __size32 eax; 		// r24
    void *eax_1; 		// r24{0}
    double st; 		// r32

    eax_1 = calloc(1, 20);
    eax = rand();
    proc_0x0804a1f4();
    proc_0x0804a1f4();
    st = fabs(param1) + global238_804b490;
    proc_0x0804a1f4();
    proc_0x0804a17c(eax_1, st, st, param1, (float)((( (eax < 0) ? -1 : 0) << 32 | eax) % 360));
    return eax_1;
}

/** address: 0x0804a4e0 */
__size32 proc_0x0804a4e0(int param1)
{
    int local0; 		// m[esp - 8]

    local0 = 0;
    while (local0 <= 499) {
        if (global2_804c280[local0] == 0) {
bb0x804a510:
            goto bb0x804a510;
        }
        local0++;
    }
    if (local0 > 499) {
        local0 = -1;
    }
    else {
        global2_804c280[local0] = param1;
    }
    return local0;
}

/** address: 0x0804a228 */
__size32 proc_0x0804a228(union { __size80; double; } param1)
{
    __size32 eax; 		// r24
    void *eax_1; 		// r24{0}

    eax_1 = calloc(1, 20);
    eax = rand();
    proc_0x0804a1f4();
    proc_0x0804a1f4();
    proc_0x0804a154(eax_1, param1, 0x3f19999a, param1, (float)((( (eax < 0) ? -1 : 0) << 32 | eax) % 360));
    return eax_1;
}

/** address: 0x0804a330 */
__size32 proc_0x0804a330(union { __size80; double; } param1)
{
    __size32 eax; 		// r24
    void *eax_1; 		// r24{0}
    double st; 		// r32

    eax_1 = calloc(1, 20);
    eax = rand();
    proc_0x0804a1f4();
    proc_0x0804a1f4();
    st = fabs(param1) + global222_804b498;
    proc_0x0804a1f4();
    proc_0x0804a1a4(eax_1, st, st, param1, (float)((( (eax < 0) ? -1 : 0) << 32 | eax) % 360));
    return eax_1;
}

/** address: 0x0804a3c4 */
__size32 proc_0x0804a3c4(union { __size80; double; } param1)
{
    __size32 eax; 		// r24
    void *eax_1; 		// r24{0}

    eax_1 = calloc(1, 20);
    eax = rand();
    proc_0x0804a1f4();
    proc_0x0804a1f4();
    proc_0x0804a1cc(eax_1, param1, 0x3ecccccd, param1, (float)((( (eax < 0) ? -1 : 0) << 32 | eax) % 360));
    return eax_1;
}

/** address: 0x0804a438 */
__size32 proc_0x0804a438(union { __size32 *; __size32; } param1)
{
    __size32 eax; 		// r24
    __size32 ebp; 		// r29
    int ecx; 		// r25
    int edx; 		// r26
    int esp; 		// r28
    __size32 local0; 		// m[esp - 20]
    __size32 local1; 		// m[esp - 24]
    __size32 local2; 		// m[esp - 28]

    glPushMatrix();
    local0 = *(param1 + 8);
    local1 = *(param1 + 4);
    local2 = *param1;
    glTranslatef(local2, local1, local0);
    local2 = *(param1 + 12);
    ecx = glRotatef(local2, 0, 0x3f800000, 0); /* Warning: also results in edx */
    eax = *(param1 + 16);
    (**(param1 + 16))(pc, param1, 0, 0x3f800000, 0, ebp, param1, eax, ecx, edx, esp - 4, SUBFLAGS32((esp - 12), 12, esp - 24), esp - 24 == 0, esp - 12 < (unsigned int)12);
    glPopMatrix();
    ebp = *ebp;
    return ebp;
}

/** address: 0x0804a5ac */
void proc_0x0804a5ac(__size32 param1, __size32 param2, int param3)
{
    __size32 eax; 		// r24
    int local0; 		// m[esp - 12]

    local0 = 0;
    glRasterPos2f(param2, param3);
    eax = local0 + param1;
    while (*(local0 + param1) != 0) {
        if (local0 <= 999) {
            eax = (int) *(local0 + param1);
            glutBitmapCharacter(0x804c240, eax);
            local0++;
        }
        else {
bb0x804a5e8:
            goto bb0x804a5e8;
        }
        eax = local0 + param1;
    }
    return;
}

/** address: 0x0804a610 */
void proc_0x0804a610()
{
    proc_0x0804a5ac(0x804b4e4, 0x40000000, 0x41100000);
    proc_0x0804a5ac(0x804b4fa, 0x40000000, 0x41000000);
    proc_0x0804a5ac(0x804b500, 0x40400000, 0x41000000);
    proc_0x0804a5ac(0x804b52d, 0x40000000, 0x40f00000);
    proc_0x0804a5ac(0x804b540, 0x40400000, 0x40f00000);
    proc_0x0804a5ac(0x804b56d, 0x40000000, 0x40e00000);
    proc_0x0804a5ac(0x804b578, 0x40400000, 0x40e00000);
    proc_0x0804a5ac(0x804b58b, 0x40000000, 0x40d00000);
    proc_0x0804a5ac(0x804b58d, 0x40400000, 0x40d00000);
    proc_0x0804a5ac(0x804b5a8, 0x40000000, 0x40c00000);
    proc_0x0804a5ac(0x804b5ab, 0x40400000, 0x40c00000);
    proc_0x0804a5ac(0x804b5be, 0x40000000, 0x40b00000);
    proc_0x0804a5ac(0x804b5c1, 0x40400000, 0x40b00000);
    proc_0x0804a5ac(0x804b5d3, 0x40000000, 0x40a00000);
    proc_0x0804a5ac(0x804b5d6, 0x40400000, 0x40a00000);
    proc_0x0804a5ac(0x804b5eb, 0x40000000, 0x40900000);
    proc_0x0804a5ac(0x804b5ee, 0x40400000, 0x40900000);
    proc_0x0804a5ac(0x804b604, 0x40000000, 0x40800000);
    proc_0x0804a5ac(0x804b607, 0x40400000, 0x40800000);
    proc_0x0804a5ac(0x804b619, 0x40000000, 0x40400000);
    proc_0x0804a5ac(0x804b620, 0x40400000, 0x40400000);
    proc_0x0804a5ac(0x804b64a, 0x40000000, 0x40200000);
    proc_0x0804a5ac(0x804b660, 0x40400000, 0x40200000);
    proc_0x0804a5ac(0x804b68a, 0x40000000, 0x40000000);
    proc_0x0804a5ac(0x804b68f, 0x40400000, 0x40000000);
    proc_0x0804a5ac(0x804b6a2, 0x40000000, 0x3fc00000);
    proc_0x0804a5ac(0x804b6a8, 0x40400000, 0x3fc00000);
    proc_0x0804a5ac(0x804b6c2, 0x40000000, 0x3f800000);
    proc_0x0804a5ac(0x804b6c4, 0x40400000, 0x3f800000);
    proc_0x0804a5ac(0x804b6df, 0x40000000, 0x3f000000);
    proc_0x0804a5ac(0x804b6e1, 0x40400000, 0x3f000000);
    return;
}

/** address: 0x0804a1f4 */
void proc_0x0804a1f4()
{
    rand();
    return;
}

/** address: 0x0804a17c */
void proc_0x0804a17c(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5)
{
    proc_0x08048ffc(param1, param2, param3, param4, param5, 0x804a138);
    return;
}

/** address: 0x0804a154 */
void proc_0x0804a154(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5)
{
    proc_0x08048ffc(param1, param2, param3, param4, param5, 0x8049450);
    return;
}

/** address: 0x0804a1a4 */
void proc_0x0804a1a4(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5)
{
    proc_0x08048ffc(param1, param2, param3, param4, param5, 0x80497dc);
    return;
}

/** address: 0x0804a1cc */
void proc_0x0804a1cc(__size32 param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5)
{
    proc_0x08048ffc(param1, param2, param3, param4, param5, 0x8049740);
    return;
}

/** address: 0x08048ffc */
void proc_0x08048ffc(union { __size32 *; __size32; } param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6)
{
    *(__size32*)param1 = param2;
    *(__size32*)(param1 + 4) = param3;
    *(__size32*)(param1 + 8) = param4;
    *(__size32*)(param1 + 12) = param5;
    *(__size32*)(param1 + 16) = param6;
    return;
}

