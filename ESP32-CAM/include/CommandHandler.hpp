#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <Arduino.h>

// Define the maximum number of commands
#define MAX_COMMANDS 10

// Define a type for command handler functions
typedef void (*CommandFunction)(const String &);

class CommandHandler
{
  private:
    struct CommandRoute {
        String command;
        CommandFunction handler;
    };

    CommandRoute routes[MAX_COMMANDS];
    int numRoutes;

    Stream &serial;

  public:
    CommandHandler(Stream &serialStream);

    // Register a command and its handler
    bool registerRoute(const String &command, CommandFunction handler);

    // Parse and execute an incoming command
    void handleIncomingCommand();

    // Send a command with optional arguments
    void sendCommand(const String &command, const String &args = "");
};

#endif
