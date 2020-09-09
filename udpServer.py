import socket

sock = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 1234))

print("UDP server up and listening")

try:
    sock.sendto(b"initialized", ("127.0.0.1", 1234))
    while(True):
        bytesAddressPair = sock.recvfrom(1024)

        message = bytesAddressPair[0]
        address = bytesAddressPair[1]

        print("{} {}".format(address, message))
except KeyboardInterrupt:
    pass
