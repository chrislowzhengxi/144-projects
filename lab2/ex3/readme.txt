(1) 0x08048362 <+14>:    sub    $0x24,%esp
0x24 is 36 in decimals, so 36 bytes is reserved. 

(2) (gdb) p {char*}0x80495d0
$1 = 0x80484b8 "topsecret"

(3) b *0x0804837f

(4) 11. After using info registers we check the value of %eax.