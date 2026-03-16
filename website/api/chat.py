#!/usr/bin/env python3
"""PyroBot API Proxy — routes chatbot requests to NVIDIA API to avoid CORS"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import urllib.request
import ssl

NVIDIA_API_URL = "https://integrate.api.nvidia.com/v1/chat/completions"
NVIDIA_API_KEY = "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"
NVIDIA_MODEL = "meta/llama-3.3-70b-instruct"
PORT = 9877

class ChatHandler(BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(200)
        self._cors_headers()
        self.end_headers()

    def do_POST(self):
        try:
            length = int(self.headers.get('Content-Length', 0))
            body = json.loads(self.rfile.read(length)) if length else {}

            messages = body.get('messages', [])

            payload = json.dumps({
                "model": NVIDIA_MODEL,
                "messages": messages,
                "temperature": 0.7,
                "max_tokens": 2048,
                "top_p": 0.9
            }).encode()

            req = urllib.request.Request(
                NVIDIA_API_URL,
                data=payload,
                headers={
                    "Content-Type": "application/json",
                    "Authorization": f"Bearer {NVIDIA_API_KEY}"
                }
            )

            ctx = ssl.create_default_context()
            with urllib.request.urlopen(req, timeout=60, context=ctx) as resp:
                data = json.loads(resp.read())

            msg = data.get("choices", [{}])[0].get("message", {})
            reply = msg.get("content") or msg.get("reasoning_content") or "I couldn't generate a response."

            self.send_response(200)
            self._cors_headers()
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"reply": reply}).encode())

        except Exception as e:
            self.send_response(500)
            self._cors_headers()
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(json.dumps({"error": str(e)}).encode())

    def _cors_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")

    def log_message(self, format, *args):
        pass  # Suppress logs

if __name__ == "__main__":
    server = HTTPServer(("127.0.0.1", PORT), ChatHandler)
    print(f"PyroBot API proxy running on port {PORT}")
    server.serve_forever()
