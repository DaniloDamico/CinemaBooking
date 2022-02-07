cmake_minimum_required(VERSION 3.17)
project(progettoSistemiOperativi C)

set(CMAKE_C_STANDARD 11)

add_executable(Client clientMain.c clientCommands/clientHandleCommand.h utilities/clientUtilities.h serverCommands/serverSendError.h)
add_executable(Server serverMain.c utilities/serverUtilities.h)