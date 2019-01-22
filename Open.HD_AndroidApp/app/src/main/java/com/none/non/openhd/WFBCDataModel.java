package com.none.non.openhd;
/**
 * Created by user2 on 7/5/2018.
 */

public class WFBCDataModel {

    public void DecCounter(int by)
    {
        FREQIsChanged -= by;
        FPSIsChanged  -= by;
        DATARATEIsChanged  -= by;
        VIDEO_BLOCKSIsChanged -= by;

        VIDEO_FECSIsChanged -= by;
        VIDEO_BLOCKLENGTHIsChanged -= by;
        VIDEO_BITRATEIsChanged  -= by;
        BITRATE_PERCENTIsChanged  -= by;
        WIDTHIsChanged  -= by;
        HEIGHTIsChanged  -= by;
        FPSIsChanged  -= by;
        KEYFRAMERATEIsChanged  -= by;
        EXTRAPARAMSIsChanged  -= by;

        //add
        FREQSCANIsChanged  -= by;
        TXMODEIsChanged  -= by;
        TELEMETRY_TRANSMISSIONIsChanged  -= by;
        TELEMETRY_UPLINKIsChanged  -= by;
        RCIsChanged  -= by;
        CTS_PROTECTIONIsChanged  -= by;
        WIFI_HOTSPOTIsChanged  -= by;
        WIFI_HOTSPOT_NICIsChanged  -= by;
        ETHERNET_HOTSPOTIsChanged  -= by;
        ENABLE_SCREENSHOTSIsChanged  -= by;
        FORWARD_STREAMIsChanged  -= by;
        VIDEO_UDP_PORTIsChanged  -= by;

        //add Camera Control
        IsCamera1EnabledIsChanged  -= by;
        IsCamera2EnabledIsChanged  -= by;
        IsCamera3EnabledIsChanged  -= by;
        IsCamera4EnabledIsChanged  -= by;
        DefaultCameraIdIsChanged  -= by;
        ChannelToListenIsChanged  -= by;
        Camera1ValueMinIsChanged  -= by;
        Camera1ValueMaxIsChanged  -= by;
        Camera2ValueMinIsChanged  -= by;
        Camera2ValueMaxIsChanged  -= by;
        Camera3ValueMinIsChanged  -= by;
        Camera3ValueMaxIsChanged  -= by;
        Camera4ValueMinIsChanged  -= by;
        Camera4ValueMaxIsChanged -= by;

        //Requested Menu Selections
        CELLSIsChanged -= by;
        EncryptionOrRangeIsChanged -= by;
                IsBandSwicherEnabledIsChanged -= by;
                BandwidthIsChanged -= by;
                UplinkSpeedIsChanged -= by;
                ChannelToListen2IsChanged -= by;
                PrimaryCardMACIsChanged -= by;
                SlaveCardMACIsChanged -= by;
                Band5BelowIsChanged -= by;
                Band10ValueMinIsChanged -= by;
                Band10ValueMaxIsChanged -= by;
                Band20AfterIsChanged -= by;
    }

