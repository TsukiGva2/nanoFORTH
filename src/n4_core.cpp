/**
 * @file
 * @brief nanoForth Core Utilities
 *        - low level memory and IO management
 */
#include "n4_core.h"

#if ARDUINO
DU freemem() {
    extern U8 *__flp;
    DU bsz = (DU)((uintptr_t)&bsz - (uintptr_t)&__flp);
    return bsz;
}
#else
int  Serial;           				     ///< fake serial interface
#endif // !ARDUINO

namespace N4Core {
///
///@name MMU controls
///@{
U8      dic[N4_RAM_SZ];                        ///< base of dictionary
N4Task  vm     { NULL, NULL };                 ///< VM context
///@}
///@name IO controls
///@{
Stream  *io    { &Serial };                    ///< default to Arduino Serial Monitor
U8      trc    { 0 };                          ///< tracing control flag
char    *_pre  { NULL };                       ///< preload Forth code
U8      *_tib  { &dic[N4_DIC_SZ+N4_STK_SZ] };  ///< base of terminal input buffer
U8      _empty { 1 };                          ///< empty flag for terminal input buffer
U8      _hex   { 0 };                          ///< numeric radix for display
U8      _ucase { 0 };                          ///< empty flag for terminal input buffer
///@}
///
void set_pre(const char *code) { _pre = (char*)code; }
void set_io(Stream *s)  { io   = s; }          ///< initialize or redirect IO stream
void set_hex(U8 f)      { _hex = f; }          ///< enable/disable hex numeric radix
void set_ucase(U8 uc)   { _ucase = uc; }       ///< set case sensitiveness
char uc(char c)      {                         ///< upper case for case-insensitive matching
    return (_ucase && (c>='A')) ? c&0x5f : c;
}
///
///> show system memory allocation info
///
void mstat()
{
    show(APP_NAME);
    log("[DIC=$");    logx(N4_DIC_SZ);         ///> dictionary size
    log("|SS,RS=$");  logx(N4_STK_SZ);         ///> stack size
    log("|TIB=$");    logx(N4_TIB_SZ);         ///> terminal input buf
    log("] total=$"); logx(N4_RAM_SZ);         /// * total forth memory block
#if ARDUINO
    log(", auto=$");   logx(freemem());  log("\n");
#endif // ARDUINO
}
///@}
///@name Console IO Functions with Cooperative Threading support
///@{
#if ARDUINO
#include <avr/pgmspace.h>
///
///> char IO from console i.e. RX/TX
///
char key()
{
    while (!io->available()) NanoForth::yield();
    return io->read();
}
void d_chr(char c)     {
    io->print(c);
    if (c=='\n') {
        io->flush();
        NanoForth::yield();
    }
}
void d_adr(IU a)         { d_nib(a>>8); d_nib((a>>4)&0xf); d_nib(a&0xf); }
void d_ptr(U8 *p)        { IU a=(IU)p; d_chr('p'); d_adr(a); }
void d_num(DU  n)        { _hex ? io->print(n&0xffff,HEX) : io->print(n); }
void d_pin(U16 p, U16 v) { pinMode(p, v); }
U16  d_in(U16 p)         { return digitalRead(p); }
void d_out(U16 p, U16 v) {
    switch (p & 0x300) {
    case 0x100:            // PORTD (0~7)
        DDRD  = DDRD | (p & 0xfc);  /// * mask out RX,TX
        PORTD = (U8)(v & p) | (PORTD & ~p);
        break;
    case 0x200:            // PORTB (8~13)
        DDRB  = DDRB | (p & 0xff);
        PORTB = (U8)(v & p) | (PORTB & ~p);
        break;
    case 0x300:            // PORTC (A0~A6)
        DDRC  = DDRC | (p & 0xff);
        PORTC = (U8)(v & p) | (PORTC & ~p);
        break;
    default: digitalWrite(p, v);
    }
}
U16  a_in(U16 p)         { return analogRead(p); }
void a_out(U16 p, U16 v) { analogWrite(p, v); }
#else
char key()               { return getchar();  }
void d_chr(char c)       { printf("%c", c);   }
void d_adr(IU a)         { printf("%03x", a); }
void d_ptr(U8 *p)        { printf("%p", p);   }
void d_num(DU n)         { printf(_hex ? "%x" : "%d", _hex ? (U16)n : n); }
void d_pin(U16 p, U16 v) { /* do nothing */ }
U16  d_in(U16 p)         { return 0; }
void d_out(U16 p, U16 v) { /* do nothing */ }
U16  a_in(U16 p)         { return 0; }
void a_out(U16 p, U16 v) { /* do nothing */ }
#endif //ARDUINO
void d_str(U8 *p)        { for (int i=0, sz=*p++; i<sz; i++) d_chr(*p++); }
void d_nib(U8 n)         { d_chr((n) + ((n)>9 ? 'a'-10 : '0')); }
void d_u8(U8 c)          { d_nib(c>>4); d_nib(c&0xf); }
///@}
///
///> dump byte-stream between pointers with delimiter option
///
void d_mem(U8* base, U8 *p0, IU sz, U8 delim)
{
    d_adr((IU)(p0 - base)); d_chr(':');
    for (int n=0; n<sz; n++) {
        if (delim && (n&0x3)==0) d_chr(delim);
        d_u8(*p0++);
    }
    if (delim) d_chr(delim);
}
///
///> display the opcode name
///
void d_name(U8 *p) { d_chr(*p); d_chr(*(p+1)); d_chr(*(p+2)); }
void d_name(U8 op, const char *lst, U8 space)
{
#if ARDUINO
    PGM_P p = reinterpret_cast<PGM_P>(lst)+1+op*3;
#else
    U8 *p = (U8*)lst+1+op*3;
#endif //ARDUINO
    char  c;
    d_chr(pgm_read_byte(p));
    if ((c=pgm_read_byte(p+1))!=' ' || space) d_chr(c);
    if ((c=pgm_read_byte(p+2))!=' ' || space) d_chr(c);
}
///
///> parse a literal from string
///
U8 number(U8 *str, DU *num)
{
    DU  n   = 0;
    U8  dg  = 0;                                          /// * digits
    U8  c   = *str;
    U8  neg = (c=='-') ? (c=*++str, 1)  : 0;              /// * handle negative sign
    U8  base= c=='$' ? (str++, 16) : (_hex ? 16 : 10);    /// * handle hex number

    while ((c=*str++) >= '0') {
        n *= base;
        if (base==10 && c > '9') return 0;
        if (c <= '9') n += c - '0';
        else {
            c &= 0x5f;
            if (c < 'A' || c > 'F') return 0;
            n += c - 'A' + 10;
        }
        dg++;
    }
    return dg ? (*num = neg ? -n : n, dg) : 0;
}
///
///> clear terminal input buffer
///
void clear_tib() {
    get_token(1);                            ///> empty the static tib inside #get_token
}
///
///> fill input buffer from console char-by-char til CR or LF hit
///
char vkey() {
	static char *p = _pre;                   /// capture preload Forth code
#if ARDUINO
    char c = p ? pgm_read_byte(p) : 0;
#else
    char c = *p;
#endif // ARDUINO
	return c ? (p++, c) : key();             /// feed key() after preload exhausted
}

void _console_input()
{
    U8 *p = _tib;
    d_chr('\n');
    for (;;) {
        char c = vkey();                     /// * get one char from input stream
        if (c=='\r' || c=='\n') {            /// * split on RETURN
            if (p > _tib) {
                *p     = ' ';                /// * pad extra space (in case word is 1-char)
                *(p+1) = 0;                  /// * terminate input string
                break;                       /// * skip empty token
            }
        }
        else if (c=='\b' && p > _tib) {      /// * backspace
            *(--p) = ' ';
            d_chr(' ');
            d_chr('\b');
        }
        else if (p >= &_tib[N4_TIB_SZ]) { /// * prevent buffer overrun (into auto vars)
            show("TIB!\n");
            *p = 0;
            break;
        }
        else *p++ = c;
    }
    _empty = (p==_tib);
}
///
///> display OK prompt if input buffer is empty
///
U8 ok()
{
	if (_empty) {
		///
		///> console prompt with stack dump
        /*        
         *                             SP0 (sp max to protect overwritten of vm object)
         * mem[...dic_sz...|...stk_sz...|......heap......]max
         *    |            |            |                |
         *    dic-->       +-->rp  sp<--+-->tib   auto<--+
         *                         TOS NOS
         */
		DU  *sp0 = (DU*)_tib;          /// * fetch top of heap
		DU  *rp1 = (DU*)(vm.rp+1);
	    if (vm.sp <= rp1) {            /// * check stack overflow
	        show("OVF!\n");
	        vm.sp = rp1;               /// * stack max out
	    }
	    for (DU *p=sp0-1; p >= vm.sp; p--) { /// * dump stack content
	        d_num(*p); d_chr('_');
	    }
	    show("ok");                          /// * user input prompt
	}
    return _empty;
}
///
///> capture a token from console input buffer
///
U8 *get_token(U8 rst)
{
    static U8 *tp = _tib;                    ///> token pointer to input buffer
    static U8 dq  = 0;                       ///> dot_string flag

    if (rst) { tp = _tib; _empty = 1; return 0; }  /// * reset TIB for new input
    while (_empty || *tp==0 || *tp=='\\') {
        _console_input();                    ///>  read from console (with trailing blank)
        while (*tp==' ') tp++;               ///>  skip leading spaces
    }
    if (!dq) {
        while (*tp=='(' && *(tp+1)==' ') {   /// * handle ( ...) comment, TODO: multi-line
            while (*tp && *tp++!=')');       ///> find the end of comment
            while (*tp==' ') tp++;           ///> skip trailing spaces
        }
    }
    U8 *p = (U8*)tp;
    U8 cx = dq ? '"' : ' ';                  /// * set delimiter
    U8 sz = 0;
    while (*tp && *tp!='(' && *tp++!=cx) sz++;/// * count token length
    if (trc) {                               /// * optionally print token for debugging
        d_chr('\n');
        for (int i=0; i<5; i++) {
            d_chr(i<sz ? (*(p+i)<0x20 ? '_' : *(p+i)) : ' ');
        }
    }
    while (*tp==' ') tp++;                   /// * skip spaces, advance pointer to next token
    if (*tp==0 || *tp=='\\') { tp = _tib; _empty = 1; }

    dq  = (*p=='.' && *(p+1)=='"');          /// * flag token was dot_string

    return p;                                /// * return pointer to token
}
///
///> search keyword in a nanoForth name field list
///  * one blank byte padded at the end of input string
///
U8 scan(U8 *tkn, const char *lst, IU *id)
{
    for (int n=1, m=pgm_read_byte(lst); n < m*3; n+=3) {
        if (uc(tkn[0])==pgm_read_byte(lst+n)   &&
            uc(tkn[1])==pgm_read_byte(lst+n+1) &&
            (tkn[1]==' ' || uc(tkn[2])==pgm_read_byte(lst+n+2))) {
            *id = (IU)(n/3);  // 3-char a word
            return 1;
        }
    }
    return 0;
}

} // namespace N4Core
