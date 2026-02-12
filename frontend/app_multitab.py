#!/usr/bin/env python3
"""
NLP Terminal - Multi-Tab Terminal GUI with LIVE Monitoring
PYTHON IS ONLY FOR GUI DISPLAY - ALL LOGIC IS IN C BACKEND
Theme: Classic Terminal - Black/Green
"""

import sys
import os
import re
import subprocess
import math

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QTextEdit, QLineEdit, QPushButton, QTabWidget, QLabel, QDialog,
    QFrame, QSpinBox, QGraphicsDropShadowEffect, QListWidget, QListWidgetItem
)
from PyQt6.QtCore import Qt, QProcess, QTimer, pyqtSignal, QRectF, QPointF
from PyQt6.QtGui import (
    QFont, QColor, QTextCharFormat, QTextCursor, QAction, QKeySequence,
    QPalette, QFontDatabase, QPainter, QPen, QBrush, QPainterPath
)

# Classic Terminal Theme - Black/Green
COLORS = {
    'bg_darkest': '#000000',
    'bg_dark': '#0a0a0a',
    'bg_mid': '#111111',
    'bg_light': '#1a1a1a',
    'bg_hover': '#222222',
    
    'green': '#00ff00',
    'green_dim': '#00aa00',
    'white': '#ffffff',
    'gray': '#888888',
    'cyan': '#00ffff',
    
    # Aliases for ANSI
    'eco_green': '#00ff00',
    'eco_blue': '#4488ff',
    'eco_cyan': '#00ffff',
    
    'border': '#333333',
    'border_focus': '#00ff00',
    
    'error': '#ff4444',
    'warning': '#ffaa00',
    'live_pulse': '#ff0000',
}

FONT_MONO = "monospace"

# All available commands for suggestions
ALL_COMMANDS = [
    # Basic File Operations
    "ls", "dir", "cd", "pwd", "mkdir", "rmdir", "rm", "touch", "cat", "head", "tail",
    "cp", "mv", "find", "grep", "wc", "echo", "clear", "tree", "help", "exit",
    # Advanced File Operations
    "search", "backup", "compare", "diff", "fileinfo", "hexdump", "duplicate",
    "sizeof", "age", "freq", "lines", "recent", "checksum", "crc32",
    # Compression & Encryption
    "compress", "decompress", "encrypt", "decrypt",
    # System Monitoring
    "sysmon", "cpuinfo", "meminfo", "diskinfo", "swapinfo", "proclist", "proc", "ps",
    "proctop", "prockill", "netinfo", "netstat", "connections", "loadavg", "uptime",
    "paging", "vmstat", "modules", "mounts", "filesystems", "kernelinfo", "battery",
    "sensors", "users", "openfiles", "sockets", "interrupts", "zoneinfo", "envvar",
    "whoami", "hostname", "date",
    # IPC
    "mypid", "register", "unregister", "terminals", "sendmsg", "broadcast", 
    "checkmsg", "readmsg", "clearmsg", "inbox",
    # Math & Expressions
    "calc", "random", "uuid",
    "infix2postfix", "postfix2infix", "infix2prefix", "prefix2infix",
    "visualfix", "visualpost", "visualeval", "evaluate", "exprtype",
    # Encoding/Decoding
    "base64", "morse", "rot13", "reverse", "binary", "hex", "octal", "baseconv",
    # Visualization
    "visualize", "colorpalette", "ascii", "banner", "progress", "table",
    # Text Processing
    "sort", "uniq", "jsonformat", "jsoncat", "lorem",
    # Productivity
    "quicknote", "bookmark", "macro", "history", "undo", "redo", "teach",
    "timer", "stopwatch", "stats", "bulk_rename",
    # Misc
    "uniquecmds", "categories", "oshelp", "ipchelp",
]

def levenshtein(s1, s2):
    """Calculate edit distance between two strings"""
    if len(s1) < len(s2):
        return levenshtein(s2, s1)
    if len(s2) == 0:
        return len(s1)
    prev = range(len(s2) + 1)
    for i, c1 in enumerate(s1):
        curr = [i + 1]
        for j, c2 in enumerate(s2):
            curr.append(min(prev[j + 1] + 1, curr[j] + 1, prev[j] + (c1 != c2)))
        prev = curr
    return prev[-1]

def get_suggestions(prefix, max_results=5):
    """Get command suggestions for prefix"""
    if not prefix:
        return []
    prefix = prefix.lower()
    # Exact prefix matches first
    exact = [c for c in ALL_COMMANDS if c.startswith(prefix)]
    # Fuzzy matches
    fuzzy = []
    if len(exact) < max_results and len(prefix) >= 2:
        for c in ALL_COMMANDS:
            if c not in exact and levenshtein(prefix, c[:len(prefix)]) <= 2:
                fuzzy.append(c)
    return (exact + fuzzy)[:max_results]

def get_did_you_mean(cmd, max_results=3):
    """Get 'did you mean' suggestions for unknown command"""
    cmd = cmd.lower().split()[0] if cmd else ""
    if not cmd or cmd in ALL_COMMANDS:
        return []
    suggestions = []
    for c in ALL_COMMANDS:
        dist = levenshtein(cmd, c)
        if dist <= 3:
            suggestions.append((dist, c))
    suggestions.sort(key=lambda x: x[0])
    return [s[1] for s in suggestions[:max_results]]


# ═══════════════════════════════════════════════════════════════════════════════
#                     CANVAS WIDGET FOR LIVE MONITORS
# ═══════════════════════════════════════════════════════════════════════════════