    public void AddData(String Variable, String Data, int IsChanged)
    {
        switch(Variable)
        {
            case "FREQ":
                FREQ = Data;
                FREQIsChanged  += IsChanged;
                break;

            case "Imperial":
                Imperial = Data;
                ImperialIsChanged  += IsChanged;
                break;

            case "Copter":
                Copter = Data;
                CopterIsChanged  += IsChanged;
                break;

            case "DATARATE":
                DATARATE = Data;
                if(DATARATE.equals(DATARATEb) == false)
                    DATARATEIsChanged += IsChanged;
                break;

            case "UPDATE_NTH_TIME":
                UPDATE_NTH_TIME = Data;
                if(UPDATE_NTH_TIME.equals(UPDATE_NTH_TIMEb) == false)
                    UPDATE_NTH_TIMEIsChanged += IsChanged;
                break;

            case "RemoteSettingsEnabled":
                RemoteSettingsEnabled = Data;
                if(RemoteSettingsEnabled.equals(RemoteSettingsEnabledb) == false)
                    RemoteSettingsEnabledIsChanged += IsChanged;
                break;

            case "FPS":
                FPS = Data;
                if(FPS.equals(FPSb) == false)
                    FPSIsChanged += IsChanged;
                break;

            case "VIDEO_BLOCKS":
                VIDEO_BLOCKS = Data;
                if(VIDEO_BLOCKS.equals(VIDEO_BLOCKSb) == false)
                    VIDEO_BLOCKSIsChanged +=IsChanged;
                break;

            case "VIDEO_FECS":
                VIDEO_FECS = Data;
                if(VIDEO_FECS.equals(VIDEO_FECSb) == false)
                    VIDEO_FECSIsChanged += IsChanged;
                break;
            case "VIDEO_BLOCKLENGTH":
                VIDEO_BLOCKLENGTH = Data;
                if(VIDEO_BLOCKLENGTH.equals(VIDEO_BLOCKLENGTHb) == false)
                    VIDEO_BLOCKLENGTHIsChanged +=IsChanged;
                break;

            case "VIDEO_BITRATE":
                VIDEO_BITRATE = Data;
                if(VIDEO_BITRATE.equals(VIDEO_BITRATEb) == false)
                    VIDEO_BITRATEIsChanged += IsChanged;
                break;

            case "BITRATE_PERCENT":
                BITRATE_PERCENT = Data;
                if(BITRATE_PERCENT.equals(BITRATE_PERCENTb) == false)
                    BITRATE_PERCENTIsChanged += IsChanged;
                break;

            case "WIDTH":
                WIDTH = Data;
                if(WIDTH.equals(WIDTHb) == false)
                    WIDTHIsChanged += IsChanged;
                break;

            case "HEIGHT":
                HEIGHT = Data;
                if(HEIGHT.equals(HEIGHTb) == false)
                    HEIGHTIsChanged += IsChanged;
                break;

            case "KEYFRAMERATE":
                KEYFRAMERATE = Data;
                if(KEYFRAMERATE.equals(KEYFRAMERATEb) == false)
                    KEYFRAMERATEIsChanged += IsChanged;
                break;

            case "EXTRAPARAMS":
                EXTRAPARAMS = Data;
                if(EXTRAPARAMS.equals(EXTRAPARAMSb) == false)
                    EXTRAPARAMSIsChanged += IsChanged;
                break;

            case "FREQSCAN":
                FREQSCAN = Data;
                if(FREQSCAN.equals(FREQSCANb) == false)
                    FREQSCANIsChanged += IsChanged;
                break;

            case "TXMODE":
                TXMODE = Data;
                if(TXMODE.equals(TXMODEb) == false)
                    TXMODEIsChanged +=IsChanged;
                break;

            case "TELEMETRY_TRANSMISSION":
                TELEMETRY_TRANSMISSION = Data;
                if(TELEMETRY_TRANSMISSION.equals(TELEMETRY_TRANSMISSIONb) == false)
                    TELEMETRY_TRANSMISSIONIsChanged += IsChanged;
                break;

            case "TELEMETRY_UPLINK":
                TELEMETRY_UPLINK = Data;
                if(TELEMETRY_UPLINK.equals(TELEMETRY_UPLINKb) == false)
                    TELEMETRY_UPLINKIsChanged += IsChanged;
                break;

            case "RC":
                RC = Data;
                if(RC.equals(RCb) == false)
                    RCIsChanged += IsChanged;
                break;

            case "CTS_PROTECTION":
                CTS_PROTECTION = Data;
                if(CTS_PROTECTION.equals(CTS_PROTECTIONb) == false)
                    CTS_PROTECTIONIsChanged +=IsChanged;
                break;

            case "FC_RC_SERIALPORT":
                FC_RC_SERIALPORT = Data;
                if(FC_RC_SERIALPORT.equals(FC_RC_SERIALPORTb) == false)
                    FC_RC_SERIALPORTIsChanged += IsChanged;

                break;
            case "FC_RC_BAUDRATE":
                FC_RC_BAUDRATE = Data;
                if(FC_RC_BAUDRATE.equals(FC_RC_BAUDRATEb) == false)
                    FC_RC_BAUDRATEIsChanged += IsChanged;
                break;

            case "FC_TELEMETRY_SERIALPORT":
                FC_TELEMETRY_SERIALPORT = Data;
                if(FC_TELEMETRY_SERIALPORT.equals(FC_TELEMETRY_SERIALPORTb) == false)
                    FC_TELEMETRY_SERIALPORTIsChanged += IsChanged;
                break;

            case "FC_TELEMETRY_BAUDRATE":
                FC_TELEMETRY_BAUDRATE = Data;
                if(FC_TELEMETRY_BAUDRATE.equals(FC_TELEMETRY_BAUDRATEb) == false)
                    FC_TELEMETRY_BAUDRATEIsChanged += IsChanged;
                break;

            case "FC_MSP_SERIALPORT":
                FC_MSP_SERIALPORT = Data;
                if(FC_MSP_SERIALPORT.equals(FC_MSP_SERIALPORTb) == false)
                    FC_MSP_SERIALPORTIsChanged += IsChanged;
                break;

            case "FC_MSP_BAUDRATE":
                FC_MSP_BAUDRATE = Data;
                if(FC_MSP_BAUDRATE.equals(FC_MSP_BAUDRATEb) == false)
                    FC_MSP_BAUDRATEIsChanged += IsChanged;
                break;

            case "AIRODUMP":
                AIRODUMP = Data;
                if(AIRODUMP.equals(AIRODUMPb) == false)
                    AIRODUMPIsChanged += IsChanged;
                break;

            case "AIRODUMP_SECONDS":
                AIRODUMP_SECONDS = Data;
                if(AIRODUMP_SECONDS.equals(AIRODUMP_SECONDSb) == false)
                    AIRODUMP_SECONDSIsChanged += IsChanged;
                break;

            case "WIFI_HOTSPOT":
                WIFI_HOTSPOT = Data;
                if(WIFI_HOTSPOT.equals(WIFI_HOTSPOTb) == false)
                    WIFI_HOTSPOTIsChanged += IsChanged;
                break;

            case "WIFI_HOTSPOT_NIC":
                WIFI_HOTSPOT_NIC = Data;
                if(WIFI_HOTSPOT_NIC.equals(WIFI_HOTSPOT_NICb) == false)
                    WIFI_HOTSPOT_NICIsChanged += IsChanged;
                break;

            case "ETHERNET_HOTSPOT":
                ETHERNET_HOTSPOT = Data;
                if(ETHERNET_HOTSPOT.equals(ETHERNET_HOTSPOTb) == false)
                    ETHERNET_HOTSPOTIsChanged += IsChanged;
                break;

            case "ENABLE_SCREENSHOTS":
                ENABLE_SCREENSHOTS = Data;
                if(ENABLE_SCREENSHOTS.equals(ENABLE_SCREENSHOTSb) == false)
                    ENABLE_SCREENSHOTSIsChanged += IsChanged;
                break;

            case "VIDEO_TMP":
                VIDEO_TMP = Data;
                if(VIDEO_TMP.equals(VIDEO_TMPb) == false)
                    VIDEO_TMPIsChanged +=IsChanged;
                break;

            case "RELAY_NIC":
                RELAY_NIC = Data;
                if(RELAY_NIC.equals(RELAY_NICb) == false)
                    RELAY_NICIsChanged += IsChanged;
                break;

            case "RELAY_FREQ":
                RELAY_FREQ = Data;
                if(RELAY_FREQ.equals(RELAY_FREQb) == false)
                    RELAY_FREQIsChanged += IsChanged;
                break;

            case "QUIET":
                QUIET = Data;
                if(QUIET.equals(QUIETb) == false)
                    QUIETIsChanged +=IsChanged;
                break;

            case "EXTERNAL_TELEMETRY_SERIALPORT_GROUND":
                EXTERNAL_TELEMETRY_SERIALPORT_GROUND = Data;
                if(EXTERNAL_TELEMETRY_SERIALPORT_GROUND.equals(EXTERNAL_TELEMETRY_SERIALPORT_GROUNDb) == false)
                    EXTERNAL_TELEMETRY_SERIALPORT_GROUNDIsChanged += IsChanged;
                break;

            case "EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE":
                EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE = Data;
                if(EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE.equals(EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATEb) == false)
                    EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATEIsChanged += IsChanged;
                break;

            case "TELEMETRY_OUTPUT_SERIALPORT_GROUND":
                TELEMETRY_OUTPUT_SERIALPORT_GROUND = Data;
                if(TELEMETRY_OUTPUT_SERIALPORT_GROUND.equals(TELEMETRY_OUTPUT_SERIALPORT_GROUNDb) == false)
                    TELEMETRY_OUTPUT_SERIALPORT_GROUNDIsChanged += IsChanged;
                break;

            case "TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATE":
                TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATE = Data;
                if(TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATE.equals(TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATEb) == false)
                    TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATEIsChanged += IsChanged;
                break;

            case "ENABLE_SERIAL_TELEMETRY_OUTPUT":
                ENABLE_SERIAL_TELEMETRY_OUTPUT = Data;
                if(ENABLE_SERIAL_TELEMETRY_OUTPUT.equals(ENABLE_SERIAL_TELEMETRY_OUTPUTb) == false)
                    ENABLE_SERIAL_TELEMETRY_OUTPUTIsChanged += IsChanged;
                break;

            case "FORWARD_STREAM":
                FORWARD_STREAM = Data;
                if(FORWARD_STREAM.equals(FORWARD_STREAMb) == false)
                    FORWARD_STREAMIsChanged += IsChanged;

                break;
            case "VIDEO_UDP_PORT":
                VIDEO_UDP_PORT = Data;
                if(VIDEO_UDP_PORT.equals(VIDEO_UDP_PORTb) == false)
                    VIDEO_UDP_PORTIsChanged += IsChanged;

                break;
            case "MAVLINK_FORWARDER":
                MAVLINK_FORWARDER = Data;
                if(MAVLINK_FORWARDER.equals(MAVLINK_FORWARDERb) == false)
                    MAVLINK_FORWARDERIsChanged += IsChanged;
                break;

            case "RELAY":
                RELAY = Data;
                if(RELAY.equals(RELAYb) == false)
                    RELAYIsChanged +=IsChanged;
                break;

            case "DEBUG":
                DEBUG = Data;
                if(DEBUG.equals(DEBUGb) == false)
                    DEBUGIsChanged += IsChanged;
                break;


            case "IsCamera1Enabled":
                IsCamera1Enabled = Data;
                if(IsCamera1Enabled.equals(IsCamera1Enabledb) == false)
                    IsCamera1EnabledIsChanged += IsChanged;
                break;

            case "IsCamera2Enabled":
                IsCamera2Enabled = Data;
                if(IsCamera2Enabled.equals(IsCamera2Enabledb) == false)
                    IsCamera2EnabledIsChanged += IsChanged;
                break;

            case "IsCamera3Enabled":
                IsCamera3Enabled = Data;
                if(IsCamera3Enabled.equals(IsCamera3Enabledb) == false)
                    IsCamera3EnabledIsChanged += IsChanged;
                break;

            case "IsCamera4Enabled":
                IsCamera4Enabled = Data;
                if(IsCamera4Enabled.equals(IsCamera4Enabledb) == false)
                    IsCamera4EnabledIsChanged += IsChanged;
                break;

            case "DefaultCameraId":
                DefaultCameraId = Data;
                if(DefaultCameraId.equals(DefaultCameraIdb) == false)
                    DefaultCameraIdIsChanged += IsChanged;
                break;

            case "ChannelToListen":
                ChannelToListen = Data;
                if(ChannelToListen.equals(ChannelToListenb) == false)
                    ChannelToListenIsChanged += IsChanged;
                break;

            case "Camera1ValueMin":
                Camera1ValueMin = Data;
                if(Camera1ValueMin.equals(Camera1ValueMinb) == false)
                    Camera1ValueMinIsChanged += IsChanged;
                break;

            case "Camera1ValueMax":
                Camera1ValueMax = Data;
                if(Camera1ValueMax.equals(Camera1ValueMaxb) == false)
                    Camera1ValueMaxIsChanged += IsChanged;
                break;
            ////
            case "Camera2ValueMin":
                Camera2ValueMin = Data;
                if(Camera2ValueMin.equals(Camera2ValueMinb) == false)
                    Camera2ValueMinIsChanged += IsChanged;
                break;

            case "Camera2ValueMax":
                Camera2ValueMax = Data;
                if(Camera2ValueMax.equals(Camera2ValueMaxb) == false)
                    Camera2ValueMaxIsChanged += IsChanged;
                break;

            case "Camera3ValueMin":
                Camera3ValueMin = Data;
                if(Camera3ValueMin.equals(Camera3ValueMinb) == false)
                    Camera3ValueMinIsChanged += IsChanged;
                break;

            case "Camera3ValueMax":
                Camera3ValueMax = Data;
                if(Camera3ValueMax.equals(Camera3ValueMaxb) == false)
                    Camera3ValueMaxIsChanged += IsChanged;
                break;

            case "Camera4ValueMin":
                Camera4ValueMin = Data;
                if(Camera4ValueMin.equals(Camera4ValueMinb) == false)
                    Camera4ValueMinIsChanged += IsChanged;
                break;

            case "Camera4ValueMax":
                Camera4ValueMax = Data;
                if(Camera4ValueMax.equals(Camera4ValueMaxb) == false)
                    Camera4ValueMaxIsChanged += IsChanged;
                break;

            case "CELLS":
                CELLS = Data;
                if(CELLS.equals(CELLSb) == false)
                    CELLSIsChanged += IsChanged;
                break;

            case "EncryptionOrRange":
                EncryptionOrRange = Data;
                if(EncryptionOrRange.equals(EncryptionOrRangeb) == false)
                    EncryptionOrRangeIsChanged += IsChanged;
                break;

            case "IsBandSwicherEnabled":
                IsBandSwicherEnabled = Data;
                if(IsBandSwicherEnabled.equals(IsBandSwicherEnabledb) == false)
                    IsBandSwicherEnabledIsChanged += IsChanged;
                break;

            case "Bandwidth":
                Bandwidth = Data;
                if(Bandwidth.equals(Bandwidthb) == false)
                    BandwidthIsChanged += IsChanged;
                break;

            case "UplinkSpeed":
                UplinkSpeed = Data;
                if(UplinkSpeed.equals(UplinkSpeedb) == false)
                    UplinkSpeedIsChanged += IsChanged;
                break;

            case "ChannelToListen2":
                ChannelToListen2 = Data;
                if(ChannelToListen2.equals(ChannelToListen2b) == false)
                    ChannelToListen2IsChanged += IsChanged;
                break;

            case "PrimaryCardMAC":
                PrimaryCardMAC = Data;
                if(PrimaryCardMAC.equals(PrimaryCardMACb) == false)
                    PrimaryCardMACIsChanged += IsChanged;
                break;

            case "SlaveCardMAC":
                SlaveCardMAC = Data;
                if(SlaveCardMAC.equals(SlaveCardMACb) == false)
                    SlaveCardMACIsChanged += IsChanged;
                break;

            case "Band5Below":
                Band5Below = Data;
                if(Band5Below.equals(Band5Belowb) == false)
                    Band5BelowIsChanged += IsChanged;
                break;

            case "Band10ValueMin":
                Band10ValueMin = Data;
                if(Band10ValueMin.equals(Band10ValueMinb) == false)
                    Band10ValueMinIsChanged += IsChanged;
                break;

            case "Band10ValueMax":
                Band10ValueMax = Data;
                if(Band10ValueMax.equals(Band10ValueMaxb) == false)
                    Band10ValueMaxIsChanged += IsChanged;
                break;

            case "Band20After":
                Band20After = Data;
                if(Band20After.equals(Band20Afterb) == false)
                    Band20AfterIsChanged += IsChanged;
                break;

            case "DefaultAudioOut":
                DefaultAudioOut = Data;
                if(DefaultAudioOut.equals(DefaultAudioOutb) == false)
                    DefaultAudioOutIsChanged += IsChanged;
                break;

            case "IsAudioTransferEnabled":
                IsAudioTransferEnabled = Data;
                if(IsAudioTransferEnabled.equals(IsAudioTransferEnabledb) == false)
                    IsAudioTransferEnabledIsChanged += IsChanged;
                break;

            case "txpowerA":
                txpowerA = Data;
                if(txpowerA.equals(txpowerAb) == false)
                    txpowerAIsChanged += IsChanged;
                break;

            case "txpowerR":
                txpowerR = Data;
                if(txpowerR.equals(txpowerRb) == false)
                    txpowerRIsChanged += IsChanged;
                break;

            default:

        }

    }

