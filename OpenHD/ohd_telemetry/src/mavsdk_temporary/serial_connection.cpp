#include "serial_connection.h"
#include "log.h"

#if defined(APPLE) || defined(LINUX)
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>

#include <utility>
#endif

namespace mavsdk {

#ifndef WINDOWS
#define GET_ERROR() strerror(errno)
#else
#define GET_ERROR() GetLastErrorStdStr()
// Taken from:
// https://coolcowstudio.wordpress.com/2012/10/19/getlasterror-as-stdstring/
std::string GetLastErrorStdStr()
{
    DWORD error = GetLastError();
    if (error) {
        LPVOID lpMsgBuf;
        DWORD bufLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0,
            NULL);
        if (bufLen) {
            LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
            std::string result(lpMsgStr, lpMsgStr + bufLen);

            LocalFree(lpMsgBuf);

            return result;
        }
    }
    return std::string();
}
#endif

SerialConnection::SerialConnection(
    Connection::receiver_callback_t receiver_callback,
    std::string path,
    int baudrate,
    bool flow_control,
    ForwardingOption forwarding_option) :
    Connection(std::move(receiver_callback), forwarding_option),
    _serial_node(std::move(path)),
    _baudrate(baudrate),
    _flow_control(flow_control)
{}

SerialConnection::~SerialConnection()
{
    // If no one explicitly called stop before, we should at least do it.
    stop();
}

ConnectionResult SerialConnection::start()
{
    if (!start_mavlink_receiver()) {
        return ConnectionResult::ConnectionsExhausted;
    }

    ConnectionResult ret = setup_port();
    if (ret != ConnectionResult::Success) {
        return ret;
    }

    start_recv_thread();

    return ConnectionResult::Success;
}

ConnectionResult SerialConnection::setup_port()
{
#if defined(LINUX) || defined(APPLE)
    // open() hangs on macOS or Linux devices(e.g. pocket beagle) unless you give it O_NONBLOCK
    _fd = open(_serial_node.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (_fd == -1) {
        LogErr() << "open failed: " << GET_ERROR();
        return ConnectionResult::ConnectionError;
    }
    // We need to clear the O_NONBLOCK again because we can block while reading
    // as we do it in a separate thread.
    if (fcntl(_fd, F_SETFL, 0) == -1) {
        LogErr() << "fcntl failed: " << GET_ERROR();
        return ConnectionResult::ConnectionError;
    }
#elif defined(WINDOWS)
    // Required for COM ports > 9.
    const auto full_serial_path = "\\\\.\\" + _serial_node;

    _handle = CreateFile(
        full_serial_path.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0, // exclusive-access
        NULL, //  default security attributes
        OPEN_EXISTING,
        0, //  not overlapped I/O
        NULL); //  hTemplate must be NULL for comm devices

    if (_handle == INVALID_HANDLE_VALUE) {
        LogErr() << "CreateFile failed with: " << GET_ERROR();
        return ConnectionResult::ConnectionError;
    }
#endif

#if defined(LINUX) || defined(APPLE)
    struct termios tc;
    bzero(&tc, sizeof(tc));

    if (tcgetattr(_fd, &tc) != 0) {
        LogErr() << "tcgetattr failed: " << GET_ERROR();
        close(_fd);
        return ConnectionResult::ConnectionError;
    }
#endif

#if defined(LINUX) || defined(APPLE)
    tc.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
    tc.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OPOST);
    tc.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG | TOSTOP);
    tc.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
    tc.c_cflag |= CS8;

    tc.c_cc[VMIN] = 0; // We are ok with 0 bytes.
    tc.c_cc[VTIME] = 10; // Timeout after 1 second.

    if (_flow_control) {
        tc.c_cflag |= CRTSCTS;
    }
#endif

#if defined(LINUX) || defined(APPLE)
    tc.c_cflag |= CLOCAL; // Without this a write() blocks indefinitely.

#if defined(LINUX)
    const int baudrate_or_define = define_from_baudrate(_baudrate);
#elif defined(APPLE)
    const int baudrate_or_define = _baudrate;
#endif

    if (baudrate_or_define == -1) {
        return ConnectionResult::BaudrateUnknown;
    }

    if (cfsetispeed(&tc, baudrate_or_define) != 0) {
        LogErr() << "cfsetispeed failed: " << GET_ERROR();
        close(_fd);
        return ConnectionResult::ConnectionError;
    }

    if (cfsetospeed(&tc, baudrate_or_define) != 0) {
        LogErr() << "cfsetospeed failed: " << GET_ERROR();
        close(_fd);
        return ConnectionResult::ConnectionError;
    }

    if (tcsetattr(_fd, TCSANOW, &tc) != 0) {
        LogErr() << "tcsetattr failed: " << GET_ERROR();
        close(_fd);
        return ConnectionResult::ConnectionError;
    }
#endif

