#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h> 
#include <netinet/in.h>  
#include <sys/stat.h>
#include <arpa/inet.h> 
#include <sys/types.h> 

#define FILE_NAME       "/home/jinhyeok/pi/retigrid_data.txt"

#define MAX_BUF_SIZE	1024
#define SERVER_IP       "192.168.100.2"
#define PORT_NUM        502

#define PHASE_VOLTAGE_ADDR           26495
#define LINE_VOLTAGE_ADDR            26497
#define VOLTAGE_CREST_FACTOR_ADDR    26499
#define PHASE_CURRENT_ADDR           26501
#define CURRENT_CREST_FACTOR_ADDR    26503
#define ACTIVE_POWER_ADDR            26505
#define REACTIVE_POWER_ADDR          26507
#define POWER_FACTOR_ADDR            26509
#define FREQUENCY_ADDR               26511
#define THD_PHASE_VOLTAGE_ADDR       26513
#define THD_PHASE_CURRENT_ADDR       26515
#define TEMPERATURE_ADDR             26517
#define VOLTAGE_HARMONICS_ADDR       26519
#define CURRENT_HARMONICS_ADDR       26521
#define APPARENT_POWER_ADDR          26523

#define MODULE_NUM              1


unsigned short TI = 0x0000;



typedef enum Retigrid{
    Phase_Voltage           = 0,
    Line_Volatage           = 1,
    Voltage_Crest_Factor    = 2,
    Phase_Current           = 3,
    Current_Crest_Factor    = 4,
    Active_Power            = 5,
    Reactive_Power          = 6,
    Power_Factor            = 7,
    Frequency               = 8,
    THD_Phase_Voltage       = 9,
    THD_Phase_Current       = 10,
    Temperature             = 11,
    Voltage_Harmonics       = 12,
    Current_Harmonics       = 13,
    Apparent_Power          = 14,
}retigrid;

int Make_Modbus_TCP_Packet(unsigned char* buf, unsigned short addr, int mod_num, int register_num); 
int Read_Modbus_TCP_Packet(float* data, unsigned char* buf, unsigned short addr, int register_num);

int main()
{
	struct sockaddr_in server_addr;
	int comm_sock = 0;
	int server_addr_len = 0;
	unsigned char recvBuf[MAX_BUF_SIZE]={0,};
	unsigned char sendBuf[MAX_BUF_SIZE]={0,};
    int i = 0;
    int ret = 0;

    int packet_len = 0;
    int write_len = 0;
    int read_len = 0;

    int start_array_num = 0;
    int end_array_num = 0;

    unsigned short addr = 0x0000;
    unsigned short addr_tmp = 0x0000;
    int mod_num = 0;
    int register_num = 0;
    float retigrid[15] = {0x00, };

    FILE* reti_fp = NULL;



	comm_sock = socket(PF_INET, SOCK_STREAM, 0);

	if (comm_sock == -1)
	{
		printf("error :\n");
		return 0;
	}

	memset(&server_addr, 0x00, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(PORT_NUM);

	server_addr_len = sizeof(server_addr);
    ret = connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len);
	if (ret == -1)
	{
		printf("connect error :\n");
		return 0;
	}

	


/***************************************************************************************************************************/
    addr = PHASE_VOLTAGE_ADDR;  // starting address
    mod_num = MODULE_NUM; // module num
    register_num = 15;    // Number of registers 
    
    

    while(1)
    {
        memset(sendBuf, 0x00, MAX_BUF_SIZE);
        ret = Make_Modbus_TCP_Packet(sendBuf, addr, mod_num, register_num);
        if(ret == 0)
        {
            printf("Close main\n");
            return 0;
        }

        packet_len = (int)(sendBuf[5]) + 6;
        write_len = write(comm_sock, sendBuf, packet_len);
        if (write_len <= 0)
        {
            printf("write error\n");
            return 0;
        }


        memset(recvBuf, 0x00, MAX_BUF_SIZE);
        read_len = read(comm_sock, recvBuf, MAX_BUF_SIZE) 
        if (read_len <= 0)
        {
            printf("read error\n");
            return 0;
        }
        
        ret = Read_Modbus_TCP_Packet(retigrid, recvBuf, addr, register_num);
        if(ret == 0)
        {
            printf("Close main\n");
            return 0;
        }

        printf("receive data (hex): ")
        for(i=0; i < ret; i++)
        {
            printf("%02X ",recvBuf[0]);
        }
        printf("\n");

        start_array_num = addr;
        start_array_num -= PHASE_VOLTAGE_ADDR;
        end_array_num = start_array_num + register_num
            
        for(i = start_array_num; i < end_array_num; i++)
        {
            reti_fp = fopen(FILE_NAME, "a");

            if(reti_fp == NULL)
            {
                printf("File Open Error\n");
                return 0;
            }

            printf("%4.2f\n",retigrid[i]);
            ret = fprintf(reti_fp, "%4.2f\n",retigrid[i]);   

            fclose(reti_fp);
        }
            
        printf("\n");
       
        sleep(10);
    }
    
     
	close(comm_sock);

	return 0;
}


