.global _start
.section .text,"ax"

_start: // comment
  movz x0, 1
  adr x1, label_int
  movz x2, #13
  movz x8, #64
  svc #0

  movz x0, #0
  movz x8, #93
  svc #0

.section .data,"wa"
label_int:
  .ascii "Hello, World!"
