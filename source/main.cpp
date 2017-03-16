/*
 * MbedのBLE用サンプルプログラム「BLE_EddystoneObserverの拡張版」でアドバタイズのうちビーコンの信号
 * と思しきものだけを処理(出力)するプログラム
 * 
 * 元のサンプルプログラムのURL
 * https://github.com/ARMmbed/mbed-os-example-ble/tree/master/BLE_EddystoneObserver
 */


#include <events/mbed_events.h>
#include "mbed.h"
#include "ble/BLE.h"

/*
 * 受信するビーコンの種類
 */
#define IBEACON
#define EDDYSTONE
#define ALTBEACON
#define UCODE
#define OMRON_ENV_SENSOR


// ビーコン信号の出力フォーマットの選択 (有効にすると，アドバタイズの内容を16進数でダンプ
//#define ADV_DUMP

// デバッグメッセージの出力と受信したアドバタイズを全て出力するか否か
//#define DEBUG
//#define DUMP_ADV_DATA

// ビーコンの信号を出力する際の行末文字の選択
//#define EOL "\r\n"
#define EOL "\n"

// Arduino等と連携する場合に，送信可能か否かのピンを使うか否かの選択と利用する場合のピン番号の指定
//#define USE_SERIAL_ACTIVE_PIN
//#define SERIAL_ACTIVE_PIN p8


/*
 * ビーコンの種別の定数
 */
#define EDDIYSTONE_UID 1
#define EDDIYSTONE_URL 2
#define EDDIYSTONE_TML 3

/*
 * 各種のデータのサイズの指定
 */
#define MAX_BUFF_LEN 256
#define MAC_ADDR_STR_LEN 20
#define MAX_ADV_SIZE 31

/*
 * 各種の文字列処理用のバッファ
 */
char macString[MAC_ADDR_STR_LEN]; // MACを文字列に変換する時の収納先文字列
char message[MAX_BUFF_LEN];       // アドバタイズの情報をまとめて格納する文字列

#ifdef EDDYSTONE
char url[MAX_BUFF_LEN]; // Eddystone URL型のビーコンのURLを解析する時に使う文字列
#endif /* EDDYSTONE */

#ifdef USE_SERIAL_ACTIVE_PIN
DigitalIn enable(SERIAL_ACTIVE_PIN); // 外部マイコン(Arduino等)と連携する時のピンの有効化
#endif /* USE_SERIAL_ACTIVE_PIN */

#ifdef ADV_DUMP
char adv[MAX_BUFF_LEN]; // アドバタイズを文字列に変換する時に使う文字列
#endif /* ADV_DUMP */

static EventQueue eventQueue(
    /* event count */ 16 * /* event size */ 32
);

/*
 * MACアドレスを文字列に変更
 */
void convMAC(const Gap::AdvertisementCallbackParams_t *params){
    memset(macString,0,MAC_ADDR_STR_LEN);
    snprintf(macString,MAC_ADDR_STR_LEN,"%02x:%02x:%02x:%02x:%02x:%02x",params->peerAddr[5], params->peerAddr[4], params->peerAddr[3], params->peerAddr[2], params->peerAddr[1], params->peerAddr[0]);
}

/*
 * アドバタイズを出力する関数
 */
void logging(char *buff){
#ifdef USE_SERIAL_ACTIVE_PIN
    if (enable==1) { // 外部ピンがHIGHの時だけ出力
        printf("%s",buff);
    };
#else /* USE_SERIAL_ACTIVE_PIN */
    printf("%s",buff);
#endif /* USE_SERIAL_ACTIVE_PIN */
}

/*
 * アドバタイズを文字列に変換
 */
#ifdef ADV_DUMP
char * outputADV(char * buff,const Gap::AdvertisementCallbackParams_t *params) {
    //char * originalBuff=buff;
    memset(buff,0,MAX_BUFF_LEN);
    sprintf(buff,"%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x"
        ,params->advertisingData[0],params->advertisingData[1],params->advertisingData[2],params->advertisingData[3],params->advertisingData[4],params->advertisingData[5]
        ,params->advertisingData[6],params->advertisingData[7],params->advertisingData[8],params->advertisingData[9],params->advertisingData[10]
        ,params->advertisingData[11],params->advertisingData[12],params->advertisingData[13],params->advertisingData[14],params->advertisingData[15]
        ,params->advertisingData[16],params->advertisingData[17],params->advertisingData[18],params->advertisingData[19],params->advertisingData[20]
        ,params->advertisingData[21],params->advertisingData[22],params->advertisingData[23],params->advertisingData[24],params->advertisingData[25]
        ,params->advertisingData[26],params->advertisingData[27],params->advertisingData[28],params->advertisingData[29],params->advertisingData[30]);
    return buff;
}
#endif /* ADV_DUMP */

