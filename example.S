.global _start
.section .text,"ax"
// comment
_start: // comment
  MOVZ X8, #93 // MOV X8, #93
  MOVZ X0, #1  // MOV X0, #1 
  SVC #0
