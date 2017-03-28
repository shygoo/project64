# Project64d
A fork of Project64 with additional debugging features

![Image](http://i.imgur.com/KJhd94F.png)

#### Features:
* MIPS debugger
    * Built-in assembler functionality for testing code
* Memory read and write breakpoints
    * Support for cart-to-RAM DMA writes
* Symbol management for subroutines and variables
    * Live view of variable values
* Cart-to-RAM DMA logging
    * RAM-ROM address conversion
	* 4-byte ASCII signature detection
* Javascript API
    * CPU read/write/execute event hooking
	* Access to virtual memory
	    * Variable and object binding
	* Access to cartridge memory
	* Access to general purpose and floating point registers
	* Basic socket and server support
	* Console logging & input evaluation
* Static RDRAM allocation
    * Memory base is always 0x10000000 for programs like Cheat Engine
	
#### Discussion & support thread:
* http://origami64.net/showthread.php?tid=549

#### Video demos:
* [Debugger Example](https://www.youtube.com/watch?v=UgHuGIKO9hs)
* [Javascript Event Hooking](https://www.youtube.com/watch?v=PC0Tlz6oiN0)
* [Javascript Socket Example](https://www.youtube.com/watch?v=vG24uvidU_Y)