import sys
import struct

OPCODES = {
    # ---- R-Type
    # Shift
    "SLL": {"type": "R", "op": 0x00, "func": 0x04},  # LOGIC LEFT SHIFT
    "SRL": {"type": "R", "op": 0x00, "func": 0x06},  # LOGIC RIGHT SHIFT
    "SRA": {"type": "R", "op": 0x00, "func": 0x07},  # ARITHMETIC RIGHT SHIFT
    # Arithmetic operations
    "ADD": {"type": "R", "op": 0x00, "func": 0x20},
    "SUB": {"type": "R", "op": 0x00, "func": 0x22},
    # Logic operations
    "AND": {"type": "R", "op": 0x00, "func": 0x24},
    "OR": {"type": "R", "op": 0x00, "func": 0x25},
    "XOR": {"type": "R", "op": 0x00, "func": 0x26},
    # Set if condition
    "SGT": {"type": "R", "op": 0x00, "func": 0x29},  # SET GREATER THAN
    "SEQ": {"type": "R", "op": 0x00, "func": 0x2A},  # SET EQUAL
    "SGE": {"type": "R", "op": 0x00, "func": 0x2B},  # SET GREATER EQUAL
    "SLT": {"type": "R", "op": 0x00, "func": 0x2C},  # SET LESS THAN
    "SNE": {"type": "R", "op": 0x00, "func": 0x2D},  # SET NOT EQUAL
    "SLE": {"type": "R", "op": 0x00, "func": 0x2E},  # SET LESS EQUAL

    # ---- J-Type
    # Jumps
    "J": {"type": "J", "op": 0x02},
    "JAL": {"type": "J", "op": 0x03},
    # Special instructions
    "RFE": {"type": "J", "op": 0x3F},
    "INT": {"type": "J", "op": 0x39},

    # ---- I-Type
    # Branch
    "BEQZ": {"type": "I", "op": 0x04},
    "BNEZ": {"type": "I", "op": 0x05},
    # Arithmetic operations
    "ADDI": {"type": "I", "op": 0x08},
    "ADDUI": {"type": "I", "op": 0x09},
    "SUBI": {"type": "I", "op": 0x0A},
    "SUBUI": {"type": "I", "op": 0x0B},
    # Logic operations
    "ANDI": {"type": "I", "op": 0x0C},
    "ORI": {"type": "I", "op": 0x0D},
    "XORI": {"type": "I", "op": 0x0E},
    # Load high
    "LHI": {"type": "I", "op": 0x0F},
    # Jump
    "JR": {"type": "I", "op": 0x12},
    "JALR": {"type": "I", "op": 0x13},
    # Shift
    "SLLI": {"type": "I", "op": 0x14},  # SHIFT LEFT LOGICAL IMMEDIATE
    "SRLI": {"type": "I", "op": 0x16},  # SHIFT RIGHT LOGICAL IMMEDIATE
    "SRAI": {"type": "I", "op": 0x17},  # SHIFT RIGHT ARITHMETIC IMMEDIATE
    # Set if condition
    "SGTI": {"type": "I", "op": 0x19},  # SET GREATER THAN IMMEDIATE
    "SEQI": {"type": "I", "op": 0x1A},  # SET EQUAL IMMEDIATE
    "SGEI": {"type": "I", "op": 0x1B},  # SET GREATER EQUAL IMMEDIATE
    "SLTI": {"type": "I", "op": 0x1C},  # SET LESS THAN IMMEDIATE
    "SNEI": {"type": "I", "op": 0x1D},  # SET NOT EQUAL IMMEDIATE
    "SLEI": {"type": "I", "op": 0x1E},  # SET LESS EQUAL IMMEDIATE
    # Load / Store (special I-Type, M for memory)
    "LB": {"type": "M", "op": 0x20},
    "LH": {"type": "M", "op": 0x21},
    "LW": {"type": "M", "op": 0x23},
    "LBU": {"type": "M", "op": 0x24},
    "LHU": {"type": "M", "op": 0x25},
    "SB": {"type": "M", "op": 0x28},
    "SH": {"type": "M", "op": 0x29},
    "SW": {"type": "M", "op": 0x2B},
}


