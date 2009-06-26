Tarun Solanki
703-477-208
Comments on BTreeNode
This part was very challenging because of the fact that we had to interface our class
with nonexiting methods. It was very challenging to think a suitable structure for the 
leaf and non leaf nodes and then implement them correctly. Testing was a pain too, because 
we had to move through the array manually and look at key, pointer pair.
The way i made my node is as follows.
For leaf, first 16 bytes hold information about the leaf. Its type, the number of items stored
in it, and its own pid and its Parents pid. this is followed by about 80 rid, key pairs.
For non leaf, first 16 bytes are reserved in the same way. However, there is one initial pointer
in the begining, follwed by the key, and pid pairs rather than other way round.
Insert and InsertandSplit were a little challenging to implement because of the large amount of
pointer arithmetic involved.