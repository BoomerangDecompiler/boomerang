#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/hppa/simple.m"
#include "global.h"
#include "decoder.h"

extern char _assembly[];
char* astr;

DWord getDword (unsigned lc)
/* get4Bytes - returns next 4-Byte from image pointed to by lc.
   Fetch in a big-endian manner  */
{
    return
      (DWord)
      ((((((
          *(Byte *)lc << 8
      ) + *(Byte *)(lc+1)) << 8
      ) + *(Byte *)(lc+2)) << 8
      ) + *(Byte *)(lc+3));
}


void NJMCDecoder::dis_faddr(ADDRESS faddr)
{


#line 23 "machine/hppa/simple.m"
{ 
  dword MATCH_p = 
    
    #line 23 "machine/hppa/simple.m"
    faddr
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ == 1) { 
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned d = 
        (sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 1) << 4) + 
        (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      
      #line 29 "machine/hppa/simple.m"
       {

                  astr += sprintf(astr, " %d(sr%d,r%d)", d, s, b);

                  strcat(constrName, "sdisps_faddr ");

              }

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned x = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;
      
      #line 25 "machine/hppa/simple.m"
       {

                  astr += sprintf(astr, " r%d(sr%d,r%d)", x, s, b);

                  strcat(constrName, "index_faddr ");

              }

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_d; 
  
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}

#line 34 "machine/hppa/simple.m"
}

void NJMCDecoder::dis_c_faddr(ADDRESS c_faddr)
{


#line 37 "machine/hppa/simple.m"
{ 
  dword MATCH_p = 
    
    #line 37 "machine/hppa/simple.m"
    c_faddr
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
      if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) 
        if ((MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1) 
          assert(0);  /* no match */ /*opt-block+*/
        else 
          
          #line 57 "machine/hppa/simple.m"
           {

                      astr += sprintf(astr, ",ma");

                      strcat(constrName, "ins_faddr_ma ");

                  }

          
           /*opt-block+*/ /*opt-block+*/
      else 
        if ((MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1) 
          
          #line 53 "machine/hppa/simple.m"
           {

                      astr += sprintf(astr, ",mb");

                      strcat(constrName, "ins_faddr_mb ");

                  }

          
           /*opt-block+*/
        else 
          
          #line 61 "machine/hppa/simple.m"
           {

                      strcat(constrName, "ins_faddr_si ");

                  }

          
           /*opt-block+*/ /*opt-block+*/ 
    else 
      if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) 
        if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) 
          
          #line 47 "machine/hppa/simple.m"
           {

                      astr += sprintf(astr, ",sm");

                      strcat(constrName, "ins_faddr_sm ");

                  }

          
           /*opt-block+*/
        else 
          
          #line 43 "machine/hppa/simple.m"
           {

                      astr += sprintf(astr, ",m");

                      strcat(constrName, "ins_faddr_m ");

                  }

          
           /*opt-block+*/ /*opt-block+*/
      else 
        if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) 
          
          #line 39 "machine/hppa/simple.m"
           {

                      astr += sprintf(astr, ",s");

                      strcat(constrName, "ins_faddr_s ");

                  }

          
           /*opt-block+*/
        else 
          
          #line 50 "machine/hppa/simple.m"
          

                      strcat(constrName, "ins_faddr_x ");

          
           /*opt-block+*/ /*opt-block+*/ 
    
  }goto MATCH_finished_c; 
  
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 65 "machine/hppa/simple.m"
}

void NJMCDecoder::dis_flt_fmt(int fmt)
{
    // Completer for floating point operand size
    switch(fmt) {
        case 0: astr += sprintf(astr, ",sgl"); break;
        case 1: astr += sprintf(astr, ",dbl"); break;
        case 3: astr += sprintf(astr, ",quad"); break;
        default:astr += sprintf(astr, ",?"); break;
    }
}

