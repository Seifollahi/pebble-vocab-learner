import http.server
import socketserver
import os
import socket

PORT = 8000

class PebbleHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Prevent caching
        self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Expires', '0')
        super().end_headers()

    def guess_type(self, path):
        if path.endswith('.pbw'):
            return 'application/octet-stream'
        return super().guess_type(path)

def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        # doesn't even have to be reachable
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
        IP = '127.0.0.1'
    finally:
        s.close()
    return IP

# Change to the serve directory
os.chdir(os.path.dirname(os.path.abspath(__file__)))

with socketserver.TCPServer(("", PORT), PebbleHandler) as httpd:
    ip = get_local_ip()
    print(f"Server started!")
    print(f"On your phone, open your browser and go to:")
    print(f"http://{ip}:{PORT}")
    print(f"Make sure your phone and computer are on the same Wi-Fi network.")
    httpd.serve_forever()
