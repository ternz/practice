import socket
import sys

def main():
	if len(sys.argv) != 3:
		usage()
		return
	port = sys.argv[1]
	if port == none:
		usage()
		return
	addr = ("127.0.0.1", argv[1])
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.connect(addr)
	
def usage():
	print "usage: ", sys.argv[0], "<port>\n"
	
if __name__ == "__main__":
	main()