/*
 * アドバタイズがiBeaconか否かを判定し，iBeaconなら出力
 */
#ifdef IBEACON
bool iBeaconCheck(const Gap::AdvertisementCallbackParams_t *params) {
    memset(message,0,MAX_BUFF_LEN);
    convMAC(params);
    if ((params->advertisingData[7]==0x02)&&(params->advertisingData[8]==0x15)) {
        //int8_t rssi = calcRSSI(params->advertisingData[29]);
#ifdef ADV_DUMP // ビーコンの信号をダンプする場合
        outputADV(adv,params);
        snprintf(message,MAX_BUFF_LEN,"iBeacon MAC = %s , RSSI = %hhx , ADV = %s%s",macString,params->rssi,adv,EOL);
#else /* ADV_DUMP */ // 成型して出力する場合
        snprintf(message,MAX_BUFF_LEN,"iBeacon MAC = %s , RSSI = %hhx , vendor = %02x %02x , UUID = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x , Major = %02x %02x , Minor = %02x %02x , Tx = %02x%s",
            macString,
            params->rssi,
            params->advertisingData[5],params->advertisingData[6],
            params->advertisingData[9],params->advertisingData[10],params->advertisingData[11],params->advertisingData[12],params->advertisingData[13],params->advertisingData[14],params->advertisingData[15],params->advertisingData[16],params->advertisingData[17],params->advertisingData[18],params->advertisingData[19],params->advertisingData[20],params->advertisingData[21],params->advertisingData[22],params->advertisingData[23],params->advertisingData[24],
            params->advertisingData[25],params->advertisingData[26],
            params->advertisingData[27],params->advertisingData[28],
            params->advertisingData[29],
            //rssi,
            EOL);
            //params->advertisingData[29]); 
#endif /* ADV_DUMP */
        logging(message);
        return true;
    };
    return false;
};
#endif /* IBEACON */

/*
 * Eddystone関連の関数
 */
 
 /*
  * アドバタイズに含まれるURLを文字列に変換
  */
#ifdef EDDYSTONE
void convURL(int8_t length,int8_t urlType,uint8_t *urlArray){
#ifdef DEBUG
    for (int i=0;i<17;i++){
        printf(" %02x",urlArray[i]);
    }
    printf("\n length=%d\n",length);
#endif /* DEBUG */
    memset(url,0,MAX_BUFF_LEN);
    int currentPointer=0;
    switch(urlType){
        case 0 :
            currentPointer=11;
            sprintf(url,"%s","http://www.");
            break;
        case 1:
            currentPointer=12;
            sprintf(url,"%s","https://www.");
            break;
        case 2:
            currentPointer=7;
            sprintf(url,"%s","http://");
            break;
        case 3:
            currentPointer=8;
            sprintf(url,"%s","https://");
            break;
    }
    for (int i=0;i<length;i++){
        if (urlArray[i] < 14){
            switch(urlArray[i]) {
                case 0:
                    sprintf((url+currentPointer),"%s",".com/");
                    currentPointer+=5;
                    break;
                case 1:
                    sprintf((url+currentPointer),"%s",".org/");
                    currentPointer+=5;
                    break;
                case 2:
                    sprintf((url+currentPointer),"%s",".edu/");
                    currentPointer+=5;
                    break;
                case 3:
                    sprintf((url+currentPointer),"%s",".net/");
                    currentPointer+=5;
                    break;
                case 4:
                    sprintf((url+currentPointer),"%s",".info/");
                    currentPointer+=6;
                    break;
                case 5:
                    sprintf((url+currentPointer),"%s",".biz/");
                    currentPointer+=5;
                    break;
                case 6:
                    sprintf((url+currentPointer),"%s",".gov/");
                    currentPointer+=5;
                    break;
                case 7:
                    sprintf((url+currentPointer),"%s",".com");
                    currentPointer+=4;
                    break;
                case 8:
                    sprintf((url+currentPointer),"%s",".org");
                    currentPointer+=4;
                    break;
                case 9:
                    sprintf((url+currentPointer),"%s",".edu");
                    currentPointer+=4;
                    break;
                case 10:
                    sprintf((url+currentPointer),"%s",".net");
                    currentPointer+=4;
                    break;
                case 11:
                    sprintf((url+currentPointer),"%s",".info");
                    currentPointer+=5;
                    break;
                case 12:
                    sprintf((url+currentPointer),"%s",".biz");
                    currentPointer+=4;
                    break;
                case 13:
                    sprintf((url+currentPointer),"%s",".gov");
                    currentPointer+=4;
            }
        } else if ((urlArray[i] > 32) && (urlArray[i] < 127)) {
            sprintf((url+currentPointer),"%c",(char)urlArray[i]);
            currentPointer++;
        }
    }
}

