/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */
#include "m2mconnectionhandler_stub.h"
#include "m2mconnectionsecurity.h"
#include "common_stub.h"

int m2mconnectionhandler_stub::int_value;
uint16_t m2mconnectionhandler_stub::uint_value;
bool m2mconnectionhandler_stub::bool_value;

void m2mconnectionhandler_stub::clear()
{
    int_value = -1;
    uint_value = 0;
    bool_value = false;
}

M2MConnectionHandler::M2MConnectionHandler(M2MConnectionObserver &observer,
                                                   M2MConnectionSecurity* sec,
                                           M2MInterface::NetworkStack)
:_observer(observer)
{
    if( sec ){
        delete sec;
        sec = NULL;
    }
}

M2MConnectionHandler::~M2MConnectionHandler()
{
}

bool M2MConnectionHandler::bind_connection(const uint16_t)
{
    return m2mconnectionhandler_stub::bool_value;
}

bool M2MConnectionHandler::resolve_server_address(const String&,
                                                  const uint16_t,
                                                  M2MConnectionObserver::ServerType,
                                                  const M2MSecurity* )
{
    return m2mconnectionhandler_stub::bool_value;
}

bool M2MConnectionHandler::start_listening_for_data()
{
    return m2mconnectionhandler_stub::bool_value;
}

bool M2MConnectionHandler::send_data(uint8_t *,
                                     uint16_t ,
                                     sn_nsdl_addr_s *)
{
    return m2mconnectionhandler_stub::bool_value;
}

void M2MConnectionHandler::stop_listening()
{
}

int M2MConnectionHandler::sendToSocket(const unsigned char *, size_t ){
    return m2mconnectionhandler_stub::int_value;
}

int M2MConnectionHandler::receiveFromSocket(unsigned char *buf, size_t len){
    return m2mconnectionhandler_stub::int_value;
}
