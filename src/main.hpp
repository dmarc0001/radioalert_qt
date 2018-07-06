#ifndef MAIN_HPP
#define MAIN_HPP

#include <qglobal.h>
#include <csignal>
#include "maindaemon.hpp"

//! Funktionszeiger auf Funktion void f(void)
static std::function< void() > mSignalHahdler;

//! Naja, main halt
int main( int argc, char *argv[] );

//! Signalhandler statisch, füft Funktion im MainObjekt auf
void signalHandler( int signal );

#endif  // MAIN_HPP
