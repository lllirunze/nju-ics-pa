typedef uint32_t socklen_t;
typedef unsigned short int sa_family_t;

typedef uint32_t in_addr_t;
struct in_addr { in_addr_t s_addr; };

typedef uint16_t in_port_t;

struct sockaddr_in {
  sa_family_t sin_family;
  in_port_t sin_port;
  struct in_addr sin_addr;
};

struct sockaddr {
  sa_family_t sa_family;
  char sa_data[14];
};

typedef unsigned long int nfds_t;

struct hostent {
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
};

#define h_errno 0

static inline const char *hstrerror(int err) {
  (void)err;
  return "Unknown host";
}

static inline struct hostent *gethostbyname(const char *name) {
  (void)name;
  return 0;
}