    public String FREQ;
    public String FREQb;
    public int FREQIsChanged = -1;
    public int FREQAirAck = 0;
    public int FREQGroundAck = 0;

    public String Copter;
    public String Copterb;
    public int CopterIsChanged = 0;
    public int CopterAirAck = 0;
    public int CopterGroundAck = 0;

    public String Imperial;
    public String Imperialb;
    public int ImperialIsChanged = 0;
    public int ImperialAirAck = 0;
    public int ImperialGroundAck = 0;

    public String DATARATE;
    public String DATARATEb;
    public int DATARATEIsChanged = -1;
    public int DATARATEAirAck = 0;
    public int DATARATEGroundAck = 0;

    public String FPS;
    public String FPSb;
    public int FPSIsChanged = -1;
    public int FPSAirAck = 0;
    public int FPSGroundAck = 0;

    public String VIDEO_BLOCKS;
    public String VIDEO_BLOCKSb;
    public int VIDEO_BLOCKSIsChanged = -1;
    public int VIDEO_BLOCKSAirAck = 0;
    public int VIDEO_BLOCKSGroundAck = 0;

    public String VIDEO_FECS;
    public String VIDEO_FECSb;
    public int VIDEO_FECSIsChanged = -1;
    public int VIDEO_FECSAirAck = 0;
    public int VIDEO_FECSGroundAck = 0;

