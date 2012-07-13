'''NitroShare - A simple network file sharing tool.
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.'''

from xmlrpclib import ServerProxy, Fault
from gi.repository import Nautilus, GObject

class NitroShareClient:
    
    # This is the standard port for NitroShare's RPC server
    port = 41722
    
    # Attempts a connection to the server on one of the 10
    # predefined ports.
    def _connect(self):
        for port in range(self.port, self.port + 10):
            rpc = ServerProxy('http://localhost:%d' % port)
            try:
                if rpc.IsRunningNitroShare():
                    return rpc
            except:
                pass
        return False
    
    # Invokes a method on the RPC server, attempting to ensure
    # that everything is in order before the request is made.
    def _invoke_method(self, method, *args):
        rpc = self._connect()
        if not rpc is False:
            return getattr(rpc, method)(*args)
    
    def is_running(self):
        return not self._invoke_method('IsRunningNitroShare') is None
    
    def send_files(self, files):
        return self._invoke_method('SendFiles', files)

class NitroShareMenu(GObject.GObject, Nautilus.MenuProvider):
    
    def __init__(self):
        # Initialize our connection to NitroShare
        self.nitroshare = NitroShareClient()
    
    def send_files(self, menu, files):
        # Get the URIs for each of the files and send them to NitroShare
        filename_list = [x.get_uri() for x in files]
        self.nitroshare.send_files(filename_list)
    
    def get_file_items(self, window, files):
        # If no files were provided, we don't show anything
        if not len(files):
            return
        
        # Determine the caption to display for the menu item
        caption = "Send item%s with NitroShare" % ('' if len(files) == 1 else 's')
        
        # Create the menu item
        item = Nautilus.MenuItem(name="NitroShare::SendFiles",
                                 label=caption,
                                 tip=caption,
                                 sensitive=self.nitroshare.is_running())
        item.connect('activate', self.send_files, files)
        
        return [item]
