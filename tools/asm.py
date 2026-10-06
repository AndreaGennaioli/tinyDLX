import os
import sys
import struct
from typing import NamedTuple, Literal

type CostantTable = dict[str, int]


class SourceLine(NamedTuple):
    text: str
    file_path: str
    line_num: int


class Instruction(NamedTuple):
    source_line: SourceLine
    address: int


class InstructionInfo(NamedTuple):
    type: Literal["R", "I", "J", "S", "M"]
    op: int
    func: int | None = None


OPCODES: dict[str, InstructionInfo] = {
    # ---- R-Type
    # Shift
    "SLL": InstructionInfo("R", 0x00, 0x04),  # LOGIC LEFT SHIFT
    "SRL": InstructionInfo("R", 0x00, 0x06),  # LOGIC RIGHT SHIFT
    "SRA": InstructionInfo("R", 0x00, 0x07),  # ARITHMETIC RIGHT SHIFT
    # Arithmetic operations
    "ADD": InstructionInfo("R", 0x00, 0x20),
    "SUB": InstructionInfo("R", 0x00, 0x22),
    # Logic operations
    "AND": InstructionInfo("R", 0x00, 0x24),
    "OR": InstructionInfo("R", 0x00, 0x25),
    "XOR": InstructionInfo("R", 0x00, 0x26),
    # Set if condition
    "SGT": InstructionInfo("R", 0x00, 0x29),  # SET GREATER THAN
    "SEQ": InstructionInfo("R", 0x00, 0x2A),  # SET EQUAL
    "SGE": InstructionInfo("R", 0x00, 0x2B),  # SET GREATER EQUAL
    "SLT": InstructionInfo("R", 0x00, 0x2C),  # SET LESS THAN
    "SNE": InstructionInfo("R", 0x00, 0x2D),  # SET NOT EQUAL
    "SLE": InstructionInfo("R", 0x00, 0x2E),  # SET LESS EQUAL
    # Special register operations
    "MOVI2S": InstructionInfo("S", 0x00, 0x30),  # MOVE INTEGER (register) TO SPECIAL (register)
    "MOVS2I": InstructionInfo("S", 0x00, 0x31),  # MOVE SPECIAL (register) TO INTEGER (register)

    # ---- J-Type
    # Jumps
    "J": InstructionInfo("J", 0x02),
    "JAL": InstructionInfo("J", 0x03),
    # Special instructions
    "RFE": InstructionInfo("J", 0x3F),
    "INT": InstructionInfo("J", 0x39),

    # ---- I-Type
    # Branch
    "BEQZ": InstructionInfo("I", 0x04),
    "BNEZ": InstructionInfo("I", 0x05),
    # Arithmetic operations
    "ADDI": InstructionInfo("I", 0x08),
    "ADDUI": InstructionInfo("I", 0x09),
    "SUBI": InstructionInfo("I", 0x0A),
    "SUBUI": InstructionInfo("I", 0x0B),
    # Logic operations
    "ANDI": InstructionInfo("I", 0x0C),
    "ORI": InstructionInfo("I", 0x0D),
    "XORI": InstructionInfo("I", 0x0E),
    # Load high
    "LHI": InstructionInfo("I", 0x0F),
    # Jump
    "JR": InstructionInfo("I", 0x12),
    "JALR": InstructionInfo("I", 0x13),
    # Shift
    "SLLI": InstructionInfo("I", 0x14),  # SHIFT LEFT LOGICAL IMMEDIATE
    "SRLI": InstructionInfo("I", 0x16),  # SHIFT RIGHT LOGICAL IMMEDIATE
    "SRAI": InstructionInfo("I", 0x17),  # SHIFT RIGHT ARITHMETIC IMMEDIATE
    # Set if condition
    "SGTI": InstructionInfo("I", 0x19),  # SET GREATER THAN IMMEDIATE
    "SEQI": InstructionInfo("I", 0x1A),  # SET EQUAL IMMEDIATE
    "SGEI": InstructionInfo("I", 0x1B),  # SET GREATER EQUAL IMMEDIATE
    "SLTI": InstructionInfo("I", 0x1C),  # SET LESS THAN IMMEDIATE
    "SNEI": InstructionInfo("I", 0x1D),  # SET NOT EQUAL IMMEDIATE
    "SLEI": InstructionInfo("I", 0x1E),  # SET LESS EQUAL IMMEDIATE
    # Load / Store (special I-Type, M for memory)
    "LB": InstructionInfo("M", 0x20),
    "LH": InstructionInfo("M", 0x21),
    "LW": InstructionInfo("M", 0x23),
    "LBU": InstructionInfo("M", 0x24),
    "LHU": InstructionInfo("M", 0x25),
    "SB": InstructionInfo("M", 0x28),
    "SH": InstructionInfo("M", 0x29),
    "SW": InstructionInfo("M", 0x2B),
}


