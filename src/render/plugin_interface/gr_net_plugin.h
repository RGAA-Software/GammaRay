//
// Created by RGAA on 21/11/2024.
//

#ifndef GAMMARAY_GR_NET_PLUGIN_H
#define GAMMARAY_GR_NET_PLUGIN_H

#include "gr_plugin_interface.h"
#include "gr_net_plugin_type.h"

namespace tc
{

    class Data;

    class NetSyncInfo {
    public:
        int64_t socket_fd_{0};
        std::string device_id_{};
        std::string stream_id_{};
    };

    // connected client information
    class GrConnectedClientInfo {
    public:
        std::string device_id_;
        // direct mode
        std::string stream_id_;
        // relay mode
        std::string relay_room_id_;
        // device name. like: DESKTOP-N3GIEVQ
        std::string device_name_;
    };

    // local webrtc request info
    enum class GrLocalRtcContentType {
        kDesktop,
        kGameStream,
    };

    class GrLocalRtcRequestInfo {
    public:
        std::string device_id_;
        std::string stream_id_;
        std::string req_ip_;
        std::string sdp_;
        GrLocalRtcContentType content_type_;
    };

    // local webrtc reply info
    class GrLocalRtcReplyInfo {
    public:
        std::string answer_sdp_;
    };

    class GrNetPlugin : public GrPluginInterface {
    public:
        GrNetPlugin();
        ~GrNetPlugin() override;

        // Serialized proto message from Renderer
        // to see format details in tc_message_new/tc_message.proto
        // such as : message VideoFrame { ... }
        // you can send it to any clients
        //                       -> client 1
        // Renderer Messages ->  -> client 2
        //                       -> client 3
        // run_through: send the message even if stream was paused
        virtual void PostProtoMessage(std::shared_ptr<Data> msg, bool run_through);

        // Serialized proto message from Renderer
        // to a specific stream
        virtual bool PostTargetStreamProtoMessage(const std::string& stream_id, std::shared_ptr<Data> msg, bool run_through);

        // Serialized proto message from Renderer
        // to file transfer
        virtual bool PostTargetFileTransferProtoMessage(const std::string& stream_id, std::shared_ptr<Data> msg, bool run_through);

        // messages from remote(client) -> this plugin -> exe processes it
        // client 1 ->
        // client 2 ->  -> Renderer
        // client 3 ->
        void OnClientEventCame(bool is_proto,
                               int64_t socket_fd,
                               const NetPluginType& nt_plugin_type,
                               const NetChannelType& ch_type,
                               std::shared_ptr<Data> msg);

        virtual bool IsOnlyAudioClients();

        virtual int GetConnectedClientsCount();

        virtual void SyncInfo(const NetSyncInfo& info);

        // how many messages in queue but not be sent now
        virtual int64_t GetQueuingMediaMsgCount();

        // how many message in ft queue but not be sent now
        virtual int64_t GetQueuingFtMsgCount();

        virtual bool HasEnoughBufferForQueuingMediaMessages();
        virtual bool HasEnoughBufferForQueuingFtMessages();

        // sent data statistics
        void ReportSentDataSize(int size);

        virtual std::vector<std::shared_ptr<GrConnectedClientInfo>> GetConnectedClientInfo();

        // alloc a new local rtc server
        virtual bool AllocNewLocalRtcInstance(const std::shared_ptr<GrLocalRtcRequestInfo>& info,
                                              std::function<void(const std::shared_ptr<GrLocalRtcReplyInfo>&)>&& callback) {
            return false;
        }

        // message ack
        virtual void OnMessageAck(const std::shared_ptr<NetMessageAck>& ack) {

        }

    protected:
        NetSyncInfo sync_info_{};

    };

}

#endif //GAMMARAY_GR_NET_PLUGIN_H