/**
    * @output = buf
    * @input1 = addr : Starting Address of data
    * @input2 = mod_num : Module Number
    * @input3 = register_num
    * @return = 0, FAIL
                1, SUCCESS
*/
int Make_Modbus_TCP_Packet(unsigned char* buf, unsigned short addr, int mod_num, int register_num) 
{
    unsigned short addr_tmp = 0x00;
    unsigned int register_num_tmp = 0x00;
    TI += 1;

    addr_tmp = addr;

    if(mod_num > 300)
    {
        printf("Fail : Module number is at most 300!\n");
        return 0;
    }

    addr_tmp = (addr + (40*(mod_num - 1)));


    buf[0] = (unsigned char)(TI >> 4);
    buf[1] = (unsigned char)(TI);

    buf[5] = 6; // Length Field

    buf[7] = 0x03; // Function code : Read Multiple Registers

    buf[8] = (unsigned char)(addr_tmp >> 8);
    buf[9] = (unsigned char)(addr_tmp); // Starting address


    register_num_tmp = (register_num * 2);
    buf[10] = (unsigned char)(register_num_tmp >> 8);
    buf[11] = (unsigned char)(register_num_tmp); // Number of registers

    return 1;
}
	


/**
    * @output = data
    * @input1 = buf : unsigned receive data array
    * @input2 = addr : Starting Address of data
    * @input3 = register_num
    * @return = 0, FAIL
                1, SUCCESS
*/    
int Read_Modbus_TCP_Packet(float* data, unsigned char* buf, unsigned short addr, int register_num)
{
    int function_code_high_bit = 0;
    int status = 0;
    int start_array_num = 0;
    int end_array_num = 0;
    int i = 0;
    int j = 9;
    unsigned long tmp[15] = {0,};

    function_code_high_bit = (buf[7] >> 7);

    if(function_code_high_bit == 1) // exception check
    {
        status = buf[8];

        switch (status)
        {
            case 1:
                printf("ERROR : Illegal Funstion\n");
                return 0;
                break;

            case 2:
                printf("ERROR : Illegal Data Address\n");
                return 0;
                break;   

            case 3:
                printf("ERROR : Illegal Data Value\n");
                return 0;
                break;

            case 4:
                printf("ERROR : Server Failure\n");
                return 0;
                break;

            case 5:
                printf("ERROR : Acknowledge\n");
                return 0;
                break;

            case 6:
                printf("ERROR : Server Busy\n");
                return 0;
                break;   

            default:
                break;
        }
    }

    start_array_num = (addr - PHASE_VOLTAGE_ADDR);
    end_array_num = (start_array_num + register_num);
    printf("data byte length (int) : %d\n", buf[8]);
    for(i = start_array_num; i < end_array_num; i++)
    {
      
        tmp[i]  = (((unsigned long)buf[j + 2] & 0xFF) << 24);
      
        tmp[i] += (((unsigned long)buf[j + 3] & 0xFF) << 16);
     
        tmp[i] += (((unsigned long)buf[j] & 0xFF) << 8);
      
        tmp[i] += (((unsigned long)buf[j + 1] & 0xFF));
        
        data[i] = *((float *)&tmp[i]);
       
        j+=4;
        
    }

    return 1;
}
  	