class ParseException(Exception):
    pass


def register_to_int(register: str):
    """
    Converts 'R1' -> 1, 'R10' -> 10
    """
    register_int = int(register.upper().replace('R', ''))

    if register_int < 0 or register_int > 31:
        raise ParseException("Invalid register " + register)

    return register_int


# Special Purpose Registers, used by MOVI2S and MOVS2I instructions.
# (see include/dlx_defs.h)
SPR_NAMES = {"SR": 0, "IAR": 1, "CR": 2}


def spr_to_int(register: str):
    """
    Converts 'SR' -> 0, 'IAR' -> 1, 'CR' -> 2
    """
    if register.upper() not in SPR_NAMES:
        raise ParseException(
            f"Invalid special register '{register}'" +
            f" (accepted names {', '.join(SPR_NAMES)})")

    return SPR_NAMES[register.upper()]


def get_address_value(
    token: str, costants: CostantTable, instr_address: int | None = None
) -> int:
    """
    Get the relative or absolute address value from a token.
    The token can be an immediate or a costant.
    """
    if token.upper() in costants:
        if instr_address is not None:
            return costants[token.upper()] - instr_address - 4

        return costants[token.upper()]
    try:
        # Automatically parses 0xff, 0b10
        return int(token, 0)
    except ValueError:
        raise ParseException(f"Invalid immediate or costant: {token}")


# Limits of the immediate fields:
#   min         most negative value accepted
#   signed_max  biggest value that survives the sign extension, see resolve_disp
#   mask        biggest raw bit pattern accepted, also the encoding mask
IMM_LIMITS = {
    16: {"min": -0x8000, "signed_max": 0x7FFF, "mask": 0xFFFF},
    26: {"min": -0x2000000, "signed_max": 0x1FFFFFF, "mask": 0x3FFFFFF},
}


def check_imm(value: int, width: int, maximum: int) -> int:
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
            f"Value of '{hex(value)}' overflows imm{width}" +
            f" (accepted range {limits['min']}..{maximum})")

    return value & limits["mask"]


def resolve_imm(token: str, costants: CostantTable, width: int) -> int:
    """
    Resolves a data immediate: an ALU operand, a memory offset, an interrupt
    code. It is taken as a raw bit pattern, so -1 and 0xFFFF describe the same
    16 bit field.
    """
    return check_imm(get_address_value(token, costants), width,
                     IMM_LIMITS[width]["mask"])


def resolve_disp(
    token: str, costants: CostantTable, width: int, instr_address: int
) -> int:
    """
    Resolves a PC relative displacement for a branch or a jump. The hardware
    always sign extends this field, so a positive displacement cannot go past
    signed_max: the bit patterns above it read back as negative numbers and the
    jump would silently go the other way.
    """
    return check_imm(get_address_value(token, costants, instr_address), width,
                     IMM_LIMITS[width]["signed_max"])


# See the Notation section of docs/architecture/ISA.md
def encode_r(opcode: int, ra: int, rb: int, rc: int, func: int):
    return (opcode << 26) | (ra << 21) | (rb << 16) | (rc << 11) | func


def encode_i(opcode: int, ra: int, rb: int, imm16: int):
    return (opcode << 26) | (ra << 21) | (rb << 16) | (imm16 & 0xFFFF)


