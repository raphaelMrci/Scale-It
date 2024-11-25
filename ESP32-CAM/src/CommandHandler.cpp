#include "CommandHandler.hpp"

// Constructor

CommandHandler::CommandHandler(Stream &serialStream)
    : serial(serialStream), numRoutes(0)
{
}

// Register a command and its handler
bool CommandHandler::registerRoute(const String &command,
                                   CommandFunction handler)
{
    if (numRoutes >= MAX_COMMANDS) {
        return false; // Command table full
    }
    routes[numRoutes].command = command;
    routes[numRoutes].handler = handler;
    numRoutes++;
    return true;
}

// Parse and execute an incoming command
void CommandHandler::handleIncomingCommand()
{
    if (serial.available()) {
        String command = serial.readStringUntil('\n');
        command.trim(); // Remove leading/trailing whitespace

        int spaceIndex = command.indexOf(' ');
        String cmd = command.substring(0, spaceIndex);
        String args =
            (spaceIndex != -1) ? command.substring(spaceIndex + 1) : "";

        for (int i = 0; i < numRoutes; i++) {
            if (routes[i].command == cmd) {
                routes[i].handler(args); // Call the corresponding handler
                return;
            }
        }
    }
}

// Send a command with optional arguments
void CommandHandler::sendCommand(const String &command, const String &args)
{
    String fullCommand = command;
    if (args.length() > 0) {
        fullCommand += " " + args;
    }
    serial.println(fullCommand);
}
