Group Members:
Reed Villanueva
Conner H. 
Curtis F.


How to run program:
	make
	./net367 <topology file>

Two example topology files are included, they are called testNetwork-1.dat and testNetwork-2.dat.
testNetwork-1.dat is a star topology
testNetwork-2.dat is a tree topology


You can use the following command sequence to test the start topology:
	d 
   	f 
   	h 
   	m 
   	TestDir0 
   	c 
   	1 
   	m 
   	TestDir1
   	c
   	2
   	m
   	TestDir2
   	u
   	testmsg2
   	t
   	0
   	c
   	0
   	f
   	u
   	testmsg0
   	t
   	1
   	c
   	1
   	f
   	u
   	testmsg1
   	t
   	2
   	c
   	2
   	f
   	c
   	1
   	t
   	2
   	c
   	2
   	f
   	c
   	1
   	t
   	456
   	
This sequence sets up the directories for each host,
sends iniital messages between them all to set them up in the switching table,
and send a message to a non-valid address to test that the switch behaves as expected.
The behavior of the program can then be seen in the generated log file log.txt.
