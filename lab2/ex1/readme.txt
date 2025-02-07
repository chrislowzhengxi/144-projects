Ex.1 
I set breakpoints break multiStrlen and break twoFingerSort to let it 
stop at the beginning of the functions. i used run, continue, print p, print q, 
print length, then step, next to determine what's wrong. 

For the first task, the check of the while loop is printed before we did 
anything for p and q. Therefore, it's incremented by 1. I used print p and 
print q after every step execution to see if the p is what I expected. Initially,
instead of the '\0' that would be expected, I got an empty string, indicating 
that something is wrong. 