class ParseException(Exception):
    pass


def register_to_int(register):
    """
    Converts 'R1' -> 1, 'R10' -> 10
    """
    register_int = int(register.upper().replace('R', ''))

    if register_int < 0 or register_int > 31:
        raise ParseException("Invalid register " + register)

    return register_int


def get_address_value(token, labels, instr_address=None):
    """
    Get the relative or absolute address value from a token.
    The token can be an immediate or a label.
    """
    if token.upper() in labels:
        if instr_address is not None:
            return labels[token.upper()] - instr_address - 4

        return labels[token.upper()]
    try:
        # Automatically parses 0xff, 0b10
        return int(token, 0)
    except ValueError:
        raise ParseException(f"Invalid immediate or label: {token}")


# Limits of the immediate fields:
#   min         most negative value accepted
#   signed_max  biggest value that survives the sign extension, see resolve_disp
#   mask        biggest raw bit pattern accepted, also the encoding mask
IMM_LIMITS = {
    16: {"min": -0x8000, "signed_max": 0x7FFF, "mask": 0xFFFF},
    26: {"min": -0x2000000, "signed_max": 0x1FFFFFF, "mask": 0x3FFFFFF},
}


def check_imm(value, width, maximum):
    """
    Range checks a resolved immediate and returns it masked to the field width.

    The floor and the ceiling are checked separately because neither accepted
    range is symmetric, so no single limit on the absolute value can express
    them: a data immediate spans -0x8000..0xFFFF, since -1 and 0xFFFF are two
    spellings of the same field, and a displacement spans -0x8000..0x7FFF,
    because in two's complement the negative side holds one value more.
    """
    limits = IMM_LIMITS[width]

    if value < limits["min"] or value > maximum:
        raise ParseException(
            f"Value of '{hex(value)}' overflows imm{width}"
            f" (accepted range {limits['min']}..{maximum})")

    return value & limits["mask"]


def resolve_imm(token, labels, width):
    """
    Resolves a data immediate: an ALU operand, a memory offset, an interrupt
    code. It is taken as a raw bit pattern, so -1 and 0xFFFF describe the same
    16 bit field.
    """
    return check_imm(get_address_value(token, labels), width,
                     IMM_LIMITS[width]["mask"])


def resolve_disp(token, labels, width, instr_address):
    """
    Resolves a PC relative displacement for a branch or a jump. The hardware
    always sign extends this field, so a positive displacement cannot go past
    signed_max: the bit patterns above it read back as negative numbers and the
    jump would silently go the other way.
    """
    return check_imm(get_address_value(token, labels, instr_address), width,
                     IMM_LIMITS[width]["signed_max"])


# See the Notation section of docs/ISA.md
def encode_r(opcode, ra, rb, rc, func):
    return (opcode << 26) | (ra << 21) | (rb << 16) | (rc << 11) | func


def encode_i(opcode, ra, rb, imm16):
    return (opcode << 26) | (ra << 21) | (rb << 16) | (imm16 & 0xFFFF)


def encode_j(opcode, imm26):
    return (opcode << 26) | (imm26 & 0x3FFFFFF)


