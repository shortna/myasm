.global _start

.section .data,"wa"
msg:
  .ascii "Hello, World! "
var:
  .ascii "z"
  adr x8, arr 

.section .text,"ax"

_start:
  adr x8, var   // load address of var to x8
  ldrb w0, [x8] // get byte from x8
  adr x1, arr   // adr of arr to x1
  movz x2, 25   // counter

  begin:
    strb w0,[x1, x2] // store byte at x1 + x2
    sub x0, x0, 1
    sub x2, x2, 1
  // compare x1 to 0 and branch if true
  cbnz x2, begin
  strb w0,[x1, x2] // store last char at array[0]

  bl print
  b end

print:
  // print msg
  movz x0, 1
  adr x1, msg
  movz x2, 14
  movz x8, 64
  svc 0

  // print arr
  movz x0, 1
  adr x1, arr
  movz x2, 26
  movz x8, 64
  svc 0

  ret

end:
  movz x0, 90
  movz x8, 93
  svc 0

.section .bss,"wa"
arr:
  .zero 26
