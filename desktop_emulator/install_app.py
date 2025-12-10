#!/usr/bin/env python3
"""
PocketMage Emulator App Installer

Automatically installs a standalone app (from tar or folder) into the
desktop emulator, handling all code modifications and rebuilding.

Usage: 
    python3 install_app.py <app.tar>
    python3 install_app.py <app_folder>

The app should follow the StarterApp/BlankApp template structure with:
    - src/appMain.cpp (main app code)
    - Optional: icon.bmp or similar icon file
"""

import os
import sys
import re
import tarfile
import tempfile
import shutil
import subprocess

# Paths relative to this script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
POCKETMAGE_SRC = os.path.join(SCRIPT_DIR, '..', 'Code', 'PocketMage_V3', 'src')
GLOBALS_H = os.path.join(SCRIPT_DIR, '..', 'Code', 'PocketMage_V3', 'include', 'globals.h')
POCKETMAGE_V3_CPP = os.path.join(POCKETMAGE_SRC, 'PocketMageV3.cpp')
HOME_CPP = os.path.join(POCKETMAGE_SRC, 'OS_APPS', 'HOME.cpp')
CMAKE_FILE = os.path.join(SCRIPT_DIR, 'CMakeLists.txt')
DATA_APPS = os.path.join(SCRIPT_DIR, 'data', 'apps')
BUILD_DIR = os.path.join(SCRIPT_DIR, 'build')


def read_file(path):
    with open(path, 'r') as f:
        return f.read()


def write_file(path, content):
    with open(path, 'w') as f:
        f.write(content)


def find_source_file(folder):
    """Find the main app source file."""
    candidates = [
        os.path.join(folder, 'src', 'appMain.cpp'),
        os.path.join(folder, 'appMain.cpp'),
    ]
    for c in candidates:
        if os.path.exists(c):
            return c
    # Search for any .cpp
    for root, dirs, files in os.walk(folder):
        for f in files:
            if f.endswith('.cpp') and 'main' in f.lower():
                return os.path.join(root, f)
    return None


def find_icon_file(folder):
    """Find icon file in the folder."""
    for root, dirs, files in os.walk(folder):
        for f in files:
            if 'icon' in f.lower() and (f.endswith('.bmp') or f.endswith('.bin')):
                return os.path.join(root, f)
    return None


def remove_function(code, func_signature_pattern):
    """Remove a function by finding matching braces."""
    match = re.search(func_signature_pattern, code)
    if not match:
        return code
    
    start = match.start()
    # Find the opening brace
    brace_start = code.find('{', match.end())
    if brace_start == -1:
        return code
    
    # Count braces to find the end
    depth = 1
    pos = brace_start + 1
    while pos < len(code) and depth > 0:
        if code[pos] == '{':
            depth += 1
        elif code[pos] == '}':
            depth -= 1
        pos += 1
    
    # Remove from start to end of function
    return code[:start] + '\n// Function removed - handled by main firmware\n' + code[pos:]


def convert_app_code(source_code, app_name):
    """Convert standalone app code to integrated format."""
    app_upper = app_name.upper()
    
    code = source_code
    
    # Replace includes
    code = re.sub(r'#include\s*<pocketmage\.h>', '#include <globals.h>', code)
    
    # Rename main functions
    code = re.sub(r'\bvoid\s+processKB\s*\(\s*\)', f'void processKB_{app_upper}()', code)
    code = re.sub(r'\bvoid\s+applicationEinkHandler\s*\(\s*\)', f'void einkHandler_{app_upper}()', code)
    
    # Replace rebootToPocketMage with state change back to HOME
    code = re.sub(
        r'rebootToPocketMage\s*\(\s*\)\s*;',
        'CurrentAppState = HOME; HOME_INIT(); return;',
        code
    )
    
    # Remove standalone-only functions using brace matching
    code = remove_function(code, r'void\s+setup\s*\(\s*\)')
    code = remove_function(code, r'void\s+loop\s*\(\s*\)')
    code = remove_function(code, r'void\s+einkHandler\s*\(\s*void\s*\*')
    
    # Remove ASCII art banners (MAIN section markers)
    code = re.sub(r'/{40,}[^/]*/{40,}', '', code, flags=re.DOTALL)
    
    # Clean up multiple blank lines
    code = re.sub(r'\n{4,}', '\n\n\n', code)
    
    # Find where to insert INIT function (before the first function definition)
    # Look for processKB_ or einkHandler_ which we just renamed
    # Check if appInit() function exists in the code
    has_app_init = 'void appInit()' in code or 'void appInit ()' in code
    
    if has_app_init:
        # Rename appInit to appInit_APPNAME to avoid conflicts
        code = code.replace('void appInit()', f'void appInit_{app_upper}()')
        code = code.replace('appInit();', f'appInit_{app_upper}();')
        
        init_code = f'''
// Forward declaration for app init
void appInit_{app_upper}();

// Auto-generated INIT function
void {app_upper}_INIT() {{
    CurrentAppState = {app_upper};
    newState = true;
    appInit_{app_upper}();  // Call app's init function
}}

'''
    else:
        init_code = f'''
// Auto-generated INIT function
void {app_upper}_INIT() {{
    CurrentAppState = {app_upper};
    newState = true;
}}

'''
    
    # Insert before processKB_APPNAME function
    match = re.search(rf'void\s+processKB_{app_upper}\s*\(', code)
    if match:
        code = code[:match.start()] + init_code + code[match.start():]
    else:
        # Fallback: insert before einkHandler
        match = re.search(rf'void\s+einkHandler_{app_upper}\s*\(', code)
        if match:
            code = code[:match.start()] + init_code + code[match.start():]
    
    return code


