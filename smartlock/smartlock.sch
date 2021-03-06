EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 8504 5512
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
L Motor:Motor_Servo M1
U 1 1 6245E282
P 5500 2700
F 0 "M1" H 5832 2765 50  0000 L CNN
F 1 "Motor_Servo" H 5832 2674 50  0000 L CNN
F 2 "Connector_PinHeader_1.00mm:PinHeader_1x03_P1.00mm_Vertical" H 5500 2510 50  0001 C CNN
F 3 "http://forums.parallax.com/uploads/attachments/46831/74481.png" H 5500 2510 50  0001 C CNN
	1    5500 2700
	1    0    0    -1  
$EndComp
NoConn ~ 3750 3000
NoConn ~ 3750 3100
NoConn ~ 3750 3200
NoConn ~ 4950 2700
NoConn ~ 4950 2500
$Comp
L Connector:Screw_Terminal_01x02 J1
U 1 1 6246DA2A
P 2000 2650
F 0 "J1" H 1918 2325 50  0000 C CNN
F 1 "Screw_Terminal_01x02" H 1918 2416 50  0000 C CNN
F 2 "TerminalBlock_Phoenix:TerminalBlock_Phoenix_MKDS-1,5-2_1x02_P5.00mm_Horizontal" H 2000 2650 50  0001 C CNN
F 3 "~" H 2000 2650 50  0001 C CNN
	1    2000 2650
	-1   0    0    1   
$EndComp
Wire Wire Line
	2200 2550 2400 2550
Wire Wire Line
	2200 2650 2400 2650
Wire Wire Line
	3400 2700 3500 2700
$Comp
L Converter_ACDC:RAC04-05SGB PS1
U 1 1 62462064
P 3000 2600
F 0 "PS1" H 3000 2967 50  0000 C CNN
F 1 "RAC04-05SGB" H 3000 2876 50  0000 C CNN
F 2 "Converter_ACDC:Converter_ACDC_RECOM_RAC04-xxSGx_THT" H 3000 2250 50  0001 C CNN
F 3 "https://www.recom-power.com/pdf/Powerline-AC-DC/RAC04-GB.pdf" H 2900 2900 50  0001 C CNN
	1    3000 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2600 2700 2400 2700
Wire Wire Line
	2400 2700 2400 2650
Wire Wire Line
	2400 2550 2400 2500
Wire Wire Line
	2400 2500 2600 2500
NoConn ~ 3750 2900
$Comp
L power:GND #PWR01
U 1 1 6247133C
P 3500 2750
F 0 "#PWR01" H 3500 2500 50  0001 C CNN
F 1 "GND" H 3505 2577 50  0000 C CNN
F 2 "" H 3500 2750 50  0001 C CNN
F 3 "" H 3500 2750 50  0001 C CNN
	1    3500 2750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 624718DE
P 5150 3200
F 0 "#PWR03" H 5150 2950 50  0001 C CNN
F 1 "GND" H 5155 3027 50  0000 C CNN
F 2 "" H 5150 3200 50  0001 C CNN
F 3 "" H 5150 3200 50  0001 C CNN
	1    5150 3200
	1    0    0    -1  
$EndComp
$Comp
L ESP32-CAM:ESP32-CAM U1
U 1 1 624686A7
P 4350 3100
F 0 "U1" H 4350 2825 50  0000 C CNN
F 1 "ESP32-CAM" H 4350 3100 50  0001 L BNN
F 2 "ESP32-CAM lib:ESP32-CAM" H 4350 3100 50  0001 L BNN
F 3 "" H 4350 3100 50  0001 L BNN
	1    4350 3100
	1    0    0    -1  
$EndComp
NoConn ~ 3750 2800
Wire Wire Line
	3500 2700 3500 2750
Wire Wire Line
	3750 2600 3500 2600
Wire Wire Line
	3500 2600 3500 2700
Connection ~ 3500 2700
Wire Wire Line
	3400 2500 3750 2500
NoConn ~ 3750 2700
NoConn ~ 4950 3100
NoConn ~ 4950 3000
Wire Wire Line
	4950 2900 5050 2900
Wire Wire Line
	5050 2900 5050 2700
Wire Wire Line
	5050 2700 5200 2700
Wire Wire Line
	5200 2600 4950 2600
Wire Wire Line
	5200 2800 5150 2800
Wire Wire Line
	4950 3200 5150 3200
Connection ~ 5150 3200
Wire Wire Line
	5150 2800 5150 3200
Wire Wire Line
	4950 2800 5150 2800
Connection ~ 5150 2800
$EndSCHEMATC