def assemble_instr(instr, labels):
    """
    Assembles the instruction.
    """
    parts = instr[0].replace(',', ' ').replace(
        '(', ' ').replace(')', ' ').split()

    if parts[0].upper() not in OPCODES:
        raise ParseException(parts[0] + " is not a recognised instruction")
        return 0

    mnemonic = parts[0].upper()
    op = OPCODES[mnemonic]
    opcode = op['op']

    if op['type'] == "R":
        # Syntax    OP rc, ra, rb
        # Encoding  [OP] [RA] [RB] [RC] [unused] [FUNC]
        rc = register_to_int(parts[1])      # destination
        ra = register_to_int(parts[2])      # first operand
        rb = register_to_int(parts[3])      # second operand

        return encode_r(opcode, ra, rb, rc, op['func'])
    elif op['type'] == "I":
        # Syntax    ALU / set / shift   OP rb, ra, Imm16
        #           Load high           OP rb, Imm16        (RA unused)
        #           Branch              OP ra, Imm16        (RB unused)
        #           Jump register       OP ra               (RB, Imm16 unused)
        # Encoding  [OP] [RA] [RB] [Imm16]
        if mnemonic in ["BNEZ", "BEQZ"]:
            ra = register_to_int(parts[1])  # tested register
            rb = 0
            imm16 = resolve_disp(parts[2], labels, 16, instr[1])
        elif mnemonic in ["JR", "JALR"]:
            ra = register_to_int(parts[1])  # target register
            rb = 0
            imm16 = 0
        elif mnemonic == "LHI":
            ra = 0
            rb = register_to_int(parts[1])  # destination
            imm16 = resolve_imm(parts[2], labels, 16)
        else:
            rb = register_to_int(parts[1])  # destination
            ra = register_to_int(parts[2])  # source
            imm16 = resolve_imm(parts[3], labels, 16)

        return encode_i(opcode, ra, rb, imm16)
    elif op['type'] == "M":
        # I-Type with the memory syntax
        # Syntax    OP rb, Imm16(ra)
        # Encoding  [OP] [RA] [RB] [Imm16]
        rb = register_to_int(parts[1])      # loaded / stored register
        imm16 = resolve_imm(parts[2], labels, 16)
        ra = register_to_int(parts[3])      # base address register

        return encode_i(opcode, ra, rb, imm16)
    elif op['type'] == "J":
        # Syntax    OP Imm26, RFE takes no operand
        # Encoding  [OP] [Imm26]
        if mnemonic in ["RFE"]:
            imm26 = 0
        elif mnemonic in ["INT"]:
            # An interrupt code, not an address: never PC relative
            imm26 = resolve_imm(parts[1], labels, 26)
        else:
            imm26 = resolve_disp(parts[1], labels, 26, instr[1])

        return encode_j(opcode, imm26)

    return 0


def parse(lines):
    instructions = []
    labels = {}
    i_address = 0

    for line_num, line in enumerate(lines):
        if line.endswith(':'):
            label_name = line[:-1]

            if not label_name:
                raise ParseException(
                    f"Label name cannot be blank at line {line_num}")
            if ' ' in label_name:
                raise ParseException(
                    f"Label name cannot include spaces at line {line_num}")

            label_name = line.split(':')[0].strip().upper()

            if label_name in labels:
                raise ParseException(
                    f"Label {label_name} declared more than one time")

            labels[label_name] = i_address
        else:
            instructions.append((line, i_address, line_num))
            i_address += 4

    return instructions, labels


def main():
    if len(sys.argv) != 3:
        print("Usage: python asm.py input.asm output.bin")
        sys.exit(1)

    input_lines = []
    with open(sys.argv[1], 'r') as f:
        input_lines = f.readlines()

    clean_lines = []
    # Remove comments and blank lines
    for line in input_lines:
        clean_line = line.split(';')[0].strip()
        if not clean_line:
            continue
        clean_lines.append(clean_line)

    try:
        instructions, labels = parse(clean_lines)
    except ParseException as e:
        print(f"ASSEMBLER ERROR: {e}")
        exit(1)

    with open(sys.argv[2], 'wb') as f:
        for instr in instructions:
            try:
                val = assemble_instr(instr, labels)
            except ParseException as e:
                print(f"ASSEMBLER ERROR at line {instr[2] + 1}:")
                print(f"    {instr[0]}")
                print(f"    -> {e}")
                exit(1)
            except Exception as e:
                print(f"CRITICAL ERROR at line {instr[2] + 1}:")
                print(f"    {instr[0]}")
                print(f"    -> {e}")
                exit(1)
            packed_bytes = struct.pack('>I', val)
            f.write(packed_bytes)


if __name__ == '__main__':
    main()