    public String VIDEO_BLOCKLENGTH;
    public String VIDEO_BLOCKLENGTHb;
    public int VIDEO_BLOCKLENGTHIsChanged =-1;
    public int VIDEO_BLOCKLENGTHAirAck = 0;
    public int VIDEO_BLOCKLENGTHGroundAck = 0;

    public String VIDEO_BITRATE;
    public String VIDEO_BITRATEb;
    public int VIDEO_BITRATEIsChanged = -1;
    public int VIDEO_BITRATEAirAck = 0;
    public int VIDEO_BITRATEGroundAck = 0;

    public String BITRATE_PERCENT;
    public String BITRATE_PERCENTb;
    public int BITRATE_PERCENTIsChanged = -1;
    public int BITRATE_PERCENTAirAck = 0;
    public int BITRATE_PERCENTGroundAck = 0;

    public String WIDTH;
    public String WIDTHb;
    public int WIDTHIsChanged = -1;
    public int WIDTHAirAck = 0;
    public int WIDTHGroundAck = 0;

    public String HEIGHT;
    public String HEIGHTb;
    public int HEIGHTIsChanged = -1;
    public int HEIGHTAirAck = 0;
    public int HEIGHTGroundAck = 0;

    public String KEYFRAMERATE;
    public String KEYFRAMERATEb;
    public int KEYFRAMERATEIsChanged = -1;
    public int KEYFRAMERATEAirAck = 0;
    public int KEYFRAMERATEGroundAck = 0;

