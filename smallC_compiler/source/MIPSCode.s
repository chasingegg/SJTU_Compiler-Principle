.data
temp: .space 32

.text
__printf_one:
li $v0, 1
syscall
jr $ra
__scanf_one:
li $v0,5
syscall
jr $ra

main:
la $a3, temp
add $11, $29, -28
move $13, $11
move $11, $13
jal __scanf_one
sw $2, 0($11)
move $11, $13
lw $11, 0($11)
move $4, $11
jal __printf_one
li $11, 0
add $12, $29, 4
sw $11, 0($12)
j program_end
j program_end
program_end:
li $v0, 10
syscall
