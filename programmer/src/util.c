
#include "defs.h"

#include <ctype.h>

void util_hexdump(char *msg, char *data, int len)
{
    int i, j, k;
    
    printf("%s (%d)\n", msg, len);
    
    for(i = 0; i < len; ) {
        printf("%08x: ", i);
        
        k = i;
        for(j = 0; j < 16 && i < len; j++)
            printf("%02x ", 0xFF & data[i++]);
        
        for(; j < 17; j++)
            printf("   ");
        
        for(j = 0; j < 16 && k < len; j++, k++)
            printf("%c", isprint(data[k]) ? data[k] : '.');        
        
        printf("\n");
    }
}


static uint8_t parse_hex_byte(char *str)
{
    uint8_t ret;
    int i;
    
    for(ret = 0, i = 0; i < 2; i++) {
        ret <<= 4;
        if(str[i] >= '0' && str[i] <= '9') ret |= (str[i] - '0');
        else if(str[i] >= 'a' && str[i] <= 'f') ret |= (str[i] - 'a' + 10);
        else if(str[i] >= 'A' && str[i] <= 'F') ret |= (str[i] - 'A' + 10);
        else printf("BAD hex: %c\n", str[i]);
        
    }    
    return ret;
}

/* convert text with embedded \xx hex to binary representation of that */
void util_recode(char *str, int *len)
{
    int i, j;
    
    for(j = 0, i = 0; i < *len; i++, j++) {
        if(str[i] == '\\') {
            str[j] = parse_hex_byte(str + i + 1);
            i += 2;
        } else {
            if(i != j)
                str[j] = str[i];
        }
    }
    
    *len = j;
}
