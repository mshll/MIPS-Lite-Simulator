def main():
    filename = input(
        "Enter the filename where you want to save the instructions: ")

    with open(filename, 'w') as file:
        while True:
            instruction = input(
                "Enter an instruction (e.g. 'ADDI R1 R0 1000', or 'q' to quit): ")
            if instruction.lower() == 'quit' or instruction.lower() == 'q':
                break

            encoded_instruction = encode_instruction(instruction)
            if encoded_instruction:
                file.write(encoded_instruction + '\n')
            else:
                print("Invalid instruction format.")


def encode_instruction(instruction):
    parts = instruction.split()
    opcode = parts[0].upper()
    opcodes = {
        'ADD': '000000', 'ADDI': '000001', 'SUB': '000010', 'SUBI': '000011',
        'MUL': '000100', 'MULI': '000101', 'OR': '000110', 'ORI': '000111',
        'AND': '001000', 'ANDI': '001001', 'XOR': '001010', 'XORI': '001011',
        'LDW': '001100', 'STW': '001101', 'BZ': '001110', 'BEQ': '001111',
        'JR': '010000', 'HALT': '010001'
    }

    if opcode not in opcodes:
        return None

    binary_opcode = opcodes[opcode]
    if opcode in ['ADD', 'SUB', 'MUL', 'OR', 'AND', 'XOR']:
        # R-type format
        rd, rs, rt = parts[1], parts[2], parts[3]
        rd, rs, rt = rd[1:], rs[1:], rt[1:]  # Remove 'R' prefix
        binary_string = f"{binary_opcode}{int(rs):05b}{int(rt):05b}{int(rd):05b}00000000000"
    elif opcode in ['ADDI', 'SUBI', 'MULI', 'ORI', 'ANDI', 'XORI', 'LDW', 'STW', 'BZ', 'BEQ']:
        # I-type format
        rt, rs, imm = parts[1], parts[2], parts[3]
        rt, rs = rt[1:], rs[1:]  # Remove 'R' prefix
        imm = int(imm) & 0xFFFF  # Immediate value is 16 bits
        binary_string = f"{binary_opcode}{int(rs):05b}{int(rt):05b}{imm:016b}"
    elif opcode == 'JR':
        rs = parts[1][1:]  # Remove 'R' prefix
        binary_string = f"{binary_opcode}{int(rs):05b}00000000000000000000"
    elif opcode == 'HALT':
        binary_string = f"{binary_opcode}00000000000000000000000000"

    return f"{int(binary_string, 2):08X}"


if __name__ == "__main__":
    main()