def add_to_globals_h(app_name):
    """Add app to globals.h enum and declarations."""
    app_upper = app_name.upper()
    
    content = read_file(GLOBALS_H)
    
    # Check if already added
    if f'{app_upper},' in content or f'{app_upper} ' in content:
        print(f"  {app_upper} already in globals.h")
        return True
    
    # Add to AppState enum (find the enum and add before the closing brace or last item)
    # Look for pattern like "HELLO," and add after it
    enum_pattern = r'(enum\s+AppState\s*\{[^}]*)(HELLO\s*,)'
    match = re.search(enum_pattern, content, re.DOTALL)
    if match:
        content = content[:match.end()] + f'\n    {app_upper},' + content[match.end():]
    else:
        # Try to find any enum entry to add after
        enum_pattern2 = r'(enum\s+AppState\s*\{[^}]*?)(\s*\};)'
        match2 = re.search(enum_pattern2, content, re.DOTALL)
        if match2:
            content = content[:match2.start(2)] + f',\n    {app_upper}' + content[match2.start(2):]
    
    # Add function declarations (find where other app declarations are)
    decl_pattern = r'(void\s+einkHandler_HELLO\s*\(\s*\)\s*;)'
    decl = f'''
// {app_upper} App
void {app_upper}_INIT();
void processKB_{app_upper}();
void einkHandler_{app_upper}();
'''
    
    match = re.search(decl_pattern, content)
    if match:
        content = content[:match.end()] + '\n' + decl + content[match.end():]
    else:
        # Add before the last #endif
        content = re.sub(r'(\n#endif\s*)$', decl + r'\1', content)
    
    write_file(GLOBALS_H, content)
    print(f"  Added {app_upper} to globals.h")
    return True


def add_to_pocketmage_v3(app_name):
    """Add switch cases to PocketMageV3.cpp."""
    app_upper = app_name.upper()
    
    content = read_file(POCKETMAGE_V3_CPP)
    
    # Check if already added
    if f'case {app_upper}:' in content:
        print(f"  {app_upper} already in PocketMageV3.cpp")
        return True
    
    # Add to applicationEinkHandler switch
    eink_pattern = r'(case\s+HELLO\s*:\s*\n\s*einkHandler_HELLO\s*\(\s*\)\s*;\s*\n\s*break\s*;)'
    eink_case = f'''
        case {app_upper}:
            einkHandler_{app_upper}();
            break;'''
    
    match = re.search(eink_pattern, content)
    if match:
        content = content[:match.end()] + '\n' + eink_case + content[match.end():]
    
    # Add to processKB switch
    kb_pattern = r'(case\s+HELLO\s*:\s*\n\s*processKB_HELLO\s*\(\s*\)\s*;\s*\n\s*break\s*;)'
    kb_case = f'''
        case {app_upper}:
            processKB_{app_upper}();
            break;'''
    
    match = re.search(kb_pattern, content)
    if match:
        content = content[:match.end()] + '\n' + kb_case + content[match.end():]
    
    write_file(POCKETMAGE_V3_CPP, content)
    print(f"  Added {app_upper} switch cases to PocketMageV3.cpp")
    return True


