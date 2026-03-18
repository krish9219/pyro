#!/usr/bin/env python3
"""Pyro Playground API — compile and run Pyro code in the browser"""

import subprocess
import tempfile
import os
import signal
import sys
from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

PYRO_BIN = "/projects/pyro/build/pyro"
MAX_CODE_LENGTH = 10000
TIMEOUT_SECONDS = 30

# Blocked imports that could be dangerous on a shared server
BLOCKED_IMPORTS = ["os", "sys", "subprocess", "fs", "process", "signal", "net", "http", "smtp", "dns", "ping", "websocket"]

@app.route("/run", methods=["POST"])
def run_code():
    data = request.get_json()
    if not data or "code" not in data:
        return jsonify({"error": "No code provided"}), 400

    code = data["code"]

    if len(code) > MAX_CODE_LENGTH:
        return jsonify({"error": f"Code too long (max {MAX_CODE_LENGTH} chars)"}), 400

    # Security: block dangerous imports
    for blocked in BLOCKED_IMPORTS:
        if f'import {blocked}' in code:
            return jsonify({"error": f"import {blocked} is not allowed in the playground for security reasons"}), 400

    # Write code to temp file
    with tempfile.NamedTemporaryFile(mode='w', suffix='.ro', delete=False, dir='/tmp') as f:
        f.write(code)
        temp_path = f.name

    try:
        # Run with timeout
        result = subprocess.run(
            [PYRO_BIN, "run", temp_path],
            capture_output=True,
            text=True,
            timeout=TIMEOUT_SECONDS,
            cwd='/tmp'
        )

        output = result.stdout
        error = result.stderr

        # Clean up C++ errors to be more friendly
        if error:
            # Remove temp file paths from error messages
            error = error.replace(temp_path, "playground.ro")
            # Remove noise from error output
            error = "\n".join(
                line for line in error.split("\n")
                if not line.startswith("[pyro]")
                and not line.startswith("[compiled in")
                and not line.startswith("Generated C++")
            )

        return jsonify({
            "output": output,
            "error": error.strip() if error else "",
            "exit_code": result.returncode
        })

    except subprocess.TimeoutExpired:
        return jsonify({"error": "Code execution timed out (10 second limit)", "output": "", "exit_code": 1}), 200
    except Exception as e:
        return jsonify({"error": str(e), "output": "", "exit_code": 1}), 500
    finally:
        # Cleanup temp files
        try:
            os.unlink(temp_path)
            # Also clean up compiled binary
            bin_path = temp_path.replace('.ro', '')
            if os.path.exists(bin_path):
                os.unlink(bin_path)
            cpp_path = '/tmp/pyro_' + os.path.basename(temp_path).replace('.ro', '') + '.cpp'
            if os.path.exists(cpp_path):
                os.unlink(cpp_path)
        except:
            pass

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=9878, debug=False)