    public String EXTRAPARAMS;
    public String EXTRAPARAMSb;
    public int EXTRAPARAMSIsChanged = -1;
    public int EXTRAPARAMSAirAck = 0;
    public int EXTRAPARAMSGroundAck = 0;

    public String FREQSCAN;
    public String FREQSCANb;
    public int FREQSCANIsChanged = -1;
    public int FREQSCANAirAck = 0;
    public int FREQSCANGroundAck = 0;

    public String TXMODE;
    public String TXMODEb;
    public int TXMODEIsChanged = -1;
    public int TXMODEAirAck = 0;
    public int TXMODEGroundAck = 0;


    public String UPDATE_NTH_TIME;
    public String UPDATE_NTH_TIMEb;
    public int UPDATE_NTH_TIMEIsChanged = -1;
    public int UPDATE_NTH_TIMEAirAck = 0;
    public int UPDATE_NTH_TIMEGroundAck = 0;

    /*
    public StringMAC_RX[0]=00c0ca91ee30
    public StringFREQ_RX[0]=2484
    public StringMAC_RX[1]=24050f953384
    public StringFREQ_RX[1]=2484
    public StringMAC_RX[2]=24050f953378
    public StringFREQ_RX[2]=2484
    public StringMAC_RX[3]=24050f953373
    public StringFREQ_RX[3]=2484
    public StringMAC_TX[0]=24050f953378
    public StringFREQ_TX[0]=5745
    public StringMAC_TX[1]=ec086b1c7834
    public StringFREQ_TX[1]=2472
    */

