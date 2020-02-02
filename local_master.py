import argparse
import sys
#---------------------------------------------------------------------------# 
# import the various server implementations
#---------------------------------------------------------------------------# 
from pymodbus.client.sync import ModbusTcpClient

#---------------------------------------------------------------------------#
# time - sleep, datetime 
#---------------------------------------------------------------------------#
import time
import datetime

#---------------------------------------------------------------------------# 
# configure the client logging
#---------------------------------------------------------------------------# 
import logging
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.DEBUG)

parser = argparse.ArgumentParser(description = 'option of master')
parser.add_argument("-i", "--ipaddress", type=str, help = 'ip address of the modbus server', required=True)
args = parser.parse_args()
server_ip = args.ipaddress

#---------------------------------------------------------------------------# 
# connect to server IP : 'localhost', PORT : 502 
#---------------------------------------------------------------------------# 
client = ModbusTcpClient(server_ip, port=502)
client.connect()

#---------------------------------------------------------------------------# 
# read coils 
#---------------------------------------------------------------------------# 
def reading():
		log.debug("Reading Coils")
		rq = client.read_coils(0, 5, unit=0x01)	
		time.sleep(2)
#---------------------------------------------------------------------------# 
# write coils 
#---------------------------------------------------------------------------# 
def writing_zero():
	log.debug("write Coils")
	rr = client.write_coils(0, [False]*5, unit=0x01)
		
def writing_one():
	log.debug("write Coils")
	rr = client.write_coils(0, [True]*5, unit=0x01)

def main():
	while True:
		writing_zero();
		reading();
		time.sleep(5);
		writing_one();
		reading();
		time.sleep(5);

#---------------------------------------------------------------------------# 
# main
#---------------------------------------------------------------------------# 
if __name__ == "__main__":
	main()