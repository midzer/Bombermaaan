/************************************************************************************

    Copyright (C) 2016 Billy Araujo

    This file is part of Bombermaaan.

    Bombermaaan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bombermaaan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bombermaaan.  If not, see <http://www.gnu.org/licenses/>.

    ************************************************************************************/


/**
 *  \file CNetwork.cpp
 *  \brief Network communication
 */

#include "StdAfx.h"
#include "CNetwork.h"

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

CNetwork::CNetwork()
{

    m_NetworkMode = NETWORKMODE_LOCAL;

    m_Socket = NULL;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

CNetwork::~CNetwork(void)
{


}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

ENetworkMode CNetwork::NetworkMode()
{

    return m_NetworkMode;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

void CNetwork::SetNetworkMode(ENetworkMode NetworkMode)
{

    m_NetworkMode = NetworkMode;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CNetwork::Connect(const char* IpAddressString)
{

    if (SDLNet_Init() == SDL_ERROR)
    {
        theLog.Write("init failed: %s\n", SDLNet_GetError());

        return false;
    }

    if (m_NetworkMode == NETWORKMODE_SERVER)
    {

        IPaddress ip;

        if (SDLNet_ResolveHost(&ip, NULL, 1234) == SDL_ERROR)
        {
            theLog.Write("listen failed: %s\n", SDLNet_GetError());

            return false;
        }

        m_Socket = SDLNet_TCP_Open(&ip);

        if (!m_Socket)
        {
            theLog.Write("open failed: %s\n", SDLNet_GetError());
            
            return false;
        }

        // Wait for the client
        while (1)
        { 

            m_ClientSocket = SDLNet_TCP_Accept(m_Socket);

            if (m_ClientSocket)
                break;

        }

    }
    else if (m_NetworkMode == NETWORKMODE_CLIENT)
    {

        IPaddress ip;

        if (SDLNet_ResolveHost(&ip, IpAddressString, 1234) == SDL_ERROR)
        {
            theLog.Write("connection failed: %s\n", SDLNet_GetError());

            return false;
        }

        m_Socket = SDLNet_TCP_Open(&ip);

        if (!m_Socket)
        {
            theLog.Write("open failed: %s\n", SDLNet_GetError());

            return false;
        }

    }

    return true;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CNetwork::Disconnect()
{

    if (m_NetworkMode != NETWORKMODE_LOCAL)
    {

        SDLNet_TCP_Close(m_Socket);

        if (m_NetworkMode == NETWORKMODE_SERVER)
            SDLNet_TCP_Close(m_ClientSocket);

    }

    return true;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

/**
 *  \return true, if the send was successful
 *
 *  Send packet
 */

bool CNetwork::Send(ESocketType SocketType, const char* buf, int len)
{

    int Sent = 0;

    if (SocketType == SOCKET_SERVER)
        Sent = SDLNet_TCP_Send(m_Socket, buf, len);
    else if (SocketType == SOCKET_CLIENT)
        Sent = SDLNet_TCP_Send(m_ClientSocket, buf, len);

    if (Sent == SDL_ERROR)
    {
        theLog.Write("sent error: %s\n", SDLNet_GetError());
        return false;
    }

    return true;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

/**
 *  \return true, if the send was successful
 *
 *  Receive packet
 */

int CNetwork::Receive(ESocketType SocketType, char* buf, int len)
{

    if (SocketType == SOCKET_SERVER)
        return SDLNet_TCP_Recv(m_Socket, buf, len);
    else if (SocketType == SOCKET_CLIENT)
        return SDLNet_TCP_Recv(m_ClientSocket, buf, len);
    else
        return 0;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CNetwork::SendCommandChunk(const CCommandChunk& CommandChunk)
{

    // Send client command chunk to the server
    this->Send(SOCKET_SERVER, (const char*)&CommandChunk, sizeof(CommandChunk));

    return true;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CNetwork::ReceiveCommandChunk(CCommandChunk& CommandChunk)
{

    // Receive client command chunk 
    int bufsize = sizeof(CommandChunk);
    char* recvBuf = new char[bufsize];
    int Received = 0;

    do {

        Received += this->Receive(SOCKET_CLIENT, &recvBuf[Received], bufsize);

        if (Received == SDL_ERROR)
        {
            theLog.Write("sent error: %s\n", SDLNet_GetError());
            return false;
        }

        bufsize -= Received;

    } while (Received < sizeof(CommandChunk));

    if (Received == sizeof(CommandChunk))
    {
        memcpy((char *)&CommandChunk, recvBuf, sizeof(CommandChunk));
        return true;
    }

    return false;

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CNetwork::SendSnapshot(const CArenaSnapshot& Snapshot)
{

    // Send snapshot to the client
    return this->Send(SOCKET_CLIENT, (const char*)&Snapshot, sizeof(Snapshot));

}

//******************************************************************************************************************************
//******************************************************************************************************************************
//******************************************************************************************************************************

bool CNetwork::ReceiveSnapshot(CArenaSnapshot& Snapshot)
{

    // Receive and apply the arena snapshot from the server
    int bufsize = sizeof(Snapshot);
    char* recvBuf = new char[bufsize];
    int Received = 0;

    do {

        Received += this->Receive(SOCKET_SERVER, &recvBuf[Received], bufsize);

        if (Received == SDL_ERROR)
        {
            theLog.Write("sent error: %s\n", SDLNet_GetError());
            return false;
        }

        bufsize -= Received;

    } while (Received < sizeof(Snapshot));

    if (Received == sizeof(Snapshot))
    {
        memcpy((char *)&Snapshot, recvBuf, sizeof(Snapshot));
        return true;
    }

    return false;

}

