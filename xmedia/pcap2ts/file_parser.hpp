#pragma once

#include <memory>
#include <stack>
#include <string>

#include "packet_process.hpp"
#include "pcap_ng_parser.hpp"

class FileParser {
   public:
    FileParser();
    ~FileParser();

    int parseFile(const std::string& file);
    int dealPcapngContent(const PcapngContent& content);

   private:
    // pcapng
    struct PcapngContext {
        std::stack<PcapngContent> content_stack;
        uint64_t packet_count;
        std::shared_ptr<PacketProcess> pp;
    };
    std::shared_ptr<PcapngContext> _pcapng_ctx;
};