EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector:Conn_01x02_Male PSU1
U 1 1 617ED8CD
P 2500 4850
F 0 "PSU1" H 2608 4939 50  0000 C CNN
F 1 "Conn_01x02_Male" H 2608 4940 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 2500 4850 50  0001 C CNN
F 3 "~" H 2500 4850 50  0001 C CNN
	1    2500 4850
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Male NTC1
U 1 1 617EE180
P 2500 4500
F 0 "NTC1" H 2608 4589 50  0000 C CNN
F 1 "Conn_01x02_Male" H 2608 4590 50  0001 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 2500 4500 50  0001 C CNN
F 3 "~" H 2500 4500 50  0001 C CNN
	1    2500 4500
	1    0    0    -1  
$EndComp
$Comp
L Device:R 100k1
U 1 1 617EEEED
P 2850 4150
F 0 "100k1" H 2780 4150 50  0000 R CNN
F 1 "R" V 2734 4150 50  0001 C CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 2780 4150 50  0001 C CNN
F 3 "~" H 2850 4150 50  0001 C CNN
	1    2850 4150
	-1   0    0    1   
$EndComp
$Comp
L ESP8266:NodeMCU_1.0_(ESP-12E) U2
U 1 1 617F059E
P 4200 4500
F 0 "U2" H 4200 5481 60  0001 C CNN
F 1 "NodeMCU_3.0_(ESP-12E)" H 4200 5481 60  0000 C CNN
F 2 "Module:nodeMCUv3" H 3600 3650 60  0001 C CNN
F 3 "" H 3600 3650 60  0001 C CNN
	1    4200 4500
	1    0    0    -1  
$EndComp
$Comp
L Sensor:BME2803V yautay1
U 1 1 617F2B02
P 5850 4600
F 0 "yautay1" V 6565 4517 50  0001 C CNN
F 1 "BME2803V" H 5900 4700 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical" H 5850 4600 50  0001 C CNN
F 3 "" H 5850 4600 50  0001 C CNN
	1    5850 4600
	-1   0    0    1   
$EndComp
$Comp
L Converter_DCDC:D24V10F5 U1
U 1 1 617F3A42
P 3000 6100
F 0 "U1" H 3300 5650 50  0001 C CNN
F 1 "D24V10F5" H 2572 5492 50  0000 R CNN
F 2 "Converter_DCDC:yautay-5V-stepdown" H 3000 6100 50  0001 C CNN
F 3 "" H 3000 6100 50  0001 C CNN
	1    3000 6100
	-1   0    0    1   
$EndComp
$Comp
L power:+24V #PWR0101
U 1 1 617F4997
P 2650 5100
F 0 "#PWR0101" H 2650 4950 50  0001 C CNN
F 1 "+24V" V 2665 5228 50  0000 L CNN
F 2 "" H 2650 5100 50  0001 C CNN
F 3 "" H 2650 5100 50  0001 C CNN
	1    2650 5100
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 617F4F9C
P 2850 3850
F 0 "#PWR0102" H 2850 3600 50  0001 C CNN
F 1 "GND" H 2855 3677 50  0000 C CNN
F 2 "" H 2850 3850 50  0001 C CNN
F 3 "" H 2850 3850 50  0001 C CNN
	1    2850 3850
	-1   0    0    1   
$EndComp
Wire Wire Line
	2700 4500 2850 4500
Wire Wire Line
	2850 4500 2850 4300
Wire Wire Line
	2850 4500 3250 4500
Wire Wire Line
	3250 4500 3250 3800
Wire Wire Line
	3250 3800 3400 3800
Connection ~ 2850 4500
Wire Wire Line
	2700 4950 3000 4950
Wire Wire Line
	3000 4950 3000 5300
Wire Wire Line
	3300 5300 3300 5200
Wire Wire Line
	3300 5200 3400 5200
Wire Wire Line
	5000 5200 6400 5200
Wire Wire Line
	6400 5200 6400 4300
Wire Wire Line
	6400 4300 6150 4300
Wire Wire Line
	5850 4300 5850 3900
Wire Wire Line
	5850 3900 5000 3900
Wire Wire Line
	5000 4000 5700 4000
Wire Wire Line
	5700 4000 5700 4300
Wire Wire Line
	2700 4600 3250 4600
Wire Wire Line
	3250 4600 3250 4800
Wire Wire Line
	3250 4800 3400 4800
