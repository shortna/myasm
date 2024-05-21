.global _start
.section .text,"ax"

_start: // comment
  adr x0, label_int

  // literal
  ldr w0,label_int
  ldr x0,label_int
  ldrsw x0,label_int

  b label_b
  bl label_b
  b.eq label_b
  b.ne label_b
  b.cs label_b
  b.cc label_b
  b.mi label_b
  b.pl label_b
  b.vs label_b
  b.vc label_b
  b.hi label_b
  b.ls label_b
  b.ge label_b
  b.lt label_b
  b.gt label_b
  b.le label_b
  b.al label_b
  b.nv label_b

  blr x1
  br x1
  ret
  ret x1

  cbnz w1,label_cbnz
  cbnz x1,label_cbnz

  cbz w1,label_cbz
  cbz x1,label_cbz

  tbnz w1,2,label_tbnz
  tbnz x1,63,label_tbnz
  tbz w1,2,label_tbz
  tbz x1,63,label_tbz

label_b:
label_cbnz:
label_cbz:
label_tbnz:
label_tbz:

  // register shifted
  strb w0,[x2,x3,lsl 0]
  ldrb w0,[x2,x3,lsl 0]
  ldrsb w0,[x2,x3,lsl 0]

  strb w0,[x2,x3]
  ldrb w0,[x2,x3]
  ldrsb w0,[x2,x3]

  // register
  strb w0,[x2,w3,sxtw]
  strb w0,[x2,w3,sxtw 0]
  strb w0,[x2,w3,uxtw]
  strb w0,[x2,w3,uxtw 0]
  strb w0,[x2,x3,sxtx]
  strb w0,[x2,x3,sxtx 0]
  ldrb w0,[x2,w3,sxtw]
  ldrb w0,[x2,w3,sxtw 0]
  ldrb w0,[x2,w3,uxtw]
  ldrb w0,[x2,w3,uxtw 0]
  ldrb w0,[x2,x3,sxtx]
  ldrb w0,[x2,x3,sxtx 0]
  ldrsb w0,[x2,w3,sxtw]
  ldrsb w0,[x2,w3,sxtw 0]
  ldrsb w0,[x2,w3,uxtw]
  ldrsb w0,[x2,w3,uxtw 0]
  ldrsb w0,[x2,x3,sxtx]
  ldrsb w0,[x2,x3,sxtx 0]
  ldrsh w0,[x2,w3,sxtw]
  ldrsh w0,[x2,w3,sxtw 1]
  ldrsh w0,[x2,w3,uxtw]
  ldrsh w0,[x2,w3,uxtw 1]
  ldrsh w0,[x2,x3]
  ldrsh w0,[x2,x3,lsl 1]
  ldrsh w0,[x2,x3,sxtx]
  ldrsh w0,[x2,x3,sxtx 1]
  ldr w0,[x2,w3,sxtw]
  ldr w0,[x2,w3,sxtw 2]
  ldr w0,[x2,w3,uxtw]
  ldr w0,[x2,w3,uxtw 2]
  ldr w0,[x2,x3,sxtx]
  ldr w0,[x2,x3,sxtx 2]
  ldr w0,[x2,x3,lsl 2]
  ldr x0,[x2,w3,sxtw]
  ldr x0,[x2,w3,sxtw 3]
  ldr x0,[x2,w3,uxtw]
  ldr x0,[x2,w3,uxtw 3]
  ldr x0,[x2,x3,sxtx]
  ldr x0,[x2,x3,sxtx 3]
  ldr x0,[x2,x3,lsl 3]
  ldr x0,[x2,x3]
  ldr w0,[x2,x3]

  // post index
  ldr w0,[sp],+0x8
  ldr x0,[x2],+0x8
  ldr w0,[sp],-0x8
  ldr x0,[x2],-0x8
  ldr w0,[sp],0x8
  ldr x0,[x2],0x8
 ldrsb w0,[x2],0x8
  ldrsh w0,[x2],+0x8
  ldrsw x0,[x2],+0x8

  // pre index
  ldr w0,[sp,+0x8]!
  ldr x0,[x2,+0x8]!
  ldr w0,[sp,-0x8]!
  ldr x0,[x2,-0x8]!
  ldr w0,[sp,0x8]!
  ldr x0,[x2,0x8]!
  ldrsb w0,[x2,+0x8]!
  ldrsh w0,[x2,+0x8]!
  ldrsw x0,[x2,+0x8]!

  // unsigned offset
  ldr w0,[sp,+0x8]
  ldr x0,[x2,+0x8]
  ldr w0,[sp]
  ldr x0,[x2]
  ldrsb w0,[x2,+0x8]
  ldrsh w0,[x2,+0x8]
  ldrsw x0,[x2,+0x8]

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
add sp,sp,0x123
add sp,x2,0x123
add w1,w2,0x123
add w4,w8,0x123,lsl 12
add wsp,w2,0x123
add wsp,wsp,0x123
add x1,x2,0x123
add x1,x2,0xFFF
add x1,x2,0xFFF,lsl 12
add x1,sp,0x123
adds w1,wsp,0x123
adds w8,w16,0x123,lsl 12
adds x8,x16,0x123,lsl 12
sub sp,sp,0x123
sub sp,x2,0x123
sub w1,w2,0x123
sub w4,w8,0x123,lsl 12
sub wsp,w2,0x123
sub wsp,wsp,0x123
sub x1,x2,0x123
sub x1,x2,0xFFF
sub x1,x2,0xFFF,lsl 12
sub x1,sp,0x123
subs w1,wsp,0x123
subs w8,w16,0x123,lsl 12
subs x8,x16,0x123,lsl 12
and w1,w2,0x0001FE00
and w1,w2,w3
and w1,w2,w3,asr 4
and w1,w2,w3,lsl 4
and w1,w2,w3,lsr 4
and w1,w2,w3,ror 4
and wzr,w2,w3,ror 4
and x1,x2,0x0003FC0000000000
and x1,x2,x3
and x1,x2,x3,lsl 4
and x1,x2,x3,ror 4
and xzr,x2,x3,ror 4
ands w1,w2,0xFFFFFFF1
ands w1,w2,w3
ands w1,w2,w3,lsl 4
ands w1,w2,w3,ror 4
ands x1,x2,0x5555555555555555
ands x1,x2,x3
ands x1,x2,x3,lsl 4
ands x1,x2,x3,ror 4
orn w1,w2,w3
orn w1,w2,w3,asr 4
orn w1,w2,w3,lsl 4
orn w1,w2,w3,lsr 4
orn w1,w2,w3,ror 4
orn x1,x2,x3
orn x1,x2,x3,asr 4
orn x1,x2,x3,lsl 4
orn x1,x2,x3,lsr 4
orn x1,x2,x3,ror 4
orr w1,w2,0xFFFFFFF1
orr w1,w2,w3
orr w1,w2,w3,asr 4
orr w1,w2,w3,lsl 4
orr w1,w2,w3,lsr 4
orr w1,w2,w3,ror 4
orr x1,x2,0xAAAAAAAAAAAAAAAA
orr x1,x2,x3
orr x1,x2,x3,lsl 4
orr x1,x2,x3,ror 4
eon w1,w2,w3
eon w1,w2,w3,asr 4
eon w1,w2,w3,lsl 4
eon w1,w2,w3,lsr 4
eon w1,w2,w3,ror 4
eon wzr,w2,w3,ror 4
eon x1,x2,x3
eon x1,x2,x3,lsl 4
eon x1,x2,x3,ror 4
eon xzr,x2,x3,ror 4
eor sp,x2,0xAAAAAAAAAAAAAAAA
eor w1,w2,0xFFFFFFF1
eor w1,w2,w3
eor w1,w2,w3,asr 4
eor w1,w2,w3,lsl 4
eor w1,w2,w3,lsr 4
eor w1,w2,w3,ror 4
eor wzr,w2,w3,ror 4
eor x1,x2,0xFFFFFFFFFFFFFFF1
eor x1,x2,x3
eor x1,x2,x3,lsl 4
eor x1,x2,x3,ror 4
eor xzr,x2,x3,ror 4
bic w1,w2,w3
bic w1,w2,w3,asr 4
bic w1,w2,w3,lsl 4
bic w1,w2,w3,lsr 4
bic w1,w2,w3,ror 4
bic x1,x2,x3
bic x1,x2,x3,lsl 4
bic x1,x2,x3,ror 4
bics w1,w2,w3
bics w1,w2,w3,lsl 4
bics w1,w2,w3,ror 4
bics x1,x2,x3
bics x1,x2,x3,lsl 4
bics x1,x2,x3,ror 4
svc 0xFFFF
hvc 0xFFFF
smc 0xFFFF
brk 0xFFFF
hlt 0xFFFF
dcps1 0x0
dcps1 0xFFFF
dcps2 0xFFFF
dcps3 0xFFFF

label_int:
  .int 0xffff
