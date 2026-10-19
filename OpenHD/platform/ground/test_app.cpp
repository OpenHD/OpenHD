#include "ground_config.h"
#include <fstream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <windows.h>

namespace {
std::string read_screen(HANDLE output) {
  CONSOLE_SCREEN_BUFFER_INFO info{};
  if (!GetConsoleScreenBufferInfo(output, &info)) return {};
  std::string text;
  for (int y = info.srWindow.Top; y <= info.srWindow.Bottom; ++y) {
    std::string row(info.dwSize.X, ' '); DWORD read = 0;
    ReadConsoleOutputCharacterA(output, row.data(), row.size(), COORD{0, SHORT(y)}, &read);
    text += row.substr(0, read) + '\n';
  }
  return text;
}
void send_key(HANDLE input, WORD virtual_key, char character) {
  INPUT_RECORD event{}; event.EventType = KEY_EVENT;
  event.Event.KeyEvent.bKeyDown = TRUE; event.Event.KeyEvent.wRepeatCount = 1;
  event.Event.KeyEvent.wVirtualKeyCode = virtual_key;
  event.Event.KeyEvent.uChar.AsciiChar = character;
  DWORD written = 0; WriteConsoleInputA(input, &event, 1, &written);
}
void save_attributes(HANDLE output, const std::filesystem::path& path) {
  CONSOLE_SCREEN_BUFFER_INFO info{}; GetConsoleScreenBufferInfo(output, &info);
  std::ofstream file(path, std::ios::binary);
  for (int y = info.srWindow.Top; y <= info.srWindow.Bottom; ++y) {
    std::vector<WORD> row(info.dwSize.X); DWORD read = 0;
    ReadConsoleOutputAttribute(output, row.data(), row.size(), COORD{0, SHORT(y)}, &read);
    file.write(reinterpret_cast<const char*>(row.data()), read * sizeof(WORD));
  }
}
}
int main(int argc, char** argv) {
  if (argc != 3) return 1;
  const auto directory = std::filesystem::absolute(argv[2]);
  const auto profile = directory / "ground.json";
  openhd::ground::Config config;
  config.devices = {"devourer-id-ffff-ffff"};
  openhd::ground::save_config(config, profile);
  const auto command = '"' + std::filesystem::absolute(argv[1]).string() + "\" --config \"" + profile.string() + '"';
  auto mutable_command = command;
  STARTUPINFOA start{}; start.cb = sizeof(start); start.dwFlags = STARTF_USESHOWWINDOW; start.wShowWindow = SW_HIDE;
  PROCESS_INFORMATION child{};
  if (!CreateProcessA(nullptr, mutable_command.data(), nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &start, &child)) return 2;
  HANDLE input = INVALID_HANDLE_VALUE, output = INVALID_HANDLE_VALUE;
  int result = 1;
  std::string reason;
  try {
    FreeConsole();
    for (int i = 0; i < 100 && !AttachConsole(child.dwProcessId); ++i)
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    auto wait_text = [&](const char* expected) {
      for (int i = 0; i < 150; ++i) {
        // Curses creates an alternate screen buffer during initialization.
        // Reopen CONOUT$ so early process attachment never observes the old one.
        const auto current = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (current != INVALID_HANDLE_VALUE) { if (output != INVALID_HANDLE_VALUE) CloseHandle(output); output = current; }
        const auto text = read_screen(output);
        if (text.find(expected) != std::string::npos) return text;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
      }
      { std::ofstream file(directory / "failure-screen.txt"); file << read_screen(output); }
      throw std::runtime_error(std::string("Missing rendered main-app text: ") + expected);
    };
    wait_text("See logs; C settings");
    const auto dashboard = wait_text("Cannot open wifibroadcast interfaces");
    if (dashboard.find("Link Status") == std::string::npos) throw std::runtime_error("No live main dashboard");
    { std::ofstream file(directory / "main-dashboard.txt"); file << dashboard; }
    save_attributes(output, directory / "main-dashboard.attrs");
    send_key(input, 'C', 'c'); const auto settings = wait_text("OpenHD - Ground settings");
    { std::ofstream file(directory / "main-settings.txt"); file << settings; }
    save_attributes(output, directory / "main-settings.attrs");
    send_key(input, VK_ESCAPE, 27); wait_text("[C] Settings");
    send_key(input, 'Q', 'q');
    if (WaitForSingleObject(child.hProcess, 5000) != WAIT_OBJECT_0) throw std::runtime_error("Application did not exit on Q");
    DWORD code = 1; GetExitCodeProcess(child.hProcess, &code);
    if (code != 0) throw std::runtime_error("Application returned an error on normal exit");
    result = 0;
  } catch (const std::exception& error) { reason = error.what(); }
  if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
  if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
  FreeConsole();
  if (WaitForSingleObject(child.hProcess, 0) != WAIT_OBJECT_0) TerminateProcess(child.hProcess, 1);
  CloseHandle(child.hThread); CloseHandle(child.hProcess);
  std::ofstream report(directory / "result.txt");
  report << (result ? "FAIL: " + reason : "PASS: saved profile starts app; USB failure stays in dashboard; C settings; Q clean exit") << '\n';
  return result;
}
