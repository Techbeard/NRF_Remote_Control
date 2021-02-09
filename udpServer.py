import socket
import winsound
duration = 100  # milliseconds
freq = 880  # Hz


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
        if(message == 'Button 0'):
            winsound.Beep(freq, duration)
            pass
        if address[0] != "127.0.0.1": # ignore own message to prevent loop
            # sock.sendto(message, address) # echo sent packet
            # sock.sendto(b"1234567890123456798012345678901234567890123456789012345678901234567890123456789012345678901234567890", address) 
            btn = message.decode().split(' ')[1]
            cmd = "setLed;{};1".format(btn)
            if btn == "0":
                cmd = "clearLeds"
            sock.sendto(cmd.encode(), address)
            sock.sendto(b'print;' + message + b'\n', address) # echo sent packet to display
except KeyboardInterrupt:
    pass