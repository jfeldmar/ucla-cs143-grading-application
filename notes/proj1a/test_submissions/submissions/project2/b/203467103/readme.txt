Matthew Pham
203467103
mattvpham@gmail.com

LEAF NODE STRUCTURE
|| RID, KEY | RID, KEY | ... | PADDING | TYPE | NUMKEYS | NEXTNODE ||

NONLEAF NODE STRUCTURE
|| KEY, PID | KEY, PID | ... | PADDING | TYPE | NUMKEYS | PREVNODE ||

I may change the leaf node struct to be <key, rid> pairs instead of
<rid, key> pairs.

There are a couple new functions introduced for debugging purposes:
setDebug, toggleDebug, debugOn, debugOff
get_buffer, clear_buffer, print_buffer
They should be self-explanatory.

Sendie, Tarun, and Seung-Ho provided help understanding the spec,
but they did not provide any programming assistance.

