#ifdef __arm__

#include "ARM6809/ARM6809.i"
#include "GnGVideo/GnGVideo.i"

	.global joyCfg
	.global EMUinput
	.global g_dipSwitch0
	.global g_dipSwitch1
	.global g_dipSwitch2
	.global g_dipSwitch3
	.global coinCounter0
	.global coinCounter1

	.global ioReset
	.global refreshEMUjoypads
	.global IO_R
	.global IO_W

	.syntax unified
	.arm

	.section .ewram,"ax"
	.align 2
;@----------------------------------------------------------------------------
ioReset:
;@----------------------------------------------------------------------------
	bx lr
;@----------------------------------------------------------------------------
refreshEMUjoypads:			;@ Call every frame
;@----------------------------------------------------------------------------
		ldr r4,=frameTotal
		ldr r4,[r4]
		movs r0,r4,lsr#2		;@ C=frame&2 (autofire alternates every other frame)
	ldr r4,EMUinput
	and r0,r4,#0xf0
		ldr r2,joyCfg
		andcs r4,r4,r2
		tstcs r4,r4,lsr#10		;@ L?
		andcs r4,r4,r2,lsr#16
//	ldrb r3,flipScreen
//	tst r3,#1
	adr r1,rlud2lrud
//	adrne r1,rlud2lrud180
	ldrb r0,[r1,r0,lsr#4]

	ands r1,r4,#3				;@ A/B buttons to Attack/Jump
	cmpne r1,#3
	eorne r1,r1,#3
	tst r2,#0x400				;@ Swap A/B?
	andne r1,r4,#3
	orr r0,r0,r1,lsl#4

	mov r1,#0
	mov r3,#0
	tst r4,#0x4					;@ Select
	orrne r3,r3,#0x40			;@ Coin 1
	tst r4,#0x8					;@ Start
	orrne r3,r3,#0x01			;@ Start 1
	tst r2,#0x20000000			;@ Player2?
	movne r1,r0
	movne r0,#0
	movne r3,r3,lsl#1

	strb r0,joy0State
	strb r1,joy1State
	strb r3,joy2State
	bx lr

joyCfg: .long 0x00ff01ff	;@ byte0=auto mask, byte1=(saves R), byte2=R auto mask
							;@ bit 31=single/multi, 30,29=1P/2P, 27=(multi) link active, 24=reset signal received
nrPlayers:	.long 0			;@ Number of players in multilink.
joySerial:	.byte 0
joy0State:	.byte 0
joy1State:	.byte 0
joy2State:	.byte 0
rlud2lrud:		.byte 0x00,0x01,0x02,0x03, 0x08,0x09,0x0a,0x0b, 0x04,0x05,0x06,0x07, 0x0c,0x0d,0x0e,0x0f
rlud2lrud180:	.byte 0x00,0x02,0x01,0x03, 0x04,0x06,0x05,0x07, 0x08,0x0a,0x09,0x0b, 0x0c,0x0e,0x0d,0x0f
g_dipSwitch0:	.byte 0
g_dipSwitch1:	.byte 0x00		;@ Lives, cabinet & demo sound.
g_dipSwitch2:	.byte 0
g_dipSwitch3:	.byte 0
coinCounter0:	.long 0
coinCounter1:	.long 0

EMUinput:			;@ This label here for main.c to use
	.long 0			;@ EMUjoypad (this is what Emu sees)

;@----------------------------------------------------------------------------
Input0_R:		;@  Coins, Start & Service
;@----------------------------------------------------------------------------
;@	mov r11,r11					;@ No$GBA breakpoint
	ldrb r0,joy2State
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
Input1_R:		;@ Player 1
;@----------------------------------------------------------------------------
	ldrb r0,joy0State
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
Input2_R:		;@ Player 2
;@----------------------------------------------------------------------------
	ldrb r0,joy1State
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
Input3_R:
;@----------------------------------------------------------------------------
	ldrb r0,g_dipSwitch1
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
Input4_R:
;@----------------------------------------------------------------------------
	ldrb r0,g_dipSwitch2
	eor r0,r0,#0xFF
	bx lr
;@----------------------------------------------------------------------------
Input5_R:
;@----------------------------------------------------------------------------
	ldrb r0,g_dipSwitch0
	eor r0,r0,#0xFF
	bx lr

;@----------------------------------------------------------------------------
IO_R:				;@ I/O read, 0x2000-0x3FFFF
;@----------------------------------------------------------------------------
	cmp addy,#0x3000
	bmi mem6809R1

	cmp addy,#0x3C00
	beq DMA_R
	mov r1,addy,lsr#8
	cmp r1,#0x30
	bne empty_IO_R
	and r1,addy,#0xFF
	cmp r1,#0x05
	ldrmi pc,[pc,r1,lsl#2]
;@---------------------------
	b empty_IO_R
;@ io_read_tbl
	.long Input0_R			;@ 0x3000
	.long Input1_R			;@ 0x3001
	.long Input2_R			;@ 0x3002
	.long Input3_R			;@ 0x3003
	.long Input4_R			;@ 0x3004
;@----------------------------------------------------------------------------
DMA_R:				;@
;@----------------------------------------------------------------------------
	stmfd sp!,{lr}
	mov r0,#0
	bl m6809SetIRQPin
	mov r0,#0
	ldmfd sp!,{lr}
	bx lr

;@----------------------------------------------------------------------------
IO_W:				;@ I/O write, 0x2000-0x3FFFF
;@----------------------------------------------------------------------------
	b empty_IO_W

;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