/*
 * Eddystoneか否かを判定して出力
 */
int eddyStoneCheck(const Gap::AdvertisementCallbackParams_t *params) {
    convMAC(params);
    if ((params->advertisingData[10]!=0xfe)||(params->advertisingData[9]!=0xaa)){
        return -1;
    }
#ifdef ADV_DUMP
    outputADV(adv,params);
#endif /* ADV_DUMP */
    memset(message,0,MAX_BUFF_LEN);
    switch(params->advertisingData[11]){
        case 0x00:
#ifdef ADV_DUMP
            snprintf(message,MAX_BUFF_LEN,"EddyStone-UID MAC = %s , RSSI = %hhx , ADV = %s%s",macString,params->rssi,adv,EOL);
#else /* ADV_DUMP */
            snprintf(message,MAX_BUFF_LEN,"EddyStone-UID MAC = %s , RSSI = %hhx , vendor = %02x %02x , NameSpace = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x , Instance = %02x %02x %02x %02x %02x %02x , Tx = %02x%s",
                macString,
                params->rssi,
                params->advertisingData[5],params->advertisingData[6],
                params->advertisingData[13],params->advertisingData[14],params->advertisingData[15],params->advertisingData[16],params->advertisingData[17],params->advertisingData[18],params->advertisingData[19],params->advertisingData[20],params->advertisingData[21],params->advertisingData[22],
                params->advertisingData[23],params->advertisingData[24],params->advertisingData[25],params->advertisingData[26],params->advertisingData[27],params->advertisingData[28],
                params->advertisingData[12],
                //rssi,
                EOL);
#endif /* ADV_DUMP */
            logging(message);
            return EDDIYSTONE_UID;
        case 0x10:
#ifdef ADV_DUMP
            snprintf(message,MAX_BUFF_LEN,"EddyStone-URL MAC = %s , RSSI = %hhx , ADV = %s%s",macString,params->rssi,adv,EOL);
#else /* ADV_DUMP */
            uint8_t dataArray[17];
            for (int i=0;i<17;i++){
                dataArray[i]=params->advertisingData[14+i];
            }
            convURL((params->advertisingDataLen)-14,params->advertisingData[13],dataArray);
            snprintf(message,MAX_BUFF_LEN,"EddyStone-URL MAC = %s , RSSI = %hhx , vendor = %02x %02x , URL = %s , Tx = %02x%s",
                macString,
                params->rssi,
                params->advertisingData[5],params->advertisingData[6],
                url,
                params->advertisingData[12],
                EOL);
#endif /* ADV_DUMP */
            logging(message);
            return EDDIYSTONE_URL;
        case 0x20:
#ifdef ADV_DUMP
            snprintf(message,MAX_BUFF_LEN,"EddyStone-TML MAC = %s , RSSI = %hhx , ADV = %s%s",macString,params->rssi,adv,EOL);
#else /* ADV_DUMP */
            snprintf(message,MAX_BUFF_LEN,"EddyStone-TML MAC = %s , RSSI = %hhx , vendor = %02x %02x , version = %02x , battery = %02x %02x , temperature = %02x %02x , PDU count = %02x %02x %02x %02x , uptime = %02x %02x %02x %02x%s",
                macString,
                params->rssi,
                params->advertisingData[5],params->advertisingData[6],
                params->advertisingData[12],
                params->advertisingData[13],params->advertisingData[14],
                params->advertisingData[15],params->advertisingData[16],
                params->advertisingData[17],params->advertisingData[18],params->advertisingData[19],params->advertisingData[20],
                params->advertisingData[21],params->advertisingData[22],params->advertisingData[23],params->advertisingData[24],
                EOL);
#endif /* ADV_DUMP */
            logging(message);
            return EDDIYSTONE_TML;
        default:
            return 0;
    }
}
#endif /* EDDYSTONE */

/*
 * AltBeaconか否かを判定して出力
 */