def add_to_home_commands(app_name):
    """Add command to launch the app from HOME."""
    app_upper = app_name.upper()
    app_lower = app_name.lower()
    
    content = read_file(HOME_CPP)
    
    # Check if already added
    if f'{app_upper}_INIT()' in content:
        print(f"  {app_upper} command already in HOME.cpp")
        return True
    
    # Look for the HELLO_INIT pattern with various formats
    # Pattern: else if (command == "hello" ... ) { HELLO_INIT(); }
    patterns = [
        # Multi-line with multiple command options
        r'(else\s+if\s*\([^)]*"hello"[^)]*\)\s*\{\s*\n\s*HELLO_INIT\s*\(\s*\)\s*;\s*\n\s*\})',
        # Simple single-line style
        r'(else\s+if\s*\(command\s*==\s*"hello"\)\s*\{\s*HELLO_INIT\s*\(\s*\)\s*;\s*\})',
    ]
    
    cmd_code = f'''
  /////////////////////////////
  else if (command == "{app_lower}") {{
    {app_upper}_INIT();
  }}'''
    
    inserted = False
    for pattern in patterns:
        match = re.search(pattern, content, re.DOTALL)
        if match:
            content = content[:match.end()] + cmd_code + content[match.end():]
            write_file(HOME_CPP, content)
            print(f"  Added '{app_lower}' command to HOME.cpp")
            inserted = True
            break
    
    if not inserted:
        # Try a more general approach - find any HELLO_INIT and add after that block
        match = re.search(r'HELLO_INIT\s*\(\s*\)\s*;[^}]*\}', content)
        if match:
            content = content[:match.end()] + cmd_code + content[match.end():]
            write_file(HOME_CPP, content)
            print(f"  Added '{app_lower}' command to HOME.cpp")
        else:
            print(f"  Warning: Could not add command to HOME.cpp (add manually)")
    
    return True


def add_to_applauncher(app_name):
    """Add app entry to APPLAUNCHER.cpp installed apps list."""
    app_upper = app_name.upper()
    app_lower = app_name.lower()
    
    applauncher_path = os.path.join(POCKETMAGE_SRC, 'OS_APPS', 'APPLAUNCHER.cpp')
    if not os.path.exists(applauncher_path):
        print(f"  Warning: APPLAUNCHER.cpp not found")
        return False
    
    content = read_file(applauncher_path)
    
    # Check if already added
    if f'"{app_lower}"' in content and f'{app_upper}_INIT' in content:
        print(f"  {app_upper} already in APPLAUNCHER.cpp")
        return True
    
    # Add extern declaration for the INIT function
    extern_pattern = r'(extern void HELLO_INIT\(\);)'
    extern_decl = f'\nextern void {app_upper}_INIT();'
    
    match = re.search(extern_pattern, content)
    if match and f'extern void {app_upper}_INIT()' not in content:
        content = content[:match.end()] + extern_decl + content[match.end():]
    
    # Add to installedApps array
    app_entry = f'\n    {{"{app_name}", "{app_lower}", "/apps/{app_lower}_icon.bin", {app_upper}_INIT}},'
    
    # Find the last entry in installedApps and add after it
    pattern = r'(\{"[^"]+",\s*"[^"]+",\s*"[^"]+",\s*\w+_INIT\},)\s*\n(\s*// Add more apps here)'
    match = re.search(pattern, content)
    if match:
        content = content[:match.end(1)] + app_entry + '\n' + content[match.start(2):]
        write_file(applauncher_path, content)
        print(f"  Added {app_upper} to APPLAUNCHER.cpp")
        return True
    
    # Try alternate pattern - just find the array entries
    pattern2 = r'(\{"StarterApp",\s*"starterapp",\s*"/apps/starterapp_icon\.bin",\s*STARTERAPP_INIT\},)'
    match = re.search(pattern2, content)
    if match:
        content = content[:match.end()] + app_entry + content[match.end():]
        write_file(applauncher_path, content)
        print(f"  Added {app_upper} to APPLAUNCHER.cpp")
        return True
    
    print(f"  Warning: Could not add to APPLAUNCHER.cpp (add manually)")
    return False