def encode_j(opcode: int, imm26: int):
    return (opcode << 26) | (imm26 & 0x3FFFFFF)


def assemble_instr(instr: Instruction, costants: CostantTable) -> int:
    """
    Assembles the instruction.
    """
    parts = instr.source_line.text.replace(',', ' ').replace(
        '(', ' ').replace(')', ' ').split()

    if parts[0].upper() not in OPCODES:
        raise ParseException(parts[0] + " is not a recognised instruction")

    mnemonic = parts[0].upper()
    op = OPCODES[mnemonic]
    opcode = op.op

    if op.type == "R" and op.func is not None:
        # Syntax    OP rc, ra, rb
        # Encoding  [OP] [RA] [RB] [RC] [unused] [FUNC]
        rc = register_to_int(parts[1])      # destination
        ra = register_to_int(parts[2])      # first operand
        rb = register_to_int(parts[3])      # second operand

        return encode_r(opcode, ra, rb, rc, op.func)
    elif op.type == "S" and op.func is not None:
        # Syntax    Move to special     MOVI2S spr, ra
        #           Move from special   MOVS2I rc, spr
        # Encoding  [OP] [RA] [RB] [RC] [unused] [FUNC]
        rb = 0

        if mnemonic == "MOVI2S":
            rc = spr_to_int(parts[1])       # destination
            ra = register_to_int(parts[2])  # source
        else:
            rc = register_to_int(parts[1])  # destination
            ra = spr_to_int(parts[2])       # source

        return encode_r(opcode, ra, rb, rc, op.func)
    elif op.type == "I":
        # Syntax    ALU / set / shift   OP rb, ra, Imm16
        #           Load high           OP rb, Imm16        (RA unused)
        #           Branch              OP ra, Imm16        (RB unused)
        #           Jump register       OP ra               (RB, Imm16 unused)
        # Encoding  [OP] [RA] [RB] [Imm16]
        if mnemonic in ["BNEZ", "BEQZ"]:
            ra = register_to_int(parts[1])  # tested register
            rb = 0
            imm16 = resolve_disp(parts[2], costants, 16, instr.address)
        elif mnemonic in ["JR", "JALR"]:
            ra = register_to_int(parts[1])  # target register
            rb = 0
            imm16 = 0
        elif mnemonic == "LHI":
            ra = 0
            rb = register_to_int(parts[1])  # destination
            imm16 = resolve_imm(parts[2], costants, 16)
        else:
            rb = register_to_int(parts[1])  # destination
            ra = register_to_int(parts[2])  # source
            imm16 = resolve_imm(parts[3], costants, 16)

        return encode_i(opcode, ra, rb, imm16)
    elif op.type == "M":
        # I-Type with the memory syntax
        # Syntax    OP rb, Imm16(ra)
        # Encoding  [OP] [RA] [RB] [Imm16]
        rb = register_to_int(parts[1])      # loaded / stored register
        imm16 = resolve_imm(parts[2], costants, 16)
        ra = register_to_int(parts[3])      # base address register

        return encode_i(opcode, ra, rb, imm16)
    elif op.type == "J":
        # Syntax    OP Imm26, RFE takes no operand
        # Encoding  [OP] [Imm26]
        if mnemonic in ["RFE"]:
            imm26 = 0
        elif mnemonic in ["INT"]:
            # An interrupt code, not an address: never PC relative
            imm26 = resolve_imm(parts[1], costants, 26)
        else:
            imm26 = resolve_disp(parts[1], costants, 26, instr.address)

        return encode_j(opcode, imm26)

    return 0