#if defined(WINDOWS)
    DCB dcb;
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(_handle, &dcb)) {
        LogErr() << "GetCommState failed with error: " << GET_ERROR();
        return ConnectionResult::ConnectionError;
    }

    dcb.BaudRate = _baudrate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (_flow_control) {
        dcb.fOutxCtsFlow = TRUE;
        dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
    } else {
        dcb.fDtrControl = DTR_CONTROL_DISABLE;
        dcb.fRtsControl = RTS_CONTROL_DISABLE;
    }
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fBinary = TRUE;
    dcb.fNull = FALSE;
    dcb.fDsrSensitivity = FALSE;

    if (!SetCommState(_handle, &dcb)) {
        LogErr() << "SetCommState failed with error: " << GET_ERROR();
        return ConnectionResult::ConnectionError;
    }

    COMMTIMEOUTS timeout = {0, 0, 0, 0, 0};
    timeout.ReadIntervalTimeout = 1;
    timeout.ReadTotalTimeoutConstant = 1;
    timeout.ReadTotalTimeoutMultiplier = 1;
    timeout.WriteTotalTimeoutConstant = 1;
    timeout.WriteTotalTimeoutMultiplier = 1;
    SetCommTimeouts(_handle, &timeout);

    if (!SetCommTimeouts(_handle, &timeout)) {
        LogErr() << "SetCommTimeouts failed with error: " << GET_ERROR();
        return ConnectionResult::ConnectionError;
    }

#endif

    return ConnectionResult::Success;
}

void SerialConnection::start_recv_thread()
{
    _recv_thread = std::make_unique<std::thread>(&SerialConnection::receive, this);
}

ConnectionResult SerialConnection::stop()
{
    _should_exit = true;

    if (_recv_thread) {
        _recv_thread->join();
        _recv_thread.reset();
    }

#if defined(LINUX) || defined(APPLE)
    close(_fd);
#elif defined(WINDOWS)
    CloseHandle(_handle);
#endif

    // We need to stop this after stopping the receive thread, otherwise
    // it can happen that we interfere with the parsing of a message.
    stop_mavlink_receiver();

    return ConnectionResult::Success;
}

bool SerialConnection::send_message(const mavlink_message_t& message)
{
    if (_serial_node.empty()) {
        LogErr() << "Dev Path unknown";
        return false;
    }

    if (_baudrate == 0) {
        LogErr() << "Baudrate unknown";
        return false;
    }

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t buffer_len = mavlink_msg_to_send_buffer(buffer, &message);

    int send_len;
#if defined(LINUX) || defined(APPLE)
    send_len = static_cast<int>(write(_fd, buffer, buffer_len));
#else
    if (!WriteFile(_handle, buffer, buffer_len, LPDWORD(&send_len), NULL)) {
        LogErr() << "WriteFile failure: " << GET_ERROR();
        return false;
    }
#endif

    if (send_len != buffer_len) {
        LogErr() << "write failure: " << GET_ERROR();
        return false;
    }

    return true;
}

void SerialConnection::receive()
{
    // Enough for MTU 1500 bytes.
    char buffer[2048];

#if defined(LINUX) || defined(APPLE)
    struct pollfd fds[1];
    fds[0].fd = _fd;
    fds[0].events = POLLIN;
#endif

    while (!_should_exit) {
        int recv_len;
#if defined(LINUX) || defined(APPLE)
        int pollrc = poll(fds, 1, 1000);
        if (pollrc == 0 || !(fds[0].revents & POLLIN)) {
            continue;
        } else if (pollrc == -1) {
            LogErr() << "read poll failure: " << GET_ERROR();
        }
        // We enter here if (fds[0].revents & POLLIN) == true
        recv_len = static_cast<int>(read(_fd, buffer, sizeof(buffer)));
        if (recv_len < -1) {
            LogErr() << "read failure: " << GET_ERROR();
        }
#else
        if (!ReadFile(_handle, buffer, sizeof(buffer), LPDWORD(&recv_len), NULL)) {
            LogErr() << "ReadFile failure: " << GET_ERROR();
            continue;
        }
#endif
        if (recv_len > static_cast<int>(sizeof(buffer)) || recv_len == 0) {
            continue;
        }
        _mavlink_receiver->set_new_datagram(buffer, recv_len);
        // Parse all mavlink messages in one data packet. Once exhausted, we'll exit while.
        while (_mavlink_receiver->parse_message()) {
            receive_message(_mavlink_receiver->get_last_message(), this);
        }
    }
}

#if defined(LINUX)
int SerialConnection::define_from_baudrate(int baudrate)
{
    switch (baudrate) {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        case 2500000:
            return B2500000;
        case 3000000:
            return B3000000;
        case 3500000:
            return B3500000;
        case 4000000:
            return B4000000;
        default: {
            LogErr() << "Unknown baudrate";
            return -1;
        }
    }
}
#endif

} // namespace mavsdk
