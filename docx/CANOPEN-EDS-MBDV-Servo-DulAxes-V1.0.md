# CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds

CANopen Electronic Data Sheet converted to Markdown. Object rows are sorted by
index/sub-index; `Type` is the decoded CiA 301 data type, `PDO` = PDOMapping.

## FileInfo

| Key | Value |
|---|---|
| CreatedBy | AMA |
| ModifiedBy | AMA |
| Description | EDS for Step-Servo CANopen Drives |
| CreationTime | 01:04PM |
| CreationDate | 04-14-2022 |
| ModificationTime | 01:04PM |
| ModificationDate | 04-14-2022 |
| FileName | CANOPEN_EDS_MBDV_Servo_DulAxes_V1.0.eds |
| FileVersion | 3.0 |
| FileRevision | 0 |
| EDSVersion | 1.0 |

## DeviceInfo

| Key | Value |
|---|---|
| VendorName | Shanghai AMP & Moons' Automation |
| VendorNumber | 0x000002D9 |
| ProductName | MDB CANopen Drive |
| ProductNumber | 0x00000000 |
| RevisionNumber | 0x00000000 |
| OrderCode | 0 |
| BaudRate_10 | 1 |
| BaudRate_20 | 1 |
| BaudRate_50 | 1 |
| BaudRate_125 | 1 |
| BaudRate_250 | 1 |
| BaudRate_500 | 1 |
| BaudRate_800 | 1 |
| BaudRate_1000 | 1 |
| SimpleBootUpMaster | 0 |
| SimpleBootUpSlave | 1 |
| Granularity | 8 |
| DynamicChannelsSupported | 0 |
| CompactPDO | 0 |
| GroupMessaging | 0 |
| NrOfRXPDO | 4 |
| NrOfTXPDO | 4 |
| LSS_Supported | 0 |

## DummyUsage

| Key | Value |
|---|---|
| Dummy0001 | 0 |
| Dummy0002 | 1 |
| Dummy0003 | 1 |
| Dummy0004 | 1 |
| Dummy0005 | 1 |
| Dummy0006 | 1 |
| Dummy0007 | 1 |

## Comments

| Key | Value |
|---|---|
| Lines | 4 |
| Line1 | EDS File for CANopen device |
| Line2 | CANopen DS301 & DSP402 implementation |
| Line3 | Stack Version: V1.0 |
| Line4 | Created by AMA shanghai |

## Communication profile (0x1000-0x1FFF)

