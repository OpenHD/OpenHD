#pragma once

#include <sstream>

/**
 * @brief Namespace for all mavsdk types.
 */
namespace mavsdk {

/**
 * @brief Result type returned when adding a connection.
 *
 * **Note**: Mavsdk does not throw exceptions. Instead a result of this type will be
 * returned when you add a connection: add_udp_connection().
 */
enum class ConnectionResult {
    Success = 0, /**< @brief %Connection succeeded. */
    Timeout, /**< @brief %Connection timed out. */
    SocketError, /**< @brief Socket error. */
    BindError, /**< @brief Bind error. */
    SocketConnectionError, /**< @brief Socket connection error. */
    ConnectionError, /**< @brief %Connection error. */
    NotImplemented, /**< @brief %Connection type not implemented. */
    SystemNotConnected, /**< @brief No system is connected. */
    SystemBusy, /**< @brief %System is busy. */
    CommandDenied, /**< @brief Command is denied. */
    DestinationIpUnknown, /**< @brief %Connection IP is unknown. */
    ConnectionsExhausted, /**< @brief %Connections exhausted. */
    ConnectionUrlInvalid, /**< @brief URL invalid. */
    BaudrateUnknown /**< @brief Baudrate unknown. */
};

/**
 * @brief Stream operator to print information about a `ConnectionResult`.
 *
 * @return A reference to the stream.
 */
std::ostream& operator<<(std::ostream& str, const ConnectionResult& result);

} // namespace mavsdk
