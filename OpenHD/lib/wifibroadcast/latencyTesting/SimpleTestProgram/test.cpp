#include "TimeHelper.hpp"
#include "Helper.hpp"
#include "UDPSender.h"
#include "UDPReceiver.h"
#include "StringHelper.hpp"
#include "SchedulingHelper.hpp"
#include <iostream>
#include <cstring>
#include <atomic>
#include <mutex>
#include <sys/time.h>
#include <sys/resource.h>
#include <assert.h>


class PacketInfoData{
public:
    uint64_t seqNr;
private:
    //std::chrono::steady_clock::time_point timestamp;
    int64_t timestamp;
public:
    std::chrono::steady_clock::time_point getTimestamp()const{
        return std::chrono::time_point<std::chrono::steady_clock,std::chrono::nanoseconds>(std::chrono::nanoseconds(timestamp));
    }
    void writeTimestamp(std::chrono::steady_clock::time_point ts){
        timestamp=std::chrono::duration_cast<std::chrono::nanoseconds>(ts.time_since_epoch()).count();
    }
} __attribute__ ((packed));
//static_assert(sizeof(PacketInfoData)==4+8);

uint64_t currentSequenceNumber=0;

void writeSequenceNumberAndTimestamp(std::vector<uint8_t>& data){
    assert(data.size()>=sizeof(PacketInfoData));
    PacketInfoData* packetInfoData=(PacketInfoData*)data.data();
    packetInfoData->seqNr=currentSequenceNumber;
    packetInfoData->writeTimestamp(std::chrono::steady_clock::now());
}

PacketInfoData getSequenceNumberAndTimestamp(const std::vector<uint8_t>& data){
    assert(data.size()>=sizeof(PacketInfoData));
    PacketInfoData packetInfoData;
    std::memcpy(&packetInfoData,data.data(),sizeof(PacketInfoData));
    return packetInfoData;
}

// Returns true if everyhting except the first couple of bytes (PacketInfoData) match
// first couple of bytes are the PacketInfoData (which is written after creating the packet)
bool compareSentAndReceivedPacket(const std::vector<uint8_t>& sb,const std::vector<uint8_t>& rb){
    if(sb.size()!=rb.size()){
        return false;
    }
    const int result=memcmp (&sb.data()[sizeof(PacketInfoData)],&rb.data()[sizeof(PacketInfoData)],sb.size()-sizeof(PacketInfoData));
    return result==0;
}

struct Options{
    // where data goes out
    int OUTPUT_PORT=6001;
    // Default to localhost (rx and tx are running on the same host)
    std::string OUTPUT_IP="127.0.0.1";
    // where data goes back in
    int INPUT_PORT=6001;
    const std::string INPUT_IP="127.0.0.1";
    // size of each packet
    int PACKET_SIZE=1466;
    // how many packets per second
    int WANTED_PACKETS_PER_SECOND=1024;
    // how long to run
    int WANTED_RUN_TIME_SECONDS=5;
    // n packets to send
    int N_PACKETS=WANTED_PACKETS_PER_SECOND*WANTED_RUN_TIME_SECONDS;
};

// Use this to validate received data (mutex for thread safety)
struct SentDataSave{
    std::vector<std::shared_ptr<std::vector<uint8_t>>> sentPackets;
    std::mutex mMutex;
};
SentDataSave sentDataSave{};
AvgCalculator2 avgUDPProcessingTime{0};
//AvgCalculator avgUDPProcessingTime;
std::uint64_t lastReceivedSequenceNr=0;
const bool COMPARE_RECEIVED_DATA=true;
std::vector<int> lostPacketsSeqNrDiffs;
std::size_t receivedPackets=0;
std::size_t receivedBytes=0;
std::size_t nReceivedPacketsWhereContentDoesntMatch=0;