    public String TELEMETRY_TRANSMISSION;
    public String TELEMETRY_TRANSMISSIONb;
    public int TELEMETRY_TRANSMISSIONIsChanged = -1;
    public int TELEMETRY_TRANSMISSIONAirAck = 0;
    public int TELEMETRY_TRANSMISSIONGroundAck = 0;

    public String TELEMETRY_UPLINK;
    public String TELEMETRY_UPLINKb;
    public int TELEMETRY_UPLINKIsChanged = -1;
    public int TELEMETRY_UPLINKAirAck = 0;
    public int TELEMETRY_UPLINKGroundAck = 0;

    public String RC;
    public String RCb;
    public int RCIsChanged = -1;
    public int RCAirAck = 0;
    public int RCGroundAck = 0;

    public String DefaultAudioOut;
    public String DefaultAudioOutb;
    public int DefaultAudioOutIsChanged = -1;
    public int DefaultAudioOutAirAck = 0;
    public int DefaultAudioOutGroundAck = 0;


    public String IsAudioTransferEnabled;
    public String IsAudioTransferEnabledb;
    public int IsAudioTransferEnabledIsChanged = -1;
    public int IsAudioTransferEnabledAirAck = 0;
    public int IsAudioTransferEnabledGroundAck = 0;

    public String txpowerA;
    public String txpowerAb;
    public int txpowerAIsChanged = -1;
    public int txpowerAAirAck = 0;
    public int txpowerAGroundAck = 0;

    public String txpowerR;
    public String txpowerRb;
    public int txpowerRIsChanged = -1;
    public int txpowerRAirAck = 0;
    public int txpowerRGroundAck = 0;




    public String CTS_PROTECTION;
    public String CTS_PROTECTIONb;
    public int CTS_PROTECTIONIsChanged = -1;
    public int CTS_PROTECTIONAirAck = 0;
    public int CTS_PROTECTIONGroundAck = 0;


    public String FC_RC_SERIALPORT;
    public String FC_RC_SERIALPORTb;
    public int FC_RC_SERIALPORTIsChanged = -1;
    public int FC_RC_SERIALPORTAirAck = 0;
    public int FC_RC_SERIALPORTGroundAck = 0;

    public String FC_RC_BAUDRATE;
    public String FC_RC_BAUDRATEb;
    public int FC_RC_BAUDRATEIsChanged = -1;
    public int FC_RC_BAUDRATEAirAck = 0;
    public int FC_RC_BAUDRATEGroundAck = 0;

    public String FC_TELEMETRY_SERIALPORT;
    public String FC_TELEMETRY_SERIALPORTb;
    public int FC_TELEMETRY_SERIALPORTIsChanged = -1;
    public int FC_TELEMETRY_SERIALPORTAirAck = 0;
    public int FC_TELEMETRY_SERIALPORTGroundAck = 0;

    public String FC_TELEMETRY_BAUDRATE;
    public String FC_TELEMETRY_BAUDRATEb;
    public int FC_TELEMETRY_BAUDRATEIsChanged = -1;
    public int FC_TELEMETRY_BAUDRATEAirAck = 0;
    public int FC_TELEMETRY_BAUDRATEGroundAck = 0;

    public String FC_MSP_SERIALPORT;
    public String FC_MSP_SERIALPORTb;
    public int FC_MSP_SERIALPORTIsChanged = -1;
    public int FC_MSP_SERIALPORTAirAck = 0;
    public int FC_MSP_SERIALPORTGroundAck = 0;

    public String FC_MSP_BAUDRATE;
    public String FC_MSP_BAUDRATEb;
    public int FC_MSP_BAUDRATEIsChanged = -1;
    public int FC_MSP_BAUDRATEAirAck = 0;
    public int FC_MSP_BAUDRATEGroundAck = 0;

    public String RemoteSettingsEnabled;
    public String RemoteSettingsEnabledb;
    public int RemoteSettingsEnabledIsChanged = -1;
    public int RemoteSettingsEnabledAirAck = 0;
    public int RemoteSettingsEnabledGroundAck = 0;




    public String AIRODUMP;
    public String AIRODUMPb;
    public int AIRODUMPIsChanged = -1;
    public int AIRODUMPAirAck = 0;
    public int AIRODUMPGroundAck = 0;

    public String AIRODUMP_SECONDS;
    public String AIRODUMP_SECONDSb;
    public int AIRODUMP_SECONDSIsChanged = -1;
    public int AIRODUMP_SECONDSAirAck = 0;
    public int AIRODUMP_SECONDSGroundAck = 0;

    public String WIFI_HOTSPOT;
    public String WIFI_HOTSPOTb;
    public int WIFI_HOTSPOTIsChanged = -1;
    public int WIFI_HOTSPOTAirAck = 0;
    public int WIFI_HOTSPOTGroundAck = 0;

    public String WIFI_HOTSPOT_NIC;
    public String WIFI_HOTSPOT_NICb;
    public int WIFI_HOTSPOT_NICIsChanged = -1;
    public int WIFI_HOTSPOT_NICAirAck = 0;
    public int WIFI_HOTSPOT_NICGroundAck = 0;

    public String ETHERNET_HOTSPOT;
    public String ETHERNET_HOTSPOTb;
    public int ETHERNET_HOTSPOTIsChanged = -1;
    public int ETHERNET_HOTSPOTAirAck = 0;
    public int ETHERNET_HOTSPOTGroundAck = 0;