#ifdef ALTBEACON
int altCheck(const Gap::AdvertisementCallbackParams_t *params) {
    convMAC(params);
    if ((params->advertisingData[3]!=0x1b)||(params->advertisingData[4]!=0xff)||(params->advertisingData[7]!=0xbe)||(params->advertisingData[8]!=0xac)){
        return -1;
    }
#ifdef DEBUG
    printf("Alt Beacon!\n");
#endif /* DEBUG */
    memset(message,0,MAX_BUFF_LEN);
#ifdef ADV_DUMP
    outputADV(adv,params);
    snprintf(message,MAX_BUFF_LEN,"ALT-BEACON MAC = %s , RSSI = %hhx , ADV = %s%s",macString,params->rssi,adv,EOL);
#else /* ADV_DUMP */
    snprintf(message,MAX_BUFF_LEN,"ALT-BEACON MAC = %s , RSSI = %hhx , vendor = %02x %02x , code = %02x %02x , ID = %02x  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x , ext = %02x , Tx = %02x%s",
        macString,
        params->rssi,
        params->advertisingData[5],params->advertisingData[6],
        params->advertisingData[7],params->advertisingData[8],
        params->advertisingData[9],
        params->advertisingData[10],params->advertisingData[11],params->advertisingData[12],params->advertisingData[13],params->advertisingData[14],params->advertisingData[15],params->advertisingData[16],params->advertisingData[17],params->advertisingData[18],params->advertisingData[19],
        params->advertisingData[20],params->advertisingData[21],params->advertisingData[22],params->advertisingData[23],params->advertisingData[24],params->advertisingData[25],params->advertisingData[26],params->advertisingData[27],params->advertisingData[28],
        params->advertisingData[30],
        params->advertisingData[29],
        EOL);
#endif /* ADV_DUMP */
    logging(message);
    return 0;
}
#endif /* ALTBEACON */

/*
 * Ucode型のビーコンの判定と出力
 */
#ifdef UCODE
bool ucodeCheck(const Gap::AdvertisementCallbackParams_t *params) {
    convMAC(params);
    if ((params->advertisingData[2]!=0x00)||(params->advertisingData[4]!=0x18)){
        return false;
    }
#ifdef DEBUG
    printf("UCODE!\n");
#endif /* DEBUG */
    memset(message,0,MAX_BUFF_LEN);
#ifdef ADV_DUMP
    outputADV(adv,params);
    snprintf(message,MAX_BUFF_LEN,"UCODE MAC = %s , RSSI = %hhx , ADV = %s%s",macString,params->rssi,adv,EOL);
#else /* ADV_DUMP */
    snprintf(message,MAX_BUFF_LEN,"UCODE MAC = %s , RSSI = %hhx , uuid = %02x %02x , vendor = %02x %02x , status = %02x , Tx = %02x , counter = %02x , code = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x%s",
        macString,
        params->rssi,
        params->advertisingData[2],params->advertisingData[3], // uuid
        params->advertisingData[6],params->advertisingData[7], // vendor
        params->advertisingData[25],                           // status
        params->advertisingData[26],                           // Tx power
        params->advertisingData[27],                           // Tx counter
        params->advertisingData[9],params->advertisingData[10],params->advertisingData[11],params->advertisingData[12],
        params->advertisingData[13],params->advertisingData[14],params->advertisingData[15],params->advertisingData[16],
        params->advertisingData[17],params->advertisingData[18],params->advertisingData[19],params->advertisingData[20],
        params->advertisingData[21],params->advertisingData[22],params->advertisingData[23],params->advertisingData[24],
        EOL);
#endif /* ADV_DUMP */
    logging(message);
    return true;
}
#endif /* UCODE */

/*
 * オムロンの環境センサの判定と出力
 */
#ifdef OMRON_ENV_SENSOR
/* センサパケットの種別判定 */
int omronSensorType(const Gap::AdvertisementCallbackParams_t *params) {
    if ((params->advertisingData[0]!=0x02)||(params->advertisingData[1]!=0x01)||(params->advertisingData[2]!=0x06)||(params->advertisingData[3]!=0x17)||(params->advertisingData[4]!=0xff)||(params->advertisingData[5]!=0xd5)||(params->advertisingData[6]!=0x02)||(params->advertisingData[27]!=0x03)||(params->advertisingData[28]!=0x08)){
        return 0;
    }
    if ((params->advertisingData[29]==0x49)&&(params->advertisingData[30]==0x4d)){
        return 1; // フォーマット(D)
    }
    if ((params->advertisingData[29]==0x45)&&(params->advertisingData[30]==0x50)){
        return 2; // フォーマット(E)
    }
    return 0;
}

/*
 * オムロン環境センサの判定と出力
 */
