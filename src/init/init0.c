void OSResetSystem(int,int,int);

void __check_pad3(void){
    unsigned short *t = (unsigned short *)0x800030E4;
    if((*t & 0xEEF) != 0xEEF){
        return;
    }

    OSResetSystem(0x0, 0x0, 0x0);
}

extern unsigned char Debug_BBA;

void __set_debug_bba(void) {
    Debug_BBA = 1;
}

unsigned char __get_debug_bba(void) {
    return Debug_BBA;
}