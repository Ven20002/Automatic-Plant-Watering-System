import rp2
import network
import ubinascii
import machine
import urequests as requests
import time
import utime
import socket
from machine import UART, Pin
import os

# Set country to avoid possible errors
rp2.country('DE')

# Wi-Fi configuration
ssid = 'Viktor_Wifi'
pw = 'EPAtraktor123'

# Initialize UART
uart = UART(0, 9600)
uart.init(9600, bits=8, parity=None, stop=1)
led = Pin('LED', Pin.OUT)

# Connect to Wi-Fi
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(ssid, pw)

# Wait for connection with 10 second timeout
timeout = 10
while timeout > 0:
    if wlan.status() < 0 or wlan.status() >= 3:
        break
    timeout -= 1
    print('Waiting for connection...')
    time.sleep(1)

# Define blinking function for onboard LED to indicate error codes    
def blink_onboard_led(num_blinks):
    led = machine.Pin('LED', machine.Pin.OUT)
    for i in range(num_blinks):
        led.on()
        time.sleep(.2)
        led.off()
        time.sleep(.2)

wlan_status = wlan.status()
blink_onboard_led(wlan_status)

if wlan_status != 3:
    raise RuntimeError('Wi-Fi connection failed')
else:
    print('Connected')
    status = wlan.ifconfig()
    print('ip = ' + status[0])
    led.value(1)

# Function to load HTML page    
def get_html(html_name):
    with open(html_name, 'r') as file:
        html = file.read()
    return html

def reset_data():
    try:
        os.remove('data.txt')
        print("Existing 'data.txt' removed")
    except OSError as e:
        print('Error:', e)
        print("No existing 'data.txt' found")

    try:
        with open('data.txt', 'w') as f:
            f.write('0.00,0.00,0.00,000.00,0\n')
        print("New 'data.txt' created")
        return 'Data reset successful'
    except OSError as e:
        print('Error:', e)
        return 'Error: Unable to reset data'

# Function to load text file
def get_text(text_name):
    with open(text_name, 'r') as file:
        text = file.read()
    return text

# HTTP server with socket
addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]
s = socket.socket()
s.bind(addr)
s.listen(1)

print('Listening on', addr)

def handle_client(client):
    try:
        r = client.recv(1024)
        if b'/data.txt' in r:
            response = get_text('data.txt')
            client.send('HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n')
            client.send(response)
        else:
            response = get_html('index.html')
            client.send('HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n')
            client.send(response)
    except OSError as e:
        print('Connection error:', e)
    finally:
        client.close()

while True:

    # UART Handling
    if uart == None:
        uart = UART(0, 9600)                         
        uart.init(9600, bits=8, parity=None, stop=1)
        led = Pin('LED', Pin.OUT)

    if uart.any():  
        led.value(True)
        utime.sleep(2)
        data = uart.readline()  
        file_size = os.stat('data.txt')[6]
        print(data)  
        with open('data.txt', 'a') as f:
            f.write(data)
            uart = None 
        utime.sleep(0.5)
        led.value(False)
        print('UART SAVED!')
        

    # HTTP Server Handling
    try:
        s.settimeout(1)  # Adjust the timeout value as needed
        client, addr = s.accept()
        print('Client connected from', addr)
        r = client.recv(1024)
        
        if '/reset-data' in r:
            response = reset_data()
            print('RESET DATA!')
            client.send('HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n')
            client.send(response.encode())
            continue
        
        if b'/data.txt' in r:
            response = get_text('data.txt')
            client.send('HTTP/1.0 200 OK\r\nContent-type: text/plain\r\n\r\n')
            client.send(response)
        else:
            response = get_html('index.html')
            client.send('HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n')
            client.send(response)
        client.close()
    except OSError as e:
        print('Connection error:', e)
