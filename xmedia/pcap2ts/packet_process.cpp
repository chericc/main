#include "packet_process.hpp"

#include <inttypes.h>

#include "xlog.hpp"
#include "packet.hpp"
#include "comm.hpp"

static void debug_packet(std::shared_ptr<SharedPacket> packet)
{
    xlog_dbg("PacketType: %s", PacketInfo::typeName(packet->cur_type));
    xlog_dbg("PacketData: [%zu,%zu] in [%zu]", packet->data.offset, packet->data.size, packet->data.data->size());
    xlog_dbg("PacketInfo: ");
    for (auto it : packet->parsed_info)
    {
        switch(it.first)
        {
            case PacketType::Ethernet:
            {
                std::shared_ptr<EthernetPacketInfo> ep = std::static_pointer_cast<EthernetPacketInfo>(it.second);
                xlog_dbg("Ethernet.dst_mac: %s", str_macaddr(ep->mac_dst).c_str());
                xlog_dbg("Ethernet.src_mac: %s", str_macaddr(ep->mac_src).c_str());
                xlog_dbg("Ethernet.sub_type: %s", PacketInfo::typeName(ep->convertEthType(ep->type)));
                break;
            }
            case PacketType::IPv4:
            {
                std::shared_ptr<IPv4PacketInfo> ipv4p = std::static_pointer_cast<IPv4PacketInfo>(it.second);
                xlog_dbg("ipv4->version: %" PRIu8, ipv4p->version);
                xlog_dbg("ipv4->len: %" PRIu8, ipv4p->len);
                xlog_dbg("ipv4->differentialted_service_field: %" PRIu8, ipv4p->differentialted_services_field);
                xlog_dbg("ipv4->total_length: %" PRIu16, ipv4p->total_length);
                xlog_dbg("ipv4->identification: %#" PRIx16, ipv4p->identification);
                xlog_dbg("ipv4->flag: %" PRIu16, ipv4p->flag);
                xlog_dbg("ipv4->fragment_offset: %" PRIu16, ipv4p->fragment_offset);
                xlog_dbg("ipv4->time_to_live: %" PRIu8, ipv4p->time_to_live);
                xlog_dbg("ipv4->protocol: %s", it.second->typeName(ipv4p->convertProtocol(ipv4p->protocol)));
                xlog_dbg("ipv4->header_checksum: %#" PRIx16, ipv4p->header_checksum);
                xlog_dbg("ipv4->ip_src: %s", str_ipv4(ipv4p->ip_addr_src).c_str());
                xlog_dbg("ipv4->ip_dst: %s", str_ipv4(ipv4p->ip_addr_dst).c_str());
                xlog_dbg("ipv4->padding_size: %zu", ipv4p->padding_size);
                break;
            }
            case PacketType::UDP:
            {
                std::shared_ptr<UDPPacketInfo> udp = std::static_pointer_cast<UDPPacketInfo>(it.second);
                xlog_dbg("udp->port_src: %" PRIu16, udp->port_src);
                xlog_dbg("udp->port_dst: %" PRIu16, udp->port_dst);
                xlog_dbg("udp->total_length: %" PRIu16, udp->total_length);
                xlog_dbg("udp->checksum: %" PRIx16, udp->checksum);
                break;
            }
            case PacketType::RTP:
            {
                std::shared_ptr<RTPPacketInfo> rtp = std::static_pointer_cast<RTPPacketInfo>(it.second);
                xlog_dbg("rtp->v: %" PRIu8, rtp->v);
                xlog_dbg("rtp->p: %" PRIu8, rtp->p);
                xlog_dbg("rtp->x: %" PRIu8, rtp->x);
                xlog_dbg("rtp->cc: %" PRIu8, rtp->cc);
                xlog_dbg("rtp->m: %" PRIu8, rtp->m);
                xlog_dbg("rtp->pt: %" PRIu8 "(%s)", rtp->pt, rtp->typeName(rtp->rtpPayloadConvert(rtp->pt)));
                xlog_dbg("rtp->sequence_number: %" PRIu16, rtp->sequence_number);
                xlog_dbg("rtp->time_stamp: %" PRIu32, rtp->time_stamp);
                xlog_dbg("rtp->ssrc_id: %" PRIu32, rtp->ssrc_id);
                xlog_dbg("rtp->csrc_id_list: %zu", rtp->csrc_id_list.size());
                for (auto const& it : rtp->csrc_id_list)
                {
                    xlog_dbg("rtp->csrc_id_list: %" PRIu32, it);
                }
                if (rtp->has_extended_header)
                {
                    xlog_dbg("rtp->ext_defined_by_profile: %" PRIu32, rtp->defined_by_profile);
                    xlog_dbg("rtp->ext_length: %" PRIu32, rtp->length);
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

PacketProcess::PacketProcess()
{

}

PacketProcess::~PacketProcess()
{

}

int PacketProcess::processEthernetData(std::shared_ptr<std::vector<uint8_t>> data)
{
    bool error = false;

    do 
    {
        if (!data)
        {
            break;
        }

        xlog_dbg("packet.size(%zu)", data->size());

        std::shared_ptr<SharedPacket> shared_packet = std::make_shared<SharedPacket>
            (PacketType::Ethernet, SharedData{data, 0, data->size()});

        bool end_flag = false;
        for ( ; !error && !end_flag; )
        {
            switch (shared_packet->cur_type)
            {
                case PacketType::Ethernet:
                {
                    if (processEthernetPacket(shared_packet) < 0)
                    {
                        xlog_err("Process ethernet packet failed");
                        error = true;
                    }
                    break;
                }
                case PacketType::IPv4:
                {
                    if (processIPv4Packet(shared_packet) < 0)
                    {
                        xlog_err("Process ipv4 packet failed");
                        error = true;
                    }
                    break;
                }
                case PacketType::UDP:
                {
                    if (processUDPPacket(shared_packet) < 0)
                    {
                        xlog_err("Process udp packet failed");
                        error = true;
                    }
                    break;
                }
                case PacketType::UDPData:
                default:
                {
                    debug_packet(shared_packet);
                    end_flag = true;
                    break;
                }
            }
        }
    }
    while (0);

    return (error ? -1 : 0);
}

int PacketProcess::processEthernetPacket(std::shared_ptr<SharedPacket> packet)
{
    bool error = false;
    do 
    {
        if (!packet)
        {
            xlog_err("Null packet");
            error = true;
            break;
        }

        SharedData& data = packet->data;
        std::shared_ptr<EthernetPacketInfo> eth = std::make_shared<EthernetPacketInfo>();

        if (!data.valid())
        {
            xlog_err("Invalid data");
            error = true;
            break;
        }

        uint8_t *pdata = data.offsetData();
        std::size_t size = data.offsetSize();

        std::size_t ethernet_header_size = eth->mac_dst.size()
            + eth->mac_src.size()
            + sizeof(eth->type);

        if (size < ethernet_header_size)
        {
            xlog_err("Size error");
            error = true;
            break;
        }

        // mac_dst
        net_copy(eth->mac_dst.data(), eth->mac_dst.size(),
            pdata, size);
        
        // mac_src
        pdata += eth->mac_dst.size();
        size -= eth->mac_dst.size();
        net_copy(eth->mac_src.data(), eth->mac_src.size(),
            pdata, size);

        // type
        pdata += eth->mac_src.size();
        size -= eth->mac_src.size();
        eth->type = net_u16(pdata, size);

        // copy
        SharedData data_tmp = data;
        data_tmp.offset += ethernet_header_size;
        data_tmp.size -= ethernet_header_size;

        // integer limit check
        if (data_tmp.offset < data.offset
            || data_tmp.size > data.size)
        {
            xlog_err("Inner error");
            error = true;
            break;
        }

        data = data_tmp;
        packet->parsed_info.push_back(std::make_pair(PacketType::Ethernet, eth));
        packet->cur_type = EthernetPacketInfo::convertEthType(eth->type);
    }
    while (0);

    return (error ? -1 : 0);
}

int PacketProcess::processIPv4Packet(std::shared_ptr<SharedPacket> packet)
{
    bool error = false;

    do 
    {
        std::shared_ptr<IPv4PacketInfo> ipv4 = std::make_shared<IPv4PacketInfo>();

        if (!packet)
        {
            xlog_err("Null packet");
            error = true;
            break;
        }

        SharedData &data = packet->data;

        uint8_t *pdata = data.offsetData();
        std::size_t size = data.offsetSize();

        const std::size_t head_size_without_options = sizeof(ipv4->version_and_len)
            + sizeof(ipv4->differentialted_services_field) + sizeof(ipv4->total_length) + sizeof(ipv4->identification)
            + sizeof(ipv4->flag_and_fragment_offset) + sizeof(ipv4->time_to_live)
            + sizeof(ipv4->protocol) + sizeof(ipv4->header_checksum)
            + ipv4->ip_addr_src.size() + ipv4->ip_addr_dst.size();

        if (size < head_size_without_options)
        {
            xlog_err("Size check failed(%zu < %zu)", size, head_size_without_options);
            error = true;
            break;
        }

        static_assert(sizeof(ipv4->version_and_len) == 1, "");
        ipv4->version_and_len = net_u8(pdata, size);
        pdata += sizeof(ipv4->version_and_len);
        size -= sizeof(ipv4->version_and_len);

        ipv4->version = ipv4->ipv4_version(ipv4->version_and_len);
        ipv4->len = ipv4->ipv4_len(ipv4->version_and_len);

        if (ipv4->version != 4) // version = 4 --> IPv4
        {
            xlog_err("Verson check failed(%" PRIu8 ")", ipv4->version);
            error = true;
            break;
        }

        if (ipv4->len < head_size_without_options)
        {
            xlog_err("Length check failed(%" PRIu8 ", %zu)", ipv4->len, head_size_without_options);
            error = true;
            break;
        }

        if (size < ipv4->len)
        {
            xlog_err("Size check failed(%zu < %" PRIu8 ")", size, ipv4->len);
            error = true;
            break;
        }
        
        std::size_t option_len = ipv4->len - head_size_without_options;

        static_assert(sizeof(ipv4->differentialted_services_field) == 1, "");
        ipv4->differentialted_services_field = net_u8(pdata, size);
        pdata += sizeof(ipv4->differentialted_services_field);
        size -= sizeof(ipv4->differentialted_services_field);

        static_assert(sizeof(ipv4->total_length) == 2, "");
        ipv4->total_length = net_u16(pdata, size);
        pdata += sizeof(ipv4->total_length);
        size -= sizeof(ipv4->total_length);

        if (ipv4->total_length > data.offsetSize())
        {
            xlog_err("Total length check failed(%" PRIu16 "> %zu)", ipv4->total_length, data.offsetSize());
            error = true;
            break;
        }

        ipv4->padding_size = data.offsetSize() - ipv4->total_length;

        static_assert(sizeof(ipv4->identification) == 2, "");
        ipv4->identification = net_u16(pdata, size);
        pdata += sizeof(ipv4->identification);
        size -= sizeof(ipv4->identification);

        static_assert(sizeof(ipv4->flag_and_fragment_offset) == 2, "");
        ipv4->flag_and_fragment_offset = net_u16(pdata, size);
        pdata += sizeof(ipv4->flag_and_fragment_offset);
        size -= sizeof(ipv4->flag_and_fragment_offset);

        ipv4->flag = ipv4->ipv4_flag(ipv4->flag_and_fragment_offset);
        ipv4->fragment_offset = ipv4->ipv4_fragment_offset(ipv4->flag_and_fragment_offset);

        static_assert(sizeof(ipv4->time_to_live) == 1, "");
        ipv4->time_to_live = net_u8(pdata, size);
        pdata += sizeof(ipv4->time_to_live);
        size -= sizeof(ipv4->time_to_live);

        static_assert(sizeof(ipv4->protocol) == 1, "");
        ipv4->protocol = net_u8(pdata, size);
        pdata += sizeof(ipv4->protocol);
        size -= sizeof(ipv4->protocol);

        static_assert(sizeof(ipv4->header_checksum) == 2, "");
        ipv4->header_checksum = net_u16(pdata, size);
        pdata += sizeof(ipv4->header_checksum);
        size -= sizeof(ipv4->header_checksum);

        net_copy(ipv4->ip_addr_src.data(), ipv4->ip_addr_src.size(), 
            pdata, size);
        pdata += ipv4->ip_addr_src.size();
        size -= ipv4->ip_addr_src.size();

        net_copy(ipv4->ip_addr_dst.data(), ipv4->ip_addr_dst.size(), 
            pdata, size);
        pdata += ipv4->ip_addr_dst.size();
        size -= ipv4->ip_addr_dst.size();

        // skip ipv4 options
        if (option_len > 0)
        {
            pdata += option_len;
            size -= option_len;
        }

        // EthernetPacketData = IPv4Packet + [padding]
        SharedData data_tmp = data;
        data_tmp.offset += ipv4->len;
        data_tmp.size = ipv4->total_length - ipv4->len;
        if (data_tmp.offset < data.offset
            || data_tmp.size > data.size)
        {
            xlog_err("Inner error");
            error = true;
            break;
        }

        // copy
        data = data_tmp;
        packet->parsed_info.push_back(std::make_pair(PacketType::IPv4, ipv4));
        packet->cur_type = ipv4->convertProtocol(ipv4->protocol);
        if (PacketType::None == packet->cur_type)
        {
            packet->cur_type = PacketType::IPv4Data;
        }
    }
    while (0);

    return (error ? -1 : 0);
}

int PacketProcess::processUDPPacket(std::shared_ptr<SharedPacket> packet)
{
    bool error = false;

    do 
    {
        if (!packet)
        {
            xlog_err("Null packet");
            error = true;
            break;
        }

        SharedData &data = packet->data;
        std::shared_ptr<UDPPacketInfo> udp = std::make_shared<UDPPacketInfo>();

        if (!data.valid())
        {
            xlog_err("Invalid data");
            error = true;
            break;
        }

        uint8_t *pdata = data.offsetData();
        std::size_t size = data.offsetSize();

        std::size_t udp_head_size = sizeof(udp->port_src) + sizeof(udp->port_dst)
            + sizeof(udp->total_length) + sizeof(udp->checksum);
        
        if (size < udp_head_size)
        {
            xlog_err("Size check failed(%zu < %zu)", size, udp_head_size);
            error = true;
            break;
        }

        udp->port_src = net_u16(pdata, size);
        pdata += sizeof(udp->port_src);
        size -= sizeof(udp->port_src);

        udp->port_dst = net_u16(pdata, size);
        pdata += sizeof(udp->port_dst);
        size -= sizeof(udp->port_dst);

        udp->total_length = net_u16(pdata, size);
        pdata += sizeof(udp->total_length);
        size -= sizeof(udp->total_length);

        if (udp->total_length < udp_head_size
            || udp->total_length > data.offsetSize())
        {
            xlog_err("Total length check failed(%" PRIu16 ", %zu, %zu)", 
                udp->total_length, udp_head_size, data.offsetSize());
            error = true;
            break;
        }

        udp->checksum = net_u16(pdata, size);
        pdata += sizeof(udp->checksum);
        size -= sizeof(udp->checksum);

        SharedData data_tmp = data;
        data_tmp.offset += udp_head_size;
        data_tmp.size -= udp_head_size;
        if (data_tmp.offset < data.offset
            || data_tmp.size > data.size)
        {
            xlog_err("Inner error");
            error = true;
            break;
        }

        data = data_tmp;
        packet->parsed_info.push_back(std::make_pair(PacketType::UDP, udp));
        packet->cur_type = PacketType::UDPData;
    }
    while (0);

    return (error ? -1 : 0);
}

/*

ref:
https://datatracker.ietf.org/doc/html/rfc3550


RTP header:

V(2bit) | P(1bit) | X(1bit) | CC(4bit) | M(1bit) | PT(7bit) | SeqNum(16bit)
TimeStamp(32bit)
SSRCID(32bit)
CSRCIDLIST(32bit)

*/
int PacketProcess::processRTPPacket(std::shared_ptr<SharedPacket> packet)
{
    bool error = false;

    do 
    {
        if (!packet)
        {
            xlog_err("Null packet");
            error = true;
            break;
        }

        SharedData &data = packet->data;
        std::shared_ptr<RTPPacketInfo> rtp = std::make_shared<RTPPacketInfo>();

        if (!data.valid())
        {
            xlog_err("Invalid data");
            error = true;
            break;
        }

        const uint8_t *pdata = data.offsetData();
        std::size_t size = data.offsetSize();

        const std::size_t fixed_header_size = 12;

        if (size < fixed_header_size)
        {
            xlog_err("Size too small(%zd,%zd)", size, fixed_header_size);
            error = true;
            break;
        }

        uint8_t first_byte = pdata[0];
        rtp->v = (first_byte >> 6) & 0x3;
        rtp->p = (first_byte >> 5) & 0x1;
        rtp->x = (first_byte >> 4) & 0x1;
        rtp->cc = first_byte & 0xf;
        pdata += sizeof(first_byte);
        size -= sizeof(first_byte);

        if (rtp->v != 2)
        {
            xlog_err("Version check failed");
            error = true;
            break;
        }

        uint8_t second_byte = pdata[0];
        rtp->m = (second_byte >> 7) & 0x1;
        rtp->pt = second_byte & 0x7f;
        pdata += sizeof(second_byte);
        size -= sizeof(second_byte);

        rtp->sequence_number = net_u16(pdata, size);
        pdata += sizeof(rtp->sequence_number);
        size -= sizeof(rtp->sequence_number);

        rtp->time_stamp = net_u32(pdata, size);
        pdata += sizeof(rtp->time_stamp);
        size -= sizeof(rtp->time_stamp);

        rtp->ssrc_id = net_u32(pdata, size);
        pdata += sizeof(rtp->ssrc_id);
        size -= sizeof(rtp->ssrc_id);

        const std::size_t csrc_id_list_num = std::min(15u, (unsigned int)rtp->cc);
        const std::size_t scrc_id_list_size = csrc_id_list_num * sizeof(rtp->csrc_id_list.front());
        if (size < scrc_id_list_size)
        {
            xlog_err("csrc_id size check failed(%zu, %zu)", size, scrc_id_list_size);
            error = true;
            break;
        }

        /* 0-15 */
        for (unsigned int i = 0; i < 15 && i < rtp->cc; ++i)
        {
            rtp->csrc_id_list.push_back(net_u32(pdata, size));
            pdata += sizeof(rtp->csrc_id_list);
            size -= sizeof(rtp->csrc_id_list);
        }

        /* Extended header */
        std::size_t extended_header_size = 0;
        if (rtp->x)
        {
            const std::size_t fixed_extended_header_size = sizeof(rtp->defined_by_profile)
                + sizeof(rtp->length);
            if (size < fixed_extended_header_size)
            {
                xlog_err("Extention size check failed(%zu)", size);
                error = true;
                break;
            }

            rtp->defined_by_profile = net_u16(pdata, size);
            pdata += sizeof(rtp->defined_by_profile);
            size -= sizeof(rtp->defined_by_profile);

            rtp->length = net_u16(pdata, size);
            pdata += sizeof(rtp->length);
            size -= sizeof(rtp->length);

            extended_header_size += fixed_extended_header_size;

            if (rtp->length > 0)
            {
                const std::size_t head_extension_size = rtp->length * sizeof(uint32_t);
                if (size < head_extension_size)
                {
                    xlog_err("Extension length check failed(%zu, %zu)", 
                        size, head_extension_size);
                    error = true;
                    break;
                }

                xlog_dbg("Skipping extension data");
                pdata += head_extension_size;
                size -= head_extension_size;
                extended_header_size += head_extension_size;
            }

            rtp->has_extended_header = true;
        }

        // padding size
        if (rtp->p)
        {
            if (size > 0)
            {
                const uint8_t *pdata_last_byte = pdata + size - 1;
                uint8_t padding_total_size = net_u8(pdata_last_byte, 1);
                if (!padding_total_size)
                {
                    xlog_err("Invalid padding size(size: 0)");
                    error = true;
                    break;
                }
                if (padding_total_size > size)
                {
                    xlog_err("Padding size check failed(%" PRIu8 " > %zu)", padding_total_size, size);
                    error = true;
                    break;
                }
                xlog_dbg("Skipping padding size %" PRIu8, padding_total_size);
                size -= padding_total_size;
            }
        }

        // copy
        const std::size_t total_head_size = fixed_header_size + scrc_id_list_size + extended_header_size;

        SharedData data_tmp = data;
        data_tmp.offset += total_head_size;
        data_tmp.size = size;

        // integer limit check
        if (data_tmp.offset < data.offset
            || data_tmp.size > data.size)
        {
            xlog_err("Inner error");
            error = true;
            break;
        }

        packet->parsed_info.push_back(std::make_pair(PacketType::RTP, rtp));
        packet->cur_type = RTPPacketInfo::rtpPayloadConvert(rtp->pt);
        if (PacketType::None == packet->cur_type)
        {
            packet->cur_type = PacketType::RTPData;
        }
    }
    while (0);

    return (error ? -1 : 0);
}