def parse(lines: list[SourceLine]):
    instructions: list[Instruction] = []
    costants: CostantTable = {}
    i_address = 0

    for (line, file_path, line_num) in lines:
        parts = line.split(':', 1)
        if len(parts) > 1:
            costant_name = parts[0]

            if not costant_name:
                raise ParseException(
                    f"Costant name cannot be blank at line {line_num} of {file_path}")
            if ' ' in costant_name:
                raise ParseException(
                    f"Costant name cannot include spaces at line {line_num} of {file_path}")

            costant_name = parts[0].upper()

            if costant_name in costants:
                raise ParseException(
                    f"Costant {costant_name} declared more than one time" +
                    f" at line {line_num} of {file_path}")

            costants[costant_name] = i_address

            if not parts[1].strip():
                continue
            line = parts[1].strip()

        # Check for directives
        parts = line.split()
        if parts[0] == ".equ":
            if len(parts) != 3:
                raise ParseException(
                    ".equ requires a costant name and a value" +
                    f" (syntax .equ NAME VALUE) at line {line_num} of {file_path}")

            costant_name = parts[1].upper()
            if costant_name in costants:
                raise ParseException(
                    f"Costant {costant_name} declared more than one time" +
                    f" at line {line_num} of {file_path}")

            try:
                costants[costant_name] = int(parts[2], 0)
            except ValueError:
                raise ParseException(
                    f"Invalid value '{parts[2]}' for costant {costant_name}" +
                    f" at line {line_num} of {file_path}")
            continue

        instructions.append(
            Instruction(source_line=SourceLine(text=line, file_path=file_path,
                        line_num=line_num), address=i_address)
        )
        i_address += 4

    return instructions, costants


def open_file(path: str, included_files: set[str]) -> list[SourceLine]:
    # Guard to prevent recursion in includes
    realpath = os.path.realpath(path)
    if realpath in included_files:
        return []
    included_files.add(realpath)

    lines = []
    with open(path, 'r') as f:
        lines = f.readlines()

    clean_lines: list[SourceLine] = []
    # Remove comments and blank lines
    for line_num, line in enumerate(lines, 1):
        clean_line = line.split(';')[0].strip()
        if not clean_line:
            continue

        # Manage include directives
        parts = clean_line.split(maxsplit=1)
        if parts[0] == ".include":
            arg = parts[1] if len(parts) == 2 else ""

            if len(arg) < 3 or arg[0] != '"' or arg[-1] != '"':
                raise ParseException(
                    ".include requires the path to the file to" +
                    f" include (syntax .include \"PATH\") at line {line_num}" +
                    f" of {path}")
            # Include the file
            include_path = os.path.join(os.path.dirname(path), arg[1:-1])
            try:
                clean_lines += open_file(include_path, included_files)
            except OSError as e:
                raise ParseException(
                    f"Cannot include {include_path} ({e.strerror})" +
                    f" at line {line_num} of {path}")
        else:
            clean_lines.append(
                SourceLine(text=clean_line, file_path=path, line_num=line_num)
            )

    return clean_lines


def main():
    if len(sys.argv) != 3:
        print("Usage: python asm.py input.asm output.bin")
        sys.exit(1)

    included_files: set[str] = set()
    try:
        # Include the main program file
        input_lines = open_file(sys.argv[1], included_files)
    except OSError as e:
        print(f"ASSEMBLER ERROR: cannot open {sys.argv[1]} ({e.strerror})")
        exit(1)
    except ParseException as e:
        print(f"ASSEMBLER ERROR: {e}")
        exit(1)

    try:
        instructions, costants = parse(input_lines)
    except ParseException as e:
        print(f"ASSEMBLER ERROR: {e}")
        exit(1)

    with open(sys.argv[2], 'wb') as f:
        for instr in instructions:
            try:
                val = assemble_instr(instr, costants)
            except ParseException as e:
                print(f"ASSEMBLER ERROR in {instr.source_line.file_path}" +
                      f" at line {instr.source_line.line_num}:")
                print(f"    {instr.source_line.text}")
                print(f"    -> {e}")
                exit(1)
            except Exception as e:
                print(f"CRITICAL ERROR in {instr.source_line.file_path}" +
                      f" at line {instr.source_line.line_num}:")
                print(f"    {instr.source_line.text}")
                print(f"    -> {e}")
                exit(1)
            packed_bytes = struct.pack('>I', val)
            f.write(packed_bytes)


if __name__ == '__main__':
    main()
