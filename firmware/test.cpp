#include <iostream>

main()
{
    char c = getchar();
    printf("%d", c);

    uint32_t p1 = 0b10000000000000000000000000000000;
    void* ptr = &p1;
    uint8_t* ptri = (uint8_t*)ptr+3; //alebo +0,+1,+2
    uint8_t a = *ptri;
    printf("val: %d \n", a); //0,0,0,32
}

