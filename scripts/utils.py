import os
import subprocess
import sys


def print_color(text, color=None):
    colors = {
        "green": "\033[92m",
        "red": "\033[91m",
        "yellow": "\033[93m",
        "blue": "\033[94m",
        "magenta": "\033[95m",
        "cyan": "\033[96m",
        "bold": "\033[1m",
        "reset": "\033[0m"
    }
    if color in colors:
        print(f"{colors[color]}{text}{colors['reset']}")
    else:
        print(text)


def run_command(command, check=True, capture_output=False, env=None, stream_output=False):
    """
    Runs a shell command or a list of arguments.
    
    If stream_output is True, the output is printed line-by-line as it's generated,
    and capture_output is ignored. The function returns the returncode.
    If stream_output is False, it behaves like a standard subprocess.run wrapper,
    returning the CompletedProcess instance (or exception if check=False).
    """
    if env is None:
        env = os.environ.copy()

    is_shell = isinstance(command, str)

    if stream_output:
        process = subprocess.Popen(
            command, shell=is_shell, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env
        )
        for line in iter(process.stdout.readline, b''):
            print(line.decode().rstrip())
        process.stdout.close()
        returncode = process.wait()
        if check and returncode != 0:
            print_color(f"Command failed: {command}", "red")
            sys.exit(returncode)
        return returncode
    else:
        try:
            return subprocess.run(
                command, shell=is_shell, check=check, capture_output=capture_output, text=True, env=env
            )
        except subprocess.CalledProcessError as e:
            if check:
                print_color(f"Command failed: {command if isinstance(command, str) else ' '.join(command)}", "red")
                if e.stdout: print(e.stdout)
                if e.stderr: print(e.stderr)
                sys.exit(1)
            return e
