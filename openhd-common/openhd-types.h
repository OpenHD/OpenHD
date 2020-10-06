#ifndef OPENHD_CONSTANTS_H
#define OPENHD_CONSTANTS_H

typedef enum CameraType {
    CameraTypeRaspberryPiCSI,
    CameraTypeJetsonCSI,
    CameraTypeRockchipCSI,
    CameraTypeUVC,
    CameraTypeIP,
    CameraTypeUnknown
} CameraType;


typedef enum PlatformType {
    PlatformTypeRaspberryPi,
    PlatformTypeJetson,
    PlatformTypeNanoPi,
    PlatformTypePC,
    PlatformTypeUnknown
} PlatformType;



typedef enum WiFiCardType {
    WiFiCardTypeRealtek8812au,
    WiFiCardTypeRealtek8814au,
    WiFiCardTypeRealtek88x2bu,
    WiFiCardTypeRealtek8188eu,
    WiFiCardTypeAtheros9k,
    WiFiCardTypeRalink,
    WiFiCardTypeIntel,
    WiFiCardTypeBroadcom,
    WiFiCardTypeUnknown
} WiFiCardType;


typedef enum WiFiHotspotType {
    WiFiHotspotTypeInternal2GBand,
    WiFiHotspotTypeInternal5GBand,
    WiFiHotspotTypeInternalDualBand,
    WiFiHotspotTypeExternal,
    WiFiHotspotTypeNone
} WiFiHotspotType;



#endif

