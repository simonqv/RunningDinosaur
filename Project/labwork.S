  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

.global delay
.global time2string
.global enable_interrupt

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #

.data
.text


hexasc:
  andi  $a0, $a0, 0x000f
  ble $a0, 9, zeronine  # if input less than 10, jump to zeronine.
  nop

  addi  $v0, $a0, 55    # add 55 to calculate ascii code for A-...
  jr  $ra
  nop

zeronine:
  addi  $v0, $a0, 48    # add 48 to calculate ascii for 0-9
  jr  $ra
  nop

delay:
  PUSH  $a0

  while:
  slt $t0, $0, $a0    # 0 < a0 => t0 = 1
  beq   $t0, $0, exit # If t0 == 0, jump.
  nop
  addi  $a0, $a0, -1    # a0 - 1

  # Constants
  add $t1, $0, $0   # i = 0
  li  $t2, 3850

  for:
  slt $t3, $t1, $t2   # i < c => t3 = 1
  beq $t3, $0, while
  nop
  addi  $t1, $t1, 1   # i ++

  j for
  nop

  exit:

  POP $a0

  jr $ra
  nop


time2string:
  PUSH  $ra
  PUSH  $a0
  PUSH  $a1
  PUSH  $s0

  move  $s0, $a0  # Set $s0 to $a0

  andi  $t1, $a1, 0xf000  # Take first number from timestamp (X_:__)
  srl $a0, $t1, 12    # Shift to LSB position
  jal hexasc      # Call hexasc
  nop

  sb  $v0, 0($s0)   # Store hex result

  andi  $t2, $a1, 0x0f00
  srl $a0, $t2, 8
  jal   hexasc
  nop

  sb  $v0, 1($s0)

  li  $t3, 0x3A # Load ":"
  sb  $t3, 2($s0) # Store

  andi  $t4, $a1, 0x00f0
  srl $a0, $t4, 4
  jal   hexasc
  nop

  sb  $v0, 3($s0)

  andi  $t5, $a1, 0x000f
  move  $a0, $t5
  jal hexasc
  nop

  sb  $v0, 4($s0)
  sb  $0, 5($s0)  # add Null

  POP $s0
  POP $a1
  POP $a0
  POP $ra

  jr  $ra
  nop
  

enable_interrupt:
  ei
  jr $ra
  nop
