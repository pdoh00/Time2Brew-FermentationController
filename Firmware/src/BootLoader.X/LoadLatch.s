;
; file: call2.s
;
.global _DoubleWordWrite

_DoubleWordWrite:
; W0 = PAGE Address
; W1 = Offset Address
; W2 points to the address of the data to write to the latches
; Load the destination address to be written
MOV W0,NVMADRU
MOV W1,NVMADR;

; Set up a pointer to the first latch location to be written
MOV #0xFA,W0
MOV W0,TBLPAG
MOV #0,W1
; Perform the TBLWT instructions to write the latches
TBLWTL [W2++],[W1]
TBLWTH [W2++],[W1++]
TBLWTL [W2++],[W1]
TBLWTH [W2++],[W1++]

; Setup NVMCON for word programming
MOV #0x4001,W0
MOV W0,NVMCON
; Disable interrupts < priority 7 for next 5 instructions
; Assumes no level 7 peripheral interrupts
DISI #06
; Write the key sequence
MOV #0x55,W0
MOV W0,NVMKEY
MOV #0xAA,W0
MOV W0,NVMKEY
; Start the write cycle
BSET NVMCON,#15
NOP
NOP
RETURN

.global _ErasePage

_ErasePage:
; W0 = PAGE Address
; W1 = Offset Address
;; Set up the NVMADR registers to the starting address of the page
MOV W0,NVMADRU
MOV W1,NVMADR
; Set up NVMCON to erase one page of Program Memory
MOV #0x4003,W0
MOV W0,NVMCON
; Disable interrupts < priority 7 for next 5 instructions
; Assumes no level 7 peripheral interrupts
DISI #06
; Write the KEY Sequence
MOV #0x55,W0
MOV W0,NVMKEY
MOV #0xAA,W0
MOV W0,NVMKEY
; Start the erase operation
BSET NVMCON,#15
; Insert two NOPs after the erase cycle (required)
NOP
NOP
RETURN


.global _ReadNVMWord

_ReadNVMWord:
; W1 = PAGE Address
; W0 = Offset Address
; W2 points to the destination address of the data. LONG ptr!
MOV TBLPAG,W5
MOV W1,TBLPAG
TBLRDH [W0],W3
TBLRDL [W0],W4
MOV W5,TBLPAG
MOV W3,W0
MOV W4,W1
RETURN

