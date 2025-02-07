I created a breakpoint at main. 
I used "until 19" to get to the end of the for loop, firstly using next. 
I realised that data becomes data = "Xfmm-!nbzc" 

(gdb) until 28
main (argc=0, argv=0xffffc5f4) at main.c:30
30        return 0;
(gdb) info locals
offset = 7
i = 13
data = "Xfmm-!nbzc"
stringToPrint = "Some additional text."
data3 = "A final bit of text."
data4 = "Well, m\000\321\000\000`\000\000\000\000\321\000\000\000\000\217``
\000c\000\331\000\000\000\000\321\067\000\000\000\000\000\000\000!\217\000`\000`
a\000\305c\377\000", <incomplete sequence \331>