class MonitorCanvas(QWidget):
    """Custom canvas for drawing system monitor visualizations"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.data = {}
        self.history = {'cpu': [], 'mem': [], 'disk': []}
        self.setMinimumSize(800, 500)
    
    def update_data(self, data):
        self.data = data
        # Keep history for sparklines
        if 'cpu_pct' in data:
            self.history['cpu'].append(data['cpu_pct'])
            self.history['cpu'] = self.history['cpu'][-60:]
        if 'mem_pct' in data:
            self.history['mem'].append(data['mem_pct'])
            self.history['mem'] = self.history['mem'][-60:]
        self.update()
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        # Background
        painter.fillRect(self.rect(), QColor(COLORS['bg_darkest']))
        
        w, h = self.width(), self.height()
        
        if not self.data:
            painter.setPen(QColor(COLORS['gray']))
            painter.setFont(QFont(FONT_MONO, 14))
            painter.drawText(self.rect(), Qt.AlignmentFlag.AlignCenter, "Loading...")
            return
        
        # Draw based on data type
        if 'cpu_pct' in self.data:
            self.draw_sysmon(painter, w, h)
        elif 'mem_total' in self.data and 'cpu_pct' not in self.data:
            self.draw_meminfo(painter, w, h)
        elif 'disk_total' in self.data:
            self.draw_diskinfo(painter, w, h)
        else:
            self.draw_generic(painter, w, h)
    
    def draw_gauge(self, painter, x, y, size, pct, label, color):
        """Draw a circular gauge"""
        # Background arc
        painter.setPen(QPen(QColor(COLORS['bg_light']), 8))
        rect = QRectF(x, y, size, size)
        painter.drawArc(rect, 225 * 16, -270 * 16)
        
        # Value arc
        painter.setPen(QPen(QColor(color), 8))
        span = int(-270 * 16 * pct / 100)
        painter.drawArc(rect, 225 * 16, span)
        
        # Center text
        painter.setPen(QColor(COLORS['green']))
        painter.setFont(QFont(FONT_MONO, 16, QFont.Weight.Bold))
        painter.drawText(rect, Qt.AlignmentFlag.AlignCenter, f"{pct:.0f}%")
        
        # Label
        painter.setFont(QFont(FONT_MONO, 10))
        painter.drawText(QRectF(x, y + size + 5, size, 20), 
                        Qt.AlignmentFlag.AlignCenter, label)
    
    def draw_bar(self, painter, x, y, w, h, pct, label, value_str, color):
        """Draw a horizontal bar"""
        # Background
        painter.fillRect(int(x), int(y), int(w), int(h), QColor(COLORS['bg_light']))
        
        # Fill
        fill_w = int(w * pct / 100)
        painter.fillRect(int(x), int(y), fill_w, int(h), QColor(color))
        
        # Border
        painter.setPen(QColor(COLORS['border']))
        painter.drawRect(int(x), int(y), int(w), int(h))
        
        # Label on left
        painter.setPen(QColor(COLORS['green']))
        painter.setFont(QFont(FONT_MONO, 10))
        painter.drawText(int(x - 100), int(y), 95, int(h), 
                        Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter, label)
        
        # Value on right
        painter.drawText(int(x + w + 10), int(y), 100, int(h),
                        Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter, value_str)
    
    def draw_sparkline(self, painter, x, y, w, h, values, color):
        """Draw a sparkline graph"""
        if not values:
            return
        
        # Background
        painter.fillRect(int(x), int(y), int(w), int(h), QColor(COLORS['bg_mid']))
        
        # Line
        painter.setPen(QPen(QColor(color), 2))
        max_val = max(values) if max(values) > 0 else 100
        points = []
        for i, v in enumerate(values):
            px = x + (i / max(len(values) - 1, 1)) * w
            py = y + h - (v / max_val) * h
            points.append(QPointF(px, py))
        
        for i in range(len(points) - 1):
            painter.drawLine(points[i], points[i + 1])
    
    def draw_sysmon(self, painter, w, h):
        """Draw system monitor with CPU, Memory, Disk gauges"""
        d = self.data
        
        # Title
        painter.setPen(QColor(COLORS['cyan']))
        painter.setFont(QFont(FONT_MONO, 14, QFont.Weight.Bold))
        painter.drawText(20, 30, "SYSTEM MONITOR")
        
        # Three gauges
        gauge_size = 120
        gauge_y = 60
        spacing = (w - 3 * gauge_size - 40) // 4
        
        # CPU gauge
        cpu_color = COLORS['green'] if d.get('cpu_pct', 0) < 70 else COLORS['warning'] if d.get('cpu_pct', 0) < 90 else COLORS['error']
        self.draw_gauge(painter, spacing, gauge_y, gauge_size, d.get('cpu_pct', 0), "CPU", cpu_color)
        
        # Memory gauge
        mem_color = COLORS['green'] if d.get('mem_pct', 0) < 70 else COLORS['warning'] if d.get('mem_pct', 0) < 90 else COLORS['error']
        self.draw_gauge(painter, spacing * 2 + gauge_size, gauge_y, gauge_size, d.get('mem_pct', 0), "MEMORY", mem_color)
        
        # Disk gauge
        disk_color = COLORS['green'] if d.get('disk_pct', 0) < 70 else COLORS['warning'] if d.get('disk_pct', 0) < 90 else COLORS['error']
        self.draw_gauge(painter, spacing * 3 + gauge_size * 2, gauge_y, gauge_size, d.get('disk_pct', 0), "DISK", disk_color)
        
        # Stats text
        painter.setPen(QColor(COLORS['green']))
        painter.setFont(QFont(FONT_MONO, 11))
        stats_y = gauge_y + gauge_size + 50
        
        stats = [
            f"CPU Model: {d.get('cpu_model', 'Unknown')[:50]}",
            f"Cores: {d.get('cpu_cores', 'N/A')}",
            f"Memory: {d.get('mem_used', 'N/A')} / {d.get('mem_total', 'N/A')}",
            f"Disk: {d.get('disk_used', 'N/A')} / {d.get('disk_total', 'N/A')}",
            f"Load Average: {d.get('load_avg', 'N/A')}",
        ]
        
        for i, stat in enumerate(stats):
            painter.drawText(30, stats_y + i * 22, stat)
        
        # Sparklines
        spark_y = stats_y + len(stats) * 22 + 30
        painter.drawText(30, spark_y - 5, "CPU History (60s)")
        self.draw_sparkline(painter, 30, spark_y, w - 60, 50, self.history['cpu'], COLORS['cyan'])
        
        painter.drawText(30, spark_y + 70, "Memory History (60s)")
        self.draw_sparkline(painter, 30, spark_y + 75, w - 60, 50, self.history['mem'], COLORS['green'])
    
    def draw_meminfo(self, painter, w, h):
        """Draw memory info with bars"""
        d = self.data
        
        painter.setPen(QColor(COLORS['cyan']))
        painter.setFont(QFont(FONT_MONO, 14, QFont.Weight.Bold))
        painter.drawText(20, 30, "MEMORY INFORMATION")
        
        bar_w = w - 250
        bar_h = 25
        bar_x = 120
        bar_y = 60
        
        bars = [
            ("Total", d.get('mem_total', 'N/A'), 100, COLORS['cyan']),
            ("Used", d.get('mem_used', 'N/A'), d.get('mem_pct', 0), COLORS['green']),
            ("Available", d.get('mem_avail', 'N/A'), d.get('mem_avail_pct', 0), COLORS['eco_blue']),
            ("Buffers", d.get('mem_buffers', 'N/A'), d.get('buf_pct', 0), COLORS['warning']),
            ("Cached", d.get('mem_cached', 'N/A'), d.get('cache_pct', 0), COLORS['gray']),
        ]
        
        for i, (label, value, pct, color) in enumerate(bars):
            self.draw_bar(painter, bar_x, bar_y + i * 45, bar_w, bar_h, pct, label, str(value), color)
        
        # Sparkline
        spark_y = bar_y + len(bars) * 45 + 40
        painter.setPen(QColor(COLORS['green']))
        painter.drawText(30, spark_y - 5, "Memory Usage History")
        self.draw_sparkline(painter, 30, spark_y, w - 60, 60, self.history['mem'], COLORS['green'])
    
    def draw_diskinfo(self, painter, w, h):
        """Draw disk info"""
        d = self.data
        
        painter.setPen(QColor(COLORS['cyan']))
        painter.setFont(QFont(FONT_MONO, 14, QFont.Weight.Bold))
        painter.drawText(20, 30, "DISK INFORMATION")
        
        # Single large gauge
        gauge_size = 180
        self.draw_gauge(painter, (w - gauge_size) // 2, 60, gauge_size, 
                       d.get('disk_pct', 0), "DISK USAGE", COLORS['green'])
        
        # Stats
        painter.setPen(QColor(COLORS['green']))
        painter.setFont(QFont(FONT_MONO, 12))
        stats_y = 280
        
        stats = [
            f"Filesystem: {d.get('disk_fs', '/')}",
            f"Total: {d.get('disk_total', 'N/A')}",
            f"Used: {d.get('disk_used', 'N/A')}",
            f"Available: {d.get('disk_avail', 'N/A')}",
            f"Mount: {d.get('disk_mount', '/')}",
        ]
        
        for i, stat in enumerate(stats):
            painter.drawText(w // 2 - 100, stats_y + i * 25, stat)
    
    def draw_generic(self, painter, w, h):
        """Draw generic key-value data"""
        painter.setPen(QColor(COLORS['green']))
        painter.setFont(QFont(FONT_MONO, 11))
        
        y = 30
        for key, value in self.data.items():
            painter.drawText(30, y, f"{key}: {value}")
            y += 22


# ═══════════════════════════════════════════════════════════════════════════════
#                     LIVE MONITOR WINDOW (AUTO-UPDATES)
# ═══════════════════════════════════════════════════════════════════════════════

class LiveMonitorWindow(QDialog):
    """Live monitoring window - auto-refreshes data from C backend"""
    
    closed_signal = pyqtSignal()
    
    def __init__(self, title, command, backend_path, parent=None):
        super().__init__(parent)
        self.command = command
        self.backend_path = backend_path
        self.is_running = True
        self.refresh_rate = 1000
        self.monitor_type = command.split()[0].lower()
        
        self.setWindowTitle(f"● LIVE — {title}")
        self.setMinimumSize(850, 550)
        self.setModal(False)
        
        self.setStyleSheet(f"""
            QDialog {{
                background: {COLORS['bg_darkest']};
            }}
            QPushButton {{
                background: {COLORS['bg_light']};
                color: {COLORS['green']};
                border: 1px solid {COLORS['border']};
                padding: 6px 14px;
                font-family: {FONT_MONO};
            }}
            QPushButton:hover {{
                background: {COLORS['bg_hover']};
                border-color: {COLORS['green']};
            }}
            QPushButton#closeBtn {{
                color: {COLORS['error']};
                border-color: {COLORS['error']};
            }}
            QLabel {{
                color: {COLORS['green']};
                font-family: {FONT_MONO};
            }}
            QSpinBox {{
                background: {COLORS['bg_darkest']};
                color: {COLORS['green']};
                border: 1px solid {COLORS['border']};
                padding: 4px;
            }}
        """)
        
        layout = QVBoxLayout(self)
        layout.setContentsMargins(10, 10, 10, 10)
        layout.setSpacing(8)
        
        # Header
        header = QHBoxLayout()
        
        self.live_dot = QLabel("●")
        self.live_dot.setStyleSheet(f"color: {COLORS['error']}; font-size: 14px;")
        header.addWidget(self.live_dot)
        
        title_lbl = QLabel(f" {title}")
        title_lbl.setStyleSheet(f"color: {COLORS['green']}; font-size: 12px;")
        header.addWidget(title_lbl)
        
        header.addStretch()
        
        header.addWidget(QLabel("Rate:"))
        self.rate_spin = QSpinBox()
        self.rate_spin.setRange(1, 10)
        self.rate_spin.setValue(1)
        self.rate_spin.setSuffix("s")
        self.rate_spin.setFixedWidth(55)
        self.rate_spin.valueChanged.connect(self.change_rate)
        header.addWidget(self.rate_spin)
        
        layout.addLayout(header)
        
        # Canvas
        self.canvas = MonitorCanvas()
        layout.addWidget(self.canvas)
        
        # Footer
        footer = QHBoxLayout()
        
        self.status = QLabel("Updating...")
        self.status.setStyleSheet(f"color: {COLORS['gray']}; font-size: 10px;")
        footer.addWidget(self.status)
        
        footer.addStretch()
        
        self.pause_btn = QPushButton("Pause")
        self.pause_btn.clicked.connect(self.toggle_pause)
        footer.addWidget(self.pause_btn)
        
        close_btn = QPushButton("Close")
        close_btn.setObjectName("closeBtn")
        close_btn.clicked.connect(self.close)
        footer.addWidget(close_btn)
        
        layout.addLayout(footer)
        
        # Timer
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.do_refresh)
        self.timer.start(self.refresh_rate)
        
        # Pulse
        self.pulse_timer = QTimer(self)
        self.pulse_timer.timeout.connect(self.pulse_dot)
        self.pulse_timer.start(500)
        self.pulse_state = True
        
        self.do_refresh()
    
    def pulse_dot(self):
        if self.is_running:
            self.pulse_state = not self.pulse_state
            color = COLORS['error'] if self.pulse_state else COLORS['bg_darkest']
            self.live_dot.setStyleSheet(f"color: {color}; font-size: 14px;")
    
    def change_rate(self, val):
        self.refresh_rate = val * 1000
        if self.is_running:
            self.timer.setInterval(self.refresh_rate)
    
    def toggle_pause(self):
        if self.is_running:
            self.timer.stop()
            self.is_running = False
            self.pause_btn.setText("Resume")
            self.live_dot.setStyleSheet(f"color: {COLORS['warning']}; font-size: 14px;")
        else:
            self.timer.start(self.refresh_rate)
            self.is_running = True
            self.pause_btn.setText("Pause")
    
    def do_refresh(self):
        try:
            # Get raw data from backend
            result = subprocess.run(
                [self.backend_path, "-c", self.command],
                capture_output=True, text=True, timeout=5
            )
            
            # Parse output into data dict
            data = self.parse_output(result.stdout)
            self.canvas.update_data(data)
            
            import datetime
            self.status.setText(f"Updated: {datetime.datetime.now().strftime('%H:%M:%S')}")
        except Exception as e:
            self.status.setText(f"Error: {e}")
    
    def parse_output(self, output):
        """Parse backend output into structured data"""
        data = {}
        lines = output.split('\n')
        
        # Remove ANSI codes
        ansi_escape = re.compile(r'\x1b\[[0-9;]*m')
        lines = [ansi_escape.sub('', l) for l in lines]
        
        if self.monitor_type == 'sysmon':
            for line in lines:
                if 'Model:' in line:
                    data['cpu_model'] = line.split(':', 1)[1].strip()[:50]
                elif 'Cores:' in line:
                    data['cpu_cores'] = line.split(':', 1)[1].strip()
                elif 'Usage:' in line and '%' in line:
                    try:
                        pct = float(re.search(r'([\d.]+)%', line).group(1))
                        if 'cpu_pct' not in data:
                            data['cpu_pct'] = pct
                    except: pass
                elif 'Total:' in line and 'mem_total' not in data:
                    data['mem_total'] = line.split(':', 1)[1].strip().split()[0]
                elif 'Used:' in line and 'mem_used' not in data:
                    parts = line.split(':', 1)[1].strip()
                    data['mem_used'] = parts.split()[0]
                    try:
                        pct = float(re.search(r'([\d.]+)%', parts).group(1))
                        data['mem_pct'] = pct
                    except: pass
                elif 'Load' in line:
                    data['load_avg'] = line.split(':', 1)[1].strip() if ':' in line else ''
            
            # Disk from df
            try:
                df = subprocess.run(['df', '-h', '/'], capture_output=True, text=True)
                for l in df.stdout.split('\n')[1:]:
                    parts = l.split()
                    if len(parts) >= 5:
                        data['disk_total'] = parts[1]
                        data['disk_used'] = parts[2]
                        data['disk_pct'] = float(parts[4].replace('%', ''))
                        break
            except: 
                data['disk_pct'] = 0
                
        elif self.monitor_type in ['meminfo']:
            for line in lines:
                if 'Total:' in line:
                    data['mem_total'] = line.split(':', 1)[1].strip()
                elif 'Available:' in line:
                    data['mem_avail'] = line.split(':', 1)[1].strip()
                elif 'Free:' in line:
                    data['mem_free'] = line.split(':', 1)[1].strip()
                elif 'Buffers:' in line:
                    data['mem_buffers'] = line.split(':', 1)[1].strip()
                elif 'Cached:' in line:
                    data['mem_cached'] = line.split(':', 1)[1].strip()
                elif 'used' in line.lower() and '%' in line:
                    try:
                        pct = float(re.search(r'([\d.]+)%', line).group(1))
                        data['mem_pct'] = pct
                    except: pass
            data['mem_avail_pct'] = 100 - data.get('mem_pct', 50)
            data['buf_pct'] = 5
            data['cache_pct'] = 15
            
        elif self.monitor_type in ['diskinfo']:
            for line in lines:
                if 'Total:' in line:
                    data['disk_total'] = line.split(':', 1)[1].strip()
                elif 'Used:' in line:
                    data['disk_used'] = line.split(':', 1)[1].strip()
                elif 'Available:' in line:
                    data['disk_avail'] = line.split(':', 1)[1].strip()
                elif 'Mount:' in line:
                    data['disk_mount'] = line.split(':', 1)[1].strip()
                elif '%' in line:
                    try:
                        pct = float(re.search(r'([\d.]+)%', line).group(1))
                        if 'disk_pct' not in data:
                            data['disk_pct'] = pct
                    except: pass
            if 'disk_pct' not in data:
                data['disk_pct'] = 50
                
        elif self.monitor_type in ['paging', 'vmstat']:
            # paging/vmstat output: "key      value" (whitespace separated)
            for line in lines:
                line = line.strip()
                # Skip headers, empty lines, and decorations
                if not line or line.startswith('=') or line.startswith('-') or \
                   line.startswith('Source') or line.startswith('Key') or \
                   line.startswith('[') or line.startswith('...'):
                    continue
                # Parse space-separated key value pairs
                parts = line.split()
                if len(parts) >= 2:
                    key = parts[0][:25]
                    val = parts[1]
                    # Skip header row
                    if key.lower() in ['statistic', 'count']:
                        continue
                    if key and val:
                        data[key] = val
                        
        else:
            # Generic - try colon-separated first, then space-separated
            for line in lines:
                line = line.strip()
                if not line or line.startswith('=') or line.startswith('-'):
                    continue
                if ':' in line and not line.startswith('#'):
                    parts = line.split(':', 1)
                    if len(parts) == 2:
                        key = parts[0].strip()[:25]
                        val = parts[1].strip()[:40]
                        if key and val:
                            data[key] = val
                else:
                    # Try space-separated
                    parts = line.split()
                    if len(parts) >= 2:
                        key = parts[0][:25]
                        val = ' '.join(parts[1:])[:40]
                        if key and val and not key.lower() in ['#', 'source']:
                            data[key] = val
        
        return data
    
    def closeEvent(self, event):
        self.timer.stop()
        self.pulse_timer.stop()
        self.closed_signal.emit()
        event.accept()


# ═══════════════════════════════════════════════════════════════════════════════
#                          TERMINAL TAB (GUI ONLY)
# ═══════════════════════════════════════════════════════════════════════════════

class SuggestionPopup(QListWidget):
    """Popup for command suggestions"""
    
    suggestion_selected = pyqtSignal(str)
    
    def __init__(self, parent=None):
        super().__init__(parent)
        # Use Window flag to make it float above, but not steal focus
        self.setWindowFlags(
            Qt.WindowType.Window | 
            Qt.WindowType.FramelessWindowHint |
            Qt.WindowType.WindowStaysOnTopHint |
            Qt.WindowType.Tool
        )
        self.setAttribute(Qt.WidgetAttribute.WA_ShowWithoutActivating)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.setStyleSheet(f"""
            QListWidget {{
                background: {COLORS['bg_mid']};
                color: {COLORS['green']};
                border: 2px solid {COLORS['green']};
                font-family: {FONT_MONO};
                font-size: 12px;
                padding: 4px;
            }}
            QListWidget::item {{
                padding: 6px 10px;
            }}
            QListWidget::item:selected {{
                background: {COLORS['green']};
                color: {COLORS['bg_darkest']};
            }}
            QListWidget::item:hover {{
                background: {COLORS['green_dim']};
            }}
        """)
        self.setMaximumHeight(200)
        self.setMinimumWidth(180)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        self.itemClicked.connect(self.on_item_clicked)
        self.itemDoubleClicked.connect(self.on_item_clicked)
        self.target_input = None
    
    def on_item_clicked(self, item):
        if self.target_input:
            self.target_input.setText(item.text() + " ")
            self.target_input.setFocus()
        self.hide()
        self.suggestion_selected.emit(item.text())
    
    def show_suggestions(self, suggestions, input_widget):
        if not suggestions:
            self.hide()
            return
        
        self.clear()
        self.target_input = input_widget
        
        for s in suggestions:
            item = QListWidgetItem(s)
            self.addItem(item)
        
        # Position below input widget  
        global_pos = input_widget.mapToGlobal(input_widget.rect().bottomLeft())
        self.move(global_pos.x(), global_pos.y() + 2)
        
        # Size based on content
        width = max(200, input_widget.width() // 2)
        height = min(len(suggestions) * 32 + 12, 200)
        self.setFixedSize(width, height)
        
        self.setCurrentRow(0)
        self.show()
        self.raise_()
        
        # Return focus to input
        input_widget.setFocus()
    
    def select_current(self):
        """Select the current item and apply it"""
        item = self.currentItem()
        if item and self.target_input:
            self.target_input.setText(item.text() + " ")
            self.target_input.setFocus()
            self.hide()
            return True
        return False


class CommandLineEdit(QLineEdit):
    """Custom line edit with Tab completion support"""
    
    tab_pressed = pyqtSignal()
    up_pressed = pyqtSignal()
    down_pressed = pyqtSignal()
    escape_pressed = pyqtSignal()
    
    def keyPressEvent(self, event):
        if event.key() == Qt.Key.Key_Tab:
            self.tab_pressed.emit()
            event.accept()
            return
        elif event.key() == Qt.Key.Key_Up:
            self.up_pressed.emit()
            event.accept()
            return
        elif event.key() == Qt.Key.Key_Down:
            self.down_pressed.emit()
            event.accept()
            return
        elif event.key() == Qt.Key.Key_Escape:
            self.escape_pressed.emit()
            event.accept()
            return
        super().keyPressEvent(event)


class TerminalTab(QWidget):
    """Single terminal tab - GUI wrapper for C backend process"""
    
    def __init__(self, tab_id, backend_path, parent=None):
        super().__init__(parent)
        self.tab_id = tab_id
        self.backend_path = backend_path
        self.process = None
        self.command_history = []
        self.history_index = -1
        self.cwd = os.getcwd()
        self.pid = None
        self.live_windows = []
        self.suggestion_popup = None
        
        self.live_commands = [
            'paging', 'vmstat', 'meminfo', 'cpuinfo', 'proclist', 'proctop',
            'netinfo', 'netstat', 'connections', 'loadavg', 'diskinfo',
            'swapinfo', 'sysmon', 'modules', 'mounts', 'filesystems',
            'kernelinfo', 'battery', 'sensors', 'users', 'openfiles', 'sockets',
            'interrupts', 'zoneinfo'
        ]
        
        self.setup_ui()
        self.start_backend()
    
    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        
        # Output area
        self.output = QTextEdit()
        self.output.setReadOnly(True)
        self.output.setFont(QFont("monospace", 11))
        self.output.setStyleSheet(f"""
            QTextEdit {{
                background-color: {COLORS['bg_darkest']};
                color: {COLORS['green']};
                border: none;
                padding: 8px 12px;
            }}
            QScrollBar:vertical {{
                background: {COLORS['bg_darkest']};
                width: 8px;
            }}
            QScrollBar::handle:vertical {{
                background: {COLORS['border']};
                min-height: 30px;
            }}
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
                height: 0;
            }}
        """)
        layout.addWidget(self.output)
        
        # Input bar
        input_bar = QFrame()
        input_bar.setStyleSheet(f"""
            QFrame {{
                background: {COLORS['bg_darkest']};
                border-top: 1px solid {COLORS['border']};
            }}
        """)
        input_layout = QHBoxLayout(input_bar)
        input_layout.setContentsMargins(12, 6, 12, 6)
        input_layout.setSpacing(8)
        
        # CWD
        self.cwd_label = QLabel(self.get_short_cwd())
        self.cwd_label.setStyleSheet(f"""
            color: {COLORS['cyan']};
            font-family: {FONT_MONO};
            font-size: 11px;
        """)
        self.cwd_label.setMinimumWidth(60)
        input_layout.addWidget(self.cwd_label)
        
        # Prompt
        prompt = QLabel("$")
        prompt.setStyleSheet(f"color: {COLORS['green']}; font-size: 12px; font-weight: bold;")
        input_layout.addWidget(prompt)
        
        # Input
        self.input_line = CommandLineEdit()
        self.input_line.setFont(QFont("monospace", 11))
        self.input_line.setPlaceholderText("")
        self.input_line.setStyleSheet(f"""
            QLineEdit {{
                background-color: {COLORS['bg_darkest']};
                color: {COLORS['green']};
                border: none;
                padding: 6px;
            }}
        """)
        self.input_line.returnPressed.connect(self.execute_command)
        self.input_line.textChanged.connect(self.on_text_changed)
        self.input_line.tab_pressed.connect(self.on_tab_pressed)
        self.input_line.up_pressed.connect(self.on_up_pressed)
        self.input_line.down_pressed.connect(self.on_down_pressed)
        self.input_line.escape_pressed.connect(self.on_escape_pressed)
        input_layout.addWidget(self.input_line)
        
        # Suggestion popup
        self.suggestion_popup = SuggestionPopup(self)
        
        # PID
        self.pid_label = QLabel("---")
        self.pid_label.setStyleSheet(f"color: {COLORS['gray']}; font-family: {FONT_MONO}; font-size: 10px;")
        self.pid_label.setFixedWidth(50)
        input_layout.addWidget(self.pid_label)
        
        # Message indicator
        self.msg_indicator = QLabel("")
        self.msg_indicator.setStyleSheet(f"color: {COLORS['warning']}; font-size: 12px;")
        self.msg_indicator.setFixedWidth(15)
        input_layout.addWidget(self.msg_indicator)
        
        layout.addWidget(input_bar)
        
        # Timers
        self.msg_timer = QTimer(self)
        self.msg_timer.timeout.connect(self.check_messages)
        self.msg_timer.start(2000)
        
        self.show_welcome()
    
    def show_welcome(self):
        welcome = """
