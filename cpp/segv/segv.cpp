#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int __libc_argc;
char **__libc_argv;

void segmentation_fault()
{
	char *s = "LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so report "
			"information about Segmentation fault\n"
			"useful tools: readelf objdump addr2line \n"
			"(gdb) info symbol 0x4007f1 \n"
			"(gdb) info line * 0x4007f1 \n"
			"(gdb) list * 0x4007f1 \n";
	puts(s);
	const char* preload = "LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so ";
	write(STDOUT_FILENO, preload, strlen(preload));
	write(STDOUT_FILENO, __libc_argv[0], strlen(__libc_argv[0]));
	puts("");
	*s = 'F'; // trigger Segmentation fault
}

void double_free()
{
	char* buf = new char[1024];
	strcpy(buf, "double free is not Segmentation fault, not handle by libSegFault.so");
	delete[] buf;
	char* buf2 = new char[512];
	delete buf;
	delete buf2;
}

int main(int argc, char* argv[]) {
	int mode = 0;

	if (getenv("LD_PRELOAD")==nullptr)
	{
		printf("missing LD_PRELOAD environment variable\n");
		printf("usage: LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so %s\n", argv[0]);
		return 1;
	}

	if (strstr(getenv("LD_PRELOAD"), "SegFault")==nullptr)
	{
		printf("missing SegFault in LD_PRELOAD environment variable\n");
		printf("usage: LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so %s\n", argv[0]);
		return 1;
	}

	if(argc>1)
		mode = atoi(argv[1]);

	if (mode) {
		double_free();
	}
	else
		segmentation_fault();
	return 0;
}

