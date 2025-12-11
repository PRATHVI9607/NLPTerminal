"""
Advanced NLP Module for Natural Language Command Translation
Uses pattern matching, keyword extraction, and context understanding
"""

import re
from typing import Optional, Dict, List, Tuple

class NLPCommandTranslator:
    def __init__(self):
        # Action verbs mapping
        self.action_verbs = {
            'show': ['display', 'list', 'view', 'see', 'print'],
            'create': ['make', 'new', 'add', 'generate'],
            'delete': ['remove', 'erase', 'destroy', 'kill'],
            'move': ['relocate', 'transfer', 'shift'],
            'copy': ['duplicate', 'clone', 'replicate'],
            'find': ['search', 'locate', 'look'],
            'change': ['modify', 'alter', 'update', 'switch'],
            'compare': ['diff', 'check', 'verify'],
        }
        
        # Object types
        self.object_types = {
            'file': ['document', 'txt', 'data'],
            'folder': ['directory', 'dir'],
            'files': ['contents', 'items'],
        }
        
        # Command patterns with regex
        self.patterns = [
            # Show/List patterns
            (r'(?:show|display|list|view)\s+(?:all\s+)?(?:the\s+)?files?(?:\s+in\s+(.+))?', 
             lambda m: f"ls {m.group(1) if m.group(1) else ''}"),
            
            (r'(?:show|display|list)\s+(?:the\s+)?(?:directory\s+)?tree(?:\s+of\s+(.+))?',
             lambda m: f"tree {m.group(1) if m.group(1) else ''}"),
            
            (r'(?:show|display)\s+(?:the\s+)?(?:current\s+)?(?:working\s+)?directory',
             lambda m: "pwd"),
            
            (r'(?:where\s+am\s+i|current\s+location|current\s+path)',
             lambda m: "pwd"),
            
            # Create patterns
            (r'(?:create|make|new)\s+(?:a\s+)?(?:new\s+)?folder(?:\s+(?:called|named))?\s+(\S+)',
             lambda m: f"mkdir {m.group(1)}"),
            
            (r'(?:create|make|new)\s+(?:a\s+)?(?:new\s+)?(?:file|document)(?:\s+(?:called|named))?\s+(\S+)',
             lambda m: f"touch {m.group(1)}"),
            
            # Delete patterns
            (r'(?:delete|remove|erase)\s+(?:the\s+)?folder(?:\s+(?:called|named))?\s+(\S+)',
             lambda m: f"rmdir {m.group(1)}"),
            
            (r'(?:delete|remove|erase)\s+(?:the\s+)?(?:file|document)(?:\s+(?:called|named))?\s+(\S+)',
             lambda m: f"rm {m.group(1)}"),
            
            # Copy patterns
            (r'(?:copy|duplicate)\s+(\S+)\s+to\s+(\S+)',
             lambda m: f"cp {m.group(1)} {m.group(2)}"),
            
            (r'(?:copy|duplicate)\s+(?:the\s+)?(?:file|document)\s+(\S+)\s+(?:to|as)\s+(\S+)',
             lambda m: f"cp {m.group(1)} {m.group(2)}"),
            
            # Move patterns
            (r'(?:move|relocate)\s+(\S+)\s+to\s+(\S+)',
             lambda m: f"mv {m.group(1)} {m.group(2)}"),
            
            (r'(?:rename)\s+(\S+)\s+(?:to|as)\s+(\S+)',
             lambda m: f"mv {m.group(1)} {m.group(2)}"),
            
            # Read/Display file content
            (r'(?:read|show|display|view|print)\s+(?:the\s+)?(?:content(?:s)?(?:\s+of)?|file)\s+(\S+)',
             lambda m: f"cat {m.group(1)}"),
            
            (r'(?:what(?:\'?s|\s+is)\s+in(?:side)?)\s+(\S+)',
             lambda m: f"cat {m.group(1)}"),
            
            # Search patterns
            (r'(?:search|find|look)\s+for\s+["\']?(.+?)["\']?(?:\s+in\s+files)?',
             lambda m: f"search {m.group(1)}"),
            
            # Change directory
            (r'(?:go\s+to|change\s+(?:to|directory\s+to)|navigate\s+to|cd\s+to)\s+(\S+)',
             lambda m: f"cd {m.group(1)}"),
            
            (r'(?:go|move)\s+(?:up|back)(?:\s+one\s+level)?',
             lambda m: "cd .."),
            
            (r'(?:go|return)\s+(?:to\s+)?home',
             lambda m: "cd ~"),
            
            # Backup
            (r'(?:backup|save\s+copy\s+of)\s+(\S+)',
             lambda m: f"backup {m.group(1)}"),
            
            # Compare
            (r'(?:compare|diff|check\s+difference)\s+(\S+)\s+(?:and|with|vs)\s+(\S+)',
             lambda m: f"compare {m.group(1)} {m.group(2)}"),
            
            # Recent files
            (r'(?:show|list|display)\s+(?:recent|latest|new)\s+files',
             lambda m: "recent"),
            
            # Bulk rename
            (r'(?:rename|change)\s+all\s+(?:files\s+)?(?:with|containing)\s+(\S+)\s+to\s+(\S+)',
             lambda m: f"bulk_rename {m.group(1)} {m.group(2)}"),
            
            # Stats
            (r'(?:show|display)\s+(?:shell\s+)?(?:statistics|stats|info)',
             lambda m: "stats"),
            
            # Bookmark
            (r'(?:save|bookmark|mark)\s+(?:this\s+)?(?:location|directory|path)(?:\s+as)?\s+(\S+)',
             lambda m: f"bookmark {m.group(1)} ."),
            
            (r'(?:go\s+to|jump\s+to)\s+bookmark\s+(\S+)',
             lambda m: f"bookmark {m.group(1)}"),
            
            (r'(?:show|list)\s+(?:all\s+)?bookmarks',
             lambda m: "bookmark"),
            
            # Help
            (r'(?:help|show\s+commands|what\s+can\s+(?:i|you)\s+do)',
             lambda m: "help"),
            
            # History
            (r'(?:show|display)\s+(?:command\s+)?history',
             lambda m: "history"),
            
            # Echo
            (r'(?:say|print|echo)\s+(.+)',
             lambda m: f"echo {m.group(1)}"),
        ]
    
    def normalize_text(self, text: str) -> str:
        """Normalize input text"""
        text = text.lower().strip()
        # Remove extra whitespace
        text = re.sub(r'\s+', ' ', text)
        return text
    
    def expand_synonyms(self, text: str) -> str:
        """Expand synonyms to base action verbs"""
        words = text.split()
        expanded = []
        
        for word in words:
            found = False
            for base_verb, synonyms in self.action_verbs.items():
                if word in synonyms:
                    expanded.append(base_verb)
                    found = True
                    break
            if not found:
                expanded.append(word)
        
        return ' '.join(expanded)
    
    def translate(self, natural_language: str) -> str:
        """
        Translate natural language to shell command
        Returns the original text if no translation found
        """
        if not natural_language or not natural_language.strip():
            return natural_language
        
        # Normalize
        text = self.normalize_text(natural_language)
        
        # Expand synonyms
        text = self.expand_synonyms(text)
        
        # Try pattern matching
        for pattern, translator in self.patterns:
            match = re.search(pattern, text, re.IGNORECASE)
            if match:
                try:
                    command = translator(match)
                    return command.strip()
                except:
                    continue
        
        # If no pattern matched, return original
        return natural_language
    
    def get_suggestions(self, partial_command: str, available_commands: List[str]) -> List[str]:
        """Get command suggestions based on partial input"""
        partial = partial_command.lower()
        suggestions = []
        
        for cmd in available_commands:
            if cmd.lower().startswith(partial):
                suggestions.append(cmd)
        
        # Also check for natural language hints
        if len(suggestions) == 0:
            if 'show' in partial or 'list' in partial:
                suggestions.extend(['ls', 'tree', 'recent', 'history'])
            elif 'create' in partial or 'make' in partial:
                suggestions.extend(['mkdir', 'touch'])
            elif 'delete' in partial or 'remove' in partial:
                suggestions.extend(['rm', 'rmdir'])
            elif 'find' in partial or 'search' in partial:
                suggestions.append('search')
        
        return suggestions[:5]  # Return top 5

# Example usage and testing
if __name__ == "__main__":
    translator = NLPCommandTranslator()
    
    test_cases = [
        "show all files",
        "create a new folder called test",
        "delete the file named old.txt",
        "copy file1.txt to file2.txt",
        "what's in myfile.txt",
        "search for hello in files",
        "go to home",
        "backup important.doc",
        "show recent files",
        "rename all files with old to new",
    ]
    
    print("=== NLP Translation Tests ===")
    for test in test_cases:
        result = translator.translate(test)
        print(f"'{test}' -> '{result}'")