    public String ENABLE_SCREENSHOTS;
    public String ENABLE_SCREENSHOTSb;
    public int ENABLE_SCREENSHOTSIsChanged = -1;
    public int ENABLE_SCREENSHOTSAirAck = 0;
    public int ENABLE_SCREENSHOTSGroundAck = 0;

    public String VIDEO_TMP;
    public String VIDEO_TMPb;
    public int VIDEO_TMPIsChanged = -1;
    public int VIDEO_TMPAirAck = 0;
    public int VIDEO_TMPGroundAck = 0;

    public String RELAY;
    public String RELAYb;
    public int RELAYIsChanged = -1;
    public int RELAYAirAck = 0;
    public int RELAYGroundAck = 0;

    public String RELAY_NIC;
    public String RELAY_NICb;
    public int RELAY_NICIsChanged = -1;
    public int RELAY_NICAirAck = 0;
    public int RELAY_NICGroundAck = 0;

    public String RELAY_FREQ;
    public String RELAY_FREQb;
    public int RELAY_FREQIsChanged = -1;
    public int RELAY_FREQAirAck = 0;
    public int RELAY_FREQGroundAck = 0;

    public String QUIET;
    public String QUIETb;
    public int QUIETIsChanged = -1;
    public int QUIETAirAck = 0;
    public int QUIETGroundAck = 0;

    public String EXTERNAL_TELEMETRY_SERIALPORT_GROUND;
    public String EXTERNAL_TELEMETRY_SERIALPORT_GROUNDb;
    public int EXTERNAL_TELEMETRY_SERIALPORT_GROUNDIsChanged = -1;
    public int EXTERNAL_TELEMETRY_SERIALPORT_GROUNDAirAck = 0;
    public int EXTERNAL_TELEMETRY_SERIALPORT_GROUNDGroundAck = 0;

    public String EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE;
    public String EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATEb;
    public int EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATEIsChanged = -1;
    public int EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATEAirAck = 0;
    public int EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATEGroundAck = 0;

    public String ENABLE_SERIAL_TELEMETRY_OUTPUT;
    public String ENABLE_SERIAL_TELEMETRY_OUTPUTb;
    public int ENABLE_SERIAL_TELEMETRY_OUTPUTIsChanged = -1;
    public int ENABLE_SERIAL_TELEMETRY_OUTPUTAirAck = 0;
    public int ENABLE_SERIAL_TELEMETRY_OUTPUTGroundAck = 0;

    public String TELEMETRY_OUTPUT_SERIALPORT_GROUND;
    public String TELEMETRY_OUTPUT_SERIALPORT_GROUNDb;
    public int TELEMETRY_OUTPUT_SERIALPORT_GROUNDIsChanged = -1;
    public int TELEMETRY_OUTPUT_SERIALPORT_GROUNDAirAck = 0;
    public int TELEMETRY_OUTPUT_SERIALPORT_GROUNDGroundAck = 0;

    public String TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATE;
    public String TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATEb;
    public int TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATEIsChanged = -1;
    public int TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATEAirAck = 0;
    public int TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATEGroundAck = 0;

    public String FORWARD_STREAM;
    public String FORWARD_STREAMb;
    public int FORWARD_STREAMIsChanged = -1;
    public int FORWARD_STREAMAirAck = 0;
    public int FORWARD_STREAMGroundAck = 0;

    public String VIDEO_UDP_PORT;
    public String VIDEO_UDP_PORTb;
    public int VIDEO_UDP_PORTIsChanged = -1;
    public int VIDEO_UDP_PORTAirAck = 0;
    public int VIDEO_UDP_PORTGroundAck = 0;

    public String MAVLINK_FORWARDER;
    public String MAVLINK_FORWARDERb;
    public int MAVLINK_FORWARDERIsChanged = -1;
    public int MAVLINK_FORWARDERAirAck = 0;
    public int MAVLINK_FORWARDERGroundAck = 0;

    public String DEBUG;
    public String DEBUGb;
    public int DEBUGIsChanged = -1;
    public int DEBUGAirAck = 0;
    public int DEBUGGroundAck = 0;

    public String IsCamera1Enabled;
    public String IsCamera1Enabledb;
    public int IsCamera1EnabledIsChanged = -1;
    public int IsCamera1EnabledAirAck = 0;
    public int IsCamera1EnabledGroundAck = 0;

    public String IsCamera2Enabled;
    public String IsCamera2Enabledb;
    public int IsCamera2EnabledIsChanged = -1;
    public int IsCamera2EnabledAirAck = 0;
    public int IsCamera2EnabledGroundAck = 0;

    public String IsCamera3Enabled;
    public String IsCamera3Enabledb;
    public int IsCamera3EnabledIsChanged = -1;
    public int IsCamera3EnabledAirAck = 0;
    public int IsCamera3EnabledGroundAck = 0;

    public String IsCamera4Enabled;
    public String IsCamera4Enabledb;
    public int IsCamera4EnabledIsChanged = -1;
    public int IsCamera4EnabledAirAck = 0;
    public int IsCamera4EnabledGroundAck = 0;