static void validateReceivedData(const uint8_t* dataP,size_t data_length){
    //std::cout<<"LOL1 got data\n";
    receivedPackets++;
	receivedBytes+=data_length;
    const auto data=std::vector<uint8_t>(dataP,dataP+data_length);
    const auto info=getSequenceNumberAndTimestamp(data);
    const auto latency=std::chrono::steady_clock::now()-info.getTimestamp();
	if(latency>std::chrono::milliseconds(1)){
        std::cout<<"XGot data"<<data_length<<" "<<info.seqNr<<" "<<MyTimeHelper::R(latency)<<"\n";
	}
    //std::cout<<"XGot data"<<data_length<<" "<<info.seqNr<<" "<<MyTimeHelper::R(latency)<<"\n";
    // do not use the first couple of packets, system needs to ramp up first
    //if(info.seqNr>10){
        avgUDPProcessingTime.add(latency);
    //}
    if(lastReceivedSequenceNr!=0){
        const auto delta=info.seqNr-lastReceivedSequenceNr;
        if(delta!=1){
            std::cout<<"Missing a packet though FEC "<<delta<<"\n";
			lostPacketsSeqNrDiffs.push_back(delta);
        }
    }
    lastReceivedSequenceNr=info.seqNr;
    if(COMPARE_RECEIVED_DATA){
       sentDataSave.mMutex.lock();
       if((info.seqNr<sentDataSave.sentPackets.size()) && sentDataSave.sentPackets.at(info.seqNr)!=nullptr){
           const auto originalPacketData=sentDataSave.sentPackets.at(info.seqNr);
           if(!compareSentAndReceivedPacket(*originalPacketData,data)){
                //Also this should never happen !
               std::cout<<"Packets do not match !"<<"\n";
               nReceivedPacketsWhereContentDoesntMatch++;
           }else{
                //std::cout<<"Packets do match"<<"\n";
           }
           // Free memory such that we do not run out of RAM (once received,we do not need the packet a second time)
           //originalPacketData.at(info.seqNr)->reset();
       }else{
            // Should never happen
           std::cerr<<"Got probably invalid seqNr "<<info.seqNr<<" "<<sentDataSave.sentPackets.size()<<"\n";
       }
       sentDataSave.mMutex.unlock();
    }else{
        nReceivedPacketsWhereContentDoesntMatch=-1;
    }
}

static void lol(const uint8_t* dataP,size_t data_length){
    std::cout<<"LOL2 got data\n";
}

