#define DEBUG_SOCKET
#define DEBUG_IP "192.168.1.218"
#define DEBUG_PORT 10023

#include "ps4.h"

static inline uint64_t rdmsr(uint64_t msr) {
    uint32_t low = 0, high = 0;
    
    __asm__ volatile (
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );
    
    return (((uint64_t)high) << 0x20) | ((uint64_t)low);
}

int do_funnykerndump() {
  uint64_t kbaseu64 = get_kernel_base(), kchunksize = 128, koffs = 0, ksiz = 60000000 + kchunksize;
  unsigned char* kbaseptr = (unsigned char*)kbaseu64;
  printf_debug("kbase=%p\n", (void*)kbaseptr);
  printf_notification("Kernel Base = %p", (void*)kbaseptr);
  
  for (koffs = 0; koffs < ksiz; koffs += kchunksize)
    SckSend(DEBUG_SOCK, (char*)&kbaseptr[koffs], (int)kchunksize);
  
  printf_debug("<<<!!!EOF!!!>>>\n", (void*)kbaseptr);
  printf_notification("Kernel Dumped");
  return 0;
}

int _main(struct thread *td) {
  UNUSED(td);

  initKernel();
  initLibc();
  initPthread();

#ifdef DEBUG_SOCKET
  initNetwork();
  DEBUG_SOCK = SckConnect(DEBUG_IP, DEBUG_PORT);
#endif

  jailbreak();

  initSysUtil();

  printf_notification("Running funnykerndump");
  do_funnykerndump();

#ifdef DEBUG_SOCKET
  printf_debug("Closing socket...\n");
  SckClose(DEBUG_SOCK);
#endif

  return 0;
}
