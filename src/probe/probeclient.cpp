
#include <boost/bind.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>
#include "probeclient.hpp"

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <signal.h>
#include <time.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

using namespace proposal;
using namespace std;
using boost::function0;
using boost::mutex;

inline int msleep(unsigned long msec);


ProbeClient::ProbeClient(char* bind_address, char* target_address, int port, bool broadcast, int window_size, int probe_length, int delay) :
  broadcast_(broadcast),
  probe_length_(probe_length),
  listen_port_(port),
  target_port_(port),
  writer_seq_(0),
  reader_seq_(-1),
  expected_seq_(-1),
  window_size_(window_size),
  delay_(delay*1000000),
  etx_(0.0f),
  client_lossrate_(0),
  quit_(false),
  threads_(),
  tracker_(window_size)
{
  int opt_val = 1;
  string addr(bind_address);
  string t_addr(target_address);
  reader_addr_ = inet_addr(bind_address);
   
 if(broadcast_) {
    int loc = addr.find_last_of('.', addr.length()-1);
    addr = addr.substr(0, loc+1);
    addr += "255";
    loc = t_addr.find_last_of('.', t_addr.length()-1);
    t_addr = t_addr.substr(0, loc+1);
    t_addr += "255";
  }
  bind_addr_ = inet_addr((char*)addr.c_str());
  writer_addr_ = inet_addr((char*)t_addr.c_str());

  cerr << "Settings [bind=" << addr << "], [target=" << t_addr << "]" << endl;

  //init the reader socket
  debug("Initialising reader socket...");
  if((reader_sock_fd_ = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0)
    {
      cerr << "Error creating reader socket" << endl;
      throw runtime_error("Cannot create writer socket");
    }
  
  if(broadcast_) {
    debug("Setting reader socket to broadcast...");
    //set reader socket to broadcast
    if(setsockopt(reader_sock_fd_,SOL_SOCKET,SO_BROADCAST,&opt_val,sizeof(opt_val)) < 0) {
      cerr << "setsockopt: " << strerror(errno) << endl;
      throw runtime_error("Cannot set reader socket to broadcast");
    }
  }

//  cerr << "Binding reader socket to: " << reader_addr_ << endl;
  struct sockaddr_in sa; 
 
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = bind_addr_;
  sa.sin_port = htons(listen_port_);
  
  if (bind(reader_sock_fd_,(struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0)
    {
      cerr << "bind(): " << strerror(errno) << endl;
      throw runtime_error("Cannot bind reader socket");
    }

  //init the writer socket
  debug("Initialising writer socket...");
  if((writer_sock_fd_ = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0)
    {
      cerr << "Error creating writer socket" << endl;
      throw runtime_error("Cannot create writer socket");
    }
  if(broadcast_) {
    debug("Setting writer socket to broadcast...");
    //set writer socket to broadcast
    if(setsockopt(writer_sock_fd_,SOL_SOCKET,SO_BROADCAST,&opt_val,sizeof(opt_val)) < 0) {
      cerr << "setsockopt: " << strerror(errno) << endl;
      throw runtime_error("Cannot set writer socket to broadcast");
    }
  }

  //set priority
  opt_val = 0;
  if(setsockopt(writer_sock_fd_,SOL_SOCKET,SO_PRIORITY,&opt_val,sizeof(opt_val)) < 0) {
    cerr << "setsockopt: " << strerror(errno) << endl;
    throw runtime_error("Cannot set writer socket priority");
  }

  //get the start time
  clock_gettime(CLOCK_REALTIME, &start_time_);
  trackerpos = 0;
  for (int i=0; i<81; i++) {
	  newtracker[i] = 0;
  }

  debug("Starting threads");
  threads_.create_thread(function0<void>(boost::bind(&ProbeClient::reader, this)));
  threads_.create_thread(function0<void>(boost::bind(&ProbeClient::writer, this)));
}

void
ProbeClient::run()
{
  string s;
  for(;;)
    {
//      if(!getline(cin, s)) break;
	usleep(100);

    }
}

ProbeClient::~ProbeClient()
{
  debug("Exiting...");
  {
    mutex::scoped_lock lock(mutex);
    quit_ = true;
    wakeup_.notify_all();
  }
  debug("Sending Kill SIG to threads...");
  kill(getpid(), SIGUSR1);
  debug("Waiting for threads to finish...");
  threads_.join_all();
}

void
ProbeClient::reader()
{
  try {
    char buff [probe_length_];
    char recv_addr [INET_ADDRSTRLEN];
    probe_packet packet(0, 0, probe_length_);
    struct sockaddr_in sa_src;
    int sa_len = sizeof(sa_src);
    memset(&sa_src, 0, sizeof sa_src);
    debug("Starting reader listen loop");
    while(!quit_) {
      if(recvfrom(reader_sock_fd_, buff, probe_length_, 0, (sockaddr*)&sa_src, (socklen_t*)&sa_len) < 0) {
	//if(recv(reader_sock_fd_, buff, PROBE_LENGTH, 0) < 0) {
	cerr << "Error reading from reader socket: " << strerror(errno) << endl;
	continue; //try again on error
      };
      inet_ntop(AF_INET, &(sa_src.sin_addr), recv_addr, INET_ADDRSTRLEN);

      if(reader_addr_ == sa_src.sin_addr.s_addr) {//check the probe isn't from us
	continue;
      }
      packet.unmarshal((char *)buff, probe_length_);
      //lock the shared structures

      newtracker[trackerpos] ++;

      {
	mutex::scoped_lock lock(mutex_);
	if (reader_seq_ + 1 != packet.seq_ && (reader_seq_ != -1)) {
		std::cerr << "Confirmed Lost Packet in Client " << reader_seq_ << "+1 != " << packet.seq_ << "\n";
	}
	reader_seq_ = packet.seq_;
//	expected_seq_ = reader_seq_+1;
	client_lossrate_ = packet.lossrate_;
      }
    }
  } catch(const exception& x) {
    cerr << "Funcion: " << __PRETTY_FUNCTION__ << " File: " << __FILE__ << " Line: " << __LINE__ << " Exception: " << x.what() << endl;
  } catch(...) {
    cerr << "Funcion: " << __PRETTY_FUNCTION__ << " File: " << __FILE__ << " Line: " << __LINE__ << endl;
  }
}

timespec diff(const timespec &start, const timespec &end)
{
        timespec temp;
        if ((end.tv_nsec-start.tv_nsec)<0) {
                temp.tv_sec = end.tv_sec-start.tv_sec-1;
                temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        } else {
                temp.tv_sec = end.tv_sec-start.tv_sec;
                temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp;
}

void
ProbeClient::writer()
{
  timespec curr_time;
  int64_t deltacorrection = 0;
  int lastlossrate = 0;
  clock_gettime(CLOCK_REALTIME, &curr_time);
  
  try {
    srand(time(NULL));
    debug("Setup target address...");
    struct sockaddr_in sa_dest; 
    probe_packet probe = probe_packet(0, 0, probe_length_);
    char buff[probe_length_];
		
		
    memset(&sa_dest, 0, sizeof sa_dest);
    sa_dest.sin_family=AF_INET;
    sa_dest.sin_addr.s_addr = writer_addr_;
    sa_dest.sin_port = htons(target_port_);

    cout << "Time,ETX,dr,tdf,nlost,twriter_seq,reader_seq" << endl;
    debug("Starting writer probe loop");
    for(;;) {
	{
	timespec nexttime;
	clock_gettime(CLOCK_REALTIME, &nexttime);

	newtracker[ (trackerpos + 1) % 81 ] = 0;
	trackerpos = (trackerpos + 1) % 81;

	timespec diff_nsec = diff(curr_time, nexttime);
	int64_t nanosec = diff_nsec.tv_nsec + (diff_nsec.tv_sec * 1E9);
	if (nanosec > delay_ - deltacorrection) {// nexttime >= curr_time + delay_) {
		mutex::scoped_lock lock(mutex_); //lock the shared structures
                /* sum all trackerpos */
                int trackersum = 0;
                for (int i=0; i<81; i++) {
                        trackersum += newtracker[i];
                }
                int newlossrate = 10 - trackersum;
		if (lastlossrate != newlossrate) {
		if (newlossrate < 0) {
			std::cerr << "Received " << trackersum << " (>10)" << std::endl;
		} else if (newlossrate > 0) {
//			std::cerr << "Received " << trackersum << " (<10)" << std::endl;
		}}
		lastlossrate = newlossrate;
		newlossrate = newlossrate < 0 ? 0 : newlossrate; // set to zero if -ve.

		//generate the new probe
		probe.seq_ = writer_seq_++;
		probe.lossrate_ = newlossrate; //tracker_.lossrate_;
		probe.marshal(buff, probe_length_);
		if (sendto(writer_sock_fd_, buff, probe_length_, 0, (struct sockaddr *)&sa_dest, sizeof(struct sockaddr)) < 0)
		  cerr << "writer: send(): " << strerror(errno) << endl;
	
		//calculate ETX
		etx_ = 1.0f / ((1.0f-(float)newlossrate/(float)window_size_) * ((1.0f-(float)client_lossrate_/(float)window_size_)));

		curr_time = nexttime;
		cout << (curr_time.tv_sec-start_time_.tv_sec) << "," << etx_ << "," << client_lossrate_ 
			<< "\tlr=" << newlossrate << "\tws=" << writer_seq_ << "\trs=" << reader_seq_ 
			<< "@sum" << trackersum << "|pos" << trackerpos << "=delta" << diff_nsec.tv_sec << ":" << diff_nsec.tv_nsec << "+-" << deltacorrection << endl;
		deltacorrection = nanosec - delay_;
		nanosec = 0L;
	}

	if (reader_seq_ == -1) { // do nothing
	} else if (expected_seq_ == -1 && reader_seq_ >= 0) {
		msleep(500-125);
		clock_gettime(CLOCK_REALTIME, &curr_time);
		expected_seq_ = reader_seq_ + 1;
		std::cerr << "Offsetting time delay" << std::endl;
	} else {
//		std::cerr << "Normal state" << std::endl;
	}
      
      int64_t nanosecleft = delay_ - deltacorrection - nanosec;
//	std::cout << nanosecleft << "nsl\n";
      if (nanosecleft > 125000L)
      	msleep(125);
      else
	msleep(nanosecleft / 1000L);

      if(quit_) {
	break;
      }
    } // end scope
    }
  } catch(const exception& x) {
    cerr << "Funcion: " << __PRETTY_FUNCTION__ << " File: " << __FILE__ << " Line: " << __LINE__ << " Exception: " << x.what() << endl;
  } catch(...) {
    cerr << "Funcion: " << __PRETTY_FUNCTION__ << " File: " << __FILE__ << " Line: " << __LINE__ << endl;
  }
}

inline int msleep(unsigned long msec) {
//cout << "sleeping ... " << msec << "\n";
	struct timespec req={0};  
	time_t sec=(int)(msec/1000);  
	msec=msec-(sec*1000);  
	req.tv_sec=sec;  
	req.tv_nsec=msec*1000000L;  
	while(nanosleep(&req,&req)==-1)  
		continue;  
	return 1;  
}

inline void
ProbeClient::debug(std::string str)
{
#ifdef DEBUG
  cerr << str << endl;
#endif
}

void
ProbeClient::sleep(int seconds)
{
  boost::xtime xt;
  boost::xtime_get(&xt, boost::TIME_UTC);
  xt.sec += seconds; // change xt to next second
  boost::thread::sleep(xt);
}

ProbeClient::probe_packet::probe_packet(int seq, int lossrate, int probe_length) :
  seq_(seq),
  lossrate_(lossrate),
  probe_length_(probe_length)
{

}

ProbeClient::probe_packet::probe_packet(char* buff, int len, int probe_length) :
  probe_length_(probe_length)
{
  if(len < probe_length) {
    throw out_of_range("buff too small for probe_packet");
  } else {
    memcpy((void *)buff, &seq_, probe_length);		
  }
	
}

ProbeClient::probe_packet::probe_packet(const probe_packet &packet) :
  seq_(packet.seq_),
  lossrate_(packet.lossrate_)
{
	
}

ProbeClient::probe_packet::~probe_packet()
{

}
	
/// Packs this probe into the buffer
void
ProbeClient::probe_packet::marshal(char* buff, int len)
{
  if(len < probe_length_) {
    throw out_of_range("buff too small for probe_packet");
  } else {
    memcpy(buff, &seq_, probe_length_);
  }
}

/// Unpacks the buffer's data into this probe
void
ProbeClient::probe_packet::unmarshal(char* buff, int len)
{
  if(len < probe_length_) {
    throw out_of_range("buff too small for probe_packet");
  } else {
    memcpy(&seq_, buff, probe_length_);
  }
	
}

string
ProbeClient::probe_packet::to_string()
{
  stringstream s;
  s << "Seq: " << seq_ << " Lossrate: " << lossrate_; //TODO: switch this to sprintf_s
  return s.str();
}

ProbeClient::link_quality::link_quality(int window_size) :
  lossrate_(window_size),
  window_(new bool[window_size]),
  window_loc_(0),
  window_size_(window_size)
{
  for(int i=0; i<window_size_; i++) window_[i] = false;
}

ProbeClient::link_quality::~link_quality()
{
  delete[] window_;
}

void
ProbeClient::link_quality::push(bool received)
{
  if(!window_[window_loc_]) 
    lossrate_--; //pop off the queue, if a loss, remove it from the loss count
  if(!received) 
    lossrate_++; //new probe, if a loss, add it to loss count
  window_[window_loc_] = received; //enqueue new probe
  ++window_loc_ %= window_size_; //wrap window loc around
}