def add_to_cmake(app_name):
    """Add source file to CMakeLists.txt."""
    app_upper = app_name.upper()
    
    content = read_file(CMAKE_FILE)
    
    source_file = f'${{POCKETMAGE_SRC}}/{app_upper}_APP.cpp'
    
    # Check if already added
    if source_file in content or f'{app_upper}_APP.cpp' in content:
        print(f"  {app_upper}_APP.cpp already in CMakeLists.txt")
        return True
    
    # Find POCKETMAGE_SOURCES and add after HELLO_WORLD.cpp
    pattern = r'(\$\{POCKETMAGE_SRC\}/HELLO_WORLD\.cpp)'
    
    match = re.search(pattern, content)
    if match:
        content = content[:match.end()] + f'\n    {source_file}' + content[match.end():]
        write_file(CMAKE_FILE, content)
        print(f"  Added {app_upper}_APP.cpp to CMakeLists.txt")
    else:
        print(f"  Warning: Could not add to CMakeLists.txt (add manually)")
    
    return True


def rebuild_emulator():
    """Rebuild the emulator."""
    print("\nRebuilding emulator...")
    
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)
        subprocess.run(['cmake', '..'], cwd=BUILD_DIR, check=True)
    
    result = subprocess.run(['make', '-j4'], cwd=BUILD_DIR, capture_output=True, text=True)
    
    if result.returncode != 0:
        print("Build failed!")
        print(result.stderr)
        return False
    
    print("Build successful!")
    return True


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 install_app.py <app.tar or app_folder>")
        print("\nExample:")
        print("  python3 install_app.py ../Code/StarterApp")
        print("  python3 install_app.py MyApp.tar")
        sys.exit(1)
    
    source_path = sys.argv[1]
    temp_dir = None
    
    # Handle tar file
    if source_path.endswith('.tar') or source_path.endswith('.tar.gz'):
        if not os.path.exists(source_path):
            print(f"Error: File not found: {source_path}")
            sys.exit(1)
        
        temp_dir = tempfile.mkdtemp()
        print(f"Extracting {source_path}...")
        
        with tarfile.open(source_path, 'r:*') as tar:
            tar.extractall(temp_dir)
        
        app_folder = temp_dir
        # Get app name from tar filename
        app_name = os.path.basename(source_path).replace('.tar.gz', '').replace('.tar', '')
    else:
        if not os.path.isdir(source_path):
            print(f"Error: Folder not found: {source_path}")
            sys.exit(1)
        
        app_folder = source_path
        app_name = os.path.basename(os.path.normpath(source_path))
    
    # Clean up app name - keep original case for display, use uppercase for code
    app_name = re.sub(r'[^a-zA-Z0-9]', '', app_name)  # Remove special chars, keep case
    
    print(f"\n{'='*60}")
    print(f"Installing app: {app_name}")
    print(f"{'='*60}\n")
    
    # Find source file
    source_file = find_source_file(app_folder)
    if not source_file:
        print("Error: No source file found (looking for appMain.cpp)")
        if temp_dir:
            shutil.rmtree(temp_dir)
        sys.exit(1)
    
    print(f"Found source: {source_file}")
    
    # Find icon
    icon_file = find_icon_file(app_folder)
    if icon_file:
        print(f"Found icon: {icon_file}")
    
    # Convert the code
    print("\nConverting app code...")
    source_code = read_file(source_file)
    converted_code = convert_app_code(source_code, app_name)
    
    # Write converted file
    output_path = os.path.join(POCKETMAGE_SRC, f'{app_name}_APP.cpp')
    write_file(output_path, converted_code)
    print(f"  Created: {output_path}")
    
    # Copy icon if present
    if icon_file:
        os.makedirs(DATA_APPS, exist_ok=True)
        icon_dest = os.path.join(DATA_APPS, f'{app_name.lower()}_icon.bin')
        shutil.copy(icon_file, icon_dest)
        print(f"  Copied icon to: {icon_dest}")
    
    # Modify other files
    print("\nUpdating project files...")
    add_to_globals_h(app_name)
    add_to_pocketmage_v3(app_name)
    add_to_home_commands(app_name)
    add_to_applauncher(app_name)
    add_to_cmake(app_name)
    
    # Rebuild
    if rebuild_emulator():
        print(f"\n{'='*60}")
        print(f"SUCCESS! App '{app_name}' installed.")
        print(f"{'='*60}")
        print(f"\nTo launch in emulator, type: {app_name.lower()}")
        print(f"Or run: ./build/PocketMage_PDA_Emulator")
    else:
        print("\nBuild failed. Check errors above.")
    
    # Cleanup
    if temp_dir:
        shutil.rmtree(temp_dir)


if __name__ == '__main__':
    main()
