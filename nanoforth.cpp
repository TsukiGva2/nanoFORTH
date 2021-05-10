/// nanoFORTH - Forth for Arduino Nano (and UNO)
/**
 * 2004-07 T. NAKAGAWA (Tiny FORTH), original reference 
 * 2020-08 circuit4u@medium.com (Tiny Forth NXT), Arduino reference
 * 2021-03 CC Lee: modularize with comments [6698,1006]; add buffer handler[6924,1036]
 * 2021-04-02: CC - add multi-tasker, byte count[6782/1056],
 *    [6930,1056] add threads
 *    [6908,1014] use F()
 *    [6860,852]  use PROGMEM, pgm_read_byte in find()
 *    [6906,852]  getnum support negative number
 *    [7012,860]  add digitalR/W
 *  2021-04-03: CC
 *    [6898,852]  add vm_delay
 *    [6912,852]  update EEPROM IO
 *    [6842,852]  execution tracing enable/disable
 *    [6922,852]  add CELL (CEL), ALLOT (ALO) opcodes
 *  2021-04-16: CC
 *    [7446,932]  add list_word, use 80-byte console buffer
 *    [7396,932]  add ASM_TRACE from EXE_TRACE options
 *    [7676,802]  grow Task at end of heap
 *    [8266,802]  if use array instead of pointer arithmetics, revert!
 *  2021-05-03: CC
 *    [7818,1428] forget Task struct; grow MEM_SZ to DIC_SZ+STK_SZ (1K+64*2)
 *    [8254,1430] add execution tracing option
 *  2021-05-06: CC
 *    [9214,1424] refactor to C++ for Arduino lib
 *    [11750,280] add mem,stk sizing; VM and ASM objects dynamically created
 *    [11956,272] add struct n4_task and N4_FUNC multi-tasker
 */
#include "nanoforth_vm.h"
///
/// user function linked-list
///
static n4_tptr _n4tsk = 0;
static N4VM    *_n4vm  = new N4VM();
///
/// * initialize NanoForth's virtual machine and assembler
///
void NanoForth::begin(U16 mem_sz, U16 stk_sz)
{
    U8 *_mem = (U8*)malloc(mem_sz);                      // allocate heap

    _n4vm->init(_mem, mem_sz, stk_sz);                   // instanciate NanoForth VM
    _n4vm->info();
}
///
/// single step for Arduino loop
///
void NanoForth::add(void (*ufunc)(n4_tptr))
{
    n4_tptr tp = (n4_tptr)malloc(sizeof(n4_task));

    tp->func = ufunc;   // assign user function
    tp->ci   = 0;       // reset case index 
    tp->next = _n4tsk;  // push into linked-list
    _n4tsk   = tp;      // reset head
}
///
/// * n4 execute one virtual machine opcode
///
bool NanoForth::step()
{
    _n4vm->step();
    yield();
}
///
/// * n4 yield to hardware tasks
///
void NanoForth::yield()
{
    for (n4_tptr tp=_n4tsk; tp; tp=tp->next) {
        tp->func(tp);
    }
}
///
/// * console input with cooperative threading
///
char NanoForth::key()
{
    while (!Serial.available()) yield();
    return Serial.read();
}
///
/// * aka Arduino delay(), yield to hardware context while waiting
///
void NanoForth::wait(U32 ms)
{
    U32 t = millis() + ms;
    while (millis()<t) yield();
}
///
/// for Eclipse debugging
///
/*
int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);		// autoflush (turn STDOUT buffering off)
    
	NanoForth n4;
    while (1) {
        n4.run();
    }
	return 0;
}
*/


