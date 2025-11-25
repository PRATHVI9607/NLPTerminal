import subprocess
import threading
import queue
import os

class BackendManager:
    def __init__(self, shell_path):
        self.shell_path = shell_path
        self.process = None
        self.output_queue = queue.Queue()
        self.running = False
        self.reader_thread = None

    def start(self):
        if not os.path.exists(self.shell_path):
            raise FileNotFoundError(f"Shell executable not found at {self.shell_path}")

        self.process = subprocess.Popen(
            [self.shell_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=0  # Unbuffered
        )
        self.running = True
        
        # Start reader thread
        self.reader_thread = threading.Thread(target=self._read_stdout, daemon=True)
        self.reader_thread.start()

    def _read_stdout(self):
        """Reads stdout char by char to handle prompts correctly."""
        buffer = ""
        while self.running and self.process.poll() is None:
            try:
                char = self.process.stdout.read(1)
                if not char:
                    break
                
                buffer += char
                
                # Check for prompt (naive check: ends with '> ')
                # In our C code: printf("%s> ", cwd);
                if buffer.endswith("> "):
                    self.output_queue.put(("PROMPT", buffer))
                    buffer = ""
                elif char == '\n':
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

    def get_output(self):
        """Non-blocking get from queue."""
        try:
            return self.output_queue.get_nowait()
        except queue.Empty:
            return None

    def stop(self):
        self.running = False
        if self.process:
            self.process.terminate()
