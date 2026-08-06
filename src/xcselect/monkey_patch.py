import urllib2
import json
import sys

# Mock response for Homebrew / CLT or fallback
class MockResponse:
    def read(self):
        return json.dumps({"packages": []}).encode('utf-8')
    def getcode(self):
        return 200

def mock_urlopen(url, data=None, timeout=None, context=None):
    print(f"Intercepted urllib2 request to: {url}")
    return MockResponse()

urllib2.urlopen = mock_urlopen
print("Monkey-patched urllib2 successfully for osxie container environment.")