/*
onzhang@work2017-VirtualBox:~/oss/toybox/cpp$ LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so segv/segv 0
LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so report information about Segmentation fault
useful tools: readelf objdump addr2line
(gdb) info symbol 0x0x40054d
(gdb) info line * 0x0x40054d
(gdb) list * 0x0x40054d

LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so *** Segmentation fault
Register dump:

 RAX: 0000000000000000   RBX: 0000000000000000   RCX: 00007f151194a6e0
 RDX: 0000000000000030   RSI: 0000000000400ab8   RDI: 0000000000000001
 RBP: 00007ffe840114e0   R8 : 0000000000000000   R9 : 1999999999999999
 R10: 000000000000086f   R11: 0000000000000246   R12: 00000000004006b0
 R13: 00007ffe840115f0   R14: 0000000000000000   R15: 0000000000000000
 RSP: 00007ffe840114d0

 RIP: 00000000004007f1   EFLAGS: 00010213

 CS: 0033   FS: 0000   GS: 0000

 Trap: 0000000e   Error: 00000004   OldMask: 00000000   CR2: 00000000

 FPUCW: 0000037f   FPUSW: 00000000   TAG: 00000000
 RIP: 00000000   RDP: 00000000

 ST(0) 0000 0000000000000000   ST(1) 0000 0000000000000000
 ST(2) 0000 0000000000000000   ST(3) 0000 0000000000000000
 ST(4) 0000 0000000000000000   ST(5) 0000 0000000000000000
 ST(6) 0000 0000000000000000   ST(7) 0000 0000000000000000
 mxcsr: 1f80
 XMM0:  00000000000000000000000000000000 XMM1:  00000000000000000000000000000000
 XMM2:  00000000000000000000000000000000 XMM3:  00000000000000000000000000000000
 XMM4:  00000000000000000000000000000000 XMM5:  00000000000000000000000000000000
 XMM6:  00000000000000000000000000000000 XMM7:  00000000000000000000000000000000
 XMM8:  00000000000000000000000000000000 XMM9:  00000000000000000000000000000000
 XMM10: 00000000000000000000000000000000 XMM11: 00000000000000000000000000000000
 XMM12: 00000000000000000000000000000000 XMM13: 00000000000000000000000000000000
 XMM14: 00000000000000000000000000000000 XMM15: 00000000000000000000000000000000

Backtrace:
segv/segv[0x4007f1]
segv/segv[0x40093a]
/lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf0)[0x7f1511874830]
segv/segv[0x4006d9]

Memory map:

00400000-00401000 r-xp 00000000 08:01 37884167                           /home/onzhang/oss/toybox/cpp/segv/segv
00600000-00601000 r--p 00000000 08:01 37884167                           /home/onzhang/oss/toybox/cpp/segv/segv
00601000-00602000 rw-p 00001000 08:01 37884167                           /home/onzhang/oss/toybox/cpp/segv/segv
014fb000-0152d000 rw-p 00000000 00:00 0                                  [heap]
7f1511335000-7f151134b000 r-xp 00000000 08:01 11538763                   /lib/x86_64-linux-gnu/libgcc_s.so.1
7f151134b000-7f151154a000 ---p 00016000 08:01 11538763                   /lib/x86_64-linux-gnu/libgcc_s.so.1
7f151154a000-7f151154b000 rw-p 00015000 08:01 11538763                   /lib/x86_64-linux-gnu/libgcc_s.so.1
7f151154b000-7f1511653000 r-xp 00000000 08:01 11534433                   /lib/x86_64-linux-gnu/libm-2.23.so
7f1511653000-7f1511852000 ---p 00108000 08:01 11534433                   /lib/x86_64-linux-gnu/libm-2.23.so
7f1511852000-7f1511853000 r--p 00107000 08:01 11534433                   /lib/x86_64-linux-gnu/libm-2.23.so
7f1511853000-7f1511854000 rw-p 00108000 08:01 11534433                   /lib/x86_64-linux-gnu/libm-2.23.so
7f1511854000-7f1511a13000 r-xp 00000000 08:01 11535663                   /lib/x86_64-linux-gnu/libc-2.23.so
7f1511a13000-7f1511c13000 ---p 001bf000 08:01 11535663                   /lib/x86_64-linux-gnu/libc-2.23.so
7f1511c13000-7f1511c17000 r--p 001bf000 08:01 11535663                   /lib/x86_64-linux-gnu/libc-2.23.so
7f1511c17000-7f1511c19000 rw-p 001c3000 08:01 11535663                   /lib/x86_64-linux-gnu/libc-2.23.so
7f1511c19000-7f1511c1d000 rw-p 00000000 00:00 0
7f1511c1d000-7f1511d8f000 r-xp 00000000 08:01 64096385                   /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21
7f1511d8f000-7f1511f8f000 ---p 00172000 08:01 64096385                   /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21
7f1511f8f000-7f1511f99000 r--p 00172000 08:01 64096385                   /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21
7f1511f99000-7f1511f9b000 rw-p 0017c000 08:01 64096385                   /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21
7f1511f9b000-7f1511f9f000 rw-p 00000000 00:00 0
7f1511f9f000-7f1511fa3000 r-xp 00000000 08:01 11535666                   /lib/x86_64-linux-gnu/libSegFault.so
7f1511fa3000-7f15121a2000 ---p 00004000 08:01 11535666                   /lib/x86_64-linux-gnu/libSegFault.so
7f15121a2000-7f15121a3000 r--p 00003000 08:01 11535666                   /lib/x86_64-linux-gnu/libSegFault.so
7f15121a3000-7f15121a4000 rw-p 00004000 08:01 11535666                   /lib/x86_64-linux-gnu/libSegFault.so
7f15121a4000-7f15121ca000 r-xp 00000000 08:01 11535664                   /lib/x86_64-linux-gnu/ld-2.23.so
7f15123a7000-7f15123ac000 rw-p 00000000 00:00 0
7f15123c7000-7f15123c9000 rw-p 00000000 00:00 0
7f15123c9000-7f15123ca000 r--p 00025000 08:01 11535664                   /lib/x86_64-linux-gnu/ld-2.23.so
7f15123ca000-7f15123cb000 rw-p 00026000 08:01 11535664                   /lib/x86_64-linux-gnu/ld-2.23.so
7f15123cb000-7f15123cc000 rw-p 00000000 00:00 0
7ffe83ff2000-7ffe84013000 rw-p 00000000 00:00 0                          [stack]
7ffe841ea000-7ffe841ec000 r--p 00000000 00:00 0                          [vvar]
7ffe841ec000-7ffe841ee000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
Segmentation fault (core dumped)

onzhang@work2017-VirtualBox:~/oss/toybox/cpp/seg$ addr2line -e a.out 0x4004ed
/home/onzhang/oss/toybox/cpp/seg/seg.cpp:8

*/