    public String DefaultCameraId;
    public String DefaultCameraIdb;
    public int DefaultCameraIdIsChanged = -1;
    public int DefaultCameraIdAirAck = 0;
    public int DefaultCameraIdGroundAck = 0;

    public String ChannelToListen;
    public String ChannelToListenb;
    public int ChannelToListenIsChanged = -1;
    public int ChannelToListenAirAck = 0;
    public int ChannelToListenGroundAck = 0;


    public String Camera1ValueMin;
    public String Camera1ValueMinb;
    public int Camera1ValueMinIsChanged = -1;
    public int Camera1ValueMinAirAck = 0;
    public int Camera1ValueMinGroundAck = 0;

    public String Camera1ValueMax;
    public String Camera1ValueMaxb;
    public int Camera1ValueMaxIsChanged = -1;
    public int Camera1ValueMaxAirAck = 0;
    public int Camera1ValueMaxGroundAck = 0;


    public String Camera2ValueMin;
    public String Camera2ValueMinb;
    public int Camera2ValueMinIsChanged = -1;
    public int Camera2ValueMinAirAck = 0;
    public int Camera2ValueMinGroundAck = 0;

    public String Camera2ValueMax;
    public String Camera2ValueMaxb;
    public int Camera2ValueMaxIsChanged = -1;
    public int Camera2ValueMaxAirAck = 0;
    public int Camera2ValueMaxGroundAck = 0;


    public String Camera3ValueMin;
    public String Camera3ValueMinb;
    public int Camera3ValueMinIsChanged = -1;
    public int Camera3ValueMinAirAck = 0;
    public int Camera3ValueMinGroundAck = 0;

    public String Camera3ValueMax;
    public String Camera3ValueMaxb;
    public int Camera3ValueMaxIsChanged = -1;
    public int Camera3ValueMaxAirAck = 0;
    public int Camera3ValueMaxGroundAck = 0;


    public String Camera4ValueMin;
    public String Camera4ValueMinb;
    public int Camera4ValueMinIsChanged = -1;
    public int Camera4ValueMinAirAck = 0;
    public int Camera4ValueMinGroundAck = 0;

    public String Camera4ValueMax;
    public String Camera4ValueMaxb;
    public int Camera4ValueMaxIsChanged = -1;
    public int Camera4ValueMaxAirAck = 0;
    public int Camera4ValueMaxGroundAck = 0;

    public String CELLS;
    public String CELLSb;
    public int CELLSIsChanged = -1;
    public int CELLSAirAck = 0;
    public int CELLSGroundAck = 0;

    public String EncryptionOrRange;
    public String EncryptionOrRangeb;
    public int EncryptionOrRangeIsChanged = -1;
    public int EncryptionOrRangeAirAck = 0;
    public int EncryptionOrRangeGroundAck = 0;

    public String IsBandSwicherEnabled;
    public String IsBandSwicherEnabledb;
    public int IsBandSwicherEnabledIsChanged = -1;
    public int IsBandSwicherEnabledAirAck = 0;
    public int IsBandSwicherEnabledGroundAck = 0;

    public String Bandwidth;
    public String Bandwidthb;
    public int BandwidthIsChanged = -1;
    public int BandwidthAirAck = 0;
    public int BandwidthGroundAck = 0;

    public String UplinkSpeed;
    public String UplinkSpeedb;
    public int UplinkSpeedIsChanged = -1;
    public int UplinkSpeedAirAck = 0;
    public int UplinkSpeedGroundAck = 0;

    public String ChannelToListen2;
    public String ChannelToListen2b;
    public int ChannelToListen2IsChanged = -1;
    public int ChannelToListen2AirAck = 0;
    public int ChannelToListen2GroundAck = 0;

    public String PrimaryCardMAC;
    public String PrimaryCardMACb;
    public int PrimaryCardMACIsChanged = -1;
    public int PrimaryCardMACAirAck = 0;
    public int PrimaryCardMACGroundAck = 0;

    public String SlaveCardMAC;
    public String SlaveCardMACb;
    public int SlaveCardMACIsChanged = -1;
    public int SlaveCardMACAirAck = 0;
    public int SlaveCardMACGroundAck = 0;

    public String Band5Below;
    public String Band5Belowb;
    public int Band5BelowIsChanged = -1;
    public int Band5BelowAirAck = 0;
    public int Band5BelowGroundAck = 0;

    public String Band10ValueMin;
    public String Band10ValueMinb;
    public int Band10ValueMinIsChanged = -1;
    public int Band10ValueMinAirAck = 0;
    public int Band10ValueMinGroundAck = 0;

    public String Band10ValueMax;
    public String Band10ValueMaxb;
    public int Band10ValueMaxIsChanged = -1;
    public int Band10ValueMaxAirAck = 0;
    public int Band10ValueMaxGroundAck = 0;

    public String Band20After;
    public String Band20Afterb;
    public int Band20AfterIsChanged = -1;
    public int Band20AfterAirAck = 0;
    public int Band20AfterGroundAck = 0;

}
