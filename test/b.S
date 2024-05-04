.section .text,"ax"

movk x0,0,lsl 0
movk x0,0,lsl 16
movk x0,0,lsl 32
movk x0,0,lsl 48
movn x0,0,lsl 0
movn x0,0,lsl 16
movn x0,0,lsl 32
movn x0,0,lsl 48
movz x0,0,lsl 0
movz x0,0,lsl 16
movz x0,0,lsl 32
movz x0,0,lsl 48
movk w1,0xFFFF
movk w1,0xFFFF,lsl 16
movk x1,0xFFFF,lsl 32
movk x1,0xFFFF,lsl 48
movn w1,0xFFFF
movn w1,0xFFFF,lsl 16
movz w1,0xffff
movz w1,0xffff,lsl 0
movz w1,0xffff,lsl 16
movz x1,0xffff,lsl 32
movz x1,0xffff,lsl 48
add	sp,sp,0x123
add	sp,x2,0x123
add	w1,w2,0x123
add	w4,w8,0x123,lsl 12
add	wsp,w2,0x123
add	wsp,wsp,0x123
add	x1,x2,0x123
add	x1,x2,0xFFF
add	x1,x2,0xFFF,lsl 12
add	x1,sp,0x123
adds	w1,wsp,0x123
adds	w8,w16,0x123,lsl 12
adds	x8,x16,0x123,lsl 12
sub	sp,sp,0x123
sub	sp,x2,0x123
sub	w1,w2,0x123
sub	w4,w8,0x123,lsl 12
sub	wsp,w2,0x123
sub	wsp,wsp,0x123
sub	x1,x2,0x123
sub	x1,x2,0xFFF
sub	x1,x2,0xFFF,lsl 12
sub	x1,sp,0x123
subs	w1,wsp,0x123
subs	w8,w16,0x123,lsl 12
subs	x8,x16,0x123,lsl 12
and	w1,w2,0x0001FE00
and	w1,w2,w3
and	w1,w2,w3,asr 4
and	w1,w2,w3,lsl 4
and	w1,w2,w3,lsr 4
and	w1,w2,w3,ror 4
and	wzr,w2,w3,ror 4
and	x1,x2,0x0003FC0000000000
and	x1,x2,x3
and	x1,x2,x3,lsl 4
and	x1,x2,x3,ror 4
and	xzr,x2,x3,ror 4
ands	w1,w2,0xFFFFFFF1
ands	w1,w2,w3
ands	w1,w2,w3,lsl 4
ands	w1,w2,w3,ror 4
ands	x1,x2,0x5555555555555555
ands	x1,x2,x3
ands	x1,x2,x3,lsl 4
ands	x1,x2,x3,ror 4
