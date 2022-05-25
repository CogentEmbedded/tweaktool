--[[!
@file wireshark-tweak.lua

@brief Wireshark dissector for Tweak protocol messages
@details This dissector extracts tweak messages with magic field
         and passes them to protobuf dissector

        To use, make a link to the dissector to one of standard wireshark paths. For example:
        mkdir -p ~/.local/lib/wireshark/plugins
        ln -s $PWD/tlprotobuf/wireshark-tweak.lua ~/.local/lib/wireshark/plugins/

@note Requires tweak.proto to be configured as described at
      @url https://www.wireshark.org/docs/wsug_html_chunked/ChProtobufSearchPaths.html

@copyright (c) 2020-2022 Cogent Embedded, Inc.

]]


-- Create a dissector
local p_tweak = Proto("tweak", "Cogent Tweak Tool");

-- Message and it fields
local f_message = ProtoField.string("tweak.message", "Message")
p_tweak.fields = { f_message }

-- Reference to protobuf dissector
local dis_protobuf = Dissector.get("protobuf")

function p_tweak.dissector(buf, pkt, tree)

        local subtree = tree:add(p_tweak, buf(0))
        subtree:add(f_message, buf(0))

        -- TODO add magic field here and select request / response based on it

        pkt.private["pb_msg_type"] = "message,tweak.tweak_TweakReq"
        dis_protobuf:call(buf(0):tvb(), pkt, tree)
end

-- Register automatic handlers
local tcp_encap_table = DissectorTable.get("tcp.port")
tcp_encap_table:add(7777, p_tweak)
