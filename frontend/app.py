import tkinter as tk
from tkinter import font
import os
import sys
from backend_comm import BackendManager

class TerminalApp:
    def __init__(self, root):
        self.root = root
        self.root.title("NLP Terminal")
        self.root.geometry("900x700")
        # Dark glossy background color
        self.root.configure(bg="#0f0f1a") 

        # Custom font
        self.custom_font = font.Font(family="Consolas", size=11)

        # Text widget for terminal with slightly lighter background for contrast
        self.text_area = tk.Text(
            self.root,
            bg="#131324", # Slightly lighter than root for depth
            fg="#e0e0e0",
            insertbackground="white",
            font=self.custom_font,
            bd=0,
            highlightthickness=1,
            highlightbackground="#2a2a40", # Border for glossy feel
            wrap=tk.WORD
        )
        self.text_area.pack(expand=True, fill='both', padx=15, pady=15)

        # Tags for coloring - Neon/Glossy Palette
        self.text_area.tag_config("prompt", foreground="#00ff9d", font=("Consolas", 11, "bold")) # Neon Green
        self.text_area.tag_config("command", foreground="#ffffff")
        self.text_area.tag_config("output", foreground="#b0b0c0")
        self.text_area.tag_config("error", foreground="#ff4d4d") # Soft Red
        self.text_area.tag_config("suggestion", foreground="#00ccff", font=("Consolas", 10, "italic")) # Cyan
        self.text_area.tag_config("nlp", foreground="#ff00ff", font=("Consolas", 10, "italic")) # Magenta

        # Backend setup
        # Determine executable name based on OS or assumption
        # We assume we are in the same env where we compiled 'mysh'
        shell_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../backend/mysh"))
        if not os.path.exists(shell_path):
             # Fallback for Windows if compiled as .exe
             shell_path += ".exe"
        
        self.backend = BackendManager(shell_path)
        try:
            self.backend.start()
        except Exception as e:
            self.text_area.insert(tk.END, f"Error starting backend: {e}\n", "error")

        # Prompt setup
        self.prompt = "shell> "
        self.prompt_end_index = "1.0"
        
        # Event bindings
        self.text_area.bind("<Return>", self.handle_enter)
        self.text_area.bind("<BackSpace>", self.handle_backspace)
        self.text_area.bind("<Key>", self.handle_keypress)
        
        # Start polling for output
        self.check_backend_output()

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
            elif type == "ERROR":
                self.text_area.insert(tk.END, f"Error: {content}\n", "error")
        
        self.root.after(50, self.check_backend_output)

    def write_prompt(self):
        # Ensure we are on a new line if needed
        if not self.text_area.get("end-2c", "end-1c").endswith("\n"):
             self.text_area.insert(tk.END, "\n")
             
        self.text_area.insert(tk.END, self.prompt, "prompt")
        self.text_area.see(tk.END)
        self.prompt_end_index = self.text_area.index(tk.END + "-1c")
        self.text_area.mark_set("insert", tk.END)

    def translate_command(self, input_text):
        """Basic NLP translation."""
        input_text = input_text.strip()
        lower_input = input_text.lower()
        
        if not input_text:
            return input_text

        # Heuristics for NLP mapping
        parts = input_text.split()
        
        # "show files" -> ls
        if "show" in lower_input and ("files" in lower_input or "list" in lower_input):
            return "ls"
            
        # "create folder <name>" -> mkdir <name>
        elif "create" in lower_input and ("folder" in lower_input or "directory" in lower_input):
            if len(parts) >= 3:
                return f"mkdir {parts[-1]}"
                
        # "remove folder <name>" -> rmdir <name>
        elif ("remove" in lower_input or "delete" in lower_input) and ("folder" in lower_input or "directory" in lower_input):
            if len(parts) >= 3:
                return f"rmdir {parts[-1]}"
                
        # "create file <name>" -> touch <name>
        elif "create" in lower_input and "file" in lower_input:
            if len(parts) >= 3:
                return f"touch {parts[-1]}"
                
        # "remove file <name>" -> rm <name>
        elif ("remove" in lower_input or "delete" in lower_input) and "file" in lower_input:
            if len(parts) >= 3:
                return f"rm {parts[-1]}"
                
        # "read <name>" or "show content of <name>" -> cat <name>
        elif ("read" in lower_input or "content" in lower_input) and len(parts) >= 2:
            return f"cat {parts[-1]}"
            
        # "copy <src> to <dst>" -> cp <src> <dst>
        elif "copy" in lower_input and "to" in lower_input:
            try:
                to_index = parts.index("to")
                if to_index > 1 and to_index + 1 < len(parts):
                    src = parts[to_index-1]
                    dst = parts[to_index+1]
                    return f"cp {src} {dst}"
            except ValueError:
                pass
                
        # "move <src> to <dst>" -> mv <src> <dst>
        elif "move" in lower_input and "to" in lower_input:
            try:
                to_index = parts.index("to")
                if to_index > 1 and to_index + 1 < len(parts):
                    src = parts[to_index-1]
                    dst = parts[to_index+1]
                    return f"mv {src} {dst}"
            except ValueError:
                pass

        # "change directory to <name>" -> cd <name>
        elif "change" in lower_input and "directory" in lower_input:
            if len(parts) >= 1:
                return f"cd {parts[-1]}"
                
        elif "who" in lower_input and "i" in lower_input:
            return "whoami"
        elif "history" in lower_input and "show" in lower_input:
            return "history"
        elif "clear" in lower_input:
            return "clear"
        elif "help" in lower_input:
            return "help"
            
        return input_text

    def handle_enter(self, event):
        # Get text from prompt_end_index to END
        # Note: tk.END includes the final newline, so we use "end-1c"
        command_text = self.text_area.get(self.prompt_end_index, "end-1c").strip()

        self.text_area.insert(tk.END, "\n")
        
        if command_text:
            # NLP Translation
            translated = self.translate_command(command_text)
            if translated != command_text:
                self.text_area.insert(tk.END, f"[NLP] Interpreted as: {translated}\n", "nlp")
                self.backend.send_command(translated)
            else:
                self.backend.send_command(command_text)
        else:
            self.backend.send_command("") 

        return "break"

    def handle_backspace(self, event):
        if self.text_area.compare("insert", "<=", self.prompt_end_index):
            return "break"
        return None

    def handle_keypress(self, event):
        if self.text_area.compare("insert", "<", self.prompt_end_index):
            self.text_area.mark_set("insert", tk.END)
        return None

if __name__ == "__main__":
    root = tk.Tk()
    app = TerminalApp(root)
    root.mainloop()
