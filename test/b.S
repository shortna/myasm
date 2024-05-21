.global _start
.section .text,"ax"

_start: // comment
  adr x0, label_int

.section .data,"wa"
label_int:
  .int 0xffff