bool omronSensorCheck(const Gap::AdvertisementCallbackParams_t *params) {
    convMAC(params);
    // オムロン環境センサのアドバタイズを識別する条件
    int type = omronSensorType(params);
    if (type==0){
        return false;
    }
#ifdef DEBUG
    printf("Omron Sensor!\n");
#endif /* DEBUG */
    memset(message,0,MAX_BUFF_LEN);
#ifdef ADV_DUMP
    outputADV(adv,params);
    if (type == 1) {
        snprintf(message,MAX_BUFF_LEN,"OMRON-D MAC = %s , RSSI = %hhx , ADV = %s%s",macString,params->rssi,adv,EOL);
    } else {
        snprintf(message,MAX_BUFF_LEN,"OMRON-E MAC = %s , RSSI = %hhx , ADV = %s%s",macString,params->rssi,adv,EOL);
    }
#else /* ADV_DUMP */
    if (type == 1) {
        snprintf(message,MAX_BUFF_LEN,"OMRON-D MAC = %s , RSSI = %hhx , sequence = %02x , temperature = %02x %02x , humidity = %02x %02x , light = %02x %02x , UV = %02x %02x , pressure = %02x %02x , sound = %02x %02x , accX = %02x %02x , accY = %02x %02x , accZ = %02x %02x , battery = %02x%s",
            macString,
            params->rssi,
            params->advertisingData[7],                              // sequence number
            params->advertisingData[8],params->advertisingData[9],   // temperature
            params->advertisingData[10],params->advertisingData[11], // humidity
            params->advertisingData[12],params->advertisingData[13], // light
            params->advertisingData[14],params->advertisingData[15], // UV
            params->advertisingData[16],params->advertisingData[17], // pressure
            params->advertisingData[18],params->advertisingData[19], // sound
            params->advertisingData[20],params->advertisingData[21], // acceleration x
            params->advertisingData[22],params->advertisingData[23], // acceleration y
            params->advertisingData[24],params->advertisingData[25], // acceleration z
            params->advertisingData[26],                             // voltage
            EOL);
    } else {
        snprintf(message,MAX_BUFF_LEN,"OMRON-E MAC = %s , RSSI = %hhx , sequence = %02x , temperature = %02x %02x , humidity = %02x %02x , light = %02x %02x , UV = %02x %02x , pressure = %02x %02x , sound = %02x %02x , discomfort = %02x %02x , heat = %02x %02x , rfu = %02x %02x , battery = %02x%s",
            macString,
            params->rssi,
            params->advertisingData[7],                              // sequence number
            params->advertisingData[8],params->advertisingData[9],   // temperature
            params->advertisingData[10],params->advertisingData[11], // humidity
            params->advertisingData[12],params->advertisingData[13], // light
            params->advertisingData[14],params->advertisingData[15], // UV
            params->advertisingData[16],params->advertisingData[17], // pressure
            params->advertisingData[18],params->advertisingData[19], // sound
            params->advertisingData[20],params->advertisingData[21], // disconfort index
            params->advertisingData[22],params->advertisingData[23], // heat stroke
            params->advertisingData[24],params->advertisingData[25], // rfu
            params->advertisingData[26],                             // voltage
            EOL);
    }
#endif /* ADV_DUMP */
    logging(message);
    return true;
}
#endif /* OMRON_ENV_SENSOR */

/*
 * BLEのアドバタイズを受信すると呼び出されるコールバック関数
 */
void advertisementCallback(const Gap::AdvertisementCallbackParams_t *params) {
#ifdef IBEACON
    iBeaconCheck(params);
#endif /* IBEACON */
#ifdef EDDYSTONE
    eddyStoneCheck(params);
#endif /* EDDYSTONE */
#ifdef ALTBEACON
    altCheck(params);
#endif /* ALTBEACON */
#ifdef OMRON_ENV_SENSOR
    omronSensorCheck(params);
#endif /* OMRON_ENV_SENSOR */

#ifdef DUMP_ADV_DATA
    for (unsigned index = 0; index < params->advertisingDataLen; index++) {
        printf("%02x ", params->advertisingData[index]);
    }
    printf("\r\n");
#endif /* DUMP_ADV_DATA */
}

void onBleInitError(BLE &ble, ble_error_t error)
{
   /* Initialization error handling should go here */
}

void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        onBleInitError(ble, error);
        return;
    }

    if (ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().setScanParams(1800 /* scan interval */, 1500 /* scan window */);
    ble.gap().startScan(advertisementCallback);
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();

    return 0;
}
