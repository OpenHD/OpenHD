//
// Created by consti10 on 31.12.21.
//

#ifndef WIFIBROADCAST_PACKETIZEDBENCHMARK_H
#define WIFIBROADCAST_PACKETIZEDBENCHMARK_H

// Wrapper for counting the packet throughput in packets per second and MBit/s.
// Print the current in 1 second intervals and the total at the end.
class PacketizedBenchmark{
public:
    /**
     * @param name1 what we are benchmarking here (for example FEC, encryption)
     * @param factor1 use a factor other than 1.0 if the packet size changes during the benchmarked step
     */
    PacketizedBenchmark(std::string name1,double factor1=1.0f):name(name1),factor(factor1){};
    void begin(){
        testBegin=std::chrono::steady_clock::now();
        logTs=std::chrono::steady_clock::now();
        int packetsDelta=0;
        int totalPacketsDelta=0;
        bytesDelta=0;
        totalBytesDelta=0;
    }
    void doneWithPacket(const double packetSizeBytes){
        packetsDelta++;
        totalPacketsDelta++;
        bytesDelta+=packetSizeBytes;
        totalBytesDelta+=packetSizeBytes;
        const auto delta=std::chrono::steady_clock::now()-logTs;
        if(delta>std::chrono::seconds(1)){
            const float currPacketsPerSecond=packetsDelta;
            const float currBitRate_MBits=bytesDelta*8.0/1024.0/1024.0;
            std::cout<<"curr. Packets per second:"<<currPacketsPerSecond<<" before "<<name<<": "<<currBitRate_MBits<<"Mbit/s";
            if(factor!=1.0f){
                std::cout<<" after "<<name<<": "<<currBitRate_MBits*factor<<"MBit/s";
            }
            std::cout<<"\n";
            logTs=std::chrono::steady_clock::now();
            packetsDelta=0;
            bytesDelta=0;
        }
    }
    void end(){
        const auto testDuration=std::chrono::steady_clock::now()-testBegin;
        const float testDurationSeconds=std::chrono::duration_cast<std::chrono::milliseconds>(testDuration).count()/1000.0f;
        //std::cout<<"Wanted duration:"<<options.benchmarkTimeSeconds<<" actual duration:"<<testDurationSeconds<<"\n";

        double totalPacketsPerSecond=totalPacketsDelta/(double)testDurationSeconds;
        double totalBitRate_MBits=totalBytesDelta*8.0/1024.0/1024.0/(double)testDurationSeconds;
        std::cout<<"Testing "<<name<<" took "<<testDurationSeconds<<" seconds"<<"\n";
        std::cout<<"TOTAL Packets per second:"<<totalPacketsPerSecond<<" before "<<name<<": "<<totalBitRate_MBits<<"Mbit/s";
        if(factor!=1.0f){
            std::cout<<" after "<<name<<": "<<totalBitRate_MBits*factor<<"MBit/s";
        }
        std::cout<<"\n";
    }
private:
    std::chrono::steady_clock::time_point testBegin=std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point logTs=std::chrono::steady_clock::now();
    int packetsDelta=0;
    int totalPacketsDelta=0;
    //
    double bytesDelta=0;
    double totalBytesDelta=0;
    const std::string name;
    const double factor=1.0f;
};


#endif //WIFIBROADCAST_PACKETIZEDBENCHMARK_H
