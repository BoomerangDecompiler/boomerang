__size32 global68_807a5e4 = 0x3f000000;// 4 bytes
__size32 global69_807a5e0 = 0;// 4 bytes
__size32 global39_807c8e4;// 4 bytes
__size32 global91_807c8c0;// 4 bytes
__size32 global53_807c8e8;// 4 bytes
__size32 global66_807a5ec = 0x3f800000;// 4 bytes
__size32 global85_807c8e0;// 4 bytes
__size32 global75_807a5f0 = 0x3bc49ba6;// 4 bytes
int global92_807c8c4;
__size32 global67_807a5e8 = 0x3f333333;// 4 bytes
__size32 global68_807a5e4 = 0x3f000000;// 4 bytes
__size32 global69_807a5e0 = 0;// 4 bytes
__size32 global39_807c8e4;// 4 bytes
__size32 global91_807c8c0;// 4 bytes
__size32 global53_807c8e8;// 4 bytes
__size32 global66_807a5ec = 0x3f800000;// 4 bytes
__size32 global85_807c8e0;// 4 bytes
__size32 global75_807a5f0 = 0x3bc49ba6;// 4 bytes
int global92_807c8c4;
__size32 global67_807a5e8 = 0x3f333333;// 4 bytes
int main(int argc, char *argv[]);
void proc_0x0804ce8c();
void proc_0x0804cd34();
void proc_0x0804cb98(union { char[] *; __size32; } param1);
void proc_0x0804cb20();

/** address: 0x0804dd8c */
int main(int argc, char *argv[])
{
    int *local0; 		// m[esp - 28]

    glutInit(&argc, argv);
    glutInitDisplayMode(18);
    local0 = *argv;
    glutCreateWindow(local0);
    proc_0x0804ce8c();
    glutDisplayFunc(0x804d640);
    glutKeyboardFunc(0x804d884);
    glutSpecialFunc(0x804db14);
    glutReshapeFunc(0x804d758);
    glutIdleFunc();
    glutFullScreen();
    glutMainLoop();
    return 0;
}

/** address: 0x0804ce8c */
void proc_0x0804ce8c()
{
    void *eax; 		// r24

    *(__size32*)(r[28] - 4) = r[29];
    *(__size32*)(r[28] - 4) = global66_807a5ec;
    *(__size32*)(r[28] - 4) = global67_807a5e8;
    *(__size32*)(r[28] - 4) = global68_807a5e4;
    *(__size32*)(r[28] - 4) = global69_807a5e0;
    glClearColor(*(r[28] + 4), *(r[28] + 8), *(r[28] + 12), *(r[28] + 16));
    *(__size32*)r[28] = 0xb71;
    glEnable(*(r[28] + 4));
    *(__size32*)r[28] = 0xb60;
    glEnable(*(r[28] + 4));
    *(__size32*)(r[28] - 4) = 0x807a5e0;
    *(__size32*)(r[28] - 4) = 0xb66;
    glFogfv();
    *(__size32*)(r[28] - 4) = 0x800;
    *(__size32*)(r[28] - 4) = 0xb65;
    glFogi();
    *(__size32*)(r[28] - 4) = global75_807a5f0;
    *(__size32*)(r[28] - 4) = 0xb62;
    glFogf();
    *(__size32*)(r[28] - 4) = 0x1102;
    *(__size32*)(r[28] - 4) = 0xc54;
    glHint();
    *(__size32*)(r[28] - 4) = 0x1102;
    *(__size32*)(r[28] - 4) = 0xc52;
    glHint();
    *(__size32*)(r[28] - 4) = 771;
    *(__size32*)(r[28] - 4) = 770;
    glBlendFunc();
    proc_0x0804cd34();
    *(__size32*)(r[28] - 4) = 0x1102;
    *(__size32*)(r[28] - 4) = 0xc50;
    glHint();
    *(__size32*)r[28] = 1029;
    glCullFace();
    *(__size32*)r[28] = 0xb44;
    glEnable(*(r[28] + 4));
    gluNewQuadric();
    global85_807c8e0 = r[24];
    *(__size32*)(r[28] - 4) = 0x804cbd0;
    *(__size32*)(r[28] - 4) = 0x18707;
    *(__size32*)(r[28] - 4) = r[24];
    gluQuadricCallback();
    *(__size32*)(r[28] - 4) = 0x186a0;
    *(__size32*)(r[28] - 4) = global85_807c8e0;
    gluQuadricNormals();
    *(__size32*)(r[28] - 4) = 2000;
    *(int*)(r[28] - 4) = 0;
    *(__size32*)(r[28] - 4) = 0x807a980;
    memset(*(r[28] + 4), *(r[28] + 8), *(r[28] + 12));
    global91_807c8c0 = 0;
    global92_807c8c4 = 0;
    glGenTextures(2, 0x807c8e4);
    glBindTexture(0xde1, global39_807c8e4);
    eax = gluBuild2DMipmaps();
    if (eax != 0) {
        printf("Error in gluBuild2DMipmaps: %i\n", eax);
        proc_0x0804cb98(0x8059156);
    }
    glTexParameteri();
    glTexParameteri();
    glBindTexture(0xde1, global53_807c8e8);
    proc_0x0804cb20();
    eax = gluBuild2DMipmaps();
    if (eax != 0) {
        printf("Error in gluBuild2DMipmaps: %i\n", eax);
        proc_0x0804cb98(0x8059156);
    }
    glTexParameteri();
    glTexParameteri();
    glTexEnvf();
    return;
}

/** address: 0x0804cd34 */
void proc_0x0804cd34()
{
    int ecx; 		// r25
    __size32 edi; 		// r31
    __size32 esi; 		// r30
    int esp; 		// r28

    edi = (esp - 28);
    esi = 0x807a60c;
    ecx = 4;
    *(__size32*)edi = *esi;
    esi +=  (DF == 0) ? 4 : -4;
    edi +=  (DF == 0) ? 4 : -4;
    ecx = ecx - 1;
    edi = (esp - 44);
    glLightModelfv();
    esi = 0x807a61c;
    ecx = 4;
    *(__size32*)edi = *esi;
    esi +=  (DF == 0) ? 4 : -4;
    edi +=  (DF == 0) ? 4 : -4;
    ecx = ecx - 1;
    glEnable(0x4000);
    glLightfv();
    glLightfv();
    edi = (esp - 92);
    glLightfv();
    esi = 0x807a64c;
    ecx = 4;
    *(__size32*)edi = *esi;
    esi +=  (DF == 0) ? 4 : -4;
    edi +=  (DF == 0) ? 4 : -4;
    ecx = ecx - 1;
    glEnable(0x4001);
    glLightfv();
    glLightfv();
    glLightfv();
    glLightf();
    glEnable(0xb50);
    glLightModeli();
    return;
}

/** address: 0x0804cb98 */
void proc_0x0804cb98(union { char[] *; __size32; } param1)
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

/** address: 0x0804cb20 */
void proc_0x0804cb20()
{
    memcpy(0x80625cd, 0x804dfa0, 0x8000);
    memcpy(0x806a5cd, 0x80725e0, 0x7fff);
    return;
}

