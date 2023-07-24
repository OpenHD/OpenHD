#include "connection_result.h"

namespace mavsdk {

std::ostream& operator<<(std::ostream& str, const ConnectionResult& result)
{
    switch (result) {
        case ConnectionResult::Success:
            return str << "Success";
        case ConnectionResult::Timeout:
            return str << "Timeout";
        case ConnectionResult::SocketError:
            return str << "Socket error";
        case ConnectionResult::BindError:
            return str << "Bind error";
        case ConnectionResult::SocketConnectionError:
            return str << "Socket connection error";
        case ConnectionResult::ConnectionError:
            return str << "Connection error";
        case ConnectionResult::NotImplemented:
            return str << "Not implemented";
        case ConnectionResult::SystemNotConnected:
            return str << "System not connected";
        case ConnectionResult::SystemBusy:
            return str << "System busy";
        case ConnectionResult::CommandDenied:
            return str << "Command denied";
        case ConnectionResult::DestinationIpUnknown:
            return str << "Destination IP unknown";
        case ConnectionResult::ConnectionsExhausted:
            return str << "Connections exhausted";
        case ConnectionResult::ConnectionUrlInvalid:
            return str << "Invalid connection URL";
        case ConnectionResult::BaudrateUnknown:
            return str << "Baudrate unknown";
        default:
            return str << "Unknown";
    }
}

} // namespace mavsdk
