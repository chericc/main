
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import os
import urllib.parse
import xml.etree.ElementTree as ET

class WebDAVHandler(SimpleHTTPRequestHandler):

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("DAV", "1")
        # self.send_header("Allow", "OPTIONS, GET, PUT, DELETE, PROPFIND, MKCOL")
        self.send_header("Allow", "OPTIONS, GET, PUT")
        self.send_header("Content-Length", "0")
        self.end_headers()
        
    def do_PUT(self):
        path = self.translate_path(self.path)
        length = int(self.headers['Content-Length'])

        os.makedirs(os.path.dirname(path), exist_ok=True)

        with open(path, 'wb') as f:
            f.write(self.rfile.read(length))

        self.send_response(201)
        self.end_headers()

    # def do_DELETE(self):
    #     path = self.translate_path(self.path)

    #     if os.path.isdir(path):
    #         os.rmdir(path)
    #     else:
    #         os.remove(path)

    #     self.send_response(204)
    #     self.end_headers()

    # def do_MKCOL(self):
    #     path = self.translate_path(self.path)
    #     os.makedirs(path, exist_ok=True)

    #     self.send_response(201)
    #     self.end_headers()

    def do_PROPFIND(self):
        path = self.translate_path(self.path)

        self.send_response(207)
        self.send_header("Content-Type", "text/xml")
        self.end_headers()

        root = ET.Element("D:multistatus", {
            "xmlns:D": "DAV:"
        })

        def add_item(p):
            href = urllib.parse.quote(
                os.path.join(self.path, os.path.basename(p))
            )

            resp = ET.SubElement(root, "D:response")

            ET.SubElement(resp, "D:href").text = href

            propstat = ET.SubElement(resp, "D:propstat")
            prop = ET.SubElement(propstat, "D:prop")

            ET.SubElement(prop, "D:displayname").text = os.path.basename(p)

            resourcetype = ET.SubElement(prop, "D:resourcetype")

            if os.path.isdir(p):
                ET.SubElement(resourcetype, "D:collection")

            ET.SubElement(propstat, "D:status").text = \
                "HTTP/1.1 200 OK"

        add_item(path)

        if os.path.isdir(path):
            for f in os.listdir(path):
                add_item(os.path.join(path, f))

        xml = ET.tostring(root, encoding="utf-8")
        self.wfile.write(xml)

def run(port=8000):
    server = ThreadingHTTPServer(("0.0.0.0", port), WebDAVHandler)

    print(f"WebDAV server running on {port}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()

if __name__ == "__main__":
    run()