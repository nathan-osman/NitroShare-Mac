/* NitroShare - A simple network file sharing tool.
   Copyright (C) 2012 Nathan Osman

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <qxmlrpc/client.h>

#include "rpc/CRPCServer.h"
#include "util/settings.h"

CRPCServer::CRPCServer()
{
    /* Register the methods we allow. Notice that we use List when
       we expect a list of strings and not StringList. */

    registerMethod("IsRunningNitroShare", QVariant::Bool);
    registerMethod("SendFiles",           QVariant::Bool, QVariant::List);

    connect(this, SIGNAL(incomingRequest(int,QString,QList<xmlrpc::Variant>)),
                  SLOT(OnRequest(int,QString,QList<xmlrpc::Variant>)));
}

bool CRPCServer::Init()
{
    /* The next set of steps involves finding either a running instance
       of NitroShare on the RPC port OR finding an unused port we can
       use for this purpose. */
    quint16 default_port = Settings::Get("RPC/Port").toUInt();

    for(quint16 port = default_port; port < default_port + 10; ++port)
    {
        /* Check this port to see if NitroShare is running on it. This isn't
           as easy as it sounds since we need to essentially create our own
           event loop. We also have a timer so that we can enforce timeouts. */
        QEventLoop event_loop;
        xmlrpc::Client client;

        connect(&client, SIGNAL(done(int,QVariant)),      SLOT(OnFoundInstance()));
        connect(&client, SIGNAL(done(int,QVariant)),      &event_loop, SLOT(quit()));
        connect(&client, SIGNAL(failed(int,int,QString)), &event_loop, SLOT(quit()));

        QTimer::singleShot(200, &event_loop, SLOT(quit()));

        m_found_instance = false;
        client.setHost("localhost", port);
        client.request("IsRunningNitroShare");
        event_loop.exec();

        /* The request has finished. If we found a running instance of
           NitroShare or one of the ports, immediately terminate, returning false. */
        if(m_found_instance)
        {
            qDebug() << "Found a running instance of NitroShare on port" << port;
            return false;
        }

        /* If an instance was not found, make an attempt to listen on this port. */
        if(listen(port, QHostAddress::LocalHost))
        {
            qDebug() << "RPC server bound to port" << port;
            return true;
        }

        /* Otherwise, we need to continue to the next port. */
        qDebug() << "RPC server unable to bind to port" << port;
    }

    /* We didn't find any ports available. */
    qDebug() << "RPC server unable to find any available ports.";
    return false;
}

void CRPCServer::OnFoundInstance()
{
    m_found_instance = true;
}

void CRPCServer::OnRequest(int request_id, QString method_name, QList<xmlrpc::Variant> parameters)
{
    qDebug() << "RPC call to" << method_name;

    if(method_name == "IsRunningNitroShare")
        sendReturnValue(request_id, true);
    else if(method_name == "SendFiles")
    {
        emit SendFiles(parameters.at(0).toStringList(), "");
        sendReturnValue(request_id, true);
    }
}