void dis_flt_c3(int& d_fmt, int& d_r1, int& d_r2, int& d_t, ADDRESS con)
{


#line 79 "machine/hppa/simple.m"
{ 
  dword MATCH_p = 
    
    #line 79 "machine/hppa/simple.m"
    con
    ;
  char *MATCH_name;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 12 || 
      13 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 12 && 
      (0 <= (MATCH_w_32_0 >> 9 & 0x3) /* class_21 at 0 */ && 
      (MATCH_w_32_0 >> 9 & 0x3) /* class_21 at 0 */ < 3) || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 12 && 
      (MATCH_w_32_0 >> 9 & 0x3) /* class_21 at 0 */ == 3 && 
      (4 <= (MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */ < 8)) 
      goto MATCH_label_b0;  /*opt-block+*/
    else { 
      MATCH_name = "flt_c3.C"; 
      { 
        char *name = MATCH_name;
        unsigned fmt = (MATCH_w_32_0 >> 11 & 0x3) /* fmt_19 at 0 */;
        unsigned r1 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
        unsigned r2 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
        unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
        
        #line 81 "machine/hppa/simple.m"
        

                    d_fmt = fmt; d_r1 = r1; d_r2 = r2; d_t = t;

        printf("Name of typed constructor is %s\n", name);

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_b; 
  
  MATCH_label_b0: (void)0; /*placeholder for label*/ 
    
    #line 87 "machine/hppa/simple.m"
    
                printf("This should never happen\n");

                assert(0);

    
     
    goto MATCH_finished_b; 
    
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 91 "machine/hppa/simple.m"
}

int NJMCDecoder::decodeAssemblyInstruction (ADDRESS pc, int delta)
{
    char sCmplt[32];
    unsigned long r1,r2;
    sCmplt[0]='\0';
    ADDRESS hostPC = pc + delta;

    astr = _assembly + sprintf(_assembly, "%x: %08x  ", pc, *(unsigned*)hostPC);



#line 101 "machine/hppa/simple.m"
{ 
  dword MATCH_p = 
    
    #line 101 "machine/hppa/simple.m"
    hostPC
    ;
  char *MATCH_name;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (13 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64) 
      goto MATCH_label_a0;  /*opt-block+*/
    else 
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
        case 8: case 10: 
          goto MATCH_label_a0; break;
        case 9: 
          if (0 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid_23 at 0 */ && 
            (MATCH_w_32_0 >> 7 & 0x3) /* uid_23 at 0 */ < 2) 
            if ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ == 1) 
              if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
                if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
                  (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                  (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0) { 
                  MATCH_name = "fstws"; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
              else { 
                MATCH_name = "fstwx"; 
                goto MATCH_label_a2; 
                
              } /*opt-block*/ 
            else 
              if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
                if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
                  (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                  (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0) { 
                  MATCH_name = "fldws"; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
              else { 
                MATCH_name = "fldwx"; 
                goto MATCH_label_a1; 
                
              } /*opt-block*/  
          else 
            goto MATCH_label_a0;  /*opt-block+*/
          break;
        case 11: 
          if (0 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid_23 at 0 */ && 
            (MATCH_w_32_0 >> 7 & 0x3) /* uid_23 at 0 */ < 2) 
            if ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ == 1) 
              if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
                if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
                  (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                  (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0) { 
                  MATCH_name = "fstds"; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
              else { 
                MATCH_name = "fstdx"; 
                goto MATCH_label_a2; 
                
              } /*opt-block*/ 
            else 
              if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
                if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
                  (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                  (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0) { 
                  MATCH_name = "fldds"; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/ /*opt-block+*/
              else { 
                MATCH_name = "flddx"; 
                goto MATCH_label_a1; 
                
              } /*opt-block*/  
          else 
            goto MATCH_label_a0;  /*opt-block+*/
          break;
        case 12: 
          if (0 <= (MATCH_w_32_0 >> 9 & 0x3) /* class_21 at 0 */ && 
            (MATCH_w_32_0 >> 9 & 0x3) /* class_21 at 0 */ < 3 || 
            (MATCH_w_32_0 >> 9 & 0x3) /* class_21 at 0 */ == 3 && 
            (4 <= (MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */ && 
            (MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */ < 8)) 
            goto MATCH_label_a0;  /*opt-block+*/
          else { 
            MATCH_name = "flt_c3"; 
            { 
              char *name = MATCH_name;
              unsigned con = addressToPC(MATCH_p);
              
              #line 119 "machine/hppa/simple.m"
               {

                          int fmt, r1, r2, t;

                          dis_flt_c3(fmt, r1, r2, t, con);

                          astr += sprintf(astr, "%s", name);

                          dis_flt_fmt(fmt);

                          astr += sprintf(astr, "  fr%d, fr%d, fr%d", r1, r2, t);

                          strcat(constrName, "flt_c3 ");

                      }

              
              
              
            }
            
          } /*opt-block*/
          
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    
    #line 127 "machine/hppa/simple.m"
    
                // Do nothing

                sprintf(astr, ".");

    

    
     
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned c_faddr = addressToPC(MATCH_p);
      unsigned faddr = addressToPC(MATCH_p);
      unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      
      #line 103 "machine/hppa/simple.m"
       {

      // Floating point loads and stores

                  astr += sprintf(astr, "%s  ", name);

                  dis_c_faddr(c_faddr);

                  dis_faddr(faddr);

                  astr += sprintf(astr, ",fr%d", t_27);

                  strcat(constrName, "floads ");

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a2: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned c_faddr = addressToPC(MATCH_p);
      unsigned faddr = addressToPC(MATCH_p);
      unsigned r = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      
      #line 111 "machine/hppa/simple.m"
       {

                  astr += sprintf(astr, "%s", name);

                  dis_c_faddr(c_faddr);

                  astr += sprintf(astr, "  fr%d,", r);

                  dis_faddr(faddr);

                  strcat(constrName, "fstores ");

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 132 "machine/hppa/simple.m"

    return 4;
}


