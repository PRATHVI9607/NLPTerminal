import tkinter as tk
from tkinter import font, scrolledtext, messagebox
import os
import sys
from backend_comm import BackendManager
from nlp_translator import NLPCommandTranslator

class TerminalApp:
    def __init__(self, root):
        self.root = root
        self.root.title("NLP Terminal - Advanced Linux-like Shell")
        self.root.geometry("1200x800")
        self.root.configure(bg="#0a0e27")
        
        # Command history for Up/Down arrow keys
        self.command_history = []
        self.history_index = -1
        self.current_input = ""
        
        # Tab completion state
        self.tab_completion_options = []
        self.tab_completion_index = 0
        
        # Create menu bar
        self.create_menu()
        
        # Create toolbar
        self.create_toolbar()
        
        # Custom fonts
        self.custom_font = font.Font(family="Consolas", size=11)
        self.bold_font = font.Font(family="Consolas", size=11, weight="bold")
        
        # Main terminal frame with border
        terminal_frame = tk.Frame(self.root, bg="#1a1f3a", bd=2, relief=tk.SUNKEN)
        terminal_frame.pack(expand=True, fill='both', padx=10, pady=5)
        
        # Text widget for terminal output
        self.text_area = tk.Text(
            terminal_frame,
            bg="#0d1117",
            fg="#c9d1d9",
            insertbackground="#58a6ff",
            insertwidth=2,
            font=self.custom_font,
            bd=0,
            wrap=tk.WORD,
            selectbackground="#264f78",
            selectforeground="#ffffff",
            padx=10,
            pady=10
        )
        
        # Scrollbar
        scrollbar = tk.Scrollbar(terminal_frame, command=self.text_area.yview)
        self.text_area.configure(yscrollcommand=scrollbar.set)
        
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.text_area.pack(expand=True, fill='both')
        
        # Status bar
        self.create_status_bar()
        
        # Configure text tags for syntax highlighting
        self.text_area.tag_config("prompt", foreground="#58a6ff", font=self.bold_font)
        self.text_area.tag_config("command", foreground="#ffffff")
        self.text_area.tag_config("output", foreground="#c9d1d9")
        self.text_area.tag_config("error", foreground="#f85149", font=self.bold_font)
        self.text_area.tag_config("success", foreground="#3fb950")
        self.text_area.tag_config("warning", foreground="#d29922")
        self.text_area.tag_config("info", foreground="#79c0ff")
        self.text_area.tag_config("suggestion", foreground="#8b949e", font=("Consolas", 10, "italic"))
        self.text_area.tag_config("nlp", foreground="#d2a8ff", font=("Consolas", 10, "italic"))
        self.text_area.tag_config("header", foreground="#58a6ff", font=("Consolas", 12, "bold"))
        
        # Backend setup
        shell_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../backend/mysh"))
        if not os.path.exists(shell_path):
            shell_path += ".exe"
        
        self.backend = BackendManager(shell_path)
        try:
            self.backend.start()
            self.update_status("Backend connected", "success")
        except Exception as e:
            self.text_area.insert(tk.END, f"Error starting backend: {e}\n", "error")
            self.update_status("Backend failed", "error")
        
        self.prompt = "shell> "
        self.prompt_end_index = "1.0"
        
        # NLP Translator
        self.nlp = NLPCommandTranslator()
        
        # Key bindings
        self.setup_key_bindings()
        
        # Focus on text area
        self.text_area.focus_set()
        
        # Start output polling
        self.check_backend_output()
        
        # Show welcome message
        self.show_welcome()
    
    def create_menu(self):
        menubar = tk.Menu(self.root, bg="#1c2128", fg="#c9d1d9", activebackground="#2d333b", 
                         activeforeground="#ffffff")
        self.root.config(menu=menubar)
        
        # File menu
        file_menu = tk.Menu(menubar, tearoff=0, bg="#1c2128", fg="#c9d1d9")
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="New Terminal", command=self.new_terminal, accelerator="Ctrl+N")
        file_menu.add_command(label="Clear Screen", command=self.clear_screen, accelerator="Ctrl+L")
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit, accelerator="Ctrl+Q")
        
        # Edit menu
        edit_menu = tk.Menu(menubar, tearoff=0, bg="#1c2128", fg="#c9d1d9")
        menubar.add_cascade(label="Edit", menu=edit_menu)
        edit_menu.add_command(label="Copy", command=self.copy_text, accelerator="Ctrl+C")
        edit_menu.add_command(label="Paste", command=self.paste_text, accelerator="Ctrl+V")
        edit_menu.add_command(label="Select All", command=self.select_all, accelerator="Ctrl+A")
        
        # View menu
        view_menu = tk.Menu(menubar, tearoff=0, bg="#1c2128", fg="#c9d1d9")
        menubar.add_cascade(label="View", menu=view_menu)
        view_menu.add_command(label="Zoom In", command=self.zoom_in, accelerator="Ctrl++")
        view_menu.add_command(label="Zoom Out", command=self.zoom_out, accelerator="Ctrl+-")
        view_menu.add_command(label="Reset Zoom", command=self.reset_zoom, accelerator="Ctrl+0")
        
        # Help menu
        help_menu = tk.Menu(menubar, tearoff=0, bg="#1c2128", fg="#c9d1d9")
        menubar.add_cascade(label="Help", menu=help_menu)
        help_menu.add_command(label="Shortcuts", command=self.show_shortcuts)
        help_menu.add_command(label="About", command=self.show_about)
    
    def create_toolbar(self):
        toolbar = tk.Frame(self.root, bg="#161b22", height=35)
        toolbar.pack(fill=tk.X, padx=5, pady=2)
        
        btn_style = {
            "bg": "#21262d", "fg": "#c9d1d9", "activebackground": "#30363d",
            "activeforeground": "#ffffff", "bd": 0, "padx": 15, "pady": 5,
            "font": ("Segoe UI", 9), "cursor": "hand2"
        }
        
        tk.Button(toolbar, text="🗑️ Clear", command=self.clear_screen, **btn_style).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="📊 System Monitor", command=self.run_sysmon, **btn_style).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="📂 Files", command=self.list_files, **btn_style).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="ℹ️ Help", command=lambda: self.send_command_direct("help"), **btn_style).pack(side=tk.LEFT, padx=2)
        
        # Right side info
        info_frame = tk.Frame(toolbar, bg="#161b22")
        info_frame.pack(side=tk.RIGHT)
        
        self.cwd_label = tk.Label(info_frame, text="📁 ~/", bg="#161b22", fg="#8b949e", 
                                   font=("Consolas", 9))
        self.cwd_label.pack(side=tk.LEFT, padx=10)
    
    def create_status_bar(self):
        self.status_bar = tk.Frame(self.root, bg="#0d1117", height=25)
        self.status_bar.pack(fill=tk.X, side=tk.BOTTOM)
        
        self.status_label = tk.Label(self.status_bar, text="Ready", bg="#0d1117", 
                                      fg="#58a6ff", anchor=tk.W, padx=10,
                                      font=("Consolas", 9))
        self.status_label.pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        self.position_label = tk.Label(self.status_bar, text="Ln 1, Col 1", bg="#0d1117",
                                        fg="#8b949e", padx=10, font=("Consolas", 9))
        self.position_label.pack(side=tk.RIGHT)
    
    def setup_key_bindings(self):
        # Enter key
        self.text_area.bind("<Return>", self.handle_enter)
        self.text_area.bind("<KP_Enter>", self.handle_enter)
        
        # Backspace
        self.text_area.bind("<BackSpace>", self.handle_backspace)
        
        # Arrow keys for history
        self.text_area.bind("<Up>", self.history_up)
        self.text_area.bind("<Down>", self.history_down)
        
        # Tab completion
        self.text_area.bind("<Tab>", self.handle_tab)
        
        # Ctrl+C - Copy or interrupt
        self.text_area.bind("<Control-c>", self.handle_ctrl_c)
        
        # Ctrl+L - Clear screen
        self.text_area.bind("<Control-l>", lambda e: self.clear_screen())
        
        # Ctrl+D - Exit
        self.text_area.bind("<Control-d>", lambda e: self.send_command_direct("exit"))
        
        # Ctrl+A - Select all or beginning of line
        self.text_area.bind("<Control-a>", self.handle_ctrl_a)
        
        # Ctrl+E - End of line
        self.text_area.bind("<Control-e>", self.handle_ctrl_e)
        
        # Ctrl+U - Clear line
        self.text_area.bind("<Control-u>", self.clear_line)
        
        # Ctrl+K - Kill line from cursor
        self.text_area.bind("<Control-k>", self.kill_line)
        
        # Ctrl+W - Delete word backwards
        self.text_area.bind("<Control-w>", self.delete_word)
        
        # Copy/Paste with Ctrl+Shift+C/V
        self.text_area.bind("<Control-Shift-C>", lambda e: self.copy_text())
        self.text_area.bind("<Control-Shift-V>", lambda e: self.paste_text())
        
        # Prevent editing before prompt
        self.text_area.bind("<Key>", self.handle_keypress, add="+")
        
        # Update cursor position
        self.text_area.bind("<KeyRelease>", self.update_cursor_position, add="+")
        self.text_area.bind("<ButtonRelease-1>", self.update_cursor_position, add="+")
        
        # Zoom
        self.text_area.bind("<Control-plus>", lambda e: self.zoom_in())
        self.text_area.bind("<Control-equal>", lambda e: self.zoom_in())
        self.text_area.bind("<Control-minus>", lambda e: self.zoom_out())
        self.text_area.bind("<Control-0>", lambda e: self.reset_zoom())
    
    def show_welcome(self):
        welcome = """
╔══════════════════════════════════════════════════════════════════════╗
║           Welcome to NLP Terminal - Advanced Linux Shell             ║
║                                                                      ║
║  Type 'help' for available commands                                  ║
║  Type 'sysmon' for system resource monitor                          ║
║                                                                      ║
║  Shortcuts: Ctrl+L (clear), Ctrl+C (copy), Up/Down (history)       ║
╚══════════════════════════════════════════════════════════════════════╝

"""
        self.text_area.insert(tk.END, welcome, "header")
        self.text_area.see(tk.END)

    
    def check_backend_output(self):
        while True:
            item = self.backend.get_output()
            if item is None:
                break
            
            type, content = item
            if type == "OUTPUT":
                self.text_area.insert(tk.END, content, "output")
                self.text_area.see(tk.END)
            elif type == "PROMPT":
                self.prompt = content
                self.write_prompt()
                # Update current directory in status
                if ">" in content:
                    dir_part = content.split(">")[0].strip()
                    self.cwd_label.config(text=f"📁 {dir_part}")
            elif type == "ERROR":
                self.text_area.insert(tk.END, f"Error: {content}\n", "error")
        
        self.root.after(50, self.check_backend_output)

    def write_prompt(self):
        if not self.text_area.get("end-2c", "end-1c").endswith("\n"):
            self.text_area.insert(tk.END, "\n")
             
        self.text_area.insert(tk.END, self.prompt, "prompt")
        self.text_area.see(tk.END)
        self.prompt_end_index = self.text_area.index(tk.END + "-1c")
        self.text_area.mark_set("insert", tk.END)
        self.update_status("Ready", "success")

    def get_current_command(self):
        return self.text_area.get(self.prompt_end_index, "end-1c").strip()
    
    def set_current_command(self, text):
        self.text_area.delete(self.prompt_end_index, "end-1c")
        self.text_area.insert(self.prompt_end_index, text)

    def translate_command(self, input_text):
        return self.nlp.translate(input_text)

    def handle_enter(self, event):
        command_text = self.get_current_command()
        self.text_area.insert(tk.END, "\n")
        
        if command_text:
            # Add to history
            if not self.command_history or self.command_history[-1] != command_text:
                self.command_history.append(command_text)
            self.history_index = len(self.command_history)
            
            # NLP Translation
            translated = self.translate_command(command_text)
            if translated != command_text:
                self.text_area.insert(tk.END, f"[NLP] Interpreted as: {translated}\n", "nlp")
                self.backend.send_command(translated)
            else:
                self.backend.send_command(command_text)
            
            self.update_status(f"Executing: {command_text}", "info")
        else:
            self.backend.send_command("") 

        return "break"
    
    def history_up(self, event):
        if not self.command_history:
            return "break"
        
        if self.history_index > 0:
            if self.history_index == len(self.command_history):
                self.current_input = self.get_current_command()
            self.history_index -= 1
            self.set_current_command(self.command_history[self.history_index])
        
        return "break"
    
    def history_down(self, event):
        if not self.command_history:
            return "break"
        
        if self.history_index < len(self.command_history):
            self.history_index += 1
            if self.history_index == len(self.command_history):
                self.set_current_command(self.current_input)
            else:
                self.set_current_command(self.command_history[self.history_index])
        
        return "break"
    
    def handle_tab(self, event):
        current_cmd = self.get_current_command()
        
        # Simple tab completion for common commands
        commands = ["ls", "cd", "pwd", "mkdir", "rmdir", "cat", "echo", "cp", "mv", "rm",
                   "touch", "help", "history", "exit", "sysmon", "tree", "search", "backup",
                   "compare", "stats"]
        
        if not current_cmd:
            return "break"
        
        if not hasattr(self, '_last_tab_cmd') or self._last_tab_cmd != current_cmd:
            self.tab_completion_options = [cmd for cmd in commands if cmd.startswith(current_cmd)]
            self.tab_completion_index = 0
            self._last_tab_cmd = current_cmd
        
        if self.tab_completion_options:
            self.set_current_command(self.tab_completion_options[self.tab_completion_index])
            self.tab_completion_index = (self.tab_completion_index + 1) % len(self.tab_completion_options)
        
        return "break"
    
    def handle_ctrl_c(self, event):
        # If text is selected, copy it
        try:
            selected = self.text_area.selection_get()
            if selected:
                self.root.clipboard_clear()
                self.root.clipboard_append(selected)
                self.update_status("Copied to clipboard", "success")
                return "break"
        except tk.TclError:
            pass
        
        # Otherwise, send interrupt signal (clear current line)
        self.set_current_command("")
        self.update_status("Command interrupted", "warning")
        return "break"
    
    def handle_ctrl_a(self, event):
        # Move cursor to beginning of current command
        self.text_area.mark_set("insert", self.prompt_end_index)
        return "break"
    
    def handle_ctrl_e(self, event):
        # Move cursor to end of line
        self.text_area.mark_set("insert", "end-1c")
        return "break"
    
    def clear_line(self, event):
        # Clear from cursor to end of line (Ctrl+U)
        self.set_current_command("")
        return "break"
    
    def kill_line(self, event):
        # Kill from cursor to end (Ctrl+K)
        self.text_area.delete("insert", "end-1c")
        return "break"
    
    def delete_word(self, event):
        # Delete word backwards (Ctrl+W)
        current_pos = self.text_area.index("insert")
        line_start = self.prompt_end_index
        
        if self.text_area.compare(current_pos, "<=", line_start):
            return "break"
        
        # Find previous word boundary
        text = self.text_area.get(line_start, current_pos)
        words = text.split()
        if words:
            words.pop()
            self.set_current_command(" ".join(words) + (" " if words else ""))
        
        return "break"

    def handle_backspace(self, event):
        if self.text_area.compare("insert", "<=", self.prompt_end_index):
            return "break"
        return None

    def handle_keypress(self, event):
        # Prevent editing before prompt
        if self.text_area.compare("insert", "<", self.prompt_end_index):
            if event.keysym not in ("Left", "Right", "Up", "Down", "Home", "End"):
                self.text_area.mark_set("insert", "end-1c")
        return None
    
    def update_cursor_position(self, event=None):
        cursor_pos = self.text_area.index("insert")
        line, col = cursor_pos.split(".")
        self.position_label.config(text=f"Ln {line}, Col {col}")
    
    def update_status(self, message, status_type="info"):
        color_map = {
            "success": "#3fb950",
            "error": "#f85149",
            "warning": "#d29922",
            "info": "#58a6ff"
        }
        self.status_label.config(text=message, fg=color_map.get(status_type, "#58a6ff"))
    
    # Menu and toolbar commands
    def new_terminal(self):
        # Launch new instance
        import subprocess
        subprocess.Popen([sys.executable, __file__])
    
    def clear_screen(self):
        self.text_area.delete("1.0", tk.END)
        self.show_welcome()
        self.write_prompt()
        self.update_status("Screen cleared", "success")
    
    def copy_text(self):
        try:
            selected = self.text_area.selection_get()
            self.root.clipboard_clear()
            self.root.clipboard_append(selected)
            self.update_status("Copied to clipboard", "success")
        except tk.TclError:
            pass
    
    def paste_text(self):
        try:
            clipboard_text = self.root.clipboard_get()
            self.text_area.insert("insert", clipboard_text)
            self.update_status("Pasted from clipboard", "success")
        except tk.TclError:
            pass
    
    def select_all(self):
        self.text_area.tag_add("sel", "1.0", "end")
    
    def zoom_in(self):
        current_size = self.custom_font.actual()['size']
        if current_size < 24:
            new_size = current_size + 1
            self.custom_font.configure(size=new_size)
            self.bold_font.configure(size=new_size)
            self.update_status(f"Zoom: {new_size}pt", "info")
    
    def zoom_out(self):
        current_size = self.custom_font.actual()['size']
        if current_size > 8:
            new_size = current_size - 1
            self.custom_font.configure(size=new_size)
            self.bold_font.configure(size=new_size)
            self.update_status(f"Zoom: {new_size}pt", "info")
    
    def reset_zoom(self):
        self.custom_font.configure(size=11)
        self.bold_font.configure(size=11)
        self.update_status("Zoom reset", "info")
    
    def run_sysmon(self):
        self.send_command_direct("sysmon")
    
    def list_files(self):
        self.send_command_direct("ls")
    
    def send_command_direct(self, command):
        self.set_current_command(command)
        self.handle_enter(None)
    
    def show_shortcuts(self):
        shortcuts = """
Keyboard Shortcuts:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Navigation:
  Up/Down Arrow    - Navigate command history
  Tab              - Command completion
  Ctrl+A           - Beginning of line
  Ctrl+E           - End of line
  
Editing:
  Ctrl+U           - Clear line
  Ctrl+K           - Kill to end of line
  Ctrl+W           - Delete word backwards
  Backspace        - Delete character
  
Clipboard:
  Ctrl+C           - Copy selection / Interrupt
  Ctrl+Shift+C     - Copy
  Ctrl+Shift+V     - Paste
  
View:
  Ctrl+L           - Clear screen
  Ctrl++           - Zoom in
  Ctrl+-           - Zoom out
  Ctrl+0           - Reset zoom
  
System:
  Ctrl+D           - Exit
  Ctrl+N           - New terminal window
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
"""
        messagebox.showinfo("Keyboard Shortcuts", shortcuts)
    
    def show_about(self):
        about = """
NLP Terminal v2.0
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Advanced Linux-like Shell with:
  • Natural Language Processing
  • System Resource Monitoring
  • Full keyboard shortcut support
  • Command history and completion
  • Beautiful UI with syntax highlighting

Developed with ❤️
"""
        messagebox.showinfo("About NLP Terminal", about)if __name__ == "__main__":
    root = tk.Tk()
    app = TerminalApp(root)
    root.mainloop()
