int main(int argc, char *argv[]);
void proc_0x0804ce8c();
void proc_0x0804cd34();
void proc_0x0804cb98(char param1[]);
void proc_0x0804cb20();

__size32 global_0x0807c8c0;// 4 bytes
__size8 global_0x0807c8c4;// 1 bytes
__size32 global_0x0807c8e0;// 4 bytes

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
    __size32 eax; 		// r24
    union { GLclampf; __size32; } local0; 		// m[esp - 16]
    union { GLclampf; __size32; } local1; 		// m[esp - 20]
    union { GLclampf; __size32; } local2; 		// m[esp - 24]
    union { GLclampf; __size32; } local3; 		// m[esp - 28]
    union { GLuint; __size32; } local6; 		// m[esp - 24]

    local0 = *0x807a5ec;
    local1 = *0x807a5e8;
    local2 = *0x807a5e4;
    local3 = *0x807a5e0;
    glClearColor(local3, local2, local1, local0);
    glEnable(0xb71);
    glEnable(0xb60);
    glFogfv();
    glFogi();
    glFogf();
    glHint();
    glHint();
    glBlendFunc();
    proc_0x0804cd34();
    glHint();
    glCullFace();
    glEnable(0xb44);
    eax = gluNewQuadric();
    global_0x0807c8e0 = eax;
    gluQuadricCallback();
    gluQuadricNormals();
    memset(0x807a980, 0, 2000);
    global_0x0807c8c0 = 0;
    global_0x0807c8c4 = 0;
    glGenTextures(2, 0x807c8e4);
    local6 = *0x807c8e4;
    glBindTexture(0xde1, local6);
    eax = gluBuild2DMipmaps();
    if (eax != 0) {
        printf("Error in gluBuild2DMipmaps: %i\n", eax);
        proc_0x0804cb98(0x8059156);
    }
    glTexParameteri();
    glTexParameteri();
    local6 = *0x807c8e8;
    glBindTexture(0xde1, local6);
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
    __size32 ecx_1; 		// r25{1}
    __size32 ecx_10; 		// r25{25}
    __size32 ecx_11; 		// r25{31}
    __size32 ecx_13; 		// r25{33}
    __size32 ecx_14; 		// r25{39}
    __size32 ecx_16; 		// r25{41}
    __size32 ecx_17; 		// r25{47}
    __size32 ecx_19; 		// r25{49}
    __size32 ecx_2; 		// r25{7}
    __size32 ecx_4; 		// r25{9}
    __size32 ecx_5; 		// r25{15}
    __size32 ecx_7; 		// r25{17}
    __size32 ecx_8; 		// r25{23}
    __size32 *edi; 		// r31
    __size32 *esi; 		// r30
    int esp; 		// r28
    __size32 local10; 		// ecx_7{17}
    __size32 local11; 		// ecx_10{25}
    __size32 local12; 		// ecx_13{33}
    __size32 local13; 		// ecx_16{41}
    __size32 local8; 		// ecx_1{1}
    __size32 local9; 		// ecx_4{9}

    edi = (esp - 28);
    esi = 0x807a60c;
    ecx = 4;
    local8 = ecx;
    do {
        ecx_1 = local8;
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx_2 = ecx_1 - 1;
        local8 = ecx_2;
    } while (ecx_1 != 1);
    edi = (esp - 44);
    glLightModelfv();
    esi = 0x807a61c;
    ecx = 4;
    local9 = ecx;
    do {
        ecx_4 = local9;
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx_5 = ecx_4 - 1;
        local9 = ecx_5;
    } while (ecx_4 != 1);
    edi = (esp - 60);
    esi = 0x807a62c;
    ecx = 4;
    local10 = ecx;
    do {
        ecx_7 = local10;
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx_8 = ecx_7 - 1;
        local10 = ecx_8;
    } while (ecx_7 != 1);
    edi = (esp - 76);
    esi = 0x807a63c;
    ecx = 4;
    local11 = ecx;
    do {
        ecx_10 = local11;
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx_11 = ecx_10 - 1;
        local11 = ecx_11;
    } while (ecx_10 != 1);
    glEnable(0x4000);
    glLightfv();
    glLightfv();
    edi = (esp - 92);
    glLightfv();
    esi = 0x807a64c;
    ecx = 4;
    local12 = ecx;
    do {
        ecx_13 = local12;
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx_14 = ecx_13 - 1;
        local12 = ecx_14;
    } while (ecx_13 != 1);
    edi = (esp - 108);
    esi = 0x807a65c;
    ecx = 4;
    local13 = ecx;
    do {
        ecx_16 = local13;
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx_17 = ecx_16 - 1;
        local13 = ecx_17;
    } while (ecx_16 != 1);
    edi = (esp - 124);
    esi = 0x807a66c;
    ecx = 4;
    do {
        ecx_19 = ecx;
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx_19 - 1;
    } while (ecx_19 != 1);
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
void proc_0x0804cb98(char param1[])
{
    char *eax; 		// r24

    for(;;) {
        eax = glGetError();
        if (eax == 0) {
            break;
        }
        eax = gluErrorString(eax);
        printf("%s: %s\n", param1, eax);
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