\033[32mNLP Terminal\033[0m - Type 'help' for commands, 'uniquecmds' for custom commands

"""
        self.append_ansi(welcome)
    
    def get_short_cwd(self):
        home = os.path.expanduser('~')
        cwd = self.cwd
        if cwd.startswith(home):
            cwd = '~' + cwd[len(home):]
        if len(cwd) > 30:
            parts = cwd.split('/')
            if len(parts) > 3:
                cwd = '.../' + '/'.join(parts[-2:])
        return cwd
    
    def start_backend(self):
        self.process = QProcess(self)
        self.process.readyReadStandardOutput.connect(self.handle_output)
        self.process.readyReadStandardError.connect(self.handle_error)
        self.process.finished.connect(self.handle_finished)
        self.process.start(self.backend_path)
        QTimer.singleShot(100, self.get_pid)
    
    def get_pid(self):
        if self.process and self.process.state() == QProcess.ProcessState.Running:
            self.pid = self.process.processId()
            self.pid_label.setText(f"{self.pid}")
            self.process.write(b"register\n")
    
    def handle_output(self):
        data = self.process.readAllStandardOutput().data().decode('utf-8', errors='replace')
        self.process_output(data)
    
    def handle_error(self):
        data = self.process.readAllStandardError().data().decode('utf-8', errors='replace')
        self.append_output(data, QColor(COLORS['error']))
    
    def handle_finished(self):
        self.append_output("\n[Process ended]\n", QColor(COLORS['gray']))
    
    def process_output(self, text):
        skip = ["Goodbye!", "NLP_TRANSLATED:", "macro_rec>"]
        lines = []
        for line in text.split('\n'):
            if any(p in line for p in skip):
                continue
            s = line.strip()
            if s.endswith('>') and '/' in s:
                path = s.rstrip('>').strip()
                if path.startswith('/'):
                    self.cwd = path
                    self.cwd_label.setText(self.get_short_cwd())
                continue
            lines.append(line)
        if lines:
            output = '\n'.join(lines)
            # Ensure output ends with newline for proper separation
            if not output.endswith('\n'):
                output += '\n'
            self.append_ansi(output)
    
    def append_output(self, text, color=None):
        cursor = self.output.textCursor()
        cursor.movePosition(QTextCursor.MoveOperation.End)
        fmt = QTextCharFormat()
        fmt.setForeground(color or QColor(COLORS['green']))
        cursor.setCharFormat(fmt)
        cursor.insertText(text)
        self.output.setTextCursor(cursor)
        self.output.ensureCursorVisible()
    
    def append_ansi(self, text):
        cursor = self.output.textCursor()
        cursor.movePosition(QTextCursor.MoveOperation.End)
        
        colors = {
            '30': '#1e1e1e', '31': COLORS['error'], '32': COLORS['green'],
            '33': COLORS['warning'], '34': COLORS['cyan'], '35': '#c678dd',
            '36': COLORS['cyan'], '37': COLORS['white'],
            '90': COLORS['gray'], '91': COLORS['error'], '92': COLORS['green'],
            '93': COLORS['warning'], '94': COLORS['cyan'], '95': '#c678dd',
            '96': COLORS['cyan'], '97': COLORS['white']
        }
        
        pattern = re.compile(r'\x1b\[([0-9;]+)m')
        fmt = QTextCharFormat()
        fmt.setForeground(QColor(COLORS['green']))
        
        pos = 0
        for m in pattern.finditer(text):
            if m.start() > pos:
                cursor.setCharFormat(fmt)
                cursor.insertText(text[pos:m.start()])
            for code in m.group(1).split(';'):
                if code == '0':
                    fmt = QTextCharFormat()
                    fmt.setForeground(QColor(COLORS['green']))
                elif code == '1':
                    fmt.setFontWeight(700)
                elif code in colors:
                    fmt.setForeground(QColor(colors[code]))
            pos = m.end()
        
        if pos < len(text):
            cursor.setCharFormat(fmt)
            cursor.insertText(text[pos:])
        
        self.output.setTextCursor(cursor)
        self.output.ensureCursorVisible()
    
    def on_text_changed(self, text):
        """Show suggestions as user types"""
        if self.suggestion_popup is None:
            return
        
        # Only show for first word (command), at least 2 chars
        text = text.strip()
        if ' ' in text or len(text) < 2:
            self.suggestion_popup.hide()
            return
        
        suggestions = get_suggestions(text, 6)
        
        # Don't show if exact match
        if suggestions and text.lower() == suggestions[0].lower():
            self.suggestion_popup.hide()
            return
        
        if suggestions:
            self.suggestion_popup.show_suggestions(suggestions, self.input_line)
        else:
            self.suggestion_popup.hide()
    
    def execute_command(self):
        cmd = self.input_line.text().strip()
        if not cmd:
            return
        
        # Hide suggestions
        if self.suggestion_popup is not None:
            self.suggestion_popup.hide()
        
        self.input_line.clear()
        self.command_history.append(cmd)
        self.history_index = len(self.command_history)
        
        if cmd.lower() == 'clear':
            self.output.clear()
            return
        
        if cmd.lower() == 'uniquecmds':
            self.show_unique_commands()
            return
        
        self.append_output(f"$ {cmd}\n", QColor(COLORS['green']))
        
        base_cmd = cmd.split()[0].lower()
        
        # Check if it's a live command
        if base_cmd in self.live_commands:
            self.open_live_monitor(cmd)
            return
        
        # Check for did-you-mean before sending to backend
        # Skip for !n (history recall) and history n commands
        is_history_recall = (base_cmd.startswith('!') and len(base_cmd) > 1 and base_cmd[1:].isdigit())
        is_history_cmd = base_cmd == 'history'
        
        if not is_history_recall and not is_history_cmd:
            suggestions = get_did_you_mean(base_cmd)
            if suggestions and base_cmd not in ALL_COMMANDS:
                self.append_output(f"Did you mean: ", QColor(COLORS['gray']))
                self.append_output(f"{', '.join(suggestions)}", QColor(COLORS['cyan']))
                self.append_output("?\n", QColor(COLORS['gray']))
        
        if self.process and self.process.state() == QProcess.ProcessState.Running:
            self.process.write(f"{cmd}\n".encode())
    
    def show_unique_commands(self):
        help_text = """
\033[32m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m
  \033[1;36mUnique Commands\033[0m  —  Not in standard UNIX
\033[32m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m

\033[33m  Live OS Monitoring (opens real-time windows):\033[0m
    \033[32mpaging\033[0m        Page fault & swap stats from /proc/vmstat
    \033[32mvmstat\033[0m        Virtual memory statistics
    \033[32mproclist\033[0m      Process list (custom top)
    \033[32mproctop\033[0m       Top N processes by memory
    \033[32mzoneinfo\033[0m      Memory zone info (NUMA)
    \033[32mconnections\033[0m   Active network connections
    \033[32mopenfiles\033[0m     Open file descriptors
    \033[32msockets\033[0m       Socket statistics

\033[33m  File Utilities:\033[0m
    \033[32mfileinfo <f>\033[0m  Detailed file info (size, hash, perms)
    \033[32mhexdump <f>\033[0m   Hex view of file
    \033[32mduplicate\033[0m     Find duplicates by hash
    \033[32mfreq <file>\033[0m   Word frequency analysis
    \033[32mlines <file>\033[0m  Line/word/char stats
    \033[32mage <days>\033[0m    Find files by age
    \033[32msizeof <pat>\033[0m  Size of matching files

\033[33m  Inter-Process Communication:\033[0m
    \033[32mmypid\033[0m              This terminal's PID
    \033[32mterminals\033[0m          List all terminal tabs
    \033[32msendmsg <PID> <m>\033[0m  Send message to terminal
    \033[32mbroadcast <msg>\033[0m    Message all terminals
    \033[32mcheckmsg\033[0m           Check messages
    \033[32mreadmsg\033[0m            Read messages
    \033[32mclearmsg\033[0m           Clear inbox

\033[33m  Expression & Math:\033[0m
    \033[32mcalc <expr>\033[0m       Calculator (+,-,*,/,^,%,parens)
    \033[32minfix2postfix\033[0m     Convert to postfix
    \033[32minfix2prefix\033[0m      Convert to prefix
    \033[32mvisualeval\033[0m        Step-by-step evaluation

\033[33m  Encoding & Conversion:\033[0m
    \033[32mbase64 e|d <t>\033[0m    Base64 encode/decode
    \033[32mmorse e|d <t>\033[0m     Morse code
    \033[32mrot13 <text>\033[0m      ROT13 cipher
    \033[32mbinary <num>\033[0m      Decimal to binary
    \033[32mhex <num>\033[0m         Decimal to hex
    \033[32moctal <num>\033[0m       Decimal to octal

\033[33m  Productivity:\033[0m
    \033[32mquicknote add\033[0m     Add quick note
    \033[32mquicknote list\033[0m    List notes
    \033[32mrecent\033[0m            Recently modified files

\033[33m  Automation:\033[0m
    \033[32mmacro define <n>\033[0m  Record commands
    \033[32mmacro end\033[0m         Stop recording
    \033[32mmacro run <n>\033[0m     Run macro
    \033[32mteach on|off\033[0m      Teaching mode

\033[33m  Natural Language:\033[0m
    "show files"        → ls -la
    "go home"           → cd ~
    "find large files"  → find . -size +1M
    "calculate 2+3*4"   → calc 2+3*4

\033[32m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m
"""
        self.append_ansi(help_text)
    
    def open_live_monitor(self, cmd):
        title = cmd.split()[0].upper()
        win = LiveMonitorWindow(title, cmd, self.backend_path, self)
        win.closed_signal.connect(lambda: self.on_live_closed(win))
        win.show()
        self.live_windows.append(win)
        self.append_output(f"[● Opened live {title} monitor]\n", QColor(COLORS['eco_cyan']))
    
    def on_live_closed(self, w):
        if w in self.live_windows:
            self.live_windows.remove(w)
    
    def check_messages(self):
        if not self.pid:
            return
        inbox = f"/tmp/nlpterminal_ipc/inbox_{self.pid}"
        try:
            if os.path.exists(inbox) and os.path.getsize(inbox) > 0:
                self.msg_indicator.setText("●")
            else:
                self.msg_indicator.setText("")
        except:
            pass
    
    def on_tab_pressed(self):
        """Handle Tab key - autocomplete"""
        if self.suggestion_popup is not None and self.suggestion_popup.isVisible():
            self.suggestion_popup.select_current()
        else:
            text = self.input_line.text().strip()
            if text and ' ' not in text:
                suggestions = get_suggestions(text, 1)
                if suggestions:
                    self.input_line.setText(suggestions[0] + " ")
    
    def on_up_pressed(self):
        """Handle Up key - navigate suggestions or history"""
        if self.suggestion_popup and self.suggestion_popup.isVisible():
            row = self.suggestion_popup.currentRow()
            if row > 0:
                self.suggestion_popup.setCurrentRow(row - 1)
        else:
            if self.history_index > 0:
                self.history_index -= 1
                self.input_line.setText(self.command_history[self.history_index])
    
    def on_down_pressed(self):
        """Handle Down key - navigate suggestions or history"""
        if self.suggestion_popup and self.suggestion_popup.isVisible():
            row = self.suggestion_popup.currentRow()
            if row < self.suggestion_popup.count() - 1:
                self.suggestion_popup.setCurrentRow(row + 1)
        else:
            if self.history_index < len(self.command_history) - 1:
                self.history_index += 1
                self.input_line.setText(self.command_history[self.history_index])
            else:
                self.history_index = len(self.command_history)
                self.input_line.clear()
    
    def on_escape_pressed(self):
        """Handle Escape key - close suggestions"""
        if self.suggestion_popup and self.suggestion_popup.isVisible():
            self.suggestion_popup.hide()
    
    def cleanup(self):
        if self.process and self.process.state() == QProcess.ProcessState.Running:
            self.process.write(b"unregister\n")
            self.process.write(b"exit\n")
            self.process.waitForFinished(500)
            self.process.kill()
        for w in self.live_windows:
            if w.isVisible():
                w.close()


# ═══════════════════════════════════════════════════════════════════════════════
#                          MAIN WINDOW
# ═══════════════════════════════════════════════════════════════════════════════

class NLPTerminal(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("NLP Terminal")
        self.setMinimumSize(1000, 700)
        self.resize(1200, 800)
        
        self.backend_path = self.find_backend()
        self.tab_counter = 0
        self.terminals = {}
        
        self.setup_ui()
        self.setup_menu()
        self.new_tab()
    
    def find_backend(self):
        for p in [
            os.path.join(os.path.dirname(__file__), '..', 'backend', 'mysh'),
            os.path.join(os.getcwd(), 'backend', 'mysh'),
            './backend/mysh'
        ]:
            if os.path.exists(p):
                return os.path.abspath(p)
        return 'backend/mysh'
    
    def setup_ui(self):
        central = QWidget()
        self.setCentralWidget(central)
        layout = QVBoxLayout(central)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        
        self.tabs = QTabWidget()
        self.tabs.setTabsClosable(True)
        self.tabs.setMovable(True)
        self.tabs.setDocumentMode(True)
        self.tabs.tabCloseRequested.connect(self.close_tab)
        self.tabs.setStyleSheet(f"""
            QTabWidget::pane {{
                border: none;
                background: {COLORS['bg_darkest']};
            }}
            QTabBar::tab {{
                background: {COLORS['bg_light']};
                color: {COLORS['gray']};
                padding: 6px 16px;
                margin-right: 1px;
                font-family: {FONT_MONO};
                font-size: 11px;
            }}
            QTabBar::tab:selected {{
                background: {COLORS['bg_darkest']};
                color: {COLORS['green']};
            }}
            QTabBar::tab:hover {{
                background: {COLORS['bg_hover']};
                color: {COLORS['green_dim']};
            }}
            QTabBar::close-button {{
                image: none;
                subcontrol-position: right;
            }}
            QTabBar::close-button:hover {{
                background: {COLORS['error']};
            }}
        """)
        layout.addWidget(self.tabs)
        
        # Status
        self.status = QLabel("  Ctrl+T New | Ctrl+W Close | Ctrl+L Clear")
        self.status.setStyleSheet(f"""
            background: {COLORS['bg_darkest']};
            color: {COLORS['gray']};
            padding: 4px 12px;
            font-family: {FONT_MONO};
            font-size: 10px;
            border-top: 1px solid {COLORS['border']};
        """)
        layout.addWidget(self.status)
    
    def setup_menu(self):
        bar = self.menuBar()
        bar.setStyleSheet(f"""
            QMenuBar {{
                background: {COLORS['bg_darkest']};
                color: {COLORS['green']};
                padding: 2px;
                font-family: {FONT_MONO};
                font-size: 11px;
            }}
            QMenuBar::item:selected {{
                background: {COLORS['bg_light']};
            }}
            QMenu {{
                background: {COLORS['bg_darkest']};
                color: {COLORS['green']};
                border: 1px solid {COLORS['border']};
                font-family: {FONT_MONO};
            }}
            QMenu::item:selected {{
                background: {COLORS['green']};
                color: {COLORS['bg_darkest']};
            }}
        """)
        
        term = bar.addMenu("Terminal")
        
        new_act = QAction("New Tab", self)
        new_act.setShortcut(QKeySequence("Ctrl+T"))
        new_act.triggered.connect(self.new_tab)
        term.addAction(new_act)
        
        close_act = QAction("Close Tab", self)
        close_act.setShortcut(QKeySequence("Ctrl+W"))
        close_act.triggered.connect(self.close_current_tab)
        term.addAction(close_act)
        
        term.addSeparator()
        
        exit_act = QAction("Exit", self)
        exit_act.setShortcut(QKeySequence("Ctrl+Q"))
        exit_act.triggered.connect(self.close)
        term.addAction(exit_act)
        
        view = bar.addMenu("View")
        
        clear_act = QAction("Clear", self)
        clear_act.setShortcut(QKeySequence("Ctrl+L"))
        clear_act.triggered.connect(self.clear_output)
        view.addAction(clear_act)
        
        hlp = bar.addMenu("Help")
        
        unique_act = QAction("Unique Commands", self)
        unique_act.triggered.connect(self.show_unique)
        hlp.addAction(unique_act)
        
        cmds_act = QAction("All Commands", self)
        cmds_act.triggered.connect(lambda: self.run_cmd("help"))
        hlp.addAction(cmds_act)
        
        os_act = QAction("OS Commands", self)
        os_act.triggered.connect(lambda: self.run_cmd("oshelp"))
        hlp.addAction(os_act)
        
        ipc_act = QAction("IPC Commands", self)
        ipc_act.triggered.connect(lambda: self.run_cmd("ipchelp"))
        hlp.addAction(ipc_act)
    
    def show_unique(self):
        i = self.tabs.currentIndex()
        if i >= 0:
            w = self.tabs.widget(i)
            if hasattr(w, 'show_unique_commands'):
                w.show_unique_commands()
    
    def run_cmd(self, cmd):
        i = self.tabs.currentIndex()
        if i >= 0:
            w = self.tabs.widget(i)
            if hasattr(w, 'process') and w.process:
                w.append_output(f"\n$ {cmd}\n", QColor(COLORS['green']))
                w.process.write(f"{cmd}\n".encode())
    
    def new_tab(self):
        self.tab_counter += 1
        term = TerminalTab(self.tab_counter, self.backend_path, self)
        self.terminals[self.tab_counter] = term
        i = self.tabs.addTab(term, f"Terminal {self.tab_counter}")
        self.tabs.setCurrentIndex(i)
        term.input_line.setFocus()
    
    def close_tab(self, i):
        w = self.tabs.widget(i)
        if w:
            for tid, t in list(self.terminals.items()):
                if t == w:
                    t.cleanup()
                    del self.terminals[tid]
                    break
            self.tabs.removeTab(i)
        if self.tabs.count() == 0:
            self.close()
    
    def close_current_tab(self):
        i = self.tabs.currentIndex()
        if i >= 0:
            self.close_tab(i)
    
    def clear_output(self):
        i = self.tabs.currentIndex()
        if i >= 0:
            w = self.tabs.widget(i)
            if hasattr(w, 'output'):
                w.output.clear()
    
    def closeEvent(self, event):
        for t in self.terminals.values():
            t.cleanup()
        event.accept()


# ═══════════════════════════════════════════════════════════════════════════════
#                          MAIN
# ═══════════════════════════════════════════════════════════════════════════════

def main():
    app = QApplication(sys.argv)
    app.setStyle('Fusion')
    
    # Dark palette
    pal = QPalette()
    pal.setColor(QPalette.ColorRole.Window, QColor(COLORS['bg_darkest']))
    pal.setColor(QPalette.ColorRole.WindowText, QColor(COLORS['green']))
    pal.setColor(QPalette.ColorRole.Base, QColor(COLORS['bg_darkest']))
    pal.setColor(QPalette.ColorRole.Text, QColor(COLORS['green']))
    pal.setColor(QPalette.ColorRole.Button, QColor(COLORS['bg_mid']))
    pal.setColor(QPalette.ColorRole.ButtonText, QColor(COLORS['green']))
    pal.setColor(QPalette.ColorRole.Highlight, QColor(COLORS['green']))
    pal.setColor(QPalette.ColorRole.HighlightedText, QColor(COLORS['bg_darkest']))
    app.setPalette(pal)
    
    win = NLPTerminal()
    win.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
