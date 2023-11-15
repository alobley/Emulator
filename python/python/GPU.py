import curses

class Display:
    def __init__(self, rows, cols):
        self.screen = curses.initscr()
        self.screen = curses.newwin(rows, cols, 0, 0)
        curses.curs_set(0)  # Hide the cursor
        self.screen.clear()
        self.screen.refresh()
        

    def print_text(self, text, row, col):
        self.screen.addstr(row, col, text)
        self.screen.refresh()

    def close(self):
        curses.endwin()

        