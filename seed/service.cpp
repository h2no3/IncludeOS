//#define NDEBUG // Debug supression

#include <os>
#include <iostream>

using namespace std;

uint8_t* buf = 0;
int bufsize = 0;
uint8_t* prev_data = 0;


void Service::start()
{
  cout << "*** Service is up - with OS Included! ***" << endl;    
 
  cout << "...Starting UDP server" << endl;

  //IP_stack& net = Dev::eth(0).ip_stack();
  auto& net = Dev::eth(0).ip_stack();
  /** @note: "auto net" would cause copy-construction (!) 
      since auto drops reference, const and volatile qualifiers. */
    
  //A one-way UDP server (a primitive test)
  net.udp_listen(8080,[&net](uint8_t* const data,int len){
      
      UDP::full_header* full_hdr = (UDP::full_header*)data;
      UDP::udp_header* hdr = &full_hdr->udp_hdr;

      int data_len = __builtin_bswap16(hdr->length);
      auto data_loc = data + sizeof(UDP::full_header);
      
      // There seems to be one byte extra given by length. Padding?
      auto data_end = data + hdr->length - sizeof(UDP::udp_header) -1;

      // Don't know if 
      //*data_end = 0; 
      
      debug("<APP SERVER> Got %i b of data (%i b frame) from %s:%i -> %s:%i\n",
            data_len, len, full_hdr->ip_hdr.saddr.str().c_str(), 
            __builtin_bswap16(hdr->sport),
            full_hdr->ip_hdr.daddr.str().c_str(), 
            __builtin_bswap16(hdr->dport));

      printf("%s", data_loc);                  
      
      // Craft response
      string response("You said: '"+string((const char*)data_loc)+"' \n");
      bufsize = response.size() + sizeof(UDP::full_header);
      
      // Ethernet padding if necessary
      if (bufsize < Ethernet::minimum_payload)
        bufsize = Ethernet::minimum_payload;
      
      
      if(buf)
        delete[] buf;
      
      buf = new uint8_t[bufsize]; 
      strcpy((char*)buf + sizeof(UDP::full_header),response.c_str());
      
      
      // Respond
      debug("<APP SERVER> Sending %li b wrapped in %i b buffer \n",
            response.size(),bufsize);
      
      net.udp_send(full_hdr->ip_hdr.daddr, hdr->dport, 
                   full_hdr->ip_hdr.saddr, hdr->sport, buf, bufsize);
      
          
      return 0;
    });
  
  cout << "<APP SERVER> Listening to UDP port 8080 " << endl;
  
  // Hook up to I/O events and do something useful ...
  
  cout << "Service out! " << endl; 
}