$Comp
L power:GND #PWR0103
U 1 1 617FC671
P 2800 4850
F 0 "#PWR0103" H 2800 4600 50  0001 C CNN
F 1 "GND" V 2805 4722 50  0000 R CNN
F 2 "" H 2800 4850 50  0001 C CNN
F 3 "" H 2800 4850 50  0001 C CNN
	1    2800 4850
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 617FCF1A
P 3150 5100
F 0 "#PWR0104" H 3150 4850 50  0001 C CNN
F 1 "GND" H 3155 4927 50  0000 C CNN
F 2 "" H 3150 5100 50  0001 C CNN
F 3 "" H 3150 5100 50  0001 C CNN
	1    3150 5100
	-1   0    0    1   
$EndComp
Wire Wire Line
	2850 4000 2850 3850
Wire Wire Line
	2700 4850 2800 4850
Wire Wire Line
	3150 5100 3150 5300
Wire Wire Line
	3400 5100 3300 5100
Connection ~ 3150 5100
Wire Wire Line
	3400 4700 3300 4700
Wire Wire Line
	3300 4700 3300 5100
Connection ~ 3300 5100
Wire Wire Line
	3300 5100 3150 5100
Wire Wire Line
	2700 4950 2700 5100
Wire Wire Line
	2700 5100 2650 5100
Connection ~ 2700 4950
$Comp
L power:GND #PWR0105
U 1 1 617FF309
P 6000 4200
F 0 "#PWR0105" H 6000 3950 50  0001 C CNN
F 1 "GND" H 6005 4027 50  0000 C CNN
F 2 "" H 6000 4200 50  0001 C CNN
F 3 "" H 6000 4200 50  0001 C CNN
	1    6000 4200
	-1   0    0    1   
$EndComp
Wire Wire Line
	6000 4200 6000 4300
$Comp
L Relay_SolidState:TLP3123 U3
U 1 1 61800AE6
P 6350 3450
F 0 "U3" H 6350 3775 50  0001 C CNN
F 1 "SSD" H 6350 3683 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical" H 6150 3250 50  0001 L CIN
F 3 "https://toshiba.semicon-storage.com/info/docget.jsp?did=10047&prodName=TLP3123" H 6350 3450 50  0001 L CNN
	1    6350 3450
	1    0    0    -1  
$EndComp
$Comp
L Device:Heater R1
U 1 1 61802F33
P 7500 3100
F 0 "R1" H 7570 3146 50  0000 L CNN
F 1 "Heater" H 7570 3055 50  0000 L CNN
F 2 "" V 7430 3100 50  0001 C CNN
F 3 "~" H 7500 3100 50  0001 C CNN
	1    7500 3100
	1    0    0    -1  
$EndComp
$Comp
L power:+24V #PWR0106
U 1 1 6180388D
P 6850 2750
F 0 "#PWR0106" H 6850 2600 50  0001 C CNN
F 1 "+24V" V 6865 2878 50  0000 L CNN
F 2 "" H 6850 2750 50  0001 C CNN
F 3 "" H 6850 2750 50  0001 C CNN
	1    6850 2750
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 61803D3A
P 5150 4250
F 0 "#PWR0107" H 5150 4000 50  0001 C CNN
F 1 "GND" H 5155 4077 50  0000 C CNN
F 2 "" H 5150 4250 50  0001 C CNN
F 3 "" H 5150 4250 50  0001 C CNN
	1    5150 4250
	-1   0    0    1   
$EndComp
Wire Wire Line
	5150 4250 5150 4400
Wire Wire Line
	5150 4400 5000 4400
Wire Wire Line
	5150 4400 5150 5100
Wire Wire Line
	5150 5100 5000 5100
Connection ~ 5150 4400
Wire Wire Line
	5000 4500 5250 4500
Wire Wire Line
	5250 3350 6050 3350
Wire Wire Line
	5250 3350 5250 4500
Wire Wire Line
	6050 3550 5300 3550
Wire Wire Line
	5300 3550 5300 4300
Wire Wire Line
	5300 4300 5200 4300
Wire Wire Line
	5200 4300 5200 4400
Wire Wire Line
	5200 4400 5150 4400
Wire Wire Line
	6850 2750 7500 2750
Wire Wire Line
	7500 2750 7500 2950
Wire Wire Line
	7500 3250 6650 3250
Wire Wire Line
	6650 3250 6650 3350
$Comp
L power:GND #PWR0108
U 1 1 61808787
P 6800 3550
F 0 "#PWR0108" H 6800 3300 50  0001 C CNN
F 1 "GND" H 6805 3377 50  0000 C CNN
F 2 "" H 6800 3550 50  0001 C CNN
F 3 "" H 6800 3550 50  0001 C CNN
	1    6800 3550
	-1   0    0    1   
$EndComp
Wire Wire Line
	6650 3550 6800 3550
$EndSCHEMATC