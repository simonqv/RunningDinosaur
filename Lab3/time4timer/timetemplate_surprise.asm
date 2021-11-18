  # timetemplate.asm
  # Written 2015 by F Lundevall
  # Copyright abandonded - this file is in the public domain.

.macro	PUSH (%reg)
	addi	$sp,$sp,-4
	sw	%reg,0($sp)
.end_macro

.macro	POP (%reg)
	lw	%reg,0($sp)
	addi	$sp,$sp,4
.end_macro

	.data
	.align 2
mytime:	.word 0x5957
timstr:	.ascii "text more text lots of text\0"
	.text
main:
	# print timstr
	la	$a0,timstr
	li	$v0,4
	syscall
	nop
	# wait a little
	li	$a0,1000
	jal	delay
	nop
	# call tick
	la	$a0,mytime
	jal	tick
	nop
	# call your function time2string
	la	$a0,timstr
	la	$t0,mytime
	lw	$a1,0($t0)
	jal	time2string
	nop
	# print a newline
	li	$a0,10
	li	$v0,11
	syscall
	nop
	# go back and do it all again
	j	main
	nop
# tick: update time pointed to by $a0
tick:	lw	$t0,0($a0)	# get time
	addiu	$t0,$t0,1	# increase
	andi	$t1,$t0,0xf	# check lowest digit
	sltiu	$t2,$t1,0xa	# if digit < a, okay
	bnez	$t2,tiend
	nop
	addiu	$t0,$t0,0x6	# adjust lowest digit
	andi	$t1,$t0,0xf0	# check next digit
	sltiu	$t2,$t1,0x60	# if digit < 6, okay
	bnez	$t2,tiend
	nop
	addiu	$t0,$t0,0xa0	# adjust digit
	andi	$t1,$t0,0xf00	# check minute digit
	sltiu	$t2,$t1,0xa00	# if digit < a, okay
	bnez	$t2,tiend
	nop
	addiu	$t0,$t0,0x600	# adjust digit
	andi	$t1,$t0,0xf000	# check last digit
	sltiu	$t2,$t1,0x6000	# if digit < 6, okay
	bnez	$t2,tiend
	nop
	addiu	$t0,$t0,0xa000	# adjust last digit
tiend:	sw	$t0,0($a0)	# save updated result
	jr	$ra		# return
	nop

  # you can write your code for subroutine "hexasc" below this line
  #
hexasc:
	ble	$a0, 9, zeronine	# if input less than 10, jump to zeronine.
	nop
	
	addi	$v0, $a0, 55		# add 55 to calculate ascii code for A-...
	jr	$ra	
	nop

zeronine:	
	addi	$v0, $a0, 48		# add 48 to calculate ascii for 0-9
	jr	$ra
	nop
	
delay:  
	PUSH 	($a0)
					
	while:
	slt	$t0, $0, $a0		# 0 < a0 => t0 = 1
	beq 	$t0, $0, exit	# If t0 == 0, jump.
	nop
	addi	$a0, $a0, -1		# a0 - 1
	
	# Constants
	add	$t1, $0, $0		# i = 0
	li	$t2, 60		# Constant c = 55, works for ms == 1000 and 3000, meassured approx with syscall 30 and timer.
					# For some unknown reason, 300 works better now. We literally changed nothing.
					# Very unreliable!
	for:
	slt	$t3, $t1, $t2		# i < c => t3 = 1
	beq	$t3, $0, while
	nop
	addi	$t1, $t1, 1		# i ++
	
	j for
	nop
	
	exit:
	
	POP	($a0)
	
	jr $ra
	nop
	

time2string:
	PUSH	($ra)
	PUSH	($a0)
	PUSH	($s0)
	
	move	$s0, $a0	# Set $s0 to $a0 
	
	andi 	$t6, $a1, 0x00ff
	beq	$t6, $0, HOUR
	
	andi	$t1, $a1, 0xf000	# Take first number from timestamp (X_:__)
	srl	$a0, $t1, 12		# Shift to LSB position
	jal	hexasc			# Call hexasc
	nop
	
	sb	$v0, 0($s0)		# Store hex result 
	
	andi	$t2, $a1, 0x0f00
	srl	$a0, $t2, 8
	jal 	hexasc
	nop
	
	sb	$v0, 1($s0)
	
	li	$t3, 0x3A	# Load ":"
	sb	$t3, 2($s0)	# Store
	
	andi	$t4, $a1, 0x00f0
	srl	$a0, $t4, 4
	jal 	hexasc
	nop
	
	sb	$v0, 3($s0)
	
	andi	$t5, $a1, 0x000f
	move	$a0, $t5
	jal	hexasc
	nop
	
	sb	$v0, 4($s0)
	sb 	$0, 5($s0)	# add Null
	
	DONE:
	
	POP 	($s0)
	POP	($a0)
	POP	($ra)
	
	move	$s0, $0
	
	jr	$ra
	nop
	
	
	HOUR:
	
	li	$v0, 0x48		# H
	sb	$v0, 0($s0)
	li 	$v0, 79 	# O
	sb	$v0, 1($s0)
	li 	$v0, 85		# U
	sb	$v0, 2($s0)
	li	$v0, 82		# R
	sb	$v0, 3($s0)
	sb	$0, 4($s0)
	
	
	j	DONE
	nop
