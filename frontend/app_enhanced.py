"""
NLP Terminal - Enhanced Python Frontend (UI Only)
All command execution is done in C backend
This file handles ONLY the graphical user interface using tkinter
"""

import tkinter as tk
from tkinter import font, messagebox, ttk
import subprocess
import threading
import queue
import os
import sys
import re

class BackendManager:
    """Manages communication with the C backend shell"""
    
    def __init__(self, shell_path):
        self.shell_path = shell_path
        self.process = None
        self.output_queue = queue.Queue()
        self.running = False
        self.reader_thread = None

    def start(self):
        if not os.path.exists(self.shell_path):
            raise FileNotFoundError(f"Shell not found: {self.shell_path}")

        self.process = subprocess.Popen(
            [self.shell_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=0
        )
        self.running = True
        
        self.reader_thread = threading.Thread(target=self._read_output, daemon=True)
        self.reader_thread.start()

    def _read_output(self):
        """Read output from backend character by character"""
        buffer = ""
        while self.running and self.process.poll() is None:
            try:
                char = self.process.stdout.read(1)
                if not char:
                    break
                
                buffer += char
                
                # Check for prompt (ends with "> ")
                if buffer.endswith("> "):
                    self.output_queue.put(("PROMPT", buffer))
                    buffer = ""
                elif char == '\n':
                    # Check for special messages
                    if buffer.startswith("NLP_TRANSLATED:"):
                        parts = buffer.strip().split(":", 2)
                        if len(parts) >= 3:
                            self.output_queue.put(("NLP", (parts[1], parts[2])))
                    elif buffer.startswith("SUGGESTIONS:"):
                        suggestions = buffer.strip()[12:].split("|")
                        self.output_queue.put(("SUGGESTIONS", suggestions))
                    else:
                        self.output_queue.put(("OUTPUT", buffer))
                    buffer = ""
            except Exception as e:
                self.output_queue.put(("ERROR", str(e)))
                break
        
        if buffer:
            self.output_queue.put(("OUTPUT", buffer))

    def send_command(self, command):
        if self.process and self.process.stdin:
            try:
                self.process.stdin.write(command + "\n")
                self.process.stdin.flush()
            except Exception as e:
                print(f"Error sending command: {e}")

    def request_suggestions(self, partial):
        """Request suggestions from C backend"""
        self.send_command(f"SUGGEST:{partial}")

    def get_output(self):
        try:
            return self.output_queue.get_nowait()
        except queue.Empty:
            return None

    def stop(self):
        self.running = False
        if self.process:
            self.process.terminate()


class SuggestionPopup:
    """Intellisense-style suggestion popup"""
    
    def __init__(self, parent, text_widget):
        self.parent = parent
        self.text_widget = text_widget
        self.popup = None
        self.listbox = None
        self.suggestions = []
        self.selected_index = 0
        self.visible = False
        
    def show(self, suggestions, x, y):
        """Show suggestion popup at given position"""
        if not suggestions:
            self.hide()
            return
            
        self.suggestions = suggestions
        self.selected_index = 0
        
        if self.popup:
            self.hide()
        
        self.popup = tk.Toplevel(self.parent)
        self.popup.wm_overrideredirect(True)
        self.popup.wm_geometry(f"+{x}+{y}")
        self.popup.configure(bg="#1e1e1e")
        
        # Create frame with border
        frame = tk.Frame(self.popup, bg="#3c3c3c", bd=1, relief=tk.SOLID)
        frame.pack(fill=tk.BOTH, expand=True)
        
        # Listbox for suggestions
        self.listbox = tk.Listbox(
            frame,
            bg="#252526",
            fg="#cccccc",
            selectbackground="#094771",
            selectforeground="#ffffff",
            font=("Consolas", 10),
            bd=0,
            highlightthickness=0,
            activestyle='none',
            height=min(len(suggestions), 8),
            width=30
        )
        self.listbox.pack(fill=tk.BOTH, expand=True, padx=1, pady=1)
        
        for suggestion in suggestions:
            self.listbox.insert(tk.END, f"  {suggestion}")
        
        self.listbox.selection_set(0)
        self.visible = True
        
        # Bindings
        self.listbox.bind('<Double-Button-1>', self._on_select)
        
    def hide(self):
        """Hide the popup"""
        if self.popup:
            self.popup.destroy()
            self.popup = None
            self.listbox = None
        self.visible = False
        self.suggestions = []
        
    def move_selection(self, delta):
        """Move selection up or down"""
        if not self.visible or not self.listbox:
            return
            
        new_index = self.selected_index + delta
        if 0 <= new_index < len(self.suggestions):
            self.listbox.selection_clear(self.selected_index)
            self.selected_index = new_index
            self.listbox.selection_set(self.selected_index)
            self.listbox.see(self.selected_index)
            
    def get_selected(self):
        """Get currently selected suggestion"""
        if self.suggestions and 0 <= self.selected_index < len(self.suggestions):
            return self.suggestions[self.selected_index]
        return None
        
    def _on_select(self, event):
        """Handle double-click selection"""
        if self.listbox:
            selection = self.listbox.curselection()
            if selection:
                self.selected_index = selection[0]


class NLPTerminalApp:
    """Main Terminal Application - UI Only"""
    
    def __init__(self, root):
        self.root = root
        self.root.title("NLP Terminal - Advanced Shell")
        self.root.geometry("1300x900")
        self.root.configure(bg="#1e1e1e")
        
        # State variables
        self.command_history = []
        self.history_index = -1
        self.current_input = ""
        self.font_size = 11
        self.ghost_text = ""
        
        # Create UI components
        self.create_menu()
        self.create_toolbar()
        self.create_terminal()
        self.create_status_bar()
        
        # Backend setup
        shell_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../backend/mysh"))
        if sys.platform == "win32" and not shell_path.endswith(".exe"):
            shell_path += ".exe"
        
        self.backend = BackendManager(shell_path)
        try:
            self.backend.start()
            self.update_status("Connected to backend", "success")
        except Exception as e:
            self.text_area.insert(tk.END, f"Error starting backend: {e}\n", "error")
            self.update_status("Backend failed", "error")
        
        self.prompt = "shell> "
        self.prompt_end_index = "1.0"
        
        # Suggestion popup
        self.suggestion_popup = SuggestionPopup(self.root, self.text_area)
        
        # Key bindings
        self.setup_key_bindings()
        
        # Start output polling
        self.check_backend_output()
        
        # Show welcome
        self.show_welcome()
        
        # Focus
        self.text_area.focus_set()
        
    def create_menu(self):
        """Create menu bar"""
        menubar = tk.Menu(self.root, bg="#252526", fg="#cccccc", 
                         activebackground="#094771", activeforeground="#ffffff")
        self.root.config(menu=menubar)
        
        # File menu
        file_menu = tk.Menu(menubar, tearoff=0, bg="#252526", fg="#cccccc")
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="New Terminal", command=self.new_terminal, accelerator="Ctrl+Shift+N")
        file_menu.add_command(label="Clear Screen", command=self.clear_screen, accelerator="Ctrl+L")
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit, accelerator="Ctrl+Q")
        
        # Edit menu
        edit_menu = tk.Menu(menubar, tearoff=0, bg="#252526", fg="#cccccc")
        menubar.add_cascade(label="Edit", menu=edit_menu)
        edit_menu.add_command(label="Copy", command=self.copy_text, accelerator="Ctrl+C")
        edit_menu.add_command(label="Paste", command=self.paste_text, accelerator="Ctrl+V")
        edit_menu.add_command(label="Select All", command=self.select_all, accelerator="Ctrl+A")
        
        # View menu
        view_menu = tk.Menu(menubar, tearoff=0, bg="#252526", fg="#cccccc")
        menubar.add_cascade(label="View", menu=view_menu)
        view_menu.add_command(label="Zoom In", command=self.zoom_in, accelerator="Ctrl++")
        view_menu.add_command(label="Zoom Out", command=self.zoom_out, accelerator="Ctrl+-")
        view_menu.add_command(label="Reset Zoom", command=self.reset_zoom, accelerator="Ctrl+0")
        
        # Tools menu
        tools_menu = tk.Menu(menubar, tearoff=0, bg="#252526", fg="#cccccc")
        menubar.add_cascade(label="Tools", menu=tools_menu)
        tools_menu.add_command(label="System Monitor", command=lambda: self.send_command_direct("sysmon"))
        tools_menu.add_command(label="Process List", command=lambda: self.send_command_direct("ps"))
        tools_menu.add_command(label="Disk Usage", command=lambda: self.send_command_direct("df"))
        tools_menu.add_separator()
        tools_menu.add_command(label="Quick Notes", command=lambda: self.send_command_direct("quicknote list"))
        tools_menu.add_command(label="Calculator", command=self.show_calculator)
        
        # Help menu
        help_menu = tk.Menu(menubar, tearoff=0, bg="#252526", fg="#cccccc")
        menubar.add_cascade(label="Help", menu=help_menu)
        help_menu.add_command(label="Commands", command=lambda: self.send_command_direct("help"))
        help_menu.add_command(label="Keyboard Shortcuts", command=self.show_shortcuts)
        help_menu.add_separator()
        help_menu.add_command(label="About", command=self.show_about)
        
    def create_toolbar(self):
        """Create toolbar with quick actions"""
        toolbar = tk.Frame(self.root, bg="#333333", height=40)
        toolbar.pack(fill=tk.X, padx=0, pady=0)
        
        btn_style = {
            "bg": "#3c3c3c", "fg": "#cccccc",
            "activebackground": "#505050", "activeforeground": "#ffffff",
            "bd": 0, "padx": 12, "pady": 6,
            "font": ("Segoe UI", 9), "cursor": "hand2"
        }
        
        # Quick action buttons
        tk.Button(toolbar, text="📁 Files", command=lambda: self.send_command_direct("ls"), **btn_style).pack(side=tk.LEFT, padx=2, pady=4)
        tk.Button(toolbar, text="📊 SysMon", command=lambda: self.send_command_direct("sysmon"), **btn_style).pack(side=tk.LEFT, padx=2, pady=4)
        tk.Button(toolbar, text="🧹 Clear", command=self.clear_screen, **btn_style).pack(side=tk.LEFT, padx=2, pady=4)
        tk.Button(toolbar, text="📝 Notes", command=lambda: self.send_command_direct("quicknote list"), **btn_style).pack(side=tk.LEFT, padx=2, pady=4)
        tk.Button(toolbar, text="🔍 Find Dup", command=lambda: self.send_command_direct("duplicate"), **btn_style).pack(side=tk.LEFT, padx=2, pady=4)
        tk.Button(toolbar, text="❓ Help", command=lambda: self.send_command_direct("help"), **btn_style).pack(side=tk.LEFT, padx=2, pady=4)
        
        # Separator
        ttk.Separator(toolbar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=6)
        
        # Zoom controls
        tk.Button(toolbar, text="➕", command=self.zoom_in, width=3, **btn_style).pack(side=tk.LEFT, padx=2, pady=4)
        tk.Button(toolbar, text="➖", command=self.zoom_out, width=3, **btn_style).pack(side=tk.LEFT, padx=2, pady=4)
        
        # Current directory display
        self.cwd_label = tk.Label(toolbar, text="📂 ./", bg="#333333", fg="#8b949e", font=("Consolas", 9))
        self.cwd_label.pack(side=tk.RIGHT, padx=10)
        
    def create_terminal(self):
        """Create main terminal area"""
        # Terminal frame
        terminal_frame = tk.Frame(self.root, bg="#252526", bd=0)
        terminal_frame.pack(expand=True, fill=tk.BOTH, padx=8, pady=4)
        
        # Custom fonts
        self.custom_font = font.Font(family="Consolas", size=self.font_size)
        self.bold_font = font.Font(family="Consolas", size=self.font_size, weight="bold")
        self.italic_font = font.Font(family="Consolas", size=self.font_size, slant="italic")
        
        # Text widget
        self.text_area = tk.Text(
            terminal_frame,
            bg="#1e1e1e",
            fg="#d4d4d4",
            insertbackground="#aeafad",
            insertwidth=2,
            font=self.custom_font,
            bd=0,
            wrap=tk.WORD,
            selectbackground="#264f78",
            selectforeground="#ffffff",
            padx=12,
            pady=12,
            undo=True
        )
        
        # Scrollbar
        scrollbar = tk.Scrollbar(terminal_frame, command=self.text_area.yview, bg="#252526")
        self.text_area.configure(yscrollcommand=scrollbar.set)
        
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.text_area.pack(expand=True, fill=tk.BOTH)
        
        # Configure tags for syntax highlighting
        self.text_area.tag_config("prompt", foreground="#569cd6", font=self.bold_font)
        self.text_area.tag_config("command", foreground="#d4d4d4")
        self.text_area.tag_config("output", foreground="#d4d4d4")
        self.text_area.tag_config("error", foreground="#f14c4c", font=self.bold_font)
        self.text_area.tag_config("success", foreground="#4ec9b0")
        self.text_area.tag_config("warning", foreground="#dcdcaa")
        self.text_area.tag_config("info", foreground="#9cdcfe")
        self.text_area.tag_config("nlp", foreground="#c586c0", font=self.italic_font)
        self.text_area.tag_config("header", foreground="#569cd6", font=("Consolas", 12, "bold"))
        self.text_area.tag_config("ghost", foreground="#6a6a6a", font=self.italic_font)
        self.text_area.tag_config("suggestion_highlight", foreground="#ce9178")
        
    def create_status_bar(self):
        """Create status bar"""
        self.status_bar = tk.Frame(self.root, bg="#007acc", height=24)
        self.status_bar.pack(fill=tk.X, side=tk.BOTTOM)
        
        self.status_label = tk.Label(
            self.status_bar, text="Ready", bg="#007acc", fg="#ffffff",
            anchor=tk.W, padx=10, font=("Segoe UI", 9)
        )
        self.status_label.pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        # Position indicator
        self.position_label = tk.Label(
            self.status_bar, text="Ln 1, Col 1", bg="#007acc", fg="#ffffff",
            padx=10, font=("Segoe UI", 9)
        )
        self.position_label.pack(side=tk.RIGHT)
        
        # Zoom indicator
        self.zoom_label = tk.Label(
            self.status_bar, text=f"{self.font_size}pt", bg="#007acc", fg="#ffffff",
            padx=10, font=("Segoe UI", 9)
        )
        self.zoom_label.pack(side=tk.RIGHT)
        
    def setup_key_bindings(self):
        """Setup keyboard shortcuts"""
        # Enter key
        self.text_area.bind("<Return>", self.handle_enter)
        self.text_area.bind("<KP_Enter>", self.handle_enter)
        
        # Backspace
        self.text_area.bind("<BackSpace>", self.handle_backspace)
        
        # Arrow keys for history and suggestions
        self.text_area.bind("<Up>", self.handle_up)
        self.text_area.bind("<Down>", self.handle_down)
        self.text_area.bind("<Right>", self.handle_right)
        self.text_area.bind("<Left>", self.handle_left)
        
        # Tab completion
        self.text_area.bind("<Tab>", self.handle_tab)
        
        # Ctrl combinations
        self.text_area.bind("<Control-c>", self.handle_ctrl_c)
        self.text_area.bind("<Control-l>", lambda e: self.clear_screen())
        self.text_area.bind("<Control-d>", lambda e: self.send_command_direct("exit"))
        self.text_area.bind("<Control-a>", self.handle_ctrl_a)
        self.text_area.bind("<Control-e>", self.handle_ctrl_e)
        self.text_area.bind("<Control-u>", self.clear_line)
        self.text_area.bind("<Control-k>", self.kill_line)
        self.text_area.bind("<Control-w>", self.delete_word)
        
        # Copy/Paste
        self.text_area.bind("<Control-Shift-C>", lambda e: self.copy_text())
        self.text_area.bind("<Control-Shift-V>", lambda e: self.paste_text())
        
        # Zoom shortcuts
        self.text_area.bind("<Control-plus>", lambda e: self.zoom_in())
        self.text_area.bind("<Control-equal>", lambda e: self.zoom_in())
        self.text_area.bind("<Control-minus>", lambda e: self.zoom_out())
        self.text_area.bind("<Control-0>", lambda e: self.reset_zoom())
        
        # Mouse wheel zoom
        self.text_area.bind("<Control-MouseWheel>", self.handle_mouse_zoom)
        
        # Key press for suggestions
        self.text_area.bind("<KeyRelease>", self.on_key_release)
        
        # Escape to hide suggestions
        self.text_area.bind("<Escape>", lambda e: self.suggestion_popup.hide())
        
        # Prevent editing before prompt
        self.text_area.bind("<Key>", self.handle_keypress, add="+")
        
        # Update cursor position
        self.text_area.bind("<ButtonRelease-1>", self.update_cursor_position)
        
    def show_welcome(self):
        """Show welcome message"""
        welcome = """
╔══════════════════════════════════════════════════════════════════════════════╗
║                                                                              ║
║                    🚀 NLP TERMINAL - Advanced Shell 🚀                       ║
║                                                                              ║
║  Features:                                                                   ║
║    • Natural Language Commands - Just describe what you want!                ║
║    • Intellisense Suggestions - Start typing for smart completions           ║
║    • Command History - Up/Down arrows to navigate                            ║
║    • Right Arrow - Auto-complete with first suggestion                       ║
║    • Unique Commands - fileinfo, hexdump, duplicate, calc, quicknote...      ║
║    • System Monitor - Real-time CPU, Memory, Disk, Network info              ║
║                                                                              ║
║  Quick Start:                                                                ║
║    • Type 'help' for all commands                                            ║
║    • Try: "show all files" or "create folder called test"                    ║
║    • Press Tab for command completion                                        ║
║    • Ctrl+L to clear, Ctrl+C to copy/interrupt                              ║
║                                                                              ║
╚══════════════════════════════════════════════════════════════════════════════╝

"""
        self.text_area.insert(tk.END, welcome, "header")
        self.text_area.see(tk.END)
        
    def check_backend_output(self):
        """Poll backend for output"""
        while True:
            item = self.backend.get_output()
            if item is None:
                break
            
            msg_type, content = item
            
            if msg_type == "OUTPUT":
                self.text_area.insert(tk.END, content, "output")
                self.text_area.see(tk.END)
            elif msg_type == "PROMPT":
                self.prompt = content
                self.write_prompt()
                # Update CWD display
                if ">" in content:
                    cwd = content.split(">")[0].strip()
                    self.cwd_label.config(text=f"📂 {cwd}")
            elif msg_type == "NLP":
                cmd, explanation = content
                self.text_area.insert(tk.END, f"[NLP] → {cmd} ({explanation})\n", "nlp")
            elif msg_type == "SUGGESTIONS":
                self.show_suggestion_popup(content)
            elif msg_type == "ERROR":
                self.text_area.insert(tk.END, f"Error: {content}\n", "error")
        
        self.root.after(50, self.check_backend_output)
        
    def write_prompt(self):
        """Write prompt to terminal"""
        # Clear any ghost text first
        self.clear_ghost_text()
        
        if not self.text_area.get("end-2c", "end-1c").endswith("\n"):
            self.text_area.insert(tk.END, "\n")
        
        self.text_area.insert(tk.END, self.prompt, "prompt")
        self.text_area.see(tk.END)
        self.prompt_end_index = self.text_area.index(tk.END + "-1c")
        self.text_area.mark_set("insert", tk.END)
        self.update_status("Ready", "success")
        
    def get_current_command(self):
        """Get current command text"""
        return self.text_area.get(self.prompt_end_index, "end-1c").strip()
    
    def set_current_command(self, text):
        """Set current command text"""
        self.clear_ghost_text()
        self.text_area.delete(self.prompt_end_index, "end-1c")
        self.text_area.insert(self.prompt_end_index, text)
        self.text_area.mark_set("insert", tk.END)
        
    def clear_ghost_text(self):
        """Clear ghost/suggestion text"""
        # Remove ghost tag ranges
        ranges = self.text_area.tag_ranges("ghost")
        if ranges:
            for i in range(0, len(ranges), 2):
                self.text_area.delete(ranges[i], ranges[i+1])
        self.ghost_text = ""
        
    def show_ghost_suggestion(self, suggestion):
        """Show ghost text suggestion inline"""
        self.clear_ghost_text()
        
        current = self.get_current_command()
        if suggestion and suggestion.startswith(current) and len(suggestion) > len(current):
            ghost_part = suggestion[len(current):]
            self.ghost_text = ghost_part
            
            cursor_pos = self.text_area.index("insert")
            self.text_area.insert(cursor_pos, ghost_part, "ghost")
            self.text_area.mark_set("insert", cursor_pos)
            
    def show_suggestion_popup(self, suggestions):
        """Show suggestion popup"""
        if not suggestions or (len(suggestions) == 1 and suggestions[0] == ""):
            self.suggestion_popup.hide()
            return
        
        # Filter empty suggestions
        suggestions = [s for s in suggestions if s.strip()]
        if not suggestions:
            self.suggestion_popup.hide()
            return
        
        # Show ghost text for first suggestion
        current = self.get_current_command().split()[-1] if self.get_current_command() else ""
        if suggestions:
            for s in suggestions:
                if s.startswith(current):
                    self.show_ghost_suggestion(s)
                    break
        
        # Get popup position
        cursor_pos = self.text_area.index("insert")
        bbox = self.text_area.bbox(cursor_pos)
        if bbox:
            x = self.text_area.winfo_rootx() + bbox[0]
            y = self.text_area.winfo_rooty() + bbox[1] + bbox[3] + 5
            self.suggestion_popup.show(suggestions, x, y)
            
    def request_suggestions(self):
        """Request suggestions from backend"""
        current = self.get_current_command()
        if current:
            # Get last word for suggestions
            words = current.split()
            if words:
                self.backend.request_suggestions(words[-1])
        else:
            self.suggestion_popup.hide()
            
    # ============ EVENT HANDLERS ============
    
    def handle_enter(self, event):
        """Handle Enter key"""
        self.suggestion_popup.hide()
        self.clear_ghost_text()
        
        command_text = self.get_current_command()
        self.text_area.insert(tk.END, "\n")
        
        if command_text:
            # Add to history
            if not self.command_history or self.command_history[-1] != command_text:
                self.command_history.append(command_text)
            self.history_index = len(self.command_history)
            
            # Send to backend (with NLP prefix for natural language)
            if self.is_natural_language(command_text):
                self.backend.send_command(f"NLP:{command_text}")
            else:
                self.backend.send_command(command_text)
            
            self.update_status(f"Executing: {command_text}", "info")
        else:
            self.backend.send_command("")
        
        return "break"
    
    def is_natural_language(self, text):
        """Check if text looks like natural language"""
        # Simple heuristics
        nl_words = ['show', 'display', 'list', 'create', 'make', 'delete', 'remove',
                   'what', 'where', 'find', 'go', 'help', 'please', 'can you']
        
        text_lower = text.lower()
        
        # Check for NL indicators
        for word in nl_words:
            if text_lower.startswith(word + " "):
                return True
        
        # Check if contains multiple words and not a known command
        words = text.split()
        known_commands = ['ls', 'cd', 'pwd', 'mkdir', 'rmdir', 'cat', 'rm', 'cp', 'mv',
                         'echo', 'sysmon', 'help', 'exit', 'clear', 'tree', 'search']
        
        if words and words[0] in known_commands:
            return False
        
        return len(words) >= 2
    
    def handle_up(self, event):
        """Handle Up arrow - history or suggestions"""
        if self.suggestion_popup.visible:
            self.suggestion_popup.move_selection(-1)
            return "break"
        
        # History navigation
        if self.command_history:
            if self.history_index > 0:
                if self.history_index == len(self.command_history):
                    self.current_input = self.get_current_command()
                self.history_index -= 1
                self.set_current_command(self.command_history[self.history_index])
        
        return "break"
    
    def handle_down(self, event):
        """Handle Down arrow - history or suggestions"""
        if self.suggestion_popup.visible:
            self.suggestion_popup.move_selection(1)
            return "break"
        
        # History navigation
        if self.command_history:
            if self.history_index < len(self.command_history):
                self.history_index += 1
                if self.history_index == len(self.command_history):
                    self.set_current_command(self.current_input)
                else:
                    self.set_current_command(self.command_history[self.history_index])
        
        return "break"
    
    def handle_right(self, event):
        """Handle Right arrow - accept suggestion or move cursor"""
        # If ghost text exists, accept it
        if self.ghost_text:
            self.clear_ghost_text()
            current = self.get_current_command()
            words = current.split()
            if words:
                # Complete the last word
                for s in self.suggestion_popup.suggestions if self.suggestion_popup.visible else []:
                    if s.startswith(words[-1]):
                        words[-1] = s
                        self.set_current_command(" ".join(words) + " ")
                        break
            self.suggestion_popup.hide()
            return "break"
        
        # If popup visible, accept selected
        if self.suggestion_popup.visible:
            selected = self.suggestion_popup.get_selected()
            if selected:
                current = self.get_current_command()
                words = current.split()
                if words:
                    words[-1] = selected
                    self.set_current_command(" ".join(words) + " ")
                else:
                    self.set_current_command(selected + " ")
                self.suggestion_popup.hide()
                return "break"
        
        # Normal cursor movement
        return None
    
    def handle_left(self, event):
        """Handle Left arrow"""
        self.clear_ghost_text()
        self.suggestion_popup.hide()
        
        if self.text_area.compare("insert", "<=", self.prompt_end_index):
            return "break"
        return None
    
    def handle_tab(self, event):
        """Handle Tab - accept suggestion or cycle"""
        if self.suggestion_popup.visible:
            selected = self.suggestion_popup.get_selected()
            if selected:
                current = self.get_current_command()
                words = current.split()
                if words:
                    words[-1] = selected
                    self.set_current_command(" ".join(words) + " ")
                else:
                    self.set_current_command(selected + " ")
                self.suggestion_popup.hide()
        else:
            # Request suggestions
            self.request_suggestions()
        
        return "break"
    
    def handle_backspace(self, event):
        """Handle Backspace"""
        self.clear_ghost_text()
        self.suggestion_popup.hide()
        
        if self.text_area.compare("insert", "<=", self.prompt_end_index):
            return "break"
        return None
    
    def handle_keypress(self, event):
        """Handle general key press"""
        # Prevent editing before prompt
        if self.text_area.compare("insert", "<", self.prompt_end_index):
            if event.keysym not in ("Left", "Right", "Up", "Down", "Home", "End"):
                self.text_area.mark_set("insert", "end-1c")
        return None
    
    def on_key_release(self, event):
        """Handle key release - update suggestions"""
        # Ignore modifier keys and navigation
        if event.keysym in ('Shift_L', 'Shift_R', 'Control_L', 'Control_R', 
                           'Alt_L', 'Alt_R', 'Up', 'Down', 'Tab', 'Return',
                           'Escape', 'Left', 'Right'):
            return
        
        self.clear_ghost_text()
        self.update_cursor_position()
        
        # Request new suggestions after a short delay
        current = self.get_current_command()
        if current and len(current) >= 1:
            # Simple local suggestion for responsiveness
            self.show_local_suggestions(current)
        else:
            self.suggestion_popup.hide()
            
    def show_local_suggestions(self, current):
        """Show suggestions locally (without backend call for speed)"""
        commands = ['ls', 'cd', 'pwd', 'mkdir', 'rmdir', 'touch', 'rm', 'cat', 'cp', 'mv',
                   'echo', 'tree', 'search', 'backup', 'compare', 'sysmon', 'help', 
                   'history', 'exit', 'clear', 'fileinfo', 'hexdump', 'duplicate',
                   'encrypt', 'decrypt', 'sizeof', 'age', 'freq', 'lines', 'quicknote',
                   'calc', 'head', 'tail', 'wc', 'grep', 'sort', 'uniq', 'rev', 'date',
                   'whoami', 'hostname', 'uptime', 'df', 'ps', 'kill', 'undo', 'macro',
                   'bookmark', 'recent', 'bulk_rename', 'stats', 'teach']
        
        # Get last word
        words = current.split()
        last_word = words[-1] if words else current
        
        # Filter commands
        suggestions = [cmd for cmd in commands if cmd.startswith(last_word.lower())]
        
        if suggestions:
            self.show_suggestion_popup(suggestions[:8])
        else:
            self.suggestion_popup.hide()
            
    def handle_ctrl_c(self, event):
        """Handle Ctrl+C - copy or interrupt"""
        try:
            selected = self.text_area.selection_get()
            if selected:
                self.root.clipboard_clear()
                self.root.clipboard_append(selected)
                self.update_status("Copied", "success")
                return "break"
        except tk.TclError:
            pass
        
        # Interrupt - clear current line
        self.set_current_command("")
        self.suggestion_popup.hide()
        self.update_status("Interrupted", "warning")
        return "break"
    
    def handle_ctrl_a(self, event):
        """Move cursor to beginning of command"""
        self.text_area.mark_set("insert", self.prompt_end_index)
        return "break"
    
    def handle_ctrl_e(self, event):
        """Move cursor to end of line"""
        self.text_area.mark_set("insert", "end-1c")
        return "break"
    
    def clear_line(self, event):
        """Clear current line (Ctrl+U)"""
        self.set_current_command("")
        self.suggestion_popup.hide()
        return "break"
    
    def kill_line(self, event):
        """Kill from cursor to end (Ctrl+K)"""
        self.text_area.delete("insert", "end-1c")
        return "break"
    
    def delete_word(self, event):
        """Delete word backwards (Ctrl+W)"""
        current = self.get_current_command()
        words = current.split()
        if words:
            words.pop()
            self.set_current_command(" ".join(words) + (" " if words else ""))
        return "break"
    
    def handle_mouse_zoom(self, event):
        """Handle mouse wheel zoom"""
        if event.delta > 0:
            self.zoom_in()
        else:
            self.zoom_out()
        return "break"
    
    # ============ UTILITY METHODS ============
    
    def update_cursor_position(self, event=None):
        """Update cursor position in status bar"""
        cursor_pos = self.text_area.index("insert")
        line, col = cursor_pos.split(".")
        self.position_label.config(text=f"Ln {line}, Col {col}")
        
    def update_status(self, message, status_type="info"):
        """Update status bar"""
        colors = {
            "success": "#007acc",
            "error": "#f14c4c", 
            "warning": "#cca700",
            "info": "#007acc"
        }
        self.status_bar.config(bg=colors.get(status_type, "#007acc"))
        self.status_label.config(text=message, bg=colors.get(status_type, "#007acc"))
        self.position_label.config(bg=colors.get(status_type, "#007acc"))
        self.zoom_label.config(bg=colors.get(status_type, "#007acc"))
        
    def new_terminal(self):
        """Open new terminal window"""
        subprocess.Popen([sys.executable, __file__])
        
    def clear_screen(self):
        """Clear terminal"""
        self.text_area.delete("1.0", tk.END)
        self.show_welcome()
        self.write_prompt()
        self.update_status("Screen cleared", "success")
        
    def copy_text(self):
        """Copy selected text"""
        try:
            selected = self.text_area.selection_get()
            self.root.clipboard_clear()
            self.root.clipboard_append(selected)
            self.update_status("Copied", "success")
        except tk.TclError:
            pass
        
    def paste_text(self):
        """Paste from clipboard"""
        try:
            text = self.root.clipboard_get()
            self.text_area.insert("insert", text)
            self.update_status("Pasted", "success")
        except tk.TclError:
            pass
        
    def select_all(self):
        """Select all text"""
        self.text_area.tag_add("sel", "1.0", "end")
        
    def zoom_in(self):
        """Increase font size"""
        if self.font_size < 24:
            self.font_size += 1
            self.custom_font.configure(size=self.font_size)
            self.bold_font.configure(size=self.font_size)
            self.italic_font.configure(size=self.font_size)
            self.zoom_label.config(text=f"{self.font_size}pt")
            self.update_status(f"Zoom: {self.font_size}pt", "info")
            
    def zoom_out(self):
        """Decrease font size"""
        if self.font_size > 8:
            self.font_size -= 1
            self.custom_font.configure(size=self.font_size)
            self.bold_font.configure(size=self.font_size)
            self.italic_font.configure(size=self.font_size)
            self.zoom_label.config(text=f"{self.font_size}pt")
            self.update_status(f"Zoom: {self.font_size}pt", "info")
            
    def reset_zoom(self):
        """Reset font size"""
        self.font_size = 11
        self.custom_font.configure(size=self.font_size)
        self.bold_font.configure(size=self.font_size)
        self.italic_font.configure(size=self.font_size)
        self.zoom_label.config(text=f"{self.font_size}pt")
        self.update_status("Zoom reset", "info")
        
    def send_command_direct(self, command):
        """Send command directly"""
        self.set_current_command(command)
        self.handle_enter(None)
        
    def show_calculator(self):
        """Show calculator dialog"""
        dialog = tk.Toplevel(self.root)
        dialog.title("Calculator")
        dialog.geometry("300x150")
        dialog.configure(bg="#252526")
        dialog.transient(self.root)
        
        tk.Label(dialog, text="Expression:", bg="#252526", fg="#cccccc").pack(pady=10)
        
        entry = tk.Entry(dialog, width=30, bg="#1e1e1e", fg="#d4d4d4", insertbackground="#ffffff")
        entry.pack(pady=5)
        entry.focus_set()
        
        result_label = tk.Label(dialog, text="", bg="#252526", fg="#4ec9b0", font=("Consolas", 12))
        result_label.pack(pady=10)
        
        def calculate():
            expr = entry.get()
            self.send_command_direct(f"calc {expr}")
            dialog.destroy()
        
        tk.Button(dialog, text="Calculate", command=calculate, bg="#0e639c", fg="#ffffff").pack(pady=5)
        entry.bind("<Return>", lambda e: calculate())
        
    def show_shortcuts(self):
        """Show keyboard shortcuts"""
        shortcuts = """
╔══════════════════════════════════════════════════════════════╗
║                    KEYBOARD SHORTCUTS                        ║
╠══════════════════════════════════════════════════════════════╣
║                                                              ║
║  Navigation:                                                 ║
║    Up/Down Arrow    Navigate command history                 ║
║    Right Arrow      Accept suggestion / autocomplete         ║
║    Tab              Cycle through suggestions                ║
║    Ctrl+A           Move to beginning of line                ║
║    Ctrl+E           Move to end of line                      ║
║                                                              ║
║  Editing:                                                    ║
║    Ctrl+U           Clear entire line                        ║
║    Ctrl+K           Delete from cursor to end                ║
║    Ctrl+W           Delete word backwards                    ║
║    Backspace        Delete character                         ║
║                                                              ║
║  Clipboard:                                                  ║
║    Ctrl+C           Copy selection / Interrupt               ║
║    Ctrl+Shift+C     Copy                                     ║
║    Ctrl+Shift+V     Paste                                    ║
║                                                              ║
║  View:                                                       ║
║    Ctrl+L           Clear screen                             ║
║    Ctrl++           Zoom in                                  ║
║    Ctrl+-           Zoom out                                 ║
║    Ctrl+0           Reset zoom                               ║
║    Ctrl+Scroll      Mouse wheel zoom                         ║
║                                                              ║
║  System:                                                     ║
║    Ctrl+D           Exit                                     ║
║    Ctrl+Shift+N     New terminal window                      ║
║    Escape           Hide suggestions                         ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝
"""
        messagebox.showinfo("Keyboard Shortcuts", shortcuts)
        
    def show_about(self):
        """Show about dialog"""
        about = """
╔══════════════════════════════════════════════════════════════╗
║                    NLP TERMINAL v2.0                         ║
║                                                              ║
║  An Advanced Linux-like Shell with:                          ║
║                                                              ║
║  ✓ Natural Language Processing                               ║
║  ✓ Intellisense-style Suggestions                            ║
║  ✓ Real-time System Monitoring                               ║
║  ✓ Unique Custom Commands                                    ║
║  ✓ Full Command History                                      ║
║  ✓ Keyboard-driven Interface                                 ║
║                                                              ║
║  Architecture:                                               ║
║    • C Backend: All commands & system calls                  ║
║    • Python Frontend: UI only (tkinter)                      ║
║                                                              ║
║  Developed with ❤️ for the terminal enthusiasts              ║
╚══════════════════════════════════════════════════════════════╝
"""
        messagebox.showinfo("About NLP Terminal", about)


# ============ MAIN ============

if __name__ == "__main__":
    root = tk.Tk()
    app = NLPTerminalApp(root)
    root.mainloop()
