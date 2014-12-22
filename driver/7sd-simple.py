from time import sleep 
import serial

intraBitDelay = 0.001 # secs

def chrToBinary (character, point = False):
    convTable = {\
        'a':"1110111",\
        'b':"1111100",\
        'c':"1011000",\
        'd':"1011110",\
        'e':"1111001",\
        'f':"1110001",\
        'g':"1111101",\
        'h':"1110100",\
        'i':"0110000",\
        'j':"0011110",\
        'k':"1110110",\
        'l':"0111000",\
        'm':"0010101",\
        'n':"1010100",\
        'o':"1011100",\
        'p':"1110011",\
        'q':"1100111",\
        'r':"1010000",\
        's':"1101101",\
        't':"1111000",\
        'u':"0111110",\
        'v':"0011100",\
        'w':"0101010",\
        'x':"1110110",\
        'y':"1101110",\
        'z':"1011011",\

        # NUMEROS
        '0':"0111111",\
        '1':"0000110",\
        '2':"1011011",\
        '3':"1001111",\
        '4':"1100110",\
        '5':"1101101",\
        '6':"1111101",\
        '7':"0000111",\
        '8':"1111111",\
        '9':"1101111",\

        # TESTING DISPLAY
        'A':"0000001",\
        'B':"0000010",\
        'C':"0000100",\
        'D':"0001000",\
        'E':"0010000",\
        'F':"0100000",\
        'G':"1000000",\

        # ESPECIALES
        # '.':"0000000",\
        '-':"1000000",\
        '_':"0001000",\
        '=':"1001000",\
        ' ':"0000000",\
        '?':"1010011",\
        '"':"0100010",\
        '\'':"0100000"\
    }

    if len(character) > 1:
        raise Exception ("chrToBinary: 1 character expected. " + \
                         str(len(character)) + " recieved.")

    try:
        bitStream = convTable[character]
        if point:
            bitStream = "1" + bitStream
        else:
            bitStream = "0" + bitStream

        return bitStream

    except KeyError:
        raise Exception ("Caracter: " + character + " not found in conversion table")   

def sendChar(serialObj, chr):
    if len(chr) <> 8:
        raise Exception ("Not 8 bits character")

    for bit in chr:
        # Set data line HIGH or LOW
        if bit == '0':
            serialObj.setDTR(False)
        else:
            serialObj.setDTR(True)
        
        # Send clock signal
        serialObj.setRTS(True)
        sleep(intraBitDelay)
        serialObj.setRTS(False) 

def sendStrobe(serialObj):
    # Send strobe signal
    serialObj.write(str(0xFF))
    # Wait write to finish
    serialObj.flush()
  
def sendStr(serialObj, str, strobePerChar = False, delay=0):
    point = False
    for chr in str[::-1]:  
        if chr <> '.':
            sendChar(serialObj, chrToBinary(chr, point))
            point = False
        else:
            if point:
                sendChar(serialObj, chrToBinary(" ", point))
            point = True
        if strobePerChar:
            sendStrobe(serialObj)

        sleep(delay)
    sendStrobe(serialObj)
