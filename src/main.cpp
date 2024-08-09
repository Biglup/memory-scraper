/**
 * \file main.cpp
 *
 * \author angel.castillo
 * \date   Aug 09, 2024
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* INCLUDES ******************************************************************/

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

/* STATIC FUNCTIONS **********************************************************/

namespace
{
/**
 * \brief Determines if a character is printable.
 *
 * \param c The character to check.
 *
 * \return True if the character is printable, false otherwise.
 */
bool
isPrintable(char c)
{
  return c >= 32 && c <= 126;
}

/**
 * \brief Finds strings in a process.
 *
 * \param pid The process ID.
 */
void
findStringsInProcess(pid_t pid)
{
  if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1)
  {
    perror("ptrace attach");
    return;
  }

  waitpid(pid, NULL, 0);

  std::string memFile = "/proc/" + std::to_string(pid) + "/mem";
  int         memFd   = open(memFile.c_str(), O_RDONLY);

  if (memFd == -1)
  {
    perror("open mem file");
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    return;
  }

  std::string   mapsFile = "/proc/" + std::to_string(pid) + "/maps";
  std::ifstream maps(mapsFile);
  std::string   line;

  while (std::getline(maps, line))
  {
    std::istringstream iss(line);
    std::string        addressRange;
    iss >> addressRange;

    size_t    dashPos = addressRange.find('-');
    uintptr_t start   = std::stoul(addressRange.substr(0, dashPos), nullptr, 16);
    uintptr_t end     = std::stoul(addressRange.substr(dashPos + 1), nullptr, 16);
    uintptr_t size    = end - start;

    std::vector<char> buffer(size);

    if (pread(memFd, buffer.data(), size, start) != -1)
    {
      std::string currentString;
      size_t      currentStringStart = 0;

      for (size_t i = 0; i < size; ++i)
      {
        if (isPrintable(buffer[i]))
        {
          if (currentString.empty())
          {
            currentStringStart = i;
          }
          currentString += buffer[i];
        }
        else
        {
          if (!currentString.empty() && currentString.length() > 3)
          {
            std::cout << "Found string at " << std::hex
                      << (start + currentStringStart) << std::dec << ": "
                      << currentString << std::endl;
          }
          currentString.clear();
        }
      }
    }
  }

  close(memFd);
  ptrace(PTRACE_DETACH, pid, NULL, NULL);
}
} // namespace

/* APP ENTRY POINT ***********************************************************/

/**
 * \brief The application entry point.
 *
 * \return 0 on success, non-zero on failure.
 */
int
main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <pid>" << std::endl;
    return 1;
  }

  pid_t pid = std::stoi(argv[1]);

  findStringsInProcess(pid);

  return 0;
}
