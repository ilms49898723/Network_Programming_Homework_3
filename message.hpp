#ifndef NETWORK_PROGRAMMING_MESSAGE_HPP_
#define NETWORK_PROGRAMMING_MESSAGE_HPP_

#include <string>

const std::string optWELCOME = std::string("[R]Register  [L]Login     [Q]Quit");
const std::string optMAIN = std::string("[SU]Show User   [SF]Show File List  [R]Request File List\n") +
                            std::string("[U]Upload       [D]Download\n") +
                            std::string("[T]Talk         [L]Logout");

const std::string msgREGISTER = "REGISTER";
const std::string msgLOGIN = "LOGIN";
const std::string msgLOGOUT = "LOGOUT";
const std::string msgDELETEACCOUNT = "DELETEACCOUNT";

const std::string msgUpdateConnectInfo = "UPDATECONNECTINFO";

const std::string msgSUCCESS = "Success!";
const std::string msgFAIL = "Failed!";

#endif // NETWORK_PROGRAMMING_MESSAGE_HPP_