| Index | Sub | Name | Object | Type | Access | Default | PDO | Range |
|---|---|---|---|---|---|---|---|---|
| 0x1000 |  | Device type | VAR | UNSIGNED32 | ro | 0x60192 | 0 |  |
| 0x1001 |  | Error register | VAR | UNSIGNED8 | ro | 0x00 | 1 |  |
| 0x1002 |  | Manufacturer status register | VAR | UNSIGNED32 | ro | 0x00000000 | 1 |  |
| 0x1003 |  | Pre-defined error field (SubNumber=9) | ARRAY |  |  |  |  |  |
| 0x1003 | 0 | number of errors |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1003 | 1 | standard error field |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1003 | 2 | standard error field |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1003 | 3 | standard error field |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1003 | 4 | standard error field |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1003 | 5 | standard error field |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1003 | 6 | standard error field |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1003 | 7 | standard error field |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1003 | 8 | standard error field |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1005 |  | COB-ID SYNC message | VAR | UNSIGNED32 | rw | 0x00000080 | 0 |  |
| 0x1006 |  | Communication cycle period | VAR | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1007 |  | Synchronous window length | VAR | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1008 |  | Manufacturer device name | VAR | VISIBLE_STRING | const | AMA CANopen Motor Driver | 0 |  |
| 0x1009 |  | Manufacturer hardware version | VAR | VISIBLE_STRING | const | A001 | 0 |  |
| 0x100A |  | Manufacturer software version | VAR | VISIBLE_STRING | const | 310B | 0 |  |
| 0x1010 |  | Store parameters (SubNumber=3) | ARRAY |  |  |  |  |  |
| 0x1010 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x1010 | 1 | save config parameters |  | UNSIGNED32 | rw | 0x00000003 | 0 |  |
| 0x1010 | 2 | save Communication Parameters |  | UNSIGNED32 | rw | 0x00000003 | 0 |  |
| 0x1011 |  | Restore default parameters (SubNumber=3) | ARRAY |  |  |  |  |  |
| 0x1011 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x1011 | 1 | restore all default parameters |  | UNSIGNED32 | rw | 0x00000001 | 0 |  |
| 0x1011 | 2 | restore communication default parameters |  | UNSIGNED32 | rw | 0x00000001 | 0 |  |
| 0x1014 |  | COB-ID EMCY | VAR | UNSIGNED32 | ro | $NODEID+0x80 | 0 |  |
| 0x1017 |  | Producer heartbeat time | VAR | UNSIGNED16 | rw | 0x3e8 | 0 |  |
| 0x1018 |  | Identity (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x1018 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x4 | 0 |  |
| 0x1018 | 1 | vendor ID |  | UNSIGNED32 | ro | 0x000002D9 | 0 |  |
| 0x1018 | 2 | product code |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1018 | 3 | revision number |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1018 | 4 | serial number |  | UNSIGNED32 | ro | 0x00000000 | 0 |  |
| 0x1019 |  | Synchronous counter overflow value | VAR | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1029 |  | Error behavior (SubNumber=7) | ARRAY |  |  |  |  |  |
| 0x1029 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x6 | 0 |  |
| 0x1029 | 1 | communication error |  | UNSIGNED16 | rw | 0x00 | 0 |  |
| 0x1029 | 2 | communication other |  | UNSIGNED16 | rw | 0x00 | 0 |  |
| 0x1029 | 3 | communication passive |  | UNSIGNED16 | rw | 0x01 | 0 |  |
| 0x1029 | 4 | generic |  | UNSIGNED16 | rw | 0x00 | 0 |  |
| 0x1029 | 5 | device profile |  | UNSIGNED16 | rw | 0x00 | 0 |  |
| 0x1029 | 6 | manufacturer specific |  | UNSIGNED16 | rw | 0x00 | 0 |  |
| 0x1200 |  | SDO server parameter (SubNumber=3) | RECORD |  |  |  |  |  |
| 0x1200 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x1200 | 1 | COB-ID client to server |  | UNSIGNED32 | ro | $NODEID+0x600 | 0 |  |
| 0x1200 | 2 | COB-ID server to client |  | UNSIGNED32 | ro | $NODEID+0x580 | 0 |  |
| 0x1400 |  | RPDO communication parameter (SubNumber=3) | RECORD |  |  |  |  |  |
| 0x1400 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x1400 | 1 | COB-ID used by RPDO |  | UNSIGNED32 | rw | $NODEID+0x200 | 0 |  |
| 0x1400 | 2 | transmission type |  | UNSIGNED8 | rw | 0xff | 0 |  |
| 0x1401 |  | RPDO communication parameter (SubNumber=3) | RECORD |  |  |  |  |  |
| 0x1401 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x1401 | 1 | COB-ID used by RPDO |  | UNSIGNED32 | rw | $NODEID+0x300 | 0 |  |
| 0x1401 | 2 | transmission type |  | UNSIGNED8 | rw | 0xfe | 0 |  |
| 0x1402 |  | RPDO communication parameter (SubNumber=3) | RECORD |  |  |  |  |  |
| 0x1402 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x1402 | 1 | COB-ID used by RPDO |  | UNSIGNED32 | rw | $NODEID+0x400 | 0 |  |
| 0x1402 | 2 | transmission type |  | UNSIGNED8 | rw | 0xfe | 0 |  |
| 0x1403 |  | RPDO communication parameter (SubNumber=3) | RECORD |  |  |  |  |  |
| 0x1403 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x1403 | 1 | COB-ID used by RPDO |  | UNSIGNED32 | rw | $NODEID+0x500 | 0 |  |
| 0x1403 | 2 | transmission type |  | UNSIGNED8 | rw | 0xfe |  |  |
| 0x1600 |  | RPDO mapping parameter (SubNumber=9) | RECORD |  |  |  |  |  |
| 0x1600 | 0 | number of mapped application objects in RPDO |  | UNSIGNED8 | rw | 0x1 | 0 |  |
| 0x1600 | 1 | 1st application object |  | UNSIGNED32 | rw | 0x60400010 | 0 |  |
| 0x1600 | 2 | 2nd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1600 | 3 | 3rd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1600 | 4 | 4th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1600 | 5 | 5th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1600 | 6 | 6th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1600 | 7 | 7th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1600 | 8 | 8th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1601 |  | RPDO mapping parameter (SubNumber=9) | RECORD |  |  |  |  |  |
| 0x1601 | 0 | number of mapped application objects in RPDO |  | UNSIGNED8 | rw | 0x2 | 0 |  |
| 0x1601 | 1 | 1st application object |  | UNSIGNED32 | rw | 0x60400010 | 0 |  |
| 0x1601 | 2 | 2nd application object |  | UNSIGNED32 | rw | 0x607A0020 | 0 |  |
| 0x1601 | 3 | 3rd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1601 | 4 | 4th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1601 | 5 | 5th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1601 | 6 | 6th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1601 | 7 | 7th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1601 | 8 | 8th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1602 |  | RPDO mapping parameter (SubNumber=9) | RECORD |  |  |  |  |  |
| 0x1602 | 0 | number of mapped application objects in RPDO |  | UNSIGNED8 | rw | 0x2 | 0 |  |
| 0x1602 | 1 | 1st application object |  | UNSIGNED32 | rw | 0x60400010 | 0 |  |
| 0x1602 | 2 | 2nd application object |  | UNSIGNED32 | rw | 0x60FF0020 | 0 |  |
| 0x1602 | 3 | 3rd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1602 | 4 | 4th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1602 | 5 | 5th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1602 | 6 | 6th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1602 | 7 | 7th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1602 | 8 | 8th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1603 |  | RPDO mapping parameter (SubNumber=9) | RECORD |  |  |  |  |  |
| 0x1603 | 0 | number of mapped application objects in RPDO |  | UNSIGNED8 | rw | 0x1 | 0 |  |
| 0x1603 | 1 | 1st application object |  | UNSIGNED32 | rw | 0x60FE0120 | 0 |  |
| 0x1603 | 2 | 2nd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1603 | 3 | 3rd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1603 | 4 | 4th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1603 | 5 | 5th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1603 | 6 | 6th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1603 | 7 | 7th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1603 | 8 | 8th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1800 |  | TPDO communication parameter (SubNumber=7) | RECORD |  |  |  |  |  |
| 0x1800 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x6 | 0 |  |
| 0x1800 | 1 | COB-ID used by TPDO |  | UNSIGNED32 | rw | $NODEID+0x180 | 0 |  |
| 0x1800 | 2 | transmission type |  | UNSIGNED8 | rw | 0xff | 0 |  |
| 0x1800 | 3 | inhibit time |  | UNSIGNED16 | rw | 0x64 | 0 |  |
| 0x1800 | 4 | compatibility entry |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1800 | 5 | event timer |  | UNSIGNED16 | rw | 0x0000 | 0 |  |
| 0x1800 | 6 | SYNC start value |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1801 |  | TPDO communication parameter (SubNumber=7) | RECORD |  |  |  |  |  |
| 0x1801 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x6 | 0 |  |
| 0x1801 | 1 | COB-ID used by TPDO |  | UNSIGNED32 | rw | $NODEID+0x280 | 0 |  |
| 0x1801 | 2 | transmission type |  | UNSIGNED8 | rw | 0xff | 0 |  |
| 0x1801 | 3 | inhibit time |  | UNSIGNED16 | rw | 0x0000 | 0 |  |
| 0x1801 | 4 | compatibility entry |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1801 | 5 | event timer |  | UNSIGNED16 | rw | 0x0000 | 0 |  |
| 0x1801 | 6 | SYNC start value |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1802 |  | TPDO communication parameter (SubNumber=7) | RECORD |  |  |  |  |  |
| 0x1802 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x6 | 0 |  |
| 0x1802 | 1 | COB-ID used by TPDO |  | UNSIGNED32 | rw | $NODEID+0x380 | 0 |  |
| 0x1802 | 2 | transmission type |  | UNSIGNED8 | rw | 0xff | 0 |  |
| 0x1802 | 3 | inhibit time |  | UNSIGNED16 | rw | 0x0000 | 0 |  |
| 0x1802 | 4 | compatibility entry |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1802 | 5 | event timer |  | UNSIGNED16 | rw | 0x0000 | 0 |  |
| 0x1802 | 6 | SYNC start value |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1803 |  | TPDO communication parameter (SubNumber=7) | RECORD |  |  |  |  |  |
| 0x1803 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x6 | 0 |  |
| 0x1803 | 1 | COB-ID used by TPDO |  | UNSIGNED32 | rw | $NODEID+0x480 | 0 |  |
| 0x1803 | 2 | transmission type |  | UNSIGNED8 | rw | 0xff | 0 |  |
| 0x1803 | 3 | inhibit time |  | UNSIGNED16 | rw | 0x0000 | 0 |  |
| 0x1803 | 4 | compatibility entry |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1803 | 5 | event timer |  | UNSIGNED16 | rw | 0x0000 | 0 |  |
| 0x1803 | 6 | SYNC start value |  | UNSIGNED8 | rw | 0x00 | 0 |  |
| 0x1A00 |  | TPDO mapping parameter (SubNumber=9) | RECORD |  |  |  |  |  |
| 0x1A00 | 0 | number of mapped application objects in TPDO |  | UNSIGNED8 | rw | 0x1 | 0 |  |
| 0x1A00 | 1 | 1st application object |  | UNSIGNED32 | rw | 0x60410010 | 0 |  |
| 0x1A00 | 2 | 2nd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A00 | 3 | 3rd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A00 | 4 | 4th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A00 | 5 | 5th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A00 | 6 | 6th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A00 | 7 | 7th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A00 | 8 | 8th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A01 |  | TPDO mapping parameter (SubNumber=9) | RECORD |  |  |  |  |  |
| 0x1A01 | 0 | number of mapped application objects in TPDO |  | UNSIGNED8 | rw | 0x1 | 0 |  |
| 0x1A01 | 1 | 1st application object |  | UNSIGNED32 | rw | 0x60640020 | 0 |  |
| 0x1A01 | 2 | 2nd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A01 | 3 | 3rd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A01 | 4 | 4th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A01 | 5 | 5th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A01 | 6 | 6th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A01 | 7 | 7th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A01 | 8 | 8th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A02 |  | TPDO mapping parameter (SubNumber=9) | RECORD |  |  |  |  |  |
| 0x1A02 | 0 | number of mapped application objects in TPDO |  | UNSIGNED8 | rw | 0x1 | 0 |  |
| 0x1A02 | 1 | 1st application object |  | UNSIGNED32 | rw | 0x606C0020 | 0 |  |
| 0x1A02 | 2 | 2nd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A02 | 3 | 3rd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A02 | 4 | 4th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A02 | 5 | 5th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A02 | 6 | 6th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A02 | 7 | 7th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A02 | 8 | 8th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A03 |  | TPDO mapping parameter (SubNumber=9) | RECORD |  |  |  |  |  |
| 0x1A03 | 0 | number of mapped application objects in TPDO |  | UNSIGNED8 | rw | 0x2 | 0 |  |
| 0x1A03 | 1 | 1st application object |  | UNSIGNED32 | rw | 0x60640020 | 0 |  |
| 0x1A03 | 2 | 2nd application object |  | UNSIGNED32 | rw | 0x606C0020 | 0 |  |
| 0x1A03 | 3 | 3rd application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A03 | 4 | 4th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A03 | 5 | 5th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A03 | 6 | 6th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A03 | 7 | 7th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |
| 0x1A03 | 8 | 8th application object |  | UNSIGNED32 | rw | 0x00000000 | 0 |  |

## Manufacturer specific (0x2000-0x5FFF)

| Index | Sub | Name | Object | Type | Access | Default | PDO | Range |
|---|---|---|---|---|---|---|---|---|
| 0x2001 |  | Home switch | VAR | UNSIGNED8 | ro | 0x3 | 0 |  |
| 0x2002 |  | Output Status | VAR | UNSIGNED32 | ro |  | 0 |  |
| 0x2006 |  | DSP clear alarm | VAR | UNSIGNED8 | wo | 0x00 | 1 |  |
| 0x200B |  | DSP status code | VAR | UNSIGNED32 | ro | 0x0000 | 1 |  |
| 0x200C |  | Zero position | VAR | UNSIGNED8 | wo | 0x00 | 0 |  |
| 0x200F |  | DSP alarm code | VAR | UNSIGNED32 | ro | 0x0000 | 1 |  |
| 0x2019 |  | Device temperature (SubNumber=5) | ARRAY |  |  |  |  |  |
| 0x2019 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2019 | 1 | drive temperature |  | UNSIGNED16 | ro |  | 1 |  |
| 0x2019 | 2 | DSP temperature |  | UNSIGNED16 | ro |  | 1 |  |
| 0x2019 | 3 | reserved 1 |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2019 | 4 | reserved 2 |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2020 |  | Node ID | VAR | UNSIGNED16 | ro | 0x00 | 0 |  |
| 0x2021 |  | Bit rate | VAR | UNSIGNED16 | ro | 0x00 | 0 |  |
| 0x2030 |  | DC bus voltage | VAR | UNSIGNED16 | ro |  | 1 |  |
| 0x2031 |  | DSP version | VAR | VISIBLE_STRING | ro |  | 0 |  |
| 0x2038 |  | IO Emergency Options | VAR | UNSIGNED16 | rw | 0x5 | 0 |  |
| 0x2040 |  | PDO1 transmit mask (SubNumber=3) | ARRAY |  |  |  |  |  |
| 0x2040 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x2040 | 1 | PDO1 mask lower bytes |  | UNSIGNED32 | rw | 0xFFFFFFFF | 0 |  |
| 0x2040 | 2 | PDO1 mask upper bytes |  | UNSIGNED32 | rw | 0xFFFFFFFF | 0 |  |
| 0x2041 |  | PDO2 transmit mask (SubNumber=3) | ARRAY |  |  |  |  |  |
| 0x2041 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x2041 | 1 | PDO2 mask lower bytes |  | UNSIGNED32 | rw | 0xFFFFFFFF | 0 |  |
| 0x2041 | 2 | PDO2 mask upper bytes |  | UNSIGNED32 | rw | 0xFFFFFFFF | 0 |  |
| 0x2042 |  | PDO3 transmit mask (SubNumber=3) | ARRAY |  |  |  |  |  |
| 0x2042 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x2042 | 1 | PDO3 mask lower bytes |  | UNSIGNED32 | rw | 0xFFFFFFFF | 0 |  |
| 0x2042 | 2 | PDO3 mask upper bytes |  | UNSIGNED32 | rw | 0xFFFFFFFF | 0 |  |
| 0x2043 |  | PDO4 transmit mask (SubNumber=3) | ARRAY |  |  |  |  |  |
| 0x2043 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x2043 | 1 | PDO4 mask lower bytes |  | UNSIGNED32 | rw | 0xFFFFFFFF | 0 |  |
| 0x2043 | 2 | PDO4 mask upper bytes |  | UNSIGNED32 | rw | 0xFFFFFFFF | 0 |  |
| 0x2050 |  | Product series | VAR | VISIBLE_STRING | const | MBDV | 0 |  |
| 0x2051 |  | Customer name | VAR | OCTET_STRING | ro | ---- | 0 |  |
| 0x2060 |  | Comm. watchdog (SubNumber=6) | RECORD |  |  |  |  |  |
| 0x2060 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x5 | 0 |  |
| 0x2060 | 1 | enable |  | UNSIGNED16 | rww | 0x00 | 1 |  |
| 0x2060 | 2 | status |  | UNSIGNED16 | ro | 0x00 | 1 |  |
| 0x2060 | 3 | timeout |  | UNSIGNED16 | rw | 0x1f4 | 0 |  |
| 0x2060 | 4 | trigger event |  | UNSIGNED16 | rw | 0xF | 0 |  |
| 0x2060 | 5 | timeout option code |  | UNSIGNED16 | rw | 0x00 | 0 |  |
| 0x2070 |  | Switch value | VAR | UNSIGNED32 | ro | 0x0000 | 0 |  |
| 0x2A01 |  | Current limit | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A02 |  | Current limit method | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A03 |  | Max Current | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A04 |  | Max Current 1 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A05 |  | IO torque limit | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A06 |  | IO torque limit | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A08 |  | Hard Stop Current Limit | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A09 |  | Halt Time | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A0A |  | Reserved Motor Dir | VAR | UNSIGNED8 | rw |  | 0 |  |
| 0x2A0B |  | Encoder Resolution | VAR | UNSIGNED32 | ro |  | 0 |  |
| 0x2A10 |  | Output Configure (SubNumber=7) | ARRAY |  |  |  |  |  |
| 0x2A10 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 6 | 0 |  |
| 0x2A10 | 1 | Output Configure1 |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2A10 | 2 | Output Configure2 |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2A10 | 3 | Output Configure3 |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2A10 | 4 | Output Configure4 |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2A10 | 5 | Output Configure5 |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2A10 | 6 | Output Configure6 |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2A13 |  | Brake Output Parameters (SubNumber=3) | RECORD |  |  |  |  |  |
| 0x2A13 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 2 | 0 |  |
| 0x2A13 | 1 | Brake Disengage |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A13 | 2 | Brake Engage |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A14 |  | Absolute Position Reach | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A15 |  | In Position Parameters (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x2A15 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2A15 | 1 | Position Limit |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A15 | 2 | In Position Timing |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A15 | 3 | In Position Counts |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A15 | 4 | CSP In Position Timing |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A16 |  | Speed Clamp Parameters (SubNumber=4) | RECORD |  |  |  |  |  |
| 0x2A16 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 3 | 0 |  |
| 0x2A16 | 1 | Zero Speed Threshold |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A16 | 2 | Target Speed Reach Threshold |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A16 | 3 | Speed Ripple Limit |  | INTEGER32 | rw |  | 0 |  |
| 0x2A17 |  | Torque Ripple Limit | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A18 |  | Current Threshold | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A20 |  | Input Config Parameters (SubNumber=11) | ARRAY |  |  |  |  |  |
| 0x2A20 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 10 | 0 |  |
| 0x2A20 | 1 | Input Config 1 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 2 | Input Config 2 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 3 | Input Config 3 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 4 | Input Config 4 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 5 | Input Config 5 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 6 | Input Config 6 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 7 | Input Config 7 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 8 | Input Config 8 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 9 | Input Config 9 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A20 | 10 | Input Config 10 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 |  | Input Filter (SubNumber=11) | ARRAY |  |  |  |  |  |
| 0x2A21 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 10 | 0 |  |
| 0x2A21 | 1 | Input Filter 1 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 2 | Input Filter 2 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 3 | Input Filter 3 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 4 | Input Filter 4 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 5 | Input Filter 5 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 6 | Input Filter 6 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 7 | Input Filter 7 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 8 | Input Filter 8 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 9 | Input Filter 9 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A21 | 10 | Input Filter 10 |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A30 |  | Control Mode | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A31 |  | Control Mode 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A32 |  | Operation Mode | VAR | UNSIGNED32 | ro |  | 0 |  |
| 0x2A33 |  | Jog Mode | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A34 |  | Regen Configure (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x2A34 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2A34 | 1 | Cleamp Resistor |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A34 | 2 | Clamp Power |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A34 | 3 | Clamp Time |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A34 | 4 | Clamp Voltage |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A35 |  | Parameter Lock | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A36 |  | Default Display | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A37 |  | Mask of Alarm | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A42 |  | Jog Speed | VAR | INTEGER32 | rw |  | 0 |  |
| 0x2A43 |  | Jog Acceleration | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A44 |  | Jog Deceleration | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A45 |  | Change Velocity | VAR | INTEGER32 | rw |  | 0 |  |
| 0x2A46 |  | Jog Change Velocity (SubNumber=9) | ARRAY |  |  |  |  |  |
| 0x2A46 | 0 | NrOfObjects |  | UNSIGNED8 | ro | 8 | 0 |  |
| 0x2A46 | 1 | Jog Change Velocity1 |  | INTEGER32 | rw |  | 0 |  |
| 0x2A46 | 2 | Jog Change Velocity2 |  | INTEGER32 | rw |  | 0 |  |
| 0x2A46 | 3 | Jog Change Velocity3 |  | INTEGER32 | rw |  | 0 |  |
| 0x2A46 | 4 | Jog Change Velocity4 |  | INTEGER32 | rw |  | 0 |  |
| 0x2A46 | 5 | Jog Change Velocity5 |  | INTEGER32 | rw |  | 0 |  |
| 0x2A46 | 6 | Jog Change Velocity6 |  | INTEGER32 | rw |  | 0 |  |
| 0x2A46 | 7 | Jog Change Velocity7 |  | INTEGER32 | rw |  | 0 |  |
| 0x2A46 | 8 | Jog Change Velocity8 |  | INTEGER32 | rw |  | 0 |  |
| 0x2A47 |  | Max Torque Velocity | VAR | UNSIGNED32 | rww |  | 1 |  |
| 0x2A4C |  | S Curve Filter | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A4D |  | Jerk Filter | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A4E |  | FIR Filter | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A50 |  | Auto Tuning Mode | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A51 |  | Load Type | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A52 |  | Inertia Ratio | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A53 |  | Global Gain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A54 |  | Global Gain 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A55 |  | Pos P Gain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A56 |  | Pos I Gain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A57 |  | Pos DeriGain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A58 |  | Pos DeriFilter | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A59 |  | Velocity FF Gain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A5A |  | Velocity FF Filter | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A5B |  | Velocity Ref Gain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A5C |  | Velocity P Gain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A5D |  | Velocity I Gain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A5E |  | Torque FF Gain | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A5F |  | Torque FF Filter | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A60 |  | PID Filter | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A61 |  | Pos P Gain 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A62 |  | Pos I Gain 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A63 |  | Pos DeriGain 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A64 |  | Pos DeriFilger 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A65 |  | Velocity RefGain 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A66 |  | Velocity P Gain 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A67 |  | Velocity I Gain 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A68 |  | PID Filter 2 | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A69 |  | PID Switch (SubNumber=7) | RECORD |  |  |  |  |  |
| 0x2A69 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 6 | 0 |  |
| 0x2A69 | 1 | Switch Mode |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A69 | 2 | Switch Pos Condition |  | INTEGER32 | rw |  | 0 |  |
| 0x2A69 | 3 | Switch Vel Condition |  | INTEGER32 | rw |  | 0 |  |
| 0x2A69 | 4 | Switch Torque Condition |  | INTEGER32 | rw |  | 0 |  |
| 0x2A69 | 5 | Switch Condition Delay |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A69 | 6 | Switch Time |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A80 |  | Pos Phase Compensation (SubNumber=6) | RECORD |  |  |  |  |  |
| 0x2A80 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 5 | 0 |  |
| 0x2A80 | 1 | Phase NotchFreq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A80 | 2 | Phase NotchBW |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A80 | 3 | Phase ResFreq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A80 | 4 | Phase ResBW |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A80 | 5 | Phase Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A81 |  | Pos Notch Filter (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x2A81 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2A81 | 1 | Notch Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A81 | 2 | Notch Freq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A81 | 3 | Notch DeadBand |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A81 | 4 | Notch Ampt |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A82 |  | Vel Phase Compensation (SubNumber=6) | RECORD |  |  |  |  |  |
| 0x2A82 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 5 | 0 |  |
| 0x2A82 | 1 | Phase NotchFreq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A82 | 2 | Phase NotchBW |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A82 | 3 | Phase ResFreq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A82 | 4 | Phase ResBW |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A82 | 5 | Phase Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A83 |  | Vel Notch Filter (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x2A83 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2A83 | 1 | Notch Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A83 | 2 | Notch Freq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A83 | 3 | Notch DeadBand |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A83 | 4 | Notch Ampt |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A84 |  | Torque Phase Compensation (SubNumber=6) | RECORD |  |  |  |  |  |
| 0x2A84 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 5 | 0 |  |
| 0x2A84 | 1 | Phase NotchFreq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A84 | 2 | Phase NotchBW |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A84 | 3 | Phase ResFreq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A84 | 4 | Phase ResBW |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A84 | 5 | Phase Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A85 |  | Torque Notch Filter 0 (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x2A85 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2A85 | 1 | Notch Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A85 | 2 | Notch Freq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A85 | 3 | Notch DeadBand |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A85 | 4 | Phase Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A86 |  | Torque Notch Filter 1 (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x2A86 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2A86 | 1 | Notch Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A86 | 2 | Notch Freq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A86 | 3 | Notch DeadBand |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A86 | 4 | Phase Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A87 |  | Torque Notch Filter 2 (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x2A87 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2A87 | 1 | Notch Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A87 | 2 | Notch Freq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A87 | 3 | Notch DeadBand |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A87 | 4 | Phase Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A88 |  | Torque Notch Filter 3 (SubNumber=5) | RECORD |  |  |  |  |  |
| 0x2A88 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 4 | 0 |  |
| 0x2A88 | 1 | Notch Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A88 | 2 | Notch Freq |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A88 | 3 | Notch DeadBand |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A88 | 4 | Phase Switch |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A90 |  | Steps Per Rev | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A91 |  | Input Noise Filter | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2A94 |  | Encoder Simulator Output Configure (SubNumber=6) | RECORD |  |  |  |  |  |
| 0x2A94 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 5 | 0 |  |
| 0x2A94 | 1 | Output Mode |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A94 | 2 | Output Pulse NUM |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A94 | 3 | Output Pulse DEN |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2A94 | 4 | AB QuadPhase Lead |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A94 | 5 | Z Phase Polarity |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A9A |  | Motor Abs encoder (SubNumber=8) | RECORD |  |  |  |  |  |
| 0x2A9A | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 7 | 0 |  |
| 0x2A9A | 1 | IsConnected |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2A9A | 2 | acMotorName |  | INTEGER8 | ro |  |  |  |
| 0x2A9A | 3 | Motor SN |  | INTEGER8 | ro |  |  |  |
| 0x2A9A | 4 | Rated Current |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9A | 5 | Rated Torque |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9A | 6 | Rated Speed |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9A | 7 | Reserved |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9B |  | Motor Inc encoder (SubNumber=8) | RECORD |  |  |  |  |  |
| 0x2A9B | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 7 | 0 |  |
| 0x2A9B | 1 | IsConnected |  | UNSIGNED8 | ro |  | 0 |  |
| 0x2A9B | 2 | Motor Model |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9B | 3 | Motor SN |  | INTEGER8 | ro |  |  |  |
| 0x2A9B | 4 | Rated Current |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9B | 5 | Rated Torque |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9B | 6 | Motor Inertia |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9B | 7 | Rated Speed |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9C |  | Abs encoder (SubNumber=6) | RECORD |  |  |  |  |  |
| 0x2A9C | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 5 | 0 |  |
| 0x2A9C | 1 | Usage Type |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2A9C | 2 | Encoder ErrCode |  | UNSIGNED32 | ro |  | 0 |  |
| 0x2A9C | 3 | Encoder Temp |  | INTEGER32 | ro |  | 0 |  |
| 0x2A9C | 4 | Encoder Mode |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2A9C | 5 | Clear MultiTurn |  | UNSIGNED16 | ro |  | 0 |  |
| 0x2AA0 |  | Actual Current | VAR | INTEGER32 | ro |  | 0 |  |
| 0x2AA1 |  | Torque Constant | VAR | UNSIGNED32 | ro |  | 0 |  |
| 0x2AA2 |  | Steps Position | VAR | UNSIGNED32 | ro |  | 1 |  |
| 0x2AA3 |  | Snd Encoder Position | VAR | INTEGER32 | ro |  | 1 |  |
| 0x2AB0 |  | Virtual Inputs (SubNumber=3) | RECORD |  |  |  |  |  |
| 0x2AB0 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 2 | 0 |  |
| 0x2AB0 | 1 | Virtual Inputs |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2AB0 | 2 | Bit Mask |  | UNSIGNED16 | rw |  | 0 |  |
| 0x2AB1 |  | Dyanmic Brake Config (SubNumber=6) | RECORD |  |  |  |  |  |
| 0x2AB1 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 5 | 0 |  |
| 0x2AB1 | 1 | Brake Mode Disable |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2AB1 | 2 | Max Decel Time Disable |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2AB1 | 3 | Brake Mode Fault |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2AB1 | 4 | Max Decel Time Fault |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2AB1 | 5 | DB_ActiveVel |  | UNSIGNED32 | rw |  | 0 |  |
| 0x2AC0 |  | Sub Alarm Code | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x2AC1 |  | Current Ramp Max | VAR | INTEGER32 | rw |  | 0 |  |
| 0x2AC2 |  | Phase Lost Detect | VAR | INTEGER32 | rw |  | 0 |  |
| 0x2AC3 |  | Velocity Feedback Filter | VAR | INTEGER32 | rw |  | 0 |  |
| 0x2AC4 |  | Self Adapting Switch | VAR | INTEGER32 | rw |  | 0 |  |

## Device profile CiA 402 (0x6000-0x9FFF)

| Index | Sub | Name | Object | Type | Access | Default | PDO | Range |
|---|---|---|---|---|---|---|---|---|
| 0x603F |  | Error code | VAR | UNSIGNED16 | ro | 0x0000 | 1 |  |
| 0x6040 |  | Control word | VAR | UNSIGNED16 | rww | 0x0000 | 1 |  |
| 0x6041 |  | Status word | VAR | UNSIGNED16 | ro | 0x0000 | 1 |  |
| 0x605A |  | Quick stop option code | VAR | INTEGER16 | rw | 0x0000 | 0 |  |
| 0x605B |  | Shutdown option code | VAR | INTEGER16 | rw |  | 0 |  |
| 0x605C |  | Disable operation option code | VAR | INTEGER16 | rw |  | 0 |  |
| 0x605D |  | Halt option code | VAR | INTEGER16 | rw |  | 0 |  |
| 0x605E |  | Fault option code | VAR | INTEGER16 | rw |  | 0 |  |
| 0x6060 |  | Modes of operation | VAR | INTEGER8 | rww | 0 | 1 |  |
| 0x6061 |  | Modes of operation display | VAR | INTEGER8 | ro | 0 | 1 |  |
| 0x6064 |  | Position value calculated | VAR | INTEGER32 | ro | 0 | 1 |  |
| 0x606C |  | Velocity value calculated | VAR | INTEGER32 | ro | 0 | 1 |  |
| 0x6071 |  | Target torque | VAR | INTEGER16 | rww | 0 | 1 |  |
| 0x6073 |  | Max current | VAR | UNSIGNED16 | rww | 0x0000 | 1 |  |
| 0x6074 |  | Torque demand | VAR | INTEGER16 | rww | 0 | 1 |  |
| 0x6075 |  | Motor rated current | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x6076 |  | Motor rated torque | VAR | UNSIGNED32 | rw |  | 0 |  |
| 0x6077 |  | Torque actual value | VAR | INTEGER16 | ro |  | 0 |  |
| 0x6078 |  | Current actual value | VAR | INTEGER16 | ro | 0 | 1 |  |
| 0x607A |  | Target position | VAR | INTEGER32 | rww | 0 | 1 |  |
| 0x607C |  | Home offset | VAR | INTEGER32 | rww | 0 | 1 |  |
| 0x607D |  | Software position limit (SubNumber=3) | ARRAY |  |  |  |  |  |
| 0x607D | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 2 | 0 |  |
| 0x607D | 1 | min position range limit |  | INTEGER32 | rw |  | 0 |  |
| 0x607D | 2 | max position range limit |  | INTEGER32 | rw |  | 0 |  |
| 0x607E |  | Polarity | VAR | UNSIGNED8 | rww | 0x00 | 1 |  |
| 0x607F |  | Max profile speed | VAR | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x6081 |  | Profile velocity | VAR | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x6083 |  | Profile acceleration | VAR | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x6084 |  | Profile deceleration | VAR | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x6085 |  | Quick stop deceleration | VAR | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x6087 |  | Torque slope | VAR | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x6098 |  | Homing method | VAR | INTEGER8 | rww | 0 | 1 |  |
| 0x6099 |  | Homing speed (SubNumber=3) | ARRAY |  |  |  |  |  |
| 0x6099 | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 0x2 | 0 |  |
| 0x6099 | 1 | speed during search for switch |  | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x6099 | 2 | speed during search for zero |  | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x609A |  | Homing acceleration | VAR | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x60E0 |  | Positive torque limit value | VAR | UNSIGNED16 | rww |  | 1 |  |
| 0x60E1 |  | Negative torque limit value | VAR | UNSIGNED16 | rww |  | 1 |  |
| 0x60F4 |  | Following error actual value | VAR | INTEGER32 | ro | 0 | 1 |  |
| 0x60FD |  | Ditigal inputs | VAR | UNSIGNED32 | ro |  | 1 |  |
| 0x60FE |  | Digital outputs (SubNumber=3) | RECORD |  |  |  |  |  |
| 0x60FE | 0 | highest sub-index supported |  | UNSIGNED8 | ro | 2 | 0 |  |
| 0x60FE | 1 | physical outputs |  | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x60FE | 2 | bit mask |  | UNSIGNED32 | rww | 0x00000000 | 1 |  |
| 0x60FF |  | Target velocity | VAR | INTEGER32 | rww | 0 | 1 |  |
| 0x6502 |  | Supported drive modes | VAR | UNSIGNED32 | ro | 0x1012D | 0 |  |
