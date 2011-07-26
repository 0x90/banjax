
      unsigned long reader_addr_;
      unsigned long writer_addr_;
      unsigned long bind_addr_;

      timespec start_time_;
      int client_lossrate_;
      boost::mutex mutex_;
      boost::condition wakeup_;
      boost::thread_group threads_;
//      link_quality tracker_;
      int newtracker[81];
      int trackerpos;

#if 0

      /**
       * lq_probe is the 
       */
      struct lq_probe {
      public:
         lq_probe(int seq, int lossrate, int probe_length);
         lq_probe(char *buff, int len, int probe_length);
         lq_probe(const lq_probe &);
         ~lq_probe();
         void marshal(char *buf, int buf_sz);
         void unmarshal(const char *buf, int buf_sz);
         std::string to_string();
         int seq_;
         int lossrate_;
         int probe_length_;
      } __attribute__((packed));

      /**
       * 
       */
      class lq_window {
      public:
         lq_window(int window_size);
         ~lq_window();
         uint16_t losses() const;
         void push_back(bool);
      private:
         uint16_t losses_;
         bool *window_;
         int window_loc_;
         int window_size_;
      };

#endif
	




#if 0 

   struct sockaddr_in dst;
   memset(&dst, sizeof(dst), 0);
   dst.sin_family = AF_INET;
   dst.sin_port = htons(port_no);
   dst.sin_addr.s_addr = INADDR_BROADCAST;

   int opt_val = 1;
   string addr(bind_str);
   string t_addr("255.255.255.255");
   reader_addr_ = inet_addr(bind_address);
  
   int loc = addr.find_last_of('.', addr.length()-1);
   addr = addr.substr(0, loc+1);
   addr += "255";
   loc = t_addr.find_last_of('.', t_addr.length()-1);
   t_addr = t_addr.substr(0, loc+1);
   t_addr += "255";

   bind_addr_ = inet_addr((char*)addr.c_str());
   writer_addr_ = inet_addr((char*)t_addr.c_str());

   if((reader_sock_fd_ = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0) {
      cerr << "Error creating reader socket" << endl;
      throw runtime_error("Cannot create writer socket");
   }
  
   if(-1 == setsockopt(reader_sock_fd_,SOL_SOCKET,SO_BROADCAST,&opt_val,sizeof(opt_val))) {
      throw runtime_error("Cannot set reader socket to broadcast");
   }

   struct sockaddr_in sa; 
   sa.sin_family = AF_INET;
   sa.sin_addr.s_addr = bind_addr_;
   sa.sin_port = htons(listen_port_);
   if(bind(reader_sock_fd_,(struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0) {
      throw runtime_error("Cannot bind reader socket");
   }

   // init the writer socket
   if((writer_sock_fd_ = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) < 0) {
      throw runtime_error("Cannot create writer socket");
   }

   // set writer socket to broadcast
   if(setsockopt(writer_sock_fd_,SOL_SOCKET,SO_BROADCAST,&opt_val,sizeof(opt_val)) < 0) {
      throw runtime_error("Cannot set writer socket to broadcast");
   }

   // set priority
   opt_val = 0;
   if(setsockopt(writer_sock_fd_,SOL_SOCKET,SO_PRIORITY,&opt_val,sizeof(opt_val)) < 0) {
      throw runtime_error("Cannot set writer socket priority");
   }


timespec
diff(const timespec &start, const timespec &end)
{
   timespec temp;
   if ((end.tv_nsec-start.tv_nsec)<0) {
      temp.tv_sec = end.tv_sec-start.tv_sec-1;
      temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
   } else {
      temp.tv_sec = end.tv_sec - start.tv_sec;
      temp.tv_nsec = end.tv_nsec-start.tv_nsec;
   }
   return temp;
}

lq_monitor::probe_packet::probe_packet(int seq, int lossrate, int probe_length) :
   seq_(seq),
   lossrate_(lossrate),
   probe_sz_(probe_length)
{

}

lq_monitor::probe_packet::probe_packet(char* buf, int len, int probe_length) :
   probe_sz_(probe_length)
{
   if(len < probe_length) {
      throw out_of_range("buf too small for probe_packet");
   } else {
      memcpy((void *)buf, &seq_, probe_length);		
   }
	
}

lq_monitor::probe_packet::probe_packet(const probe_packet &other) :
   seq_(other.seq_),
   lossrate_(other.lossrate_)
{
}

lq_monitor::probe_packet::~probe_packet()
{

}
	
void
lq_monitor::probe_packet::marshal(char* buf, int len)
{
   if(len < probe_sz_) {
      throw out_of_range("buf too small for probe_packet");
   } else {
      memcpy(buf, &seq_, probe_sz_);
   }
}

void
lq_monitor::probe_packet::unmarshal(const char* buf, int len)
{
   if(len < probe_sz_) {
      throw out_of_range("buf too small for probe_packet");
   } else {
      memcpy(&seq_, buf, probe_sz_);
   }
	
}

lq_monitor::lq_window::lq_window(int window_size) :
   losses_(window_size),
   window_(new bool[window_size]),
   window_loc_(0),
   window_size_(window_size)
{
   fill(&window_[0], &window_[window_size_], false);
}

lq_monitor::lq_window::~lq_window()
{
   delete[] window_;
}

uint16_t
lq_monitor::lq_window::losses() const
{
   return losses_;
}

void
lq_monitor::lq_window::push_back(bool received)
{
   if(!window_[window_loc_]) 
      --losses_;
   if(!received) 
      ++losses_;
   window_[window_loc_] = received;
   ++window_loc_ %= window_size_;
}

#endif
