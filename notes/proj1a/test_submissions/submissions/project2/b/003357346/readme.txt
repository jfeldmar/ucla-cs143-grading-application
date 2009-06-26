Readme file:

Albert Hsieh 	003357346
Bryan Chen	603506391

Parallel Programming

Max # keys per node: 83 (+1 temporary)
Max # ptrs per node: 84 (+1 temporary)

Our Node implementation (Leaf Node = 1024 bytes):

	
	[ keyCount ] 0[ key ][ sid ][ pid ]	1[ key ][ sid ][ pid ]	.... (temp) [ key ][ sid ][ pid ]  	[rptr]
#bytes	   4 bytes	     12 bytes			12 bytes        ....	  12 bytes			4 bytes
index	   0			4			  16		....	    1000		...	1020


Our Node implementation (NonLeaf Node = 1024 bytes):
	
	[ keyCount ] 0[ key1 ][ ][ lchild1 ]		1[ key2 ][ ][ rchild1/lchild2 ]	.... (temp) [ temp key ][ ][ rchild(n)/lchild(temp) ]  	[temp rchild]
#bytes	   4 bytes	     12 bytes			12 bytes        ....	  12 bytes			4 bytes
index	   0			4			  16		....	    1000		...	1020

We allocated a temporary entry space so that we can insert into the node and then split when overflowed. Thus our capacity is actually 84 keys per node, but we only allow the caller to utilize 83 of them. 

