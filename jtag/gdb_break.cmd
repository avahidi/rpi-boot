
#target extended-remote :3333
target remote :3333

monitor halt
break

info r
x/8i $pc
disp/i $pc