static void test_latency(const Options& o){
    SchedulingHelper::setThreadParamsMaxRealtime();
    SchedulingHelper::printCurrentThreadPriority("TEST_MAIN");
    SchedulingHelper::printCurrentThreadSchedulingPolicy("TEST_MAIN");
	
	const std::chrono::nanoseconds TIME_BETWEEN_PACKETS=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1))/o.WANTED_PACKETS_PER_SECOND;
    // start the receiver in its own thread
	// Listening always happens on localhost
    UDPReceiver udpReceiver{o.INPUT_PORT,"LTUdpRec",validateReceivedData,0,false};
    udpReceiver.startReceiving();

    // XXX
    //UDPReceiver udpReceiver2{o.INPUT_PORT,"LTUdpRec2",lol,0,false};
    //udpReceiver2.addGroup=true;
    //udpReceiver2.startReceiving();

    // Wait a bit such that the OS can start the receiver before we start sending data
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    UDPSender udpSender{o.OUTPUT_IP,o.OUTPUT_PORT};
    currentSequenceNumber=0;
    avgUDPProcessingTime.reset();
    sentDataSave.sentPackets.clear();
    //

    const std::chrono::steady_clock::time_point testBegin=std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point firstPacketTimePoint=std::chrono::steady_clock::now();
    std:size_t writtenBytes=0;
    std::size_t writtenPackets=0;
    for(int i=0;i<o.N_PACKETS;i++){
        auto buff=GenericHelper::createRandomDataBuffer2(o.PACKET_SIZE);
        // If enabled,store sent data for later validation
        if(COMPARE_RECEIVED_DATA){
            sentDataSave.mMutex.lock();
            sentDataSave.sentPackets.push_back(buff);
            sentDataSave.mMutex.unlock();
        }
		//write sequence number and timestamp after random data was created
		//(We are not interested in the latency of creating random data,even though it is really fast)
        writeSequenceNumberAndTimestamp(*buff);
        udpSender.mySendTo(buff->data(),buff->size());
        writtenBytes+=buff->size();
        writtenPackets+=1;
        currentSequenceNumber++;
        // wait until as much time is elapsed such that we hit the target packets per seconds
        const auto timePointReadyToSendNextPacket=firstPacketTimePoint+i*TIME_BETWEEN_PACKETS;
        while(std::chrono::steady_clock::now()<timePointReadyToSendNextPacket){
            //uncomment for busy wait (no scheduler interruption)
			std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
    const auto testEnd=std::chrono::steady_clock::now();
    // Wait for any packet that might be still in transit
    std::this_thread::sleep_for(std::chrono::seconds(1));
    udpReceiver.stopReceiving();
    //udpReceiver2.stopReceiving();
    udpSender.logSendtoDelay();

    const double testTimeSeconds=(testEnd-testBegin).count()/1000.0f/1000.0f/1000.0f;
    const double actualPacketsPerSecond=(double)o.N_PACKETS/testTimeSeconds;
    const double actualMBytesPerSecond=(double)writtenBytes/testTimeSeconds/1024.0f/1024;
	const long nLostBytes=(writtenBytes-receivedBytes);
	const long nLostPackets=(writtenPackets-receivedPackets);
    const double lostBytesPercentage=((double)receivedBytes/(double)writtenBytes)*100.0f;


   std::cout<<"Testing took:"<<testTimeSeconds<<"s\n";
   std::cout<<"WANTED_PACKETS_PER_SECOND "<<o.WANTED_PACKETS_PER_SECOND<<" Got "<<actualPacketsPerSecond<<
   "\nBITRATE: "<<actualMBytesPerSecond<<"MB/s"<<" ("<<(actualMBytesPerSecond*8)<<"MBit/s)"<<"\n";
   std::cout<<"N of packets sent | rec | diff ["<<writtenPackets<<" | "<<receivedPackets<<" | "<<nLostPackets<<"]\n";
   //std::cout<<"N of bytes sent | rec | diff | perc lost ["<<writtenBytes<<" | "<<receivedBytes
   //<<" | "<<nLostBytes<<" | "<<lostBytesPercentage<<"]\n";
    std::cout<<"nReceivedPacketsWhereContentDoesntMatch "<<nReceivedPacketsWhereContentDoesntMatch<<"\n";
    std::cout<<"LostPacketsSeqNrDiffs "<<StringHelper::vectorAsString(lostPacketsSeqNrDiffs)<<"\n";
    std::cout<<"------- Latency between (I<=>O) ------- \n";
    std::cout<<avgUDPProcessingTime.getAvgReadable()<<"\n";
   //std::cout<<"All samples "<<avgUDPProcessingTime.getAllSamplesSortedAsString()<<"\n";
   //std::cout<<"Low&high\n"<<avgUDPProcessingTime.getOnePercentLowHigh();
    std::cout<<"Low&high\n"<<avgUDPProcessingTime.getNValuesLowHigh(20);
}


int main(int argc, char *argv[])
{
	// For testing the localhost latency just use the same udp port for input and output
	// Else you have to use different udp ports and run svpcom wfb_tx and rx accordingly
	int opt;

	Options options{};
	// default localhost
	int mode=0;
    while ((opt = getopt(argc, argv, "s:p:t:i:o:")) != -1) {
        switch (opt) {
        case 's':
            options.PACKET_SIZE = atoi(optarg);
            break;
        case 'p':
            options.WANTED_PACKETS_PER_SECOND = atoi(optarg);
            break;
        case 't':
            options.WANTED_RUN_TIME_SECONDS = atoi(optarg);
            break;
        case 'o':
            options.OUTPUT_PORT=atoi(optarg);
            break;
        case 'i':
            options.INPUT_PORT=atoi(optarg);
            break;
        default: /* '?' */
        show_usage:
            std::cout<<"Usage: [-s=packet size in bytes] [-p=packets per second] [-t=time to run in seconds]"
			<<" [-i=input udp port] [-o=output udp port]"<<"\n";
            return 1;
        }
    }
    options.N_PACKETS=options.WANTED_PACKETS_PER_SECOND*options.WANTED_RUN_TIME_SECONDS;
	// Mode test localhost
	/*const Options options0{ps,pps,pps*wantedTime,6001,6001,"127.0.0.1"};
	// Mode test wfb latency, data goes via ethernet to port 6002 on air pi where it is received and transmitted via wb
	// On the ground data is received via wb and forwarded to port 6001
	const Options options1{ps,pps,pps*wantedTime,6001,6002,"192.168.0.14"};
	// for when the tx and rx is on the same pc
	//const Options options2{ps,pps,pps*wantedTime,6100,6000,"127.0.0.1"};
	const Options optionsX{ps,pps,pps*wantedTime,input_port,output_port,"127.0.0.1"};
	//const Options options = (mode==0) ? options0 : options2;
	const Options options=optionsX;*/

    // For a packet size of 1024 bytes, 1024 packets per second equals 1 MB/s or 8 MBit/s
    // 8 MBit/s is a just enough for encoded 720p video
    std::cout<<"Selected packet size"<<options.PACKET_SIZE<<"\n";
    std::cout<<"Selected input: "<<options.INPUT_PORT<<"\n";
    std::cout<<"Selected output: "<<options.OUTPUT_PORT<<" "<<options.OUTPUT_IP<<"\n";
	test_latency(options);


    return 0;
}
