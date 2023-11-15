from CPU import CPU
from GPU import Display

cpu = CPU()

cpu.db(0, "Hello, World!")
cpu.mov("A", "0")
cpu.mov("B", "13")
cpu.int("0x01")


cpu.screen.screen.getch()

cpu.screen.close()