``tee-stats`` is a statistics reporting tool for OP-TEE. It is the client
side of the static Trusted Application `core/arch/arm/pta/stats.c`_ (which
may be enabled by setting CFG_WITH_STATS=y when building OP-TEE).

Currently, it can report information about the TEE kernel main heap as well as
about the TA heap (the pool of secure memory used by the TEE kernel to map
user-mode Trusted Applications). Other statistics may be added later. Example::

  $ tee-stats
  HEAP: heap_size 77896 cur_alloc 632 max_alloc 6096 fail 0 biggest_fail_size 0 biggest_fail_used 0

  $ tee-stats taheap; { xtest 4007 >/dev/null& sleep 2; tee-stats taheap; }; \
    wait; tee-stats -c taheap; tee-stats taheap
  TAHEAP: heap_size 13631488 cur_alloc 0 max_alloc 0 fail 0 biggest_fail_size 0 biggest_fail_used 0
  TAHEAP: heap_size 13631488 cur_alloc 147456 max_alloc 147456 fail 0 biggest_fail_size 0 biggest_fail_used 0
  TAHEAP: heap_size 13631488 cur_alloc 0 max_alloc 147456 fail 0 biggest_fail_size 0 biggest_fail_used 0
  TAHEAP: heap_size 13631488 cur_alloc 0 max_alloc 0 fail 0 biggest_fail_size 0 biggest_fail_used 0


.. _core/arch/arm/pta/stats.c: https://github.com/OP-TEE/optee_os/blob/master/core/arch/arm/pta/stats.